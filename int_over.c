#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv) {

	(void)argc;
	int over = atoi(argv[1]);
	printf("%d\n", over * 4);
}

	