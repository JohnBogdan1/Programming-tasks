/**
 * Operating Systems 2017 - Assignment 4
 *
 * Name, Group
 * Ion Bogdan-Ionut, 332CB
 *
 */

#include "so_scheduler.h"
#include "priority_table.h"
#include "t_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

/* global variables used in the scheduler*/

/* priority table */
TP *tp;

/* list of all threads created during the schedule */
TListG threads;

/* current thread and properties of the current thread */
unsigned int io_dev, initial_max_quantum, nr_threads;
static thread_t *current_thread;
unsigned int initOnce = 0;

/* mutex used for sync */
HANDLE mutex;

DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io)
{

	/* sanity checks */
	if (io > SO_MAX_NUM_EVENTS || time_quantum == 0 || initOnce)
		return -1;

	/* the table is initialized in range 0 - SO_MAX_PRIO */
	tp = priority_table_create(SO_MAX_PRIO + 1);

	if (!tp) {
		DistrugeTP(&tp);
		perror("\nCan't create the priority_table.");
		return -1;
	}

	/* initialize everything we need */

	mutex = CreateMutex( 
		NULL, /* default security attributes */
		FALSE, /* initially not owned */
		NULL  /* unnamed mutex */
	);

	io_dev = io;
	initial_max_quantum = time_quantum;
	current_thread = NULL;
	initOnce = 1;
	nr_threads = 0;

	threads = NULL;

	return 0;
}

/* function that gets the next thread from table */
void get_thread_from_queue(void)
{
	thread_t *th;
	DWORD dwRet;

	WaitForSingleObject(mutex, INFINITE);

	/* if there is a thread that waits in the table */
	if (get_head(tp) == NULL) {
		current_thread = NULL;
		ReleaseMutex(mutex);
		return;
	}

	/* remove the head and set the current thread */
	th = (thread_t *)(get_head(tp)->info);
	priority_table_remove(tp, th->priority);
	current_thread = th;

	dwRet = ReleaseSemaphore(th->semaphore, 1, NULL);
	if (dwRet == 0) {
		perror("\nCan't ReleaseSemaphore in th_handler.");
		return;
	}

	ReleaseMutex(mutex);

}

/*
 * function used to preempt the current thread and schedule a new one
 * based on the priorities and on the quantum value
 */
void compare_priorities(int expired)
{
	thread_t *th, *prev_curr_thread;
	DWORD dwWaitResult, dwRet;

	/* sanity checks */
	if (get_head(tp) == NULL || current_thread == NULL)
		return;

	WaitForSingleObject(mutex, INFINITE);

	/* get the head of the priority table */
	th = (thread_t *)(get_head(tp)->info);

	/*
	 * if the current thread has not expired, schedule a new thread
	 * or if the thread has expired, schedule one with a bigger or equal
	 * priority
	 */
	if (((th->priority > current_thread->priority && !expired) ||
		(th->priority >= current_thread->priority && expired)) &&
		current_thread->th_state == RUNNING) {

		/* save the old thread */
		prev_curr_thread = current_thread;
		prev_curr_thread->quantum_left = initial_max_quantum;

		/* set the new thread */
		current_thread = th;

		/* remove the new thread from the tp */
		priority_table_remove(tp, th->priority);

		/* add the old thread to the table */
		priority_table_insert(tp, prev_curr_thread,
			prev_curr_thread->priority);

		/* wake the new thread */
		dwRet = ReleaseSemaphore(th->semaphore, 1, NULL);
		if (dwRet == 0) {
			perror("\nCan't ReleaseSemaphore in th_handler.");
			return;
		}

		ReleaseMutex(mutex);

		/* put the old thread on wait */
		dwWaitResult = WaitForSingleObject(
			prev_curr_thread->semaphore, /* handle to semaphore */
			INFINITE);
		if (dwWaitResult == WAIT_FAILED) {
			perror("\nCan't WaitForSingleObject.");
			return;
		}
	} else
		ReleaseMutex(mutex);
}

/*
 * function the checks the state of the threads in scheduler
 * and decrement the quantum of the current thread
 */
void check_scheduler(void)
{

	/* sanity checks */
	if (current_thread == NULL)
		return;

	WaitForSingleObject(mutex, INFINITE);

	/* if it's not the first thread */
	if (get_head(tp) != NULL && nr_threads > 1)
		current_thread->quantum_left--;
	else {
		ReleaseMutex(mutex);
		return;
	}

	/* if the quantum has expired or not */
	if (current_thread->quantum_left > 0) {
		ReleaseMutex(mutex);
		compare_priorities(0);
	} else {
		ReleaseMutex(mutex);
		compare_priorities(1);
	}
}

