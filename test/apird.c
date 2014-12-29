#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sshmem_api.h>
#include <ringtest.h>
#include <assert.h>

/* referenced in test_init() */
char *pathname=0;
mode_t mode=SSYS_SHMEM_MODE_PIPE;
int readers=1;
int elements=0;
int element_size=0;
int header_size=0;

int
main(int argc, char **argv)
{
  test_init(argc, argv);

  int md;
  if (0<(md=ssys_shmem_open(pathname,
                            SSYS_SHMEM_FLAG_READ,
                            mode)))
    {
      perror("ssys_shmem_open");
      exit(1);
    }

  int done=0;
  while (!done)
    {
      char buf[READ_WRITE_PAYLOAD_SIZE];
      memset((void*)buf, 0x0, READ_WRITE_PAYLOAD_SIZE);
      int len;
    again:
      if (0>(len=ssys_shmem_read(md, buf, READ_WRITE_PAYLOAD_SIZE)))
        {
          if (EAGAIN!=errno)
            {
              perror("ssys_shmem_read");
              exit(1);
            }

          goto again;
        }

      assert(len==READ_WRITE_PAYLOAD_SIZE);

      long eof=*(long*)(buf+sizeof(long));
      if ('@'==eof)
        done=1;
    }

  if (0<ssys_shmem_close(md))
    perror("ssys_shmem_close");

  free(pathname);

  return 0;
}
