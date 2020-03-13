#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>

int returned[100] = {0};
int c = 0;

struct routineinfo {
	int no;
	void *(*fun)(void *);
	void *arg;
	void *returnval;
};

int pthread_create(int *thread_num, void *tempvoid, void *(*fun)(void *), void *arg);
void pthread_exit(void *please_set_it_null_for_time_being);
void pthread_join(int thread_id_but_dont_care, void *null_ptr_please);

int collector(void *rtinfo) {
	((struct routineinfo *)rtinfo)->returnval = 
		((struct routineinfo *)rtinfo)->fun(((struct routineinfo *)rtinfo)->arg);
	returned[((struct routineinfo *)rtinfo)->no] = 1;
	return 0;
}

int pthread_create(int *thread_num, void *tempvoid, void *(*fun)(void *), void *arg) {
	int thread_id;
	void *stack;
	struct routineinfo *r = (struct routineinfo *)malloc(sizeof(struct routineinfo));
	stack = malloc(1024 * 1024);
	if(stack == NULL) 
		return -1;
	r->no = c;
	r->fun = fun;
	r->arg = arg;
	r->returnval = NULL;
	thread_id = clone(collector, stack + (1024 * 1024), CLONE_THREAD | CLONE_SIGHAND | CLONE_VM, r);
	if(thread_id == -1)
		return -1;
	*thread_num = c;
	c++;
	return 0;
}

void pthread_exit(void *please_set_it_null_for_time_being) {
		
}

void pthread_join(int thread_id_but_dont_care, void *null_ptr_please) {
	while(!returned[thread_id_but_dont_care]);
}

