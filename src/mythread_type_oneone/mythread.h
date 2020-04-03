/* 
 * Mythread C threading library
 * This is an implementation of kernel 
 * level one-one threads in C 
 * 
 */

#ifndef MYTHREAD_H

#define MYTHREAD_H

#include <setjmp.h>
#include <bits/types.h>

#define STACK_SIZE (1024 * 1024)

#define THREAD_RUNNING 0x0
#define THREAD_NOT_STARTED 0x1
#define THREAD_TERMINATED 0x2
#define THREAD_JOIN_CALLED 0x3
#define THREAD_COLLECTED 0x4

#define pid_t __pid_t

typedef unsigned long int mythread_t;
typedef unsigned short int mythread_spinlock_t;

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
void mythread_exit(void *returnval);

#endif
