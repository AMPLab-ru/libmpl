#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int 
rand_urandom(void *buf, int len, void *unused)
{
	int fd;
	int rc;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		return -1;

	rc = read(fd, buf, len);

	close(fd);
	return rc;
}

int 
rand_null(void *buf, int len, void *unused)
{
	int fd;
	int rc;

	fd = open("/dev/null", O_RDONLY);
	if (fd == -1)
		return -1;

	rc = read(fd, buf, len);

	close(fd);
	return rc;
}

