/*
 * tracer.c - SO2 kprobe based tracer
 *
 * Author: Bogdan-Ionut Ion <bogdan_ionut.ion@cti.pub.ro>
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kprobes.h>
#include <linux/list.h>
#include <linux/slab.h>

#include "tracer.h"

#define LOG_LEVEL	KERN_DEBUG
#define NUM_MINORS	1

struct so2_device_data {
	struct miscdevice mdev;
	struct proc_dir_entry *tracer_file;
	struct kprobe kp_kfree;
	struct kprobe kp_sched;
	struct kprobe kp_up;
	struct kprobe kp_down;
	struct kprobe kp_lock;
	struct kprobe kp_unlock;
};

/* I use 2 lists, one for memory info, one for data / proc info */
/* address-size struct association */
struct mem_data_list {
	pid_t pid;
	long address;
	int mem_size;
	struct list_head list;
};

LIST_HEAD(mem_data_head);

/* proc data */
struct proc_data_list {
	pid_t pid;

	/* register number of function calls, including memory size info
	 * useing atomic operations
	 */
	atomic_t kmalloc_counter;
	atomic_t kfree_counter;
	atomic_t kmalloc_mem_counter;
	atomic_t kfree_mem_counter;
	atomic_t sched_counter;
	atomic_t up_counter;
	atomic_t down_counter;
	atomic_t lock_counter;
	atomic_t unlock_counter;
	struct list_head list;
};

LIST_HEAD(proc_data_head);

/* spinlock for proc list */
DEFINE_SPINLOCK(proc_lock);

/* spinlock for mem list, used inside the handlers */
DEFINE_SPINLOCK(mem_lock);

/* spinlock for mem list, used outside the handlers */
DEFINE_SPINLOCK(mem_lock_out);

/* per-instance private data */
struct my_data {
	int mem_size;
};

struct so2_device_data devs[NUM_MINORS];

/* read all data from proc list into /proc/tracer */
static int tracer_proc_info(struct seq_file *m, void *v)
{
	struct list_head *p = NULL;
	struct proc_data_list *pdl;

	seq_puts(m, "PID   kmalloc kfree kmalloc_mem kfree_mem  sched   up     down  lock   unlock\n");

	list_for_each(p, &proc_data_head) {
		pdl = list_entry(p, struct proc_data_list, list);
		seq_printf(m, "%d %d %d %d %d %d %d %d %d %d\n",
			pdl->pid,
			atomic_read(&pdl->kmalloc_counter),
			atomic_read(&pdl->kfree_counter),
			atomic_read(&pdl->kmalloc_mem_counter),
			atomic_read(&pdl->kfree_mem_counter),
			atomic_read(&pdl->sched_counter),
			atomic_read(&pdl->up_counter),
			atomic_read(&pdl->down_counter),
			atomic_read(&pdl->lock_counter),
			atomic_read(&pdl->unlock_counter)
			);
	}

	return 0;
}

/* open file */
static int so2_mdev_open(struct inode *inode, struct file *file)
{
	return single_open(file, tracer_proc_info, NULL);
}

/* increment the value for the proc with that pid */
static void increment_func_call(pid_t pid, int increment, char *op_name)
{
	struct list_head *p = NULL;
	struct proc_data_list *pdl;

	list_for_each(p, &proc_data_head) {
		pdl = list_entry(p, struct proc_data_list, list);
		if (pdl->pid == pid) {
			if (!strncmp(op_name, "kmalloc_counter", 15))
				atomic_add(increment, &pdl->kmalloc_counter);
			else if (!strncmp(op_name, "kfree_counter", 13))
				atomic_add(increment, &pdl->kfree_counter);
			else if (!strncmp(op_name, "kmalloc_mem_counter", 19))
				atomic_add(increment,
					&pdl->kmalloc_mem_counter);
			else if (!strncmp(op_name, "kfree_mem_counter", 17))
				atomic_add(increment, &pdl->kfree_mem_counter);
			else if (!strncmp(op_name, "sched_counter", 13))
				atomic_add(increment, &pdl->sched_counter);
			else if (!strncmp(op_name, "up_counter", 10))
				atomic_add(increment, &pdl->up_counter);
			else if (!strncmp(op_name, "down_counter", 12))
				atomic_add(increment, &pdl->down_counter);
			else if (!strncmp(op_name, "lock_counter", 12))
				atomic_add(increment, &pdl->lock_counter);
			else if (!strncmp(op_name, "unlock_counter", 14))
				atomic_add(increment, &pdl->unlock_counter);

			break;
		}
	}
}

/* save memory data for that proc pid into mem list
 * used in __kmalloc handler
 */
