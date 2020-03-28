#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "mythread.h"


static struct mythread_struct **__allthreads[16] = {0};
static int __ind;

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
	__ind++;
	return __allthreads[cur][locind];
}

int mythread_create(mythread_t *mythread, void *(*fun)(void *), void *args) {
	return 0;
}
