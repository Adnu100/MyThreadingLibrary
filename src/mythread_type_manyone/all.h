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
	int type;
	void *(*fun)(void *);
	void *args;
	void *returnval;
};

enum thread_type_t {
	ONE_ONE_THREAD,
	MANY_ONE_THREAD,
	MANY_MANY_THREAD
};

int __mythread_wrapper(struct __mythread_struct *mythread_struct_cur);
void __mythreadfill(void *(*fun)(void *), void *args);

int mythread_create(mythread_t *mythread, void *(*fun)(void *), void *args, int thread_creation_type);
int mythread_create_one_one(mythread_t *mythread, void *(*fun)(void *), void *args);
int mythread_create_many_one(mythread_t *mythread, void *(*fun)(void *), void *args);
int mythread_create_many_many(mythread_t *mythread, void *(*fun)(void *), void *args);

#endif

