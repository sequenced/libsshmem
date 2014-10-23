#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sshmem_api.h>

extern char *pathname;
extern mode_t mode;
extern int readers;
extern int elements;
extern int element_size;
extern int header_size;

void
test_init(int argc, char **argv)
{
  int opt;

  while (-1!=(opt=getopt(argc, argv, "bf:n:e:z:h:")))
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
        case 'b':
          mode=SSYS_SHMEM_MODE_BUFFER;
          break;
        case 'n':
          readers=atoi(optarg);
          break;
        case 'f':
          {
            int len=strlen(optarg);
            pathname=malloc(len+1);
            strncpy(pathname, optarg, len+1);
            break;
          }
        }
    }

  if (0==pathname)
    {
      fprintf(stderr, "-f <pathname> [-b | -n <number of readers> |\n");
      fprintf(stderr, "-e <number of elements> | -z <element size> |\n");
      fprintf(stderr, "-h <header size>]\n");
      exit(1);
    }

}
