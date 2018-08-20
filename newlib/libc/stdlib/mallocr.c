#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <string.h>
#include <clib/exec_protos.h>
#include <proto/exec.h>
#include <inline/exec.h>

#include <stdio.h>

void *	_malloc_r (struct _reent * r, size_t sz) {
	size_t* p = (size_t*)AllocMem(sz += 4, MEMF_ANY);
	if (p)
		*p++ = sz;

	static int in;
	if (!in++)
		printf("malloc %d -> %08x\n", sz, p); fflush(stdout);
	--in;
	return p;
}

void *	_calloc_r (struct _reent * r, size_t a, size_t b) {
	size_t sz = a * b;
	void * p = _malloc_r(r, sz);
	if (p)
		memset(p, 0, sz);
	return p;
}

void _free_r (struct _reent * r, void * pp) {
	if (pp) {
		size_t * p = (size_t*)pp;
		size_t sz = *--p;
		static int in;
		if (!in++)
			printf("free   %d -> %08x\n", sz, pp);fflush(stdout);
		--in;
		FreeMem(p, sz);
	}
}

void *	_realloc_r (struct _reent * r, void * old, size_t sz) {
	static int in;
	if (!in++)
		printf("realloc %d -> %08x\n", sz, old);fflush(stdout);
	--in;
	void * p = _malloc_r(r, sz);
	if (p)
	{
		if (old) {
			size_t * oldp = (size_t*)old;
			size_t copy = *--oldp;
			if (copy > sz) copy = sz;
			memcpy(p, old, copy);
			_free_r(r, old);
		}
	}
	return p;
}
