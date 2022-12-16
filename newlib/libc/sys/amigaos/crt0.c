/**
 * Based on nrct0.s from libnix
 */
#include <exec/exec.h>
#include <dos/dosextens.h>
#include <inline/exec.h>
#include <stabs.h>

extern __stdargs int main(int, char **);
extern __stdargs void perror(const char *string);

extern struct Library * DOSBase;

__attribute__((section(".list___INIT_LISA__")))
int __INIT_LIST__[1] = {0};
__attribute__((section(".list___EXIT_LISA__")))
int __EXIT_LIST__[1] = {0};

__entrypoint __stdargs int exit(int);
__entrypoint void callfuncs(int * p asm("a2"), unsigned short prioo asm("d2"));

struct ExecBase * SysBase;
static int __savedSp;
static unsigned short cleanupflag;

char * __argv[] = {0, 0};
int __argc;
int __commandlen;
void * __commandline;

struct Message * _WBenchMsg;

#if defined(__pic__) || defined (__PIC__)
extern const int __bss_init_size;
extern int _edata;
void __restore_a4(void);
__saveds
#endif

__entrypoint __regargs void ____start(int cmdlen, void * cmdline, int sp asm("sp")) {
#if defined(__pic__) || defined (__PIC__)
	asm("lea ___a4_init,a4");
	// clear bss
	int * p = &_edata;
	for (unsigned sz = __bss_init_size;sz;--sz)
	*p++ = 0;

#endif

	__savedSp = sp;
	__commandlen = cmdlen;
	__commandline = cmdline;

	SysBase = *(struct ExecBase **) 4;

	// check for wb message if not cli
	struct Process * task = (struct Process *) FindTask(0);
	if (!task->pr_CLI) {
		WaitPort(&task->pr_MsgPort);
		_WBenchMsg = GetMsg(&task->pr_MsgPort);
		__argv[0] = (char *)_WBenchMsg;
		__argc = 0;
	} else {
		__argv[0] = __commandline;
		__argc = 1;
	}
	callfuncs(&__INIT_LIST__[0] + 1, 0);
	exit(main(__argc, __argv));
}

#if defined(__pic__) || defined (__PIC__)
void __entrypoint __restore_a4(void) {
	asm("lea ___a4_init,a4");
}
__saveds
#endif

/**
 * The exit function.
 * Call cleanup before restoring the stack and setting the return code.
 */
__entrypoint __stdargs int exit(int rc) {
	register unsigned __d7 __asm("d7");
	asm("move.l %0,d7"::"r"(rc));

	cleanupflag ^= -1;
	callfuncs(&__EXIT_LIST__[0] + 1, -1);

	if (_WBenchMsg) {
		Forbid();
		ReplyMsg(_WBenchMsg);
	}

	asm("move.l %0,sp"::"r"(__savedSp));
	return __d7;
}

/**
 * Loop over functions.
 * - find next prio
 * - call functions matching last prio
 */
__entrypoint void callfuncs(int * q asm("a2"), unsigned short order asm("d2")) {
	for (;;) {
		int * p = q;
		unsigned short curprio = cleanupflag;
		unsigned short nextprio = -1;

		while (*p) {
			unsigned short prio = *((unsigned short *) p + 3) ^ order;

			// invoke
			if (prio == curprio)
				((void (*)(void)) *p)();

			// update next prio
			if (prio < nextprio && prio > curprio)
				nextprio = prio;

			p += 2;
		}
		if (nextprio == curprio)
			break;

		cleanupflag = nextprio;
	}
}

/* These are the elements pointed to by __LIB_LIST__ */
__attribute__((section(".dlist___LIB_LIST__")))
long __LIB_LIST__ = 0;
__attribute__((section(".dlist___LIB_LISZ__")))
long __ZLIB = 0;

/**
 * Open all libraries.
 */
void __initlibraries(void) {
	long * l = &__LIB_LIST__ + 1;
	while (*l) {
		long * base = l++;
		char const * const name = *(char **) (l++);
		if ((*base = (long)OpenLibrary(name, 0)) == 0) {
			if (DOSBase) {
				fputs(2, "can't open library:");
				fputs(2, name);
			}
			exit(1);
		}
	}
}

/**
 * close all opened libraries.
 */
void __exitlibraries(void) {
	long * l = &__LIB_LIST__ + 1;
	while (*l) {
		long * base = l++;
		if (*base != 0) {
			CloseLibrary((struct Library *)*base);
		}
		++l;
	}
}

typedef void (*func_ptr) (void);

__attribute__((section(".list___CTOR_LISA__")))
func_ptr __CTOR_LIST__[] = {0};
__attribute__((section(".list___DTOR_LISA__")))
func_ptr __DTOR_LIST__[] = {0};
__attribute__((section(".list___ZZZZ__")))
func_ptr __ZZZZ__[] = {0};


void __initcpp() {
  func_ptr *p0 = __CTOR_LIST__ + 1;
  func_ptr *p;
  for (p = p0; *p; p++);
  while (p > p0)
    (*--p)();
}

void __exitcpp() {
  func_ptr *p = __DTOR_LIST__ + 1;
  while (*p)
    (*p++)();
}

extern void __nocommandline() ;
static void (*used_commandline)() = __nocommandline;

ADD2INIT(__initlibraries, -100);
ADD2EXIT(__exitlibraries, -100);
ADD2INIT(__initcpp, 100);
ADD2EXIT(__exitcpp, 100);
ALIAS(_exit, exit);

