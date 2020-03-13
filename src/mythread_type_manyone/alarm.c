#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

void handler(int sig) {	
	printf("SIGNAL RECEIVED\n");
}

int main() {
	signal(SIGALRM, handler);
	ualarm(500, 500);
	while(1);
}
