#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sshmem_api.h>
#include <ringtest.h>
#include <assert.h>

/* referenced in test_init() */
char *pathname=0;
mode_t mode=SSYS_SHMEM_MODE_PIPE;
int readers=1;
int elements=0;
int element_size=0;
int header_size=0;

int
main(int argc, char **argv)
{
  test_init(argc, argv);

  struct pollfd fds;
  memset((void*)&fds, 0x0, sizeof(fds));
  if (0<(fds.fd=ssys_shmem_open(pathname,
                                SSYS_SHMEM_FLAG_CREATE|SSYS_SHMEM_FLAG_READ,
                                mode)))
    {
      perror("ssys_shmem_open");
      exit(1);
    }

  fds.events=POLLIN;
  int rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==0);

  long payload=0xdeadbeef;
  if (0>ssys_shmem_write(fds.fd, &payload, sizeof(long)))
    {
      perror("ssys_shmem_write");
      exit(1);
    }

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==POLLIN);

  payload=0L;
  if (0>ssys_shmem_read(fds.fd, &payload, sizeof(long)))
    {
      perror("ssys_shmem_read");
      exit(1);
    }

  assert(0xdeadbeef==payload);

  fds.events=(POLLIN|POLLOUT);
  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==POLLOUT);

  payload=0xcafebabe;
  if (0>ssys_shmem_write(fds.fd, &payload, sizeof(long)))
    {
      perror("ssys_shmem_write");
      exit(1);
    }

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==(POLLIN|POLLOUT));

  if (0<ssys_shmem_close(fds.fd))
    perror("ssys_shmem_close");

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==POLLERR);

  free(pathname);

  return 0;
}
