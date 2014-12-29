#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <ring.h>
#include <rcommon.h>
#include <ringtest.h>

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

  if (0>ssys_ring_open(&ring, SSYS_RING_FLAG_READ))
    {
      perror("ssys_ring_open");
      exit(1);
    }

  int done=0;
  while (!done)
    {
      char buf[READ_WRITE_PAYLOAD_SIZE];
      memset((void*)buf, 0x0, READ_WRITE_PAYLOAD_SIZE);
      int len;
    again:
      if (0>(len=ssys_ring_read(&ring, buf, READ_WRITE_PAYLOAD_SIZE)))
        {
          if (EAGAIN!=errno)
            {
              perror("ssys_ring_read");
              exit(1);
            }

          goto again;
        }

      assert(len==READ_WRITE_PAYLOAD_SIZE);

      long eof=*(long*)(buf+sizeof(long));
      if ('@'==eof)
        done=1;
    }

  free(pathname);

  return 0;
}
