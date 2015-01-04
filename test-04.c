#include "xmalloc.h"

int main(int argc, char *argv[])
{
	int i;
	void *ptrs[50];
	for (i = 0; i < 50; ++i)
	{
		ptrs[i] = xmalloc(50);
	}
	for (i = 0; i < 25; ++i)
	{
		xfree(ptrs[i] + 5);
		xfree(ptrs[i]);
		xfree(ptrs[i]);
	}
	xfree(&i);
	return 0;
}
