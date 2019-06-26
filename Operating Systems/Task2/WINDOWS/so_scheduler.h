/*
 * Threads scheduler header
 *
 * 2017, Operating Systems
 *
 * Name, Group
 * Ion Bogdan-Ionut, 332CB
 */

#ifndef SO_SCHEDULER_H_
#define SO_SCHEDULER_H_

/* OS dependent stuff */
#ifdef __linux__
#include <pthread.h>
#include <semaphore.h>

#define DECL_PREFIX

typedef pthread_t tid_t;
#elif defined(_WIN32)
#include <windows.h>

#ifdef DLL_IMPORTS
#define DECL_PREFIX __declspec(dllimport)
#else
#define DECL_PREFIX __declspec(dllexport)
#endif

typedef DWORD tid_t;
#else
#error "Unknown platform"
#endif

/*
 * the maximum priority that can be assigned to a thread
 */
#define SO_MAX_PRIO 5
/*
 * the maximum number of events
 */
#define SO_MAX_NUM_EVENTS 256

/*
 * return value of failed tasks
 */
#define INVALID_TID ((tid_t)0)

#ifdef __cplusplus
extern "C" {
#endif

/* thread states during the schedule */
typedef enum {
	NEW, READY, RUNNING, WAITING, TERMINATED
} thread_state;

/* a thread */
typedef struct {
	unsigned int io_device, priority, quantum_left;
	HANDLE semaphore;
	HANDLE thread_handle;
	thread_state th_state;
} thread_t;

/*
 * handler prototype
 */
typedef void (so_handler)(unsigned int);

/* thread arguments used in pthread_create */
typedef struct {
	thread_t *thr_t;
	so_handler *so_h;
	HANDLE semaphore;
} thread_args;

/*
 * creates and initializes scheduler
 * + time quantum for each thread
 * + number of IO devices supported
 * returns: 0 on success or negative on error
 */
DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io);

/*
 * creates a new so_task_t and runs it according to the scheduler
 * + handler function
 * + priority
 * returns: tid of the new task if successful or INVALID_TID
 */
DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority);

/*
 * waits for an IO device
 * + device index
 * returns: -1 if the device does not exist or 0 on success
 */
DECL_PREFIX int so_wait(unsigned int io);

/*
 * signals an IO device
 * + device index
 * return the number of tasks woke or -1 on error
 */
DECL_PREFIX int so_signal(unsigned int io);

/*
 * does whatever operation
 */
DECL_PREFIX void so_exec(void);

/*
 * destroys a scheduler
 */
DECL_PREFIX void so_end(void);

#ifdef __cplusplus
}
#endif

#endif /* SO_SCHEDULER_H_ */
