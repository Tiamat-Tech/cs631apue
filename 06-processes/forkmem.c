/* This program is intended to illustrate that after a
 * fork, both processes have the exact same memory
 * layout with all variables, frames, etc. located at
 * the same addresses, but that manipulating variables
 * in each process is not visible in the other process
 * -- with the exception of shared memory.
 *
 * This file is in the public domain.
 *
 * You don't have to, but if you feel like
 * acknowledging where you got this code, you may
 * reference me by name, email address, or point
 * people to the course website:
 * https://stevens.netmeister.org/631/
 *
 * See
 * https://www.netmeister.org/blog/process-memory.html
 */


#include <sys/shm.h>
#include <sys/wait.h>

#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SHM_MODE 0600

int n = 0;
char buf[BUFSIZ];

void
func(char *s) {
	int i;

	(void)snprintf(buf, BUFSIZ, "0x%012lx %04d: func frame\n" \
			      "0x%012lx %04d: func arg: \"%s\"\n",
				(uintptr_t)&i, getpid(),
				(uintptr_t)&s, getpid(), s);
}

int
main(int argc, char **argv) {
	pid_t pid;
	char s[BUFSIZ] = { 0 };
	int shmid;
	void *shptr, *hptr;
	pid_t p;

	p = getpid();

	(void)printf("0x%012lx %04d: argc\n", (uintptr_t)&argc, p);
	(void)printf("0x%012lx %04d: argv\n", (uintptr_t)&argv, p);

	(void)printf("0x%012lx %04d: main (before fork)\n", (uintptr_t)&pid, p);

	if ((hptr = malloc(BUFSIZ)) == NULL) {
		err(EXIT_FAILURE, "malloc");
		/* NOTREACHED */
	}

	if ((shmid = shmget(IPC_PRIVATE, BUFSIZ, SHM_MODE)) < 0) {
		err(EXIT_FAILURE, "shmget");
		/* NOTREACHED */
	}

	if ((shptr = shmat(shmid, 0, 0)) == (void *)-1) {
		err(EXIT_FAILURE, "shmat");
		/* NOTREACHED */
	}

	(void)snprintf(shptr, BUFSIZ, "--------");
	(void)snprintf(hptr, BUFSIZ, "++++++++");
	(void)printf("0x%012lx %04d: heap memory  : \"%s\"\n", (uintptr_t)hptr, p, (char *)hptr);
	(void)printf("0x%012lx %04d: shared memory: \"%s\"\n", (uintptr_t)shptr, p, (char *)shptr);
	(void)printf("\n");

	if (shmctl(shmid, IPC_RMID, 0) < 0) {
		err(EXIT_FAILURE, "shmctl");
		/* NOTREACHED */
	}

	if ((pid = fork()) < 0) {
		err(EXIT_FAILURE, "fork");
		/* NOTREACHED */
	}

	p = getpid();
	if (pid == 0) {
		n++;
		(void)printf("Child changes shared memory at 0x%012lx.\n", (uintptr_t)shptr);
		(void)snprintf(s, BUFSIZ, "Child");
		argv[0] = "child argv0";
		(void)snprintf(shptr, BUFSIZ, "XXXXXXXX");
	} else {
		(void)printf("Parent changes heap memory at  0x%012lx.\n", (uintptr_t)hptr);
		(void)snprintf(s, BUFSIZ, "Parent");
		(void)snprintf(hptr, BUFSIZ, "########");
	}

	func(s);

	wait(NULL);

	(void)printf("\n[ %s ]:\n", n > 0 ? "Child" : "Parent");
	(void)printf("%s", buf);
	(void)printf("0x%012lx %04d: main (after fork)\n",(uintptr_t)&pid, p);
	(void)printf("0x%012lx %04d: argv\n", (uintptr_t)&argv, p);
	(void)printf("0x%012lx %04d: argv[0]      : %s\n", (uintptr_t)&(argv[0]), p, argv[0]);

	(void)printf("0x%012lx %04d: n            : %d\n", (uintptr_t)&n, p, n);
	(void)printf("0x%012lx %04d: heap memory  : \"%s\"\n", (uintptr_t)hptr, p, (char *)hptr);
	(void)printf("0x%012lx %04d: shared memory: \"%s\"\n", (uintptr_t)shptr, p, (char *)shptr);

	return EXIT_SUCCESS;
}
