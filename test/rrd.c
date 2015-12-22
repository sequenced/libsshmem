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
int flags=SSYS_RING_FLAG_READ;
int readers=1;
int elements=RING_ELEMENTS;
int element_size=RING_ELEMENT_SIZE;
int header_size=RING_HEADER_SIZE;
char *payload=0;
int payload_len=0;
int printable=0;

int
main(int argc, char **argv)
{
  test_init(argc, argv);
  ssys_ring_t ring;
  init_default_ring(&ring);
  if (0>alloc_and_map_shmem(&ring, pathname, flags))
    {
      perror("alloc_and_map_shmem");
      exit(1);
    }

  if (0>ssys_ring_open(&ring, flags))
    {
      perror("ssys_ring_open");
      exit(1);
    }

  int done=0;
  while (!done)
    {
      const int len=(payload==0?READ_WRITE_PAYLOAD_SIZE:payload_len);
      char buf[len];
      memset((void*)buf, 0x0, len);
      int actual_len;
    again:
      if (0>(actual_len=ssys_ring_read(&ring, buf, len)))
        {
          if (EAGAIN!=errno)
            {
              perror("ssys_ring_read");
              exit(1);
            }

          goto again;
        }

      assert(actual_len==len);

      if (payload)
        {
          char *s=buf;
          while (0<actual_len--)
            putchar(*s++);
          printf("\n");
        }
      else
        {
          long eof=*(long*)(buf+sizeof(long));
          if ('@'==eof)
            done=1;
        }
    }

  free(pathname);
  if (payload)
    free(payload);

  return 0;
}
