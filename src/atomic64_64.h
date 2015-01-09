#ifndef __ATOMIC64_64_H__
#define __ATOMIC64_64_H__

typedef struct
{
  long val;
} atomic_t;

static inline void
atomic_set(atomic_t *v, long k)
{
  v->val=k;
}

static inline long
atomic_read(const atomic_t *v)
{
  return (*(volatile long*)&(v)->val);
}

static inline long
atomic_cmpxchg(atomic_t *v, long old, long new)
{
  long rv;
  volatile atomic_t *_v=(volatile atomic_t*)v;
  asm volatile("lock cmpxchgq %2, %1\n\t"
               : "=a" (rv), "+m" (_v->val)
               : "r" (new), "0" (old)
               : "memory");
  return rv;
}

static inline long
atomic_xchg(atomic_t *v, long new)
{
  long rv;
  volatile atomic_t* _v=v;
  asm volatile("xchgq %0, %1\n\t"
               : "=r" (rv), "+m" (_v->val)
               : "0" (new) : "memory");
  return rv;
}

static inline void
store_barrier()
{
  asm volatile("sfence\n\t");
}

#endif /* #ifndef __ATOMIC64_64_H__ */
