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
alloc_and_map_shmem(ssys_ring_t *pmd, const char *name)
{
  int fd;
  if (0>(fd=shm_open(name, O_RDWR, S_IRUSR|S_IWUSR)))
    if (0>(fd=shm_open(name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)))
      return -1;

  if (0>ftruncate(fd, pmd->num_elements*pmd->element_size))
    {
      shm_unlink(name);
      return -1;
    }

  if (MAP_FAILED==(pmd->p=mmap(NULL,
                               pmd->num_elements*pmd->element_size,
                               PROT_READ|PROT_WRITE,
                               MAP_SHARED,
                               fd,
                               0)))
    {
      shm_unlink(name);
      pmd->p=NULL;
      return -1;
    }

  return 0;
}

int
free_and_unmap_shmem(ssys_ring_t *pmd)
{
  //TODO
  return -1;
}
