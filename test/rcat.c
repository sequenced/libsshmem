#include <stdlib.h>
#include <stdio.h>
#include <atomic64_64.h>
#include <ring.h>
#include <ringtest.h>
#include <rcommon.h>

char *pathname=0;
mode_t mode=SSYS_RING_MODE_PIPE;
int readers=1;
int elements=RING_ELEMENTS;
int element_size=RING_ELEMENT_SIZE;
int header_size=RING_HEADER_SIZE;

int
main(int argc, char **argv)
{
  test_init(argc, argv);
  ssys_ring_t ring;
  init_default_ring(&ring);
  if (0>alloc_and_map_shmem(&ring, pathname))
    {
      perror("alloc_and_map_shmem");
      exit(1);
    }

  /* not strictly necessary but validates ring parameters */
  if (0>ssys_ring_open(&ring, SSYS_RING_FLAG_READ))
    {
      perror("ssys_ring_open");
      exit(1);
    }

  int i;
  for (i=0; i<ring.num_elements; i++)
    {
      void *p=((char*)ring.p)+i*ring.element_size;
      printf("%d: 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx\n",
             i,
             *(long*)(((char*)p)),
             *(long*)(((char*)p)+sizeof(atomic_t)),
             *(long*)(((char*)p)+2*sizeof(atomic_t)),
             *(long*)(((char*)p)+3*sizeof(atomic_t)),
             *(long*)(((char*)p)+ring.header_size));
    }

  free(pathname);

  return 0;
}
