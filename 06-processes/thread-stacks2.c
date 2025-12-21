/* This program is intended to illustrate that if you
 * know the location of another thread's stack, then
 * you can reach into that thread since all threads
 * are executing within the same virtual memory space.
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

#include <sys/resource.h>

#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM 2

int ready = 0;
void *taddrs[NUM];

void *
func(void *arg) {
	char buf[BUFSIZ] = { 0 };
	void *addr = (void*)&buf;

	taddrs[(long)arg] = addr;

	/* The dumb way to very poorly synchronize two threads.
	 * Saves me about 25 lines of mutex futzing. */
	srandom(time(0));
	(void)usleep(500);
	(void)snprintf(buf, BUFSIZ, "%ld", random());
	(void)usleep(500);

	/* We spawn only two threads, so 'arg' is exactly either 0 or 1. */
	int t1 = (long)arg;
	int t2 = !t1;
	void *p = taddrs[t2];

	(void)printf("T%d says it's at                 : %p\n" \
		     "T%d says T%d is at                : %p\n",
			t1, addr, t1, t2, p);

	(void)printf("T%d says its random number is    : %s\n",
			t1, buf);
	(void)printf("T%d says T%d's random number is   : %s\n\n",
			t1, t2, (char *)p);

	if (t1 == 0) {
		while (!ready) {
			(void)usleep(500);
		}
		(void)printf("T0 now changes T1's number.\n");
		(void)snprintf(p, BUFSIZ, "1111111111");
	} else {
		ready = 1;
		(void)usleep(750);
	}
	(void)printf("Now T%d says its random number is: %s\n", t1, buf);
	return addr;
}

int
main() {
	pthread_t threads[NUM];
	long i;

	for (i = 0; i < NUM; i++) {
		if (pthread_create(&threads[i], NULL, func, (void*)i) != 0) {
			err(EXIT_FAILURE, "pthread_create");
			/* NOTREACHED */
		}
	}

	for (i = 0; i < NUM; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			err(EXIT_FAILURE, "pthread_join");
			/* NOTREACHED */
		}
	}

	return EXIT_SUCCESS;
}
