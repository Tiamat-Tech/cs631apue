/* This program tries to determine the offset of each
 * thread's stack in a multi-threaded program.
 *
 * This file is in the public domain.
 *
 * You don't have to, but if you feel like
 * acknowledging where you got this code, you may
 * reference me by name, email address, or point
 * people to the course website:
 * https://stevens.netmeister.org/631/
 *
 * See also:
 * https://www.netmeister.org/blog/thread-stacks.html
 */
#define _GNU_SOURCE

#include <sys/resource.h>

#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <pthread_np.h>
#endif

#include <err.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NUM
#define NUM 3
#endif

int
getGuardsize() {
	pthread_attr_t attr;
	size_t guardsize;

	if (pthread_attr_init(&attr) != 0) {
		err(EXIT_FAILURE, "pthread_attr_init");
		/* NOTREACHED */
	}
	if (pthread_attr_getguardsize(&attr, &guardsize) != 0) {
		err(EXIT_FAILURE, "pthread_attr_getguardsize");
		/* NOTREACHED */
	}
	(void)pthread_attr_destroy(&attr);

	return guardsize;
}

size_t
getThreadStacksize() {
	size_t size;

#ifdef __MACH__
	if ((size = pthread_get_stacksize_np(pthread_self())) == 0) {
		err(EXIT_FAILURE, "pthread_get_stacksize_np");
		/* NOTREACHED */
	}
#elif defined(__OpenBSD__)
	stack_t ss;
	if (pthread_stackseg_np(pthread_self(), &ss) != 0) {
		err(EXIT_FAILURE, "pthread_stackseg_np");
		/* NOTREACHED */
	}

	size = ss.ss_size;

#elif defined(__NetBSD__) || defined(__FreeBSD__) || \
	defined(__illumos__) || defined(__linux__)
	pthread_attr_t attr;

	if (pthread_attr_init(&attr) != 0) {
		err(EXIT_FAILURE, "pthread_attr_init");
		/* NOTREACHED */
	}
#  if defined(__FreeBSD__) || defined(__illumos__)
	if (pthread_attr_get_np(pthread_self(), &attr) != 0) {
		err(EXIT_FAILURE, "pthread_attr_get_np");
		/* NOTREACHED */
	}
#  else
	if (pthread_getattr_np(pthread_self(), &attr) != 0) {
		err(EXIT_FAILURE, "pthread_getattr_np");
		/* NOTREACHED */
	}
#  endif

	if (pthread_attr_getstacksize(&attr, &size) != 0) {
		err(EXIT_FAILURE, "pthread_attr_getstacksize");
		/* NOTREACHED */
	}

	(void)pthread_attr_destroy(&attr);
#endif

	return size;
}

int
getStacksize() {
	struct rlimit rlp;

	if (getrlimit(RLIMIT_STACK, &rlp) != 0) {
		err(EXIT_FAILURE, "rlimit");
		/* NOTREACHED */
	}

	return (int)rlp.rlim_cur;
}

void *
func(void *arg) {
	void *var;
	void *addr = &var;
	size_t s;

	s = getThreadStacksize();

	(void)printf("Thread %ld is at %p.\n", (long)arg, addr);
	(void)printf("Thread %ld stack size: %zu.\n", (long)arg, s);
	return addr;
}

int
main(int argc, char **argv, char **envp) {
	long i;
	pthread_t threads[NUM];
	void *taddrs[NUM];

	int gsize = getGuardsize();
	int ssize = getStacksize();

	(void)printf("argc at %p\n", &argc);
	(void)printf("argv at %p\n", &argv);
	(void)printf("envp at %p\n", &envp);
	(void)printf("main at %p\n\n", &threads);

	i = (ssize == 0) ? 1 : (int)log10(ssize)+1;

	(void)printf("Guard size: %*d\n", (int)i, gsize);
	(void)printf("Stack size: %*d\n\n", (int)i, ssize);

	for (i = 0; i < NUM; i++) {
		if (pthread_create(&threads[i], NULL, func, (void*)i) != 0) {
			err(EXIT_FAILURE, "pthread_create");
		}
	}

	for (i = 0; i < NUM; i++) {
		if (pthread_join(threads[i], &taddrs[i]) != 0) {
			err(EXIT_FAILURE, "pthread_join");
		}
	}

	(void)printf("\nStack address differences between threads:\n");
	for (i = 1; i < NUM; i++) {
		long diff = taddrs[i - 1] - taddrs[i];
		(void)printf("Thread %ld - Thread %ld: %ld bytes\n", i - 1, i, diff);
	}

	return EXIT_SUCCESS;
}
