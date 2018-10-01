#include <stdio.h>
#include <stdlib.h>

#include "headers.h"
/* entry point for the program, calls the find_headers function */
int main(int argc, char **argv)
{
	find_headers(argv[1]);
}
