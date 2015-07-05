#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sshmem_api.h>

extern char *pathname;
extern mode_t mode;
extern int flags;
extern int readers;
extern int elements;
extern int element_size;
extern int header_size;
extern char *payload;
extern int payload_len;

static void
usage()
{
  fprintf(stderr, "-f <pathname> [-b | -n <number of readers> |\n");
  fprintf(stderr, "-e <number of elements> | -z <element size> |\n");
  fprintf(stderr, "-h <header size> | -s <payload string> | -c]\n");
  fprintf(stderr, "where: -b = buffer mode, -c = create shmem file flag\n");
}

void
test_init(int argc, char **argv)
{
  int opt;

  while (-1!=(opt=getopt(argc, argv, "cbf:n:e:z:h:s:")))
    {
      switch (opt)
        {
        case 'e':
          elements=atoi(optarg);
          break;
        case 'z':
          element_size=atoi(optarg);
          break;
        case 'h':
          header_size=atoi(optarg);
          break;
        case 'c':
          flags|=SSYS_SHMEM_FLAG_CREATE;
          break;
        case 'b':
          mode=SSYS_SHMEM_MODE_BUFFER;
          break;
        case 'n':
          readers=atoi(optarg);
          break;
        case 's':
          {
            payload_len=strlen(optarg);
            payload=malloc(payload_len);
            memcpy(payload, optarg, payload_len);
            break;
          }
        case 'f':
          {
            int len=strlen(optarg);
            pathname=malloc(len+1);
            strncpy(pathname, optarg, len+1);
            break;
          }
        default:
          usage();
          exit(1);
        }
    }

  /* mandatory argument */
  if (0==pathname)
    {
      usage();
      exit(1);
    }

  if (element_size<payload_len)
    {
      fprintf(stderr, "payload too long: %d char(s) maximum\n", element_size);
      exit(2);
    }
}
