#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

void goto1(int sig);
void goto2(int sig);

jmp_buf b1, b2;

void goto1(int sig) {
	puts("SIGNAL FOR GOTO1");
	signal(SIGALRM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	if(setjmp(b1) == 0) {
		signal(SIGALRM, goto2);
		signal(SIGINT, goto2);
		ualarm(80000, 0);
		longjmp(b2, 1);
	}
	signal(SIGALRM, goto1);
	signal(SIGINT, goto1);
	printf("RETURNING FROM GOTO1\n");
}

void goto2(int sig) {
	puts("SIGNAL FOR GOTO2");
	signal(SIGALRM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	if(setjmp(b2) == 0) {
		signal(SIGALRM, goto1);
		signal(SIGINT, goto1);
		ualarm(80000, 0);
		longjmp(b1, 1);
	}
	signal(SIGALRM, goto2);
	signal(SIGINT, goto2);
	printf("RETURNING FROM GOTO2\n");
}

void fun1() {
	int i, j = 0;
	while(1) {
		printf("in fun1(): %d\n", j++);
		for(i = 0; i < (INT_MAX / 5); i++);
	}
}

void fun2() {
	int i, j = 0;
	while(1) {
		printf("in fun2(): %d\n", j++);
		for(i = 0; i < (INT_MAX / 5); i++);
	}
}

int main() {
	printf("SIZEOF JMP_BUF is %ld\n", sizeof(b1));
	signal(SIGALRM, goto1);
	signal(SIGINT, goto1);
	ualarm(800000, 0);
	if(setjmp(b2))
		fun2();
	fun1();
	return 0;
}
