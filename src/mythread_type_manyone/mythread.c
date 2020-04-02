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

/* wrapper function of type void (*f)(int) which is needed to be type
 * casted in the form void (*f)(void) and passed to makecontext
 * function
 * it invokes the function of the thread, also stores the returned
 * value in the mythread_struct
 */
void __mythread_wrapper(int ind) {
	ind--;
	int cur = ind / 16;
	int locind = ind % 16;
	mythread_struct *t = __allthreads[cur][locind];
	t->returnval = t->fun(t->args);
	ind--;
	if(ind >= 0) {
		cur = ind / 16;
		locind = ind % 16;
		__allthreads[cur][locind]->thread_context.uc_link = t->thread_context.uc_link;
	}
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
	getcontext(&(__allthreads[cur][locind]->thread_context));
	__allthreads[cur][locind]->thread_context.uc_stack.ss_sp = malloc(STACK_SIZE);
	__allthreads[cur][locind]->thread_context.uc_stack.ss_size = STACK_SIZE;
	__allthreads[cur][locind]->thread_context.uc_link = &maincontext;
	__allthreads[cur][locind]->returnval = NULL;
	__allthreads[cur][locind]->state = THREAD_NOT_STARTED;
	__ind++;
	return __allthreads[cur][locind];
}

/* creates a many one thread and starts it for given function and given
 * argument (fun and args)
 * it returns 0 on success and -1 on error
 * the thread id is stored in the location pointed by mythread
 * if any error occures, it frees the allocated structure
 */
int mythread_create(mythread_t *mythread, void *(*fun)(void *), void *args) {
	struct mythread_struct *t = __mythread_fill(fun, args);
	if(t)
		t->state = THREAD_RUNNING;
	else
		return -1;
	makecontext(&(t->thread_context), (void (*)())__mythread_wrapper, 1, __ind);
	*mythread = __ind;
	return 0;
}
