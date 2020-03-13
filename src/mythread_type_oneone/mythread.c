#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "mythread.h"

static struct mythread_struct **__allthreads[16] = {0};
static int __ind = 0;

/* wrapper function of type int (*f)(void *) which wraps the function
 * of type void *(*f)(void *) in it so that it can be passed to 
 * clone() call.
 * It also stores the returned value in the mythread_struct
 */
int __mythread_wrapper(void *mythread_struct_cur) {
	((struct mythread_struct *)mythread_struct_cur)->returnval = ((struct mythread_struct *)mythread_struct_cur)->fun(((struct mythread_struct *)mythread_struct_cur)->args);
	((struct mythread_struct *)mythread_struct_cur)->state = THREAD_TERMINATED;
	return 0;
}

/* if any error occures, then this function removes the newly allocated
 * thread and decrements the index __ind by one 
 */
void __mythread_removelastfilled(void) {
	__ind--;
	int cur = __ind / 16;
	int locind = __ind % 16;
	free(__allthreads[cur][locind]);
}

/* it takes function and arguments and returns a structure of type
 * mythread_struct which contains useful information of the current 
 * thread and can be passwed to function __mythread_wrapper
 */
struct mythread_struct *__mythread_fill(void *(*fun)(void *), void *args) {
	int cur = __ind / 16;
	int locind = __ind % 16;
	if(!__allthreads[cur])
		__allthreads[cur] = (struct mythread_struct **)malloc(sizeof(struct mythread_struct *) * 16);
	__allthreads[cur][locind] = (struct mythread_struct *)malloc(sizeof(struct mythread_struct));
	__allthreads[cur][locind]->fun = fun;
	__allthreads[cur][locind]->args = args;
	__allthreads[cur][locind]->returnval = NULL;
	__allthreads[cur][locind]->state = THREAD_NOT_STARTED;
	__allthreads[cur][locind]->stack = (char *)malloc(STACK_SIZE);
	__ind++;
	return __allthreads[cur][locind];
}

/* creates a oneone thread and starts it for given function and given 
 * argument (fun and args)
 * it returns 0 on success and -1 on error
 * the thread id is stored in the location pointed by mythread
 * if any error occures, it frees the allocated structure
 */
int mythread_create(mythread_t *mythread, void *(*fun)(void *), void *args) {
	int status;
	struct mythread_struct *t = __mythread_fill(fun, args);
	t->state = THREAD_RUNNING;
	status = clone(__mythread_wrapper, (void *)(t->stack + STACK_SIZE), CLONE_THREAD | CLONE_VM | CLONE_SIGHAND | CLONE_FS | CLONE_FILES, (void *)t);
	if(status == -1) {
		__mythread_removelastfilled();
		return -1;
	}
	else
		t->tid = status;
	*mythread = __ind;
	return 0;
}

/* waits for the thread mythread to complete 
 * it returns 0 on success and EINVAL on wrong thread_t argument and ESRCH
 * if the thread with thread id mythread can not be found
 * if returnval is not NULL, then stores the value returned by thread 
 * in the location pointed by function which was running by the thread
 */
int mythread_join(mythread_t mythread, void **returnval) {
	int cur, locind, status;
	mythread--;
	cur = mythread / 16;
	locind = mythread % 16;
	if(mythread < __ind) {
		switch(__allthreads[cur][locind]->state) {
			case THREAD_NOT_STARTED: 
			case THREAD_JOIN_CALLED: 
			case THREAD_COLLECTED:
				status = EINVAL;
				break;
			case THREAD_TERMINATED:
				if(returnval) 
					*returnval = __allthreads[cur][locind]->returnval;
				status = 0;
				break;
			case THREAD_RUNNING:
				__allthreads[cur][locind]->state = THREAD_JOIN_CALLED;
				while(__allthreads[cur][locind]->state != THREAD_TERMINATED)
					;
				__allthreads[cur][locind]->state = THREAD_COLLECTED;
				if(returnval)
					*returnval = __allthreads[cur][locind]->returnval;
				status = 0;
				break;
			default:
				status = EINVAL;
				break;
		}
		return status;
	}
	else 
		return ESRCH;
}
