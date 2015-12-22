#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sshmem_api.h>
#include <ringtest.h>
#include <assert.h>

#define BUF_SIZE 64

/* referenced in test_init() */
char *pathname=0;
mode_t mode=0;
int flags=0;
int readers=1;
int elements=0;
int element_size=0;
int header_size=0;
char *payload=0;
int payload_len=0;
int printable=0;

void test_pipe();
void test_buffer();

int
main(int argc, char **argv)
{
  test_init(argc, argv);
  test_pipe();
  test_buffer();

  free(pathname);

  return 0;
}

void
test_pipe()
{
  char pipename[BUF_SIZE];
  memset((void*)pipename, 0x0, BUF_SIZE);
  strcat(pipename, pathname);
  strcat(pipename, "-pipe");

  struct pollfd fds;
  memset((void*)&fds, 0x0, sizeof(fds));
  if (0<(fds.fd=ssys_shmem_open(pipename,
                                SSYS_SHMEM_FLAG_CREATE|SSYS_SHMEM_FLAG_READ,
                                SSYS_SHMEM_MODE_PIPE)))
    {
      perror("ssys_shmem_open");
      exit(1);
    }

  fds.events=POLLIN;
  int rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.events==POLLIN);
  assert(fds.revents==0);
  assert(rv==0);

  fds.events=POLLOUT;
  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.events==POLLOUT);
  assert(fds.revents==POLLOUT);
  assert(rv==1);

  long payload=0xdeadbeef;
  if (0>ssys_shmem_write(fds.fd, &payload, sizeof(long)))
    {
      perror("ssys_shmem_write");
      exit(1);
    }

  fds.events=POLLIN;
  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==POLLIN);
  assert(rv==1);

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
  assert(rv==1);

  payload=0xcafebabe;
  if (0>ssys_shmem_write(fds.fd, &payload, sizeof(long)))
    {
      perror("ssys_shmem_write");
      exit(1);
    }

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==(POLLIN|POLLOUT));
  assert(rv==1);

  if (0<ssys_shmem_close(fds.fd))
    perror("ssys_shmem_close");

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==POLLERR);
  assert(rv==0);

  /* re-open pipe w/o create flag */
  memset((void*)&fds, 0x0, sizeof(fds));
  if (0<(fds.fd=ssys_shmem_open(pipename,
                                SSYS_SHMEM_FLAG_READ|SSYS_SHMEM_FLAG_WRITE,
                                SSYS_SHMEM_MODE_PIPE)))
    {
      perror("ssys_shmem_open");
      exit(1);
    }

  fds.events=(POLLIN|POLLOUT);
  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==(POLLIN|POLLOUT));
  assert(rv==1);

  payload=0L;
  if (0>ssys_shmem_read(fds.fd, &payload, sizeof(long)))
    {
      perror("ssys_shmem_read");
      exit(1);
    }
  assert(0xcafebabe==payload);

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==POLLOUT);
  assert(rv==1);

  payload=0xcafecafe;
  int i;
  for (i=0; i<SSYS_SHMEM_ELEMENTS; i++)
    {
      if (0>ssys_shmem_write(fds.fd, &payload, sizeof(long)))
        {
          perror("ssys_shmem_write");
          exit(1);
        }
    }

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==POLLIN);
  assert(rv==1);

  payload=0L;
  if (0>ssys_shmem_read(fds.fd, &payload, sizeof(long)))
    {
      perror("ssys_shmem_read");
      exit(1);
    }
  assert(0xcafecafe==payload);

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==(POLLIN|POLLOUT));
  assert(rv==1);

  if (0<ssys_shmem_close(fds.fd))
    perror("ssys_shmem_close");
}

void
test_buffer()
{
  char buffername[BUF_SIZE];
  memset((void*)buffername, 0x0, BUF_SIZE);
  strcat(buffername, pathname);
  strcat(buffername, "-buffer");

  struct pollfd fds;
  memset((void*)&fds, 0x0, sizeof(fds));
  if (0<(fds.fd=ssys_shmem_open(buffername,
                                SSYS_SHMEM_FLAG_CREATE|SSYS_SHMEM_FLAG_READ,
                                SSYS_SHMEM_MODE_BUFFER)))
    {
      perror("ssys_shmem_open");
      exit(1);
    }

  fds.events=POLLIN;
  int rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==0);
  assert(rv==0);
  fds.events|=POLLOUT;
  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==POLLOUT);
  assert(rv==1);

  long payload=0xcafebabe;
  if (0>ssys_shmem_write(fds.fd, &payload, sizeof(long)))
    {
      perror("ssys_shmem_write");
      exit(1);
    }

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==(POLLIN|POLLOUT));
  assert(rv==1);

  if (0<ssys_shmem_close(fds.fd))
    perror("ssys_shmem_close");

  rv=ssys_shmem_poll(&fds, 1, 0L);
  assert(fds.revents==POLLERR);
  assert(rv==0);
}
