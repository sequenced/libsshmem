#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sshmem_api.h>
#include <ringtest.h>

char *pathname=0;
mode_t mode=SSYS_SHMEM_MODE_PIPE;
int readers=1;
/* unused */
int elements=0;
int element_size=0;
int header_size=0;

int
main(int argc, char **argv)
{
  test_init(argc, argv);

  int md;
  if (0<(md=ssys_shmem_open(pathname,
                            SSYS_SHMEM_FLAG_WRITE|SSYS_SHMEM_FLAG_CREATE,
                            mode)))
    {
      perror("ssys_shmem_open");
      exit(1);
    }

  long i;
  for (i=0L; i<(READ_WRITE_SAMPLES+readers); i++)
    {
      char buf[READ_WRITE_PAYLOAD_SIZE];
      memset((void*)buf, 0x0, READ_WRITE_PAYLOAD_SIZE);

      if (i<READ_WRITE_SAMPLES)
        *(long*)(buf)=i;
      else
        {
          *(long*)(buf)=0xdeadbeef;
          *(long*)(buf+sizeof(long))='@'; /* EOF marker */
        }

    again:
      if (0>ssys_shmem_write(md, buf, READ_WRITE_PAYLOAD_SIZE))
        {
          if (EAGAIN!=errno)
            {
              perror("ssys_shmem_write");
              exit(1);
            }

          goto again;
        }
    }

  if (0<ssys_shmem_close(md))
    perror("ssys_shmem_close");

  free(pathname);

  return 0;
}
