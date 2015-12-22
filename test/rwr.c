#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ring.h>
#include <rcommon.h>
#include <ringtest.h>

char *pathname=0;
mode_t mode=SSYS_RING_MODE_PIPE;
int flags=SSYS_RING_FLAG_WRITE|SSYS_RING_FLAG_CREATE;
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

  long samples=READ_WRITE_SAMPLES;
  /* single write per reader */
  if (payload)
    samples=0L;

  long i;
  for (i=0L; i<(samples+readers); i++)
    {
      const int len=(payload==0?READ_WRITE_PAYLOAD_SIZE:payload_len);
      char buf[len];
      memset((void*)buf, 0x0, len);

      if (payload)
        memcpy(buf, payload, len);
      else
        {
          if (i<samples)
            *(long*)(buf)=i;
          else
            {
              *(long*)(buf)=0xdeadbeef;
              *(long*)(buf+sizeof(long))='@'; /* EOF marker */
            }
        }

    again:
      if (0>ssys_ring_write(&ring, buf, len))
        {
          if (EAGAIN!=errno)
            {
              perror("ssys_ring_write");
              exit(1);
            }

          goto again;
        }
    }

  free(pathname);
  if (payload)
    free(payload);

  return 0;
}
