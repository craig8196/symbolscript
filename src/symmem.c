
#include "symmem.h"


void *
memget(size_t x)
{
    return malloc(x);
}

void *
memreget(void *p, size_t l)
{
    return realloc(p, l);
}

void
memput(void *p)
{
    free(p);
}

size_t
meminc(size_t x)
{
    return x * 2;
}

