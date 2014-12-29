#ifndef __RING_H__
#define __RING_H__

#include <sys/types.h>
#include <poll.h>
#define SSYS_BIT_ON(bit, val)                   \
  ((val&bit)==bit)

/* ssys_ring_open flags */
#define SSYS_RING_FLAG_WRITE  (1<<1)
#define SSYS_RING_FLAG_READ   (1<<2)
#define SSYS_RING_FLAG_CREATE (1<<3)

/* ssys_ring_t modes */
#define SSYS_RING_MODE_PIPE   (1<<1)
#define SSYS_RING_MODE_BUFFER (1<<2)

typedef struct
{
  void *p;
  unsigned int num_elements;
  unsigned int element_size;
  unsigned int header_size;
  mode_t mode;
  long read_desc;
  long write_desc;
} ssys_ring_t;

int ssys_ring_open(ssys_ring_t *pmd, int flags);
int ssys_ring_write(ssys_ring_t *pmd, const void *buf, size_t count);
int ssys_ring_read(ssys_ring_t *pmd, void *buf, size_t count);
int ssys_ring_poll_write(ssys_ring_t *pmd);
int ssys_ring_poll_read(ssys_ring_t *pmd);

#endif /* #ifndef __RING_H__ */
