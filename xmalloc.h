#ifndef XMALLOC_H
#define XMALLOC_H
#include <stddef.h>

    void *xmalloc(size_t bytes);
    void xfree(void *ptr);

#endif