static void save_mem_data(long address, int mem_size)
{
	struct mem_data_list *mdl;

	mdl = kmalloc(sizeof(*mdl), GFP_ATOMIC);

	if (mdl == NULL)
		return;

	mdl->pid = current->pid;
	mdl->address = address;
	mdl->mem_size = mem_size;

	spin_lock(&mem_lock);
	list_add(&mdl->list, &mem_data_head);
	spin_unlock(&mem_lock);
}

/* get memory data at address for current proc pid into mem list
 * also, remove the element from list
 * used in kfree handler
 */
static int get_mem_data(long address)
{
	struct list_head *p = NULL, *q = NULL;
	struct mem_data_list *mdl;
	int size;

	spin_lock(&mem_lock);
	list_for_each_safe(p, q, &mem_data_head) {
		mdl = list_entry(p, struct mem_data_list, list);

		if (mdl->address == address && mdl->pid == current->pid) {
			size = mdl->mem_size;
			list_del(p);
			kfree(mdl);
			spin_unlock(&mem_lock);
			return size;
		}
	}
	spin_unlock(&mem_lock);

	return 0;
}

/* remove all the elements from proc list at exit */
static void destroy_proc_list(void)
{
	struct list_head *p = NULL, *q = NULL;
	struct proc_data_list *pdl;

	spin_lock(&proc_lock);
	list_for_each_safe(p, q, &proc_data_head) {
		pdl = list_entry(p, struct proc_data_list, list);
		list_del(p);
		kfree(pdl);
	}
	spin_unlock(&proc_lock);
}

/* remove all the elements from mem list at exit */
static void destroy_mem_list(void)
{
	struct list_head *p = NULL, *q = NULL;
	struct mem_data_list *mdl;

	spin_lock(&mem_lock_out);
	list_for_each_safe(p, q, &mem_data_head) {
		mdl = list_entry(p, struct mem_data_list, list);
		list_del(p);
		kfree(mdl);
	}
	spin_unlock(&mem_lock_out);
}

/* remove all the elems from mem list for the process with that pid
 * when a process is removed
 */
static void remove_by_pid_from_mem_list(pid_t pid)
{
	struct list_head *p = NULL, *q = NULL;
	struct mem_data_list *mdl;

	spin_lock(&mem_lock_out);
	list_for_each_safe(p, q, &mem_data_head) {
		mdl = list_entry(p, struct mem_data_list, list);

		if (mdl->pid == pid) {
			list_del(p);
			kfree(mdl);
		}
	}
	spin_unlock(&mem_lock_out);
}

/* add a proc element to the list */
static int add_process(pid_t pid)
{
	struct proc_data_list *pdl;

	pdl = kmalloc(sizeof(*pdl), GFP_KERNEL);

	if (pdl == NULL)
		return -ENOMEM;

	/* set its pid and initialize atomic values */
	pdl->pid = pid;

	atomic_set(&pdl->kmalloc_counter, 0);
	atomic_set(&pdl->kfree_counter, 0);
	atomic_set(&pdl->kmalloc_mem_counter, 0);
	atomic_set(&pdl->kfree_mem_counter, 0);
	atomic_set(&pdl->sched_counter, 0);
	atomic_set(&pdl->up_counter, 0);
	atomic_set(&pdl->down_counter, 0);
	atomic_set(&pdl->lock_counter, 0);
	atomic_set(&pdl->unlock_counter, 0);

	spin_lock(&proc_lock);
	list_add(&pdl->list, &proc_data_head);
	spin_unlock(&proc_lock);

	return 0;
}

/* remove a proc element from the list */
static int remove_process(pid_t pid)
{
	struct list_head *p = NULL, *q = NULL;
	struct proc_data_list *pdl;

	spin_lock(&proc_lock);
	list_for_each_safe(p, q, &proc_data_head) {
		pdl = list_entry(p, struct proc_data_list, list);

		if (pdl->pid == pid) {
			remove_by_pid_from_mem_list(pid);
			list_del(p);
			kfree(pdl);
			spin_unlock(&proc_lock);
			return 0;
		}
	}
	spin_unlock(&proc_lock);

	return -EINVAL;
}

/* ioctl interface for the miscdevice */
static long
so2_mdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	case TRACER_ADD_PROCESS:
		ret = add_process(arg);
		break;
	case TRACER_REMOVE_PROCESS:
		ret = remove_process(arg);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

/* device file operation */
static const struct file_operations so2_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = so2_mdev_ioctl,
};

/* tracer file operation */
static const struct file_operations so2_proc_fops = {
	.owner = THIS_MODULE,
	.open = so2_mdev_open,
	.read = seq_read,
	.release = single_release,
};

