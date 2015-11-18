#include <assert.h>
#include <errno.h>
#include <string.h>
#include <atomic64_64.h>
#include <ring.h>

/*
 * Ring layout:
 *
 *  <------------------------ element size ------------------------>
 *  <------------------ header size ------------------>
 *
 * 0           7 8         15 16        23 24        63 64
 * +------------+------------+------------+------------+------------+
 * |    seq     |    lock    | dirty flag |  padding   | payload... |
 * +------------+------------+------------+------------+------------+
 * |   seq+1    |    lock    | dirty flag |  padding   | payload... |
 * +------------+------------+------------+------------+------------+
 * | ...
 *
 */

#define UNASSIGNED_SEQ -1L
#define ring_mask(pmd) (pmd->num_elements-1)
#define get_nth_element(pmd, i)                         \
  (pmd->p+(i-(i&~ring_mask(pmd)))*pmd->element_size)
#define get_seq_ptr(element) (void*)(element)
#define get_lock_ptr(element)                           \
  (atomic_t*)(((char*)element)+(sizeof(atomic_t)))
#define get_dirty_flag_ptr(element)                     \
  (atomic_t*)(((char*)element)+(2*sizeof(atomic_t)))
#define get_payload_ptr(element, header_size)   \
  (void*)(((char*)element)+header_size)

static inline int
spin_lock(atomic_t *lock)
{
  int k=200;
  while (k--)
    /* 1 = locked, 0 = unlocked */
    if (0L==atomic_cmpxchg(lock, 0L, 1L))
      return 1;

  return 0;
}

static inline void
unlock(atomic_t *lock)
{
  assert(1L==(atomic_xchg(lock, 0L)));
}

static inline int
is_valid(ssys_ring_t *pmd)
{
  if (pmd &&
      pmd->p &&
      pmd->num_elements>0 &&
      pmd->element_size>0 &&
      pmd->element_size>pmd->header_size)
    return 1;

  errno=EINVAL;
  return 0;
}

static inline long
ring_seek_tail(ssys_ring_t *pmd)
{
  void *el;
 again:
  el=get_nth_element(pmd, pmd->read_desc);
  atomic_t *seqp=get_seq_ptr(el);
  long seq=atomic_read(seqp);

  if (UNASSIGNED_SEQ == seq)
    return -1;

  if (pmd->read_desc <= seq)
    {
      pmd->read_desc=seq + 1;
      goto again;
    }

  assert(pmd->read_desc > seq);

  return seq;
}

//TODO instead of descriptor use local variable to count and compare
//TODO return value is highest sequence (head) and lowest sequence (tail)
static inline long
ring_seek_head(ssys_ring_t *pmd)
{
  void *el;
 again:
  el=get_nth_element(pmd, pmd->write_desc);
  atomic_t *seqp=get_seq_ptr(el);
  long seq=atomic_read(seqp);

  if (UNASSIGNED_SEQ == seq)
    return seq;

  if (pmd->write_desc <= seq)
    {
      pmd->write_desc=seq + 1;
      goto again;
    }

  assert(pmd->write_desc>seq);

  return seq;
}

struct head_tail
{
  long head;
  long tail;
};
typedef struct head_tail head_tail_t;

static inline void
ring_seek_head_tail(const ssys_ring_t *pmd, head_tail_t *hl)
{
  void *el;
  //  long seq;

  hl->head = 0L;
  hl->tail = UNASSIGNED_SEQ;

  while (1)
    {
      el=get_nth_element(pmd, hl->head);
      atomic_t *seqp=get_seq_ptr(el);
      hl->tail=atomic_read(seqp);

      if (UNASSIGNED_SEQ == hl->tail)
        break;

      if (hl->head <= hl->tail)
        hl->head=hl->tail + 1L;
      else
        break;
    }
}
/*
static inline int
ring_seek_hi_lo(ssys_ring_t *pmd, struct hi_lo *s)
{
  s->hi=UNASSIGNED_SEQ;
  s->lo=UNASSIGNED_SEQ;

  void *el;
  long next_seq=0L;
 again:
  el=get_nth_element(pmd, next_seq);
  atomic_t *seqp=get_seq_ptr(el);
  long seq=atomic_read(seqp);

  if (UNASSIGNED_SEQ == seq)
    goto exit;

  if (next_seq <= seq)
    {
      next_seq=seq + 1;
      goto again;
    }

  assert(next_seq > seq);

 exit:
  s->hi=next_seq;
  s->lo=seq;

  return seq;
}
*/
int
ssys_ring_open(ssys_ring_t *pmd, int flags)
{
  if (!is_valid(pmd))
    return -1;

  if (SSYS_BIT_ON(SSYS_RING_FLAG_CREATE, flags))
    {
      memset(pmd->p, 0x0, pmd->element_size*pmd->num_elements);

      int i;
      for (i=0; i<pmd->num_elements; i++)
        {
          void *el=get_nth_element(pmd, i);
          atomic_t *seq=get_seq_ptr(el);
          atomic_set(seq, UNASSIGNED_SEQ);
        }

      store_barrier();
    }

  pmd->write_desc=0;
  pmd->read_desc=0;

  return 0;
}

