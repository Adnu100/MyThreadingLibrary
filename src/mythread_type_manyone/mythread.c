#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <ucontext.h>
#include "mythread.h"

static struct mythread_struct **__allthreads[16] = {0};
static ucontext_t maincontext;
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
	getcontext(&(__allthreads[cur][locind]->thread_context));
	__allthreads[cur][locind]->thread_context.uc_stack.ss_sp = malloc(STACK_SIZE);
	__allthreads[cur][locind]->thread_context.uc_stack.ss_size = STACK_SIZE;
	__allthreads[cur][locind]->thread_context.uc_link = &maincontext;
	__allthreads[cur][locind]->returnval = NULL;
	__allthreads[cur][locind]->state = THREAD_NOT_STARTED;
	__ind++;
	return __allthreads[cur][locind];
}

long fun(int a, int b) {                                                                                                                             
	long int x;
	x = a;
	x <<= 32;
	x += b;
	return x;
}

int mythread_create(mythread_t *mythread, void *(*fun)(void *), void *args) {
	struct mythread_struct *t = __mythread_fill(fun, args);
	if(t)
		t->state = THREAD_RUNNING;
	else
		return -1;
	makecontext(&(t->thread_context), __mythread_wrapper, 0);
	//makecontext(&(t->thread_context), __mythread_wrapper, 2, (int)(((long int)args) << 32), ((int)((long int)args)));
	*mythread = __ind;
	return 0;
}