/* function handle called by pthread_create */
DWORD WINAPI th_handler(LPVOID th_args)
{

	DWORD dwRet, dwWaitResult;

	thread_args *args = (thread_args *) th_args;

	/* create a new semaphore and init it with initial value 0 */
	args->thr_t->semaphore = CreateSemaphore(
		NULL, /* default security attributes */
		0, /* initial count */
		1, /* maximum count */
		NULL); /* unnamed semaphore */

	if (args->thr_t->semaphore == NULL) {
		perror("\nCan't CreateSemaphore.");
		return -1;
	}

	/* add the new thread to the structures */
	WaitForSingleObject(mutex, INFINITE);
	priority_table_insert(tp, args->thr_t, args->thr_t->priority);
	t_list_add(&threads, args->thr_t);
	ReleaseMutex(mutex);

	/* it is new */
	args->thr_t->th_state = READY;

	/* notify the parent thread that it is READY */
	dwRet = ReleaseSemaphore(args->semaphore, 1, NULL);
	if (dwRet == 0) {
		perror("\nCan't ReleaseSemaphore in th_handler.");
		return -1;
	}

	/* if the first thread */
	if (current_thread == NULL) {
		current_thread = args->thr_t;
		priority_table_remove(tp,
			((thread_t *)(get_head(tp)->info))->priority);
	} else {
		/* if not first, wait to be scheduled */
		dwWaitResult = WaitForSingleObject(
			args->thr_t->semaphore, /* handle to semaphore */
			INFINITE);

		if (dwWaitResult == WAIT_FAILED) {
			perror("\nCan't WaitForSingleObject.");
			return -1;
		}
	}

	/* it is scheduled, now it's RUNNING */
	args->thr_t->th_state = RUNNING;

	/* execute the handle */
	args->so_h(args->thr_t->priority);

	/* it finished it's execution, set the new current thread now */
	get_thread_from_queue();

	/* finished */
	args->thr_t->th_state = TERMINATED;

	CloseHandle(args->thr_t->semaphore);

	free(args);

	return -1;
}

DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority)
{

	thread_t *thread;
	thread_args *th_args;
	DWORD dwWaitResult;
	tid_t thread_id;

	/* sanity checks */
	if (priority > SO_MAX_PRIO)
		return INVALID_TID;

	if (func == NULL)
		return INVALID_TID;

	/* alloc memory for thread and args structures */
	thread = (thread_t *) malloc(sizeof(thread_t));
	th_args = (thread_args *) malloc(sizeof(thread_args));

	/* initialize them */
	thread->priority = priority;
	thread->quantum_left = initial_max_quantum;
	thread->io_device = -1;
	thread->th_state = NEW;

	th_args->semaphore = CreateSemaphore(
		NULL, /* default security attributes */
		0, /* initial count */
		1, /* maximum count */
		NULL); /* unnamed semaphore */

	if (th_args->semaphore == NULL) {
		perror("\nCan't CreateSemaphore.");
		return INVALID_TID;
	}

	th_args->so_h = func;
	th_args->thr_t = thread;

	nr_threads++;

	/* create a new thread in the context of this one */
	thread->thread_handle = CreateThread(NULL,
			0,
			th_handler,
			(LPVOID)th_args,
			0,
			&thread_id);
	if (thread->thread_handle == NULL) {
		perror("\nCan't pthread_create.");
		return INVALID_TID;
	}

	/* wait for the thread to be READY */
	dwWaitResult = WaitForSingleObject(
			th_args->semaphore, /* handle to semaphore */
			INFINITE);
	if (dwWaitResult == WAIT_FAILED) {
		perror("\nCan't WaitForSingleObject.");
		return INVALID_TID;
	}

	/* check if the new thread can be scheduled */
	if (thread->th_state == READY)
		check_scheduler();

	CloseHandle(th_args->semaphore);

	return thread_id;
}

DECL_PREFIX int so_wait(unsigned int io)
{

	thread_t *aux_curr_thread;
	DWORD dwWaitResult;

	/* sanity checks */
	if (io > io_dev - 1)
		return -1;

	/* set the device that's waiting for and reset the quantum */
	current_thread->io_device = io;
	current_thread->quantum_left = initial_max_quantum;
	current_thread->th_state = WAITING;
	aux_curr_thread = current_thread;

	/* the current thread waits, so schedule a new one */
	get_thread_from_queue();

	/* put the current thread in waiting state */
	dwWaitResult = WaitForSingleObject(
			aux_curr_thread->semaphore, /* handle to semaphore */
			INFINITE);
	if (dwWaitResult == WAIT_FAILED) {
		perror("\nCan't WaitForSingleObject.");
		return -1;
	}

	return 0;
}

DECL_PREFIX int so_signal(unsigned int io)
{

	int unblocked_threads = 0;
	thread_t *thread;
	TListG p = NULL;

	/* sanity checks */
	if (io > io_dev - 1)
		return -1;

	p = threads;

	/* wake up all the threads that are waiting on that event io */
	while (p != NULL) {
		thread = (thread_t *) p->info;

		if (thread->io_device == io && thread->th_state == WAITING) {
			thread->io_device = -1;

			/* add thread to the priority table, so it's READY */
			priority_table_insert(tp, thread, thread->priority);
			unblocked_threads++;
		}

		p = p->urm;
	}

	/* schedule a new thread */
	if (current_thread->quantum_left > 0)
		compare_priorities(0);
	else
		compare_priorities(1);

	return unblocked_threads;
}

DECL_PREFIX void so_exec(void)
{
	/* consumes time scheduling */
	check_scheduler();
}

DECL_PREFIX void so_end(void)
{

	thread_t *th;
	TListG p = NULL;
	DWORD dwReturn;

	/* if it has been initialized */
	if (initOnce) {

		p = threads;

		/* wait for the threads to finish */
		while (p != NULL) {
			th = (thread_t *) p->info;

			dwReturn = WaitForSingleObject(th->thread_handle,
				INFINITE);
			if (dwReturn == WAIT_FAILED) {
				perror("\nCan't WaitForSingleObject.");
				return;
			}

			th->th_state = TERMINATED;

			p = p->urm;
		}

		/* free the resources */
		initOnce = 0;
		current_thread = NULL;
		initial_max_quantum = 0;
		nr_threads = 0;

		CloseHandle(mutex);

		DistrugeTP(&tp);
		t_list_destroy(&threads);
	}
}
