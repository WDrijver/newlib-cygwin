#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <clib/dos_protos.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>
#include <stabs.h>

BPTR * __fh;
static int maxfh;

int _open(const char *name, int flags, ...) {
	int mode;
	if (flags & O_RDWR) {
		if (flags & O_CREAT)
			mode = MODE_NEWFILE;
		else
			mode = MODE_READWRITE;
	} else
		mode = MODE_OLDFILE;

	BPTR r = Open(name, mode);
	if (!r)
		return -1;

	int fh = 3;
	while(fh < maxfh) {
		if (!__fh[fh])
			break;
		++fh;
	}

	if (fh == maxfh) {
		int n = maxfh + maxfh + 8;
		__fh = (BPTR*)realloc(__fh, n * sizeof(BPTR));
		if (!__fh)
			abort();
		while (maxfh < n) {
			__fh[maxfh++] = 0;
		}
	}
	__fh[fh] = r;
	return fh;
}

int _close(int file) {
	int r = -1;
	if (file < maxfh) {
		 r = Close(__fh[file]) == 0;
		 __fh[file] = 0;
	}
	return r;
}


int _write(int file, char *ptr, int len) {
	if (file < maxfh)
		return Write(__fh[file], ptr, len);
	return -1;
}

int _read(int file, char *ptr, int len) {
	if (file < maxfh)
		return Read(__fh[file], ptr, len);
}


void __init_fh() {
	__fh = (BPTR *)calloc(4, sizeof(BPTR));
	if (!__fh)
		abort();

	maxfh = 4;
	__fh[0] = Input();
	__fh[1] = __fh[2] = Output();
}

void __exit_fh() {
	for (int i = 3; i < maxfh; ++i) {
		if (__fh[i])
			Close(__fh[i]);
	}
}

ADD2INIT(__init_fh, -50);
ADD2EXIT(__exit_fh, -50);

ALIAS(write, _write);
ALIAS(read, _read);
