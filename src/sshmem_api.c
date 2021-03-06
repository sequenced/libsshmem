#include <errno.h>
#include <string.h>
#include <ring.h>
#include <sshmem_api.h>
#include <rcommon.h>

#define SSYS_DESCRIPTOR_UNASSIGNED(desc) (NULL==memdesc[desc].p)

static ssys_ring_t memdesc[SSYS_SHMEM_DESC_MAX];
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
      memset(&memdesc, 0x0, sizeof(ssys_ring_t)*SSYS_SHMEM_DESC_MAX);
      once=0;
    }

  int md=0; /* first descriptor is zero */
  while (md<SSYS_SHMEM_DESC_MAX+1)
    {
      if (SSYS_SHMEM_DESC_MAX==md)
        {
          errno=EMFILE;
          return -1;
        }

      if (SSYS_DESCRIPTOR_UNASSIGNED(md))
        break;

      md++;
    }

  memdesc[md].num_elements=SSYS_SHMEM_ELEMENTS;
  memdesc[md].element_size=SSYS_SHMEM_ELEMENT_SIZE;
  memdesc[md].header_size=SSYS_SHMEM_HEADER_SIZE;
  memdesc[md].mode=mode;

  int fd;
  if (0>(fd=ring_shmem_open(pathname)))
    {
      if (SSYS_BIT_ON(SSYS_SHMEM_FLAG_CREATE, flags))
        {
          if (0>(fd=ring_shmem_create(&memdesc[md], pathname)))
            return -1;
        }
      else
        {
          errno=ENOENT;
          return -1;
        }
    }
  else
    /* ring existed, no creation necessary */
    flags=flags&~SSYS_SHMEM_FLAG_CREATE;

  if (0>ring_shmem_map(&memdesc[md], pathname, fd))
    return -1;

  return (0<ssys_ring_open(&memdesc[md], flags)?-1:md);
}

int
ssys_shmem_write(int md, const void *buf, size_t count)
{
  if (md<0 || SSYS_SHMEM_DESC_MAX<=md)
    {
      errno=EINVAL;
      return -1;
    }

  return ssys_ring_write(&memdesc[md], buf, count);
}

int
ssys_shmem_read(int md, void *buf, size_t count)
{
  if (md<0 || SSYS_SHMEM_DESC_MAX<=md)
    {
      errno=EINVAL;
      return -1;
    }

  return ssys_ring_read(&memdesc[md], buf, count);
}

int
ssys_shmem_close(int md)
{
  if (md<0 || SSYS_SHMEM_DESC_MAX<=md)
    {
      errno=EINVAL;
      return -1;
    }

  memset(&memdesc[md], 0x0, sizeof(ssys_ring_t));

  return 0;
}

int
ssys_shmem_poll(struct pollfd *fds, nfds_t nfds, int ignored)
{
  if (NULL==fds)
    {
      errno=EINVAL;
      return -1;
    }

  int rv, num=0;
  while (nfds>0)
    {
      if (fds->events && SSYS_DESCRIPTOR_UNASSIGNED(fds->fd))
        {
          /* cannot read or write to an unassigned descriptor */
          fds->revents=POLLERR;
          fds++;
          nfds--;
          continue;
        }

      fds->revents=0;

      if (POLLIN&fds->events)
        {
          rv=ssys_ring_poll_read(&memdesc[fds->fd]);
          if (rv<0)
            fds->revents|=POLLERR;
          else if (rv)
            fds->revents|=POLLIN;
        }

      if (POLLOUT&fds->events)
        {
          rv=ssys_ring_poll_write(&memdesc[fds->fd]);
          if (rv<0)
            fds->revents|=POLLERR;
          else if (rv)
            fds->revents|=POLLOUT;
        }

      /* count selectable descriptors */
      if (fds->revents)
        num++;

      fds++;
      nfds--;
    }

  return num;
}
