#include "xmalloc.h"
#include <stdio.h>

/* Test double mallocing with xmalloc, which causes a memory leak.
 */
int main(int argc, char *argv[])
{
	fprintf(stdout, "Test 02: Double mallocing (memory leak)\n");
	char *buffer = xmalloc(50);
	buffer = xmalloc(50);
	xfree(buffer);
	return 0;
}
