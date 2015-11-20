#ifndef __RCOMMON_H__
#define __RCOMMON_H__

#include <ring.h>

int ring_shmem_open(const char *name);
int ring_shmem_create(const ssys_ring_t *pmd, const char *name);
int ring_shmem_map(ssys_ring_t *pmd, const char *name, const int fd);
void init_default_ring(ssys_ring_t *pfd);
int alloc_and_map_shmem(ssys_ring_t *pfd, const char *name, int flags);

#endif /* #ifndef __RCOMMON_H__ */
