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
static int __ind = 0, __current = 0;
static struct active_thread_node *active = NULL, *mainthread = NULL, *previous = NULL, *last = NULL;
static volatile int superlock = 0;

static inline void superlock_lock() {
	while(__sync_lock_test_and_set(&superlock, 1));
}

static inline void superlock_unlock() {
	__sync_synchronize();
	superlock = 0;
}

static inline short int superlock_trylock() {
	return !__sync_lock_test_and_set(&superlock, 1);
}

static void addsignal(pending_signals_queue *q, int sig) {
	if(q->head) {
		q->tail->next = (struct pending_signal_node *)malloc(sizeof(struct pending_signal_node));
		q->tail->next->sig = sig;
		q->tail->next->next = NULL;
		q->tail = q->tail->next;
	}
	else {
		q->head = q->tail = (struct pending_signal_node *)malloc(sizeof(struct pending_signal_node));
		q->head->sig = sig;
		q->head->next = NULL;
	}
}

static void handle_pending_signals() {
	int thread, cur, locind;
	thread = active->thread - 1;
	cur = thread / 16;
	locind = thread % 16;
	struct pending_signal_node *node = __allthreads[cur][locind]->pending_signals.head, *previous;
	while(node) {
		raise(node->sig);
		previous = node;
		node = node->next;
		free(previous);
	}
	__allthreads[cur][locind]->pending_signals.head = __allthreads[cur][locind]->pending_signals.tail = NULL;
}

/* this function will be invoked after every alarm sent to program
 * this changes the current context between active thread and the 
 * thread next to it
 */
static void nextthread(int sig) {
	if(sig == SIGALRM && __current > 1 && superlock_trylock()) {
		previous = active;
		active = active->next;
		swapcontext(previous->c, active->c);
		handle_pending_signals();
		superlock_unlock();
	}
}

/* this initialisation function is needed to be called 
 * before creating any thread and calling any other thread
 * functions on that thread
 * this function does the necessary setup and sets custom 
 * alarm by ualarm()
 */
void mythread_init() {
	active = (struct active_thread_node *)malloc(sizeof(struct active_thread_node));
	active->thread = 0;
	active->c = &maincontext;
	active->next = active;
	last = mainthread = active;
	__current = 1;
	signal(SIGALRM, nextthread);	
}

/* wrapper function of type void (*f)(int) which is needed to be type
 * casted in the form void (*f)(void) and passed to makecontext
 * function
 * it invokes the function of the thread, also stores the returned
 * value in the mythread_struct
 */