static int kmalloc_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct my_data *data;
	unsigned long ret_value;

	/* get allocated memory size by the function called */
	data = (struct my_data *)ri->data;

	/* get the resturn address */
	ret_value = regs_return_value(regs);

	/* increment mem_size value */
	increment_func_call(
		current->pid,
		data->mem_size,
		"kmalloc_mem_counter");

	/* add a mem element to the list */
	save_mem_data(ret_value, data->mem_size);

	return 0;
}

static int kmalloc_entry_handler(struct kretprobe_instance *ri,
	struct pt_regs *regs)
{
	struct my_data *data;

	/* increment function call */
	increment_func_call(ri->task->pid, 1, "kmalloc_counter");

	/* save mem_size to get it into the kmalloc_handler */
	data = (struct my_data *)ri->data;
	data->mem_size = regs->ax;

	return 0;
}

/* used only one handler for the rest of functions */
static int kfree_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	int size;

	/* get the memory size for the address located in ax */
	size = get_mem_data(regs->ax);

	if (size != 0) {
		/* increment kfree size value */
		increment_func_call(current->pid, size, "kfree_mem_counter");
	}

	/* increment function call */
	increment_func_call(current->pid, 1, "kfree_counter");

	return 0;
}

static int sched_handler_pre(struct kprobe *p, struct pt_regs *regs)
{

	increment_func_call(current->pid, 1, "sched_counter");

	return 0;
}

static int up_handler_pre(struct kprobe *p, struct pt_regs *regs)
{

	increment_func_call(current->pid, 1, "up_counter");

	return 0;
}

static int down_handler_pre(struct kprobe *p, struct pt_regs *regs)
{

	increment_func_call(current->pid, 1, "down_counter");

	return 0;
}

static int lock_handler_pre(struct kprobe *p, struct pt_regs *regs)
{

	increment_func_call(current->pid, 1, "lock_counter");

	return 0;
}

static int unlock_handler_pre(struct kprobe *p, struct pt_regs *regs)
{

	increment_func_call(current->pid, 1, "unlock_counter");

	return 0;
}

static struct kretprobe my_kretprobe = {
	.handler = kmalloc_handler,
	.entry_handler = kmalloc_entry_handler,
	.maxactive = 20,
	.data_size = sizeof(struct my_data),
};

