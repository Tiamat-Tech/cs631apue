#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv) {
	int fd1, fd2;
	char buf[32];

	fd1 = open("f", O_RDWR | O_CREAT, 0777);
	fd2 = open("f", O_WRONLY);
	write(fd1, "some amount of data", 23);
	write(fd2, "blahblah", 8);
	lseek(fd1, 0, SEEK_SET);
	read(fd1, buf, 31);
	(void)printf("%s\n", buf);
	return 0;
}
