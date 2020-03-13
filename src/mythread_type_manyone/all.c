#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include "pthread.h"

struct __mythread_struct **__allthreads[16] = {0};
int totalthreadcount = 0;

struct __mythread_struct *__mythreadfill(void *(*fun)(void *), void *args, enum thread_type_t type) {
	int thbind, localind;
	thbind = totalthreadcount / 64;
	localind = totalthreadcount % 64;
	if(!__allthreads[thbind])
		__allthreads[thbind] = (struct __mythread_struct **)malloc(sizeof(struct __mythread_struct *));
	__allthreads[thbind][localind] = (struct __mythread_struct *)malloc(sizeof(struct __mythread_struct));
	__allthreads[thbind][localind]->t = ++totalthreadcount;
	__allthreads[thbind][localind]->tid = 0;
	__allthreads[thbind][localind]->fun = fun;
	__allthreads[thbind][localind]->args = args;
	__allthreads[thbind][localind]->returnval = NULL;
	return __allthreads[thbind][localind];
}

int mythread_create(mythread_t *mythread, void *(*fun)(void *), void *args, enum thread_type_t thread_creation_type) {
	int status;
	switch(thread_creation_type) {
		case ONE_ONE_THREAD:
			status = mythread_create_one_one(mythread, fun, args);
			break;
		case MANY_ONE_THREAD:
			status = mythread_create_many_one(mythread, fun, args);
			break;
		case MANY_MANY_THREAD:
			status = mythread_create_many_many(mythread, fun, args);
			break;
	}
	return status;
}

int mythread_create_one_one(mythread_t *mythread, void *(*fun)(void *), void *args) {
	struct __mythread_struct *t = __mythreadfill(fun, args, ONE_ONE_THREAD);

	return 0;
}

int mythread_create_many_one(mythread_t *mythread, void *(*fun)(void *), void *args) {
	return 0;
}

int mythread_create_many_many(mythread_t *mythread, void *(*fun)(void *), void *args) {
	return 0;
}
