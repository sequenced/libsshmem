#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <ring.h>

extern mode_t mode;
extern int elements;
extern int element_size;
extern int header_size;

void
init_default_ring(ssys_ring_t *pmd)
{
  memset(pmd, 0x0, sizeof(ssys_ring_t));
  pmd->num_elements=elements;
  pmd->element_size=element_size;
  pmd->header_size=header_size;
  pmd->mode=mode;
}

int
ring_shmem_open(const char *name)
{
  return shm_open(name, O_RDWR, S_IRUSR|S_IWUSR);
}

int
ring_shmem_create(const ssys_ring_t *pmd, const char *name)
{
  int fd;
  /* shared memory object does not exist, create it */
  if (0>(fd=shm_open(name, O_EXCL|O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)))
    return -1;

  if (0>ftruncate(fd, pmd->num_elements*pmd->element_size))
    {
      shm_unlink(name);
      close(fd);
      return -1;
    }

  return fd;
}

int
ring_shmem_map(ssys_ring_t *pmd, const char *name, const int fd)
{
  if (MAP_FAILED==(pmd->p=mmap(NULL,
                               pmd->num_elements*pmd->element_size,
                               PROT_READ|PROT_WRITE,
                               MAP_SHARED,
                               fd,
                               0)))
    {
      shm_unlink(name);
      close(fd);
      pmd->p=NULL;
      return -1;
    }

  close(fd);

  return 0;
}

int
alloc_and_map_shmem(ssys_ring_t *pmd, const char *name, int flags)
{
  int fd;
  if (0>(fd=ring_shmem_open(name)))
    {
      if (SSYS_BIT_ON(SSYS_RING_FLAG_CREATE, flags))
        {
          if (0>(fd=ring_shmem_create(pmd, name)))
            return -1;
        }
      else
        {
          errno=ENOENT;
          return -1;
        }
    }

  if (0>ring_shmem_map(pmd, name, fd))
    return -1;
}

int
free_and_unmap_shmem(ssys_ring_t *pmd)
{
  //TODO
  return -1;
}
