/**
 * Based on nrct0.s from libnix
 */
#include <exec/exec.h>
#include <dos/dosextens.h>
#include <inline/exec.h>
#include <stabs.h>

extern __stdargs int main(int, char **);
extern __stdargs void perror(const char *string);

extern int __INIT_LIST__;
extern int __EXIT_LIST__;
__entrypoint __stdargs int exit(int);
__entrypoint void callfuncs(int * p asm("a2"), unsigned short prioo asm("d2"));

struct ExecBase * SysBase;
static int __savedSp;
static unsigned short cleanupflag;

char * __argv[2];
int __argc;
int __commandlen;
void * __commandline;

struct Message * __WBenchMsg;

#if defined(__pic__) || defined (__PIC__)
extern const int __bss_size;
extern int _edata;
void __restore_a4(void);
__saveds
#endif

__entrypoint __regargs void start(int cmdlen, void * cmdline, int sp asm("sp")) {
#if defined(__pic__) || defined (__PIC__)
	asm("lea ___a4_init,a4");
	// clear bss
	int * p = &_edata;
	for (unsigned sz = __bss_size;sz;--sz)
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
		__WBenchMsg = GetMsg(&task->pr_MsgPort);
	}
	callfuncs(&__INIT_LIST__ + 1, 0);
	__argv[0] = __commandline;
	__argc = 1;
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
	register unsigned __d6 __asm("d6");
	asm("move.l %0,d6"::"r"(rc));

	cleanupflag ^= -1;
	callfuncs(&__EXIT_LIST__ + 1, -1);

	if (__WBenchMsg) {
		Forbid();
		ReplyMsg(__WBenchMsg);
	}

	asm("move.l %0,sp"::"r"(__savedSp));
	return __d6;
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
extern struct lib {
	struct Library *base;
	char *name;
}*__LIB_LIST__[];

/**
 * Open all libraries.
 */
void __initlibraries(void) {
	struct lib **list = __LIB_LIST__;
	ULONG numbases = (ULONG) *list++;

	while (numbases) {
		struct lib *l = *list++;
		if ((l->base = OpenLibrary(l->name, 0)) == 0) {
			perror("can't open library:");
			perror(l->name);
			exit(1);
		}
		--numbases;
	}
}

/**
 * close all opened libraries.
 */
void __exitlibraries(void) {
	struct lib **list = __LIB_LIST__;
	ULONG numbases = (ULONG) *list++;

	while (numbases) {
		struct lib *l = *list++;
		struct Library *lb = l->base;
		if (lb != 0) {
			/* l->base=NULL; */
			CloseLibrary(lb);
		}
		--numbases;
	}
}

typedef void (*func_ptr) (void);

extern func_ptr __CTOR_LIST__[];
extern func_ptr __DTOR_LIST__[];

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

ADD2INIT(__initlibraries, -100);
ADD2EXIT(__exitlibraries, -100);
ADD2INIT(__initcpp, 100);
ADD2EXIT(__exitcpp, 100);
ALIAS(_exit, exit);
