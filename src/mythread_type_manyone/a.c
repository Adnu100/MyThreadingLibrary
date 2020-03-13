#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

jmp_buf b1, b2;
int cur = 0;

void handlesig(int sig) {
	fprintf(stderr, "In handlesig\n");
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
	}
}

void fun() {	
	signal(SIGALRM, handlesig);
	while(1) {
		printf("I am in function fun()\n");
		for(int x = 0; x < 100000000; x++)
			;
	}
}

int main() {	
	signal(SIGALRM, handlesig);
	ualarm(900000, 900000);						//send SIGALRM after each 900000 microseconds
	if(!setjmp(b1))
		fun();									//will be run when setjmp returns 0
	signal(SIGALRM, SIG_DFL);
	signal(SIGALRM, handlesig);
	ualarm(900000, 900000);						//send SIGALRM after each 900000 microseconds
	while(1) {						
		printf("I am in function main()\n");	//will be run when setjmp returns 1
		for(int x = 0; x < 100000000; x++)
			//printf("main: %d\n", x);
			;
	}
	return 0;
}
