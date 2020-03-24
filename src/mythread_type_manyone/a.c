#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>

ucontext_t context1, context2;
int cur = 0;

void handlesig(int sig) {
	/*fprintf(stderr, "In handlesig\n");
	signal(SIGALRM, handlesig);
	if(!cur) {
		cur = 1;
		setjmp(b2);
		longjmp(b1, 1);
	}
	else {
		cur = 0;
		setjmp(b1);
		longjmp(b2, 1);
	}*/
	if(cur) {
		puts("changing to fun()");
		cur = !cur;
		getcontext(&context2);
		puts("saved 2");
		if(cur == 0)
			setcontext(&context1);
	}
	else {
		puts("changing to main()");
		cur = !cur;
		puts("saved 1");
		getcontext(&context1);
		if(cur == 1)
			setcontext(&context2);
	}
	puts("OUT OF SIGNAL HANDLER");
}

void fun() {	
	while(1) {
		printf("I am in function fun()\n");
		for(int x = 0; x < 100000000; x++)
			;
	}
}

int main() {	
	signal(SIGALRM, handlesig);
	ualarm(900000, 900000);						//send SIGALRM after each 900000 microseconds
	getcontext(&context2);
	if(cur == 0) {
		fun();	
	}		//will be run when setjmp returns 0
	else {
		while(1) {						
			printf("I am in function main()\n");	//will be run when setjmp returns 1
			for(int x = 0; x < 100000000; x++)
				;
		}
	}
	return 0;
}
