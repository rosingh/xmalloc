#include "xmalloc.h"
#include <stdio.h>

/* Test freeing in the middle of an allocated region with xfree
 */
int main(int argc, char *argv[])
{
	fprintf(stdout, "Test 01: Freeing in the middle of an allocated region\n");
	char *buffer = xmalloc(50);
	*buffer = 'a';
	buffer += 25;
	xfree(buffer);
	return 0;
}