static int so2_kprobe_tracer_init(void)
{
	int ret, i;

	for (i = 0; i < NUM_MINORS; i++) {

		/* initialize the device, the tracer file and the probes */

		devs[i].mdev.minor = TRACER_DEV_MINOR;
		devs[i].mdev.name = TRACER_DEV_NAME;
		devs[i].mdev.fops = &so2_fops;

		ret = misc_register(&devs[i].mdev);

		if (ret) {
			pr_info("failed to register miscdevice");
			return ret;
		}

		pr_info("miscdevice registered");

		devs[i].tracer_file = proc_create(
			TRACER_DEV_NAME,
			0000,
			NULL,
			&so2_proc_fops);

		if (!devs[i].tracer_file) {
			pr_info("failed to create tracer file");
			proc_remove(devs[i].tracer_file);
			return -ENOMEM;
		}

		pr_info("tracer file created");

		/* kfree kprobe */
		devs[i].kp_kfree.pre_handler = kfree_handler_pre;
		devs[i].kp_kfree.addr = (kprobe_opcode_t *)
			kallsyms_lookup_name("kfree");

		if (devs[i].kp_kfree.addr == NULL) {
			pr_info("Couldn't find %s to plant kprobe\n", "kfree");
			return -1;
		}

		ret = register_kprobe(&devs[i].kp_kfree);
		if (ret < 0) {
			pr_info("failed to register kp_kfree");
			return ret;
		}

		pr_info("Planted kprobe kp_kfree at %p\n",
			devs[i].kp_kfree.addr);

		/* schedule kprobe */
		devs[i].kp_sched.pre_handler = sched_handler_pre;
		devs[i].kp_sched.addr = (kprobe_opcode_t *)
			kallsyms_lookup_name("schedule");

		if (devs[i].kp_sched.addr == NULL) {
			pr_info(
				"Couldn't find %s to plant kprobe\n",
				"schedule");
			return -1;
		}

		ret = register_kprobe(&devs[i].kp_sched);
		if (ret < 0) {
			pr_info("failed to register kp_sched");
			return ret;
		}

		pr_info(
			"Planted kprobe kp_sched at %p\n",
			devs[i].kp_sched.addr
			);

		/* up kprobe */
		devs[i].kp_up.pre_handler = up_handler_pre;
		devs[i].kp_up.addr = (kprobe_opcode_t *)
			kallsyms_lookup_name("up");

		if (devs[i].kp_up.addr == NULL) {
			pr_info("Couldn't find %s to plant kprobe\n", "up");
			return -1;
		}

		ret = register_kprobe(&devs[i].kp_up);
		if (ret < 0) {
			pr_info("failed to register kp_up");
			return ret;
		}

		pr_info("Planted kprobe kp_up at %p\n", devs[i].kp_up.addr);

		/* down_interruptible kprobe */
		devs[i].kp_down.pre_handler = down_handler_pre;
		devs[i].kp_down.addr = (kprobe_opcode_t *)
			kallsyms_lookup_name("down_interruptible");

		if (devs[i].kp_down.addr == NULL) {
			pr_info(
				"Couldn't find %s to plant kprobe\n",
				"down_interruptible"
				);
			return -1;
		}

		ret = register_kprobe(&devs[i].kp_down);
		if (ret < 0) {
			pr_info("failed to register kp_down");
			return ret;
		}

		pr_info("Planted kprobe kp_down at %p\n", devs[i].kp_down.addr);

		/* mutex_lock_nested kprobe */
		devs[i].kp_lock.pre_handler = lock_handler_pre;
		devs[i].kp_lock.addr = (kprobe_opcode_t *)
			kallsyms_lookup_name("mutex_lock_nested");

		if (devs[i].kp_lock.addr == NULL) {
			pr_info(
				"Couldn't find %s to plant kprobe\n",
				"mutex_lock_nested"
				);
			return -1;
		}

		ret = register_kprobe(&devs[i].kp_lock);
		if (ret < 0) {
			pr_info("failed to register kp_lock");
			return ret;
		}

		pr_info("Planted kprobe kp_lock at %p\n", devs[i].kp_lock.addr);

		/* mutex_unlock kprobe */
		devs[i].kp_unlock.pre_handler = unlock_handler_pre;
		devs[i].kp_unlock.addr = (kprobe_opcode_t *)
			kallsyms_lookup_name("mutex_unlock");

		if (devs[i].kp_unlock.addr == NULL) {
			pr_info(
				"Couldn't find %s to plant kprobe\n",
				"mutex_unlock"
				);
			return -1;
		}

		ret = register_kprobe(&devs[i].kp_unlock);
		if (ret < 0) {
			pr_info("failed to register kp_unlock");
			return ret;
		}

		pr_info(
			"Planted kprobe kp_unlock at %p\n",
			devs[i].kp_unlock.addr
			);

	}

	/* __kmalloc kretprobe */
	my_kretprobe.kp.addr =
		(kprobe_opcode_t *) kallsyms_lookup_name("__kmalloc");

	if (my_kretprobe.kp.addr == NULL) {
		pr_info("Couldn't find %s to plant jprobe\n", "__kmalloc");
		return -1;
	}

	ret = register_kretprobe(&my_kretprobe);
	if (ret < 0) {
		pr_info("register_kretprobe failed, returned %d\n", ret);
		return -1;
	}

	pr_info("Planted kretprobe at %p, handler addr %p\n",
		my_kretprobe.kp.addr, my_kretprobe.handler);

	return 0;
}

static void so2_kprobe_tracer_exit(void)
{
	int i;

	/* clean memory */
	destroy_proc_list();
	destroy_mem_list();

	/* unregister the device, the tracer and the probes */
	for (i = 0; i < NUM_MINORS; i++) {
		misc_deregister(&devs[i].mdev);
		pr_info("miscdevice unregistered");

		proc_remove(devs[i].tracer_file);
		pr_info("tracer file removed");

		unregister_kprobe(&devs[i].kp_kfree);
		pr_info("kp_kfree unregistered\n");

		unregister_kprobe(&devs[i].kp_sched);
		pr_info("kp_sched unregistered\n");

		unregister_kprobe(&devs[i].kp_up);
		pr_info("kp_up unregistered\n");

		unregister_kprobe(&devs[i].kp_down);
		pr_info("kp_down unregistered\n");

		unregister_kprobe(&devs[i].kp_lock);
		pr_info("kp_lock unregistered\n");

		unregister_kprobe(&devs[i].kp_unlock);
		pr_info("kp_unlock unregistered\n");
	}

	unregister_kretprobe(&my_kretprobe);
	pr_info("kretprobe unregistered\n");
}

module_init(so2_kprobe_tracer_init);
module_exit(so2_kprobe_tracer_exit);

MODULE_DESCRIPTION("SO2 kprobe based tracer");
MODULE_AUTHOR("Bogdan-Ionut Ion <bogdan_ionut.ion@cti.pub.ro>");
MODULE_LICENSE("GPL");