void __mythread_wrapper(int ind) {
	int cur, locind;
	ind--;
	cur = ind / 16;
	locind = ind % 16;
	superlock_unlock();
	__allthreads[cur][locind]->returnval = __allthreads[cur][locind]->fun(__allthreads[cur][locind]->args);
	if(ind >= 0) {
		superlock_lock();
		__allthreads[cur][locind]->state = THREAD_TERMINATED;
		if(active->next == mainthread)
			last = previous;
		previous->next = active->next;
		free(active);
		active = mainthread;
		previous = last;
		__current--;
		if(__current == 1)
			ualarm(0, 0);
		superlock_unlock();
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
	__allthreads[cur][locind]->pending_signals.head = __allthreads[cur][locind]->pending_signals.tail = NULL;
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
	struct mythread_struct *t; 
	struct active_thread_node *newthread;
	if(__current == 1)
		ualarm(50000, 50000);
	superlock_lock();
	t = __mythread_fill(fun, args);
	*mythread = __ind;
	if(t)
		t->state = THREAD_RUNNING;
	else {
		superlock_unlock();
		return -1;
	}
	makecontext(&(t->thread_context), (void (*)())__mythread_wrapper, 1, __ind);
	newthread = (struct active_thread_node *)malloc(sizeof(struct active_thread_node));
	newthread->thread = *mythread;
	newthread->c = &(t->thread_context);
	newthread->next = mainthread->next;
	mainthread->next = newthread;
	__current++;
	if(newthread->next == mainthread)
		last = newthread;
	superlock_unlock();
	return 0;
}

/* waits for the thread mythread to complete 
 * it returns 0 on success and EINVAL on wrong thread_t argument and ESRCH
 * if the thread with thread id mythread can not be found
 * if returnval is not NULL, then stores the value returned by thread 
 * in the location pointed by function which was running by the thread
 */
int mythread_join(mythread_t mythread, void **returnval) {
	int cur, locind, status = EINVAL;
	mythread--;
	cur = mythread / 16;
	locind = mythread % 16;
	superlock_lock();
	if(mythread < __ind) {
		switch(__allthreads[cur][locind]->state) {
			case THREAD_RUNNING:
				__allthreads[cur][locind]->state = THREAD_JOIN_CALLED;
				superlock_unlock();
				while(__allthreads[cur][locind]->state != THREAD_TERMINATED)
					nextthread(SIGALRM);
				superlock_lock();
				__allthreads[cur][locind]->state = THREAD_COLLECTED;
				superlock_unlock();
				if(returnval)
					*returnval = __allthreads[cur][locind]->returnval;
				status = 0;
				break;
			case THREAD_NOT_STARTED:
			case THREAD_COLLECTED:
			case THREAD_JOIN_CALLED:
				superlock_unlock();
				break;
			case THREAD_TERMINATED:
				superlock_unlock();
				if(returnval)
					*returnval = __allthreads[cur][locind]->returnval;
				status = 0;
				break;
			default:
				superlock_unlock();
				break;
		}
	}
	return status;
}

/* sends signal sig to the thread represented by
 * mythread_t mythread
 * the signal is stored in the thread's pending signals 
 * queue which will be later processed when the thread is running
 * the scheduler will check if the queue is empty of not, if not empty
 * each signal in queue will be processed in the order in which 
 * it was received
 */
int mythread_kill(mythread_t mythread, int sig) {
	int cur, locind;
	mythread--;
	cur = mythread / 16;
	locind = mythread % 16;
	superlock_lock();
	addsignal(&(__allthreads[cur][locind]->pending_signals), sig);
	superlock_unlock();
	return 0;
}

/* when called, the calling thread exits, if returnval is not 
 * NULL, then the value returned by thread is stored in the 
 * location pointed by returnval
 */
void mythread_exit(void *returnval) {	
	int ind = active->thread - 1;
	int cur = ind / 16, locind = ind % 16;
	ucontext_t *thiscontext;
	if(ind >= 0) {
		superlock_lock();
		thiscontext = active->c;
		__allthreads[cur][locind]->state = THREAD_TERMINATED;
		__allthreads[cur][locind]->returnval = returnval;
		if(active->next == mainthread)
			last = previous;
		previous->next = active->next;
		free(active);
		active = mainthread;
		previous = last;
		__current--;
		if(__current == 1)
			ualarm(0, 0);
		swapcontext(thiscontext, &maincontext);
		superlock_unlock();
	}
}

/* initialises the mythread_spinlock_t pointed by lock
 */
inline int mythread_spin_init(mythread_spinlock_t *lock) {
	*lock = 0;
	return 0;
}

/* aquires the lock on mythread_spinlock_t pointed by lock, 
 * if the lock is already held by another thread, then it continuously
 * loops until lock is freed again
 */
inline int mythread_spin_lock(mythread_spinlock_t *lock) {
	if(*lock != 0 || *lock != 1)
		return EINVAL;
	while(__sync_lock_test_and_set(lock, 1));
	return 0;
}

/* frees the mythread_spinlock_t pointed by lock
 */
inline int mythread_spin_unlock(mythread_spinlock_t *lock) {
	if(*lock != 1)
		return EINVAL;
	__sync_synchronize();
	*lock = 0;
	return 0;
}

/* same as mythread_spin_lock with the difference that if the lock is 
 * held by some another thread, it returns immediately with the error
 * number EBUSY
 */
inline int mythread_spin_trylock(mythread_spinlock_t *lock) {
	return __sync_lock_test_and_set(lock, 1) ? EBUSY : 0;
}
