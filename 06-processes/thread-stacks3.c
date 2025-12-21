/* This program is intended to illustrate that if you
 * can guess the location of another thread's stack, then
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

int ready = 0;

#define BUF_SIZE 16

void *
thread1(void *arg) {
	char buf[BUF_SIZE] = { 0 };
	void *addr = (void *)&buf;

	(void)arg;

	srandom(time(0));

	(void)snprintf(buf, BUF_SIZE, "%ld", random());

	(void)printf("T1 is at %p\n", addr);
	(void)printf("T1 says its random number is    : %s\n", buf);

	/* The dumb way to very poorly synchronize two threads.
	 * Saves me about 25 lines of mutex futzing. */
	while (!ready) {
		usleep(1000);
	}

	(void)printf("Now T1 says its random number is: %s\n", buf);

	return NULL;
}

void *
thread2(void *arg) {
	long offset = (long)arg;

	void *p = (void *)&arg + offset;

	(void)printf("T2 is at %p\n", &arg);
	(void)printf("T2 says T1's random number is   : %s\n", (char *)p);
	usleep(500);
	(void)snprintf(p, BUF_SIZE, "1111111111");

	ready = 1;
	return NULL;
}

int
main(int argc, char **argv) {
	pthread_t threads[2];
	long offset;

	if (argc != 2) {
		errx(EXIT_FAILURE, "Usage: %s offset", argv[0]);
		/* NOTREACHED */
	}

	if ((offset = atoi(argv[1])) == 0) {
		errx(EXIT_FAILURE, "Invalid offset.");
		/* NOTREACHED */
	}

	if (pthread_create(&threads[0], NULL, thread1, NULL) != 0) {
		err(EXIT_FAILURE, "pthread_create");
		/* NOTREACHED */
	}
	if (pthread_create(&threads[1], NULL, thread2, (void *)offset) != 0) {
		err(EXIT_FAILURE, "pthread_create");
		/* NOTREACHED */
	}

	for (int i = 0; i < 2; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			err(EXIT_FAILURE, "pthread_join");
			/* NOTREACHED */
		}
	}

	return EXIT_SUCCESS;
}
