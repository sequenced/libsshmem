#include <errno.h>
#include <string.h>
#include <ring.h>
#include <sshmem_api.h>

/* defaults */
#define MEM_DESC_MAX            64
#define SSYS_SHMEM_ELEMENTS     64
#define SSYS_SHMEM_ELEMENT_SIZE 2048
#define SSYS_SHMEM_HEADER_SIZE  64

static ssys_ring_t memdesc[MEM_DESC_MAX];
static int once=1;

int
ssys_shmem_open(const char *pathname, int flags, mode_t mode)
{
  if (NULL==pathname)
    {
      errno=EINVAL;
      return -1;
    }

  if (once)
    {
      memset(&memdesc, 0x0, sizeof(ssys_ring_t)*MEM_DESC_MAX);
      once=0;
    }

  int md=0;
  while (md<MEM_DESC_MAX+1)
    {
      if (MEM_DESC_MAX==md)
        {
          errno=EMFILE;
          return -1;
        }

      if (NULL==memdesc[md].p)
        break;

      md++;
    }

  memdesc[md].num_elements=SSYS_SHMEM_ELEMENTS;
  memdesc[md].element_size=SSYS_SHMEM_ELEMENT_SIZE;
  memdesc[md].header_size=SSYS_SHMEM_HEADER_SIZE;
  memdesc[md].mode=mode;

  if (0<alloc_and_map_shmem(&memdesc[md], pathname))
    return -1;

  return (0<ssys_ring_open(&memdesc[md], flags)?-1:md);
}

int
ssys_shmem_write(int md, const void *buf, size_t count)
{
  if (md<0 || MEM_DESC_MAX<=md)
    {
      errno=EINVAL;
      return -1;
    }

  return ssys_ring_write(&memdesc[md], buf, count);
}

int
ssys_shmem_read(int md, void *buf, size_t count)
{
  if (md<0 || MEM_DESC_MAX<=md)
    {
      errno=EINVAL;
      return -1;
    }

  return ssys_ring_read(&memdesc[md], buf, count);
}

int
ssys_shmem_close(int md)
{
  if (md<0 || MEM_DESC_MAX<=md)
    {
      errno=EINVAL;
      return -1;
    }

  memset(&memdesc[md], 0x0, sizeof(ssys_ring_t));

  return 0;
}

int
ssys_shmem_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
  return -1;
}
