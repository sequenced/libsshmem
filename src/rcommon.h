#ifndef __RCOMMON_H__
#define __RCOMMON_H__

#include <ring.h>

void init_default_ring(ssys_ring_t *pfd);
int alloc_and_map_shmem(ssys_ring_t *pfd, const char *name);

#endif /* #ifndef __RCOMMON_H__ */
