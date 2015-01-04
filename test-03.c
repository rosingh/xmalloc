#include "xmalloc.h"
#include <stdio.h>

/* Tests double freeing with xfree
 */
int main(int argc, char *argv[])
{
	fprintf(stdout, "Test 03: Double freeing\n");
	char *buffer = xmalloc(64);
	xfree(buffer);
	xfree(buffer);
	return 0;
}
