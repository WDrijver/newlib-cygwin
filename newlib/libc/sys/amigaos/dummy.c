#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <clib/dos_protos.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <inline/dos.h>
#include <proto/exec.h>
#include <inline/exec.h>

int __oslibversion;
int __nocommandline;

int _kill(int pid, int sig) {

}

int _getpid() {
	return (int)FindTask(0);
}

int _open(const char *name, int flags, ...) {
	int mode;
	if (flags & O_RDWR) {
		if (flags & O_CREAT)
			mode = MODE_NEWFILE;
		else
			mode = MODE_READWRITE;
	} else
		mode = MODE_OLDFILE;

	int r = (int) Open(name, mode);
	printf("open %08x <- %s, %x\n", r, name, mode);
	if (r) return r;
	return -1;
}
int _close(int file) {
	printf("close %08x\n", file);
	if (file)
		return Close((BPTR) file) == 0;
	return 0;
}
int _write(int file, char *ptr, int len) {
	if ((unsigned)file <= 2)
		file = (int)Output();
	else
		printf("write %08x : %d\n", file, len);
	return Write((BPTR)file, ptr, len);
}
int _read(int file, char *ptr, int len) {
	if ((unsigned)file <=2)
		file = (int)Input();
	else
		printf("read %08x : %d\n", file, len);
	return Read((BPTR)file, ptr, len);
}
int _lseek(int file, int ptr, int dir) {
	printf("seek %08x : %d - %d\n", file, ptr, dir);
	SetIoErr(0);
	Seek((BPTR)file, ptr, dir - 1);
	int err = IoErr();
	if (err) {
		errno = EIO;
		return -1;
	}
	if (dir == SEEK_SET)
		return ptr;
	return Seek((BPTR)file, 0, OFFSET_CURRENT);
}
int _fstat(int file, struct stat *st) {
	return 0;
}
int _isatty(int file) {
	return (unsigned)file <= 2;
}

int _unlink(char *name) {
	return !DeleteFile(name);
}
