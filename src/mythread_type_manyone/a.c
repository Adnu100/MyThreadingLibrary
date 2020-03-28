#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>

#define STACK_SIZE (1024 * 1024)

ucontext_t context1, context2;
int cur = 1;

void handlesig(int sig) {
	if(cur == 1) {
		cur = 2;
		swapcontext(&context1, &context2);
	}
	else {
		cur = 1;
		swapcontext(&context2, &context1);
	}
}

void fun() {	
	int i = 0;
	while(1) {
		printf("I am in function fun(), i = %d\n", i++);
		for(int x = 0; x < 100000000; x++)
			;
	}
}

int main() {	
	int i = 0;
	signal(SIGALRM, handlesig);
	getcontext(&context2);
	context2.uc_stack.ss_sp = malloc(STACK_SIZE);
	context2.uc_stack.ss_size = STACK_SIZE;
	context2.uc_link = NULL;
	makecontext(&context2, fun, 0);
	ualarm(900000, 900000);						//send SIGALRM after each 900000 microseconds
	while(1) {						
		printf("I am in function main(), i = %d\n", i++);	//will be run when setjmp returns 1
		for(int x = 0; x < 100000000; x++)
			;
	}
	return 0;
}
