/* 
 * Mythread C threading library
 * This is an implementation of user level and kernel 
 * level one-one threads in C 
 * 
 */

#include <bits/types.h>

#ifndef MYTHREAD_H

#define MYTHREAD_H

#define STACK_SIZE (1024 * 1024)

#define THREAD_RUNNING 0x0
#define THREAD_NOT_STARTED 0x1
#define THREAD_TERMINATED 0x2
#define THREAD_JOIN_CALLED 0x3
#define THREAD_COLLECTED 0x4
#define THREAD_KILLED 0x5

typedef unsigned long mythread_t;

struct mythread_struct {
	int tid, state;
	__pid_t jpid;
	char *stack;
	void *(*fun)(void *);
	void *args;
	void *returnval;
};

int __mythread_wrapper(void *mythread_struct_cur);
struct mythread_struct *__mythread_fill(void *(*fun)(void *), void *args);

int mythread_create(mythread_t *mythread, void *(*fun)(void *), void *args);
int mythread_join(mythread_t mythread, void **returnval);
int mythread_kill(mythread_t mythread, int sig);

#endif
