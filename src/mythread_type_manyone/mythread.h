/* 
 * Mythread C threading library
 * This is an implementation of user level and kernel level one-one, 
 * many-one and many-many threads in C like pthread
 * 
 */

#ifndef MYTHREAD_H

#define MYTHREAD_H

typedef unsigned long mythread_t;

struct __mythread_struct {
	mythread_t t;
	int tid;
	void *(*fun)(void *);
	void *args;
	void *returnval;
};

int __mythread_wrapper(struct __mythread_struct *mythread_struct_cur);
void __mythreadfill(void *(*fun)(void *), void *args);

int mythread_create(mythread_t *mythread, void *(*fun)(void *), void *args);

#endif