int
ssys_ring_write(ssys_ring_t *pmd, const void *buf, size_t count)
{
  if (!is_valid(pmd))
    return -1;

  if (count<1 ||
      count>(pmd->element_size-pmd->header_size) ||
      NULL==buf)
    {
      errno=EINVAL;
      return -1;
    }

  head_tail_t seqs;
 again:
  ring_seek_head_tail(pmd, &seqs);
  void *el=get_nth_element(pmd, seqs.head);
  
  atomic_t *lockp=get_lock_ptr(el);
  if (!spin_lock(lockp))
    {
      errno=EAGAIN;
      return -1;
    }

  atomic_t *dirtyp;
  if (SSYS_BIT_ON(SSYS_RING_MODE_PIPE, pmd->mode))
    {
      dirtyp=get_dirty_flag_ptr(el);
      if (0L!=atomic_read(dirtyp))
        {
          /* in pipe mode and element still dirty */
          unlock(lockp);
          errno=EAGAIN;
          return -1;
        }
    }

  atomic_t *seqp=get_seq_ptr(el);
  if (seqs.tail!=atomic_cmpxchg(seqp, seqs.tail, seqs.head))
    {
      /* sequence consumed, someone was quicker */
      unlock(lockp);
      goto again;
    }

  void *p=get_payload_ptr(el, pmd->header_size);
  memcpy(p, buf, count);

  if (SSYS_BIT_ON(SSYS_RING_MODE_PIPE, pmd->mode))
    atomic_set(dirtyp, 1L);

  store_barrier();
  unlock(lockp);
  return count;
}

int
ssys_ring_read(ssys_ring_t *pmd, void *buf, size_t count)
{
  if (!is_valid(pmd))
    return -1;

  if (count<1 || NULL==buf)
    {
      errno=EINVAL;
      return -1;
    }

  struct head_tail seqs;
 again:
  ring_seek_head_tail(pmd, &seqs);

  /* ring empty */
  if (0L == seqs.head)
    {
      errno=EAGAIN;
      return -1;
    }


  /* at head, nothing left to read */
  if (pmd->read_desc >= seqs.head)
    {
      errno=EAGAIN;
      return -1;
    }
  else if (pmd->read_desc < seqs.tail)
    pmd->read_desc = seqs.tail;

  if (UNASSIGNED_SEQ == seqs.tail)
    pmd->read_desc=0L;

  void *el=get_nth_element(pmd, pmd->read_desc);

  atomic_t *lockp=get_lock_ptr(el);
  if (!spin_lock(lockp))
    {
      errno=EAGAIN;
      return -1;
    }
    
  atomic_t *seqp=get_seq_ptr(el);
  long seq=atomic_read(seqp);
    
  if (pmd->read_desc != seq)
    {
      unlock(lockp);
      goto again;
    }

  atomic_t *dirtyp;
  if (SSYS_BIT_ON(SSYS_RING_MODE_PIPE, pmd->mode))
    {
      dirtyp=get_dirty_flag_ptr(el);
      if (0L==atomic_read(dirtyp))
        {
          /* someone already read element */
          unlock(lockp);
          pmd->read_desc++;
          goto again;
        }
    }

  void *p=get_payload_ptr(el, pmd->header_size);
  unsigned int payload_size=pmd->element_size-pmd->header_size;
  int len=count<payload_size?count:payload_size;
  memcpy(buf, p, len);
  
  if (SSYS_BIT_ON(SSYS_RING_MODE_PIPE, pmd->mode))
    atomic_set(dirtyp, 0L);

  unlock(lockp);
  pmd->read_desc++;
  return len;
}

int
ssys_ring_poll_write(ssys_ring_t *pmd)
{
  if (!is_valid(pmd))
    return -1;

  if (SSYS_BIT_ON(SSYS_RING_MODE_BUFFER, pmd->mode))
    /* can always write when in buffer mode */
    return 1;

  void *el=get_nth_element(pmd, pmd->write_desc);
  atomic_t *dirtyp=get_dirty_flag_ptr(el);
  if (0L!=atomic_read(dirtyp))
    /* cannot write: page dirty */
    return 0;

  /* can write: page clean */
  return 1;
}

int
ssys_ring_poll_read(ssys_ring_t *pmd)
{
  if (!is_valid(pmd))
    return -1;

  // TODO assumption does not hold
  if (SSYS_BIT_ON(SSYS_RING_MODE_BUFFER, pmd->mode))
    /* can always read when in buffer mode */
    return 1;

  void *el=get_nth_element(pmd, pmd->read_desc);
  atomic_t *dirtyp=get_dirty_flag_ptr(el);
  if (0L==atomic_read(dirtyp))
    /* already read: page clean */
    return 0;

  /* can read: page dirty */
  return 1;
}
