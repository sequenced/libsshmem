#ifndef __SHMEM_API_H__
#define __SHMEM_API_H__

#include <poll.h>
#include <sys/types.h>

/* ssys_shmem_open flags */
#define SSYS_SHMEM_FLAG_WRITE  (1<<1)
#define SSYS_SHMEM_FLAG_READ   (1<<2)
#define SSYS_SHMEM_FLAG_CREATE (1<<3)

/* ssys_shmem_open modes */
#define SSYS_SHMEM_MODE_PIPE   (1<<1)
#define SSYS_SHMEM_MODE_BUFFER (1<<2)

int ssys_shmem_open(const char *pathname, int flags, mode_t mode);
int ssys_shmem_write(int md, const void *buf, size_t count);
int ssys_shmem_read(int md, void *buf, size_t count);
int ssys_shmem_close(int md);
int ssys_shmem_poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif /* #ifndef __SHMEM_API_H__ */
