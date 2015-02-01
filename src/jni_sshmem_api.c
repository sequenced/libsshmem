#include <poll.h> /* for struct pollfd */
#include <jni.h>
#include <jni_sshmem_channel.h>
#include <jni_sshmem_selector.h>
#include <sshmem_api.h>

mode_t mode=0;
int elements=SSYS_SHMEM_ELEMENTS;
int element_size=SSYS_SHMEM_ELEMENT_SIZE;
int header_size=SSYS_SHMEM_HEADER_SIZE;
struct pollfd fds[SSYS_SHMEM_DESC_MAX];

static void copy_pollfd_to_md(jint*, int);
static void copy_md_to_pollfd(jint*, jint*, int);

static inline void
throw_new_exception(JNIEnv *env, const char *msg)
{
  jclass jc=(*env)->FindClass(env, "java/io/IOException");
  if (NULL==jc)
    {
      fprintf(stderr, "throw_new_exception: cannot find java.io.IOException\n");
      return;
    }

  if (0>(*env)->ThrowNew(env, jc, msg))
    fprintf(stderr, "throw_new_exception: ThrowNew failed\n");
}

JNIEXPORT jint JNICALL
Java_com_ssys_io_SharedMemoryChannel_implOpen(JNIEnv *env, jobject this,
                                              jstring pathname, jint flags,
                                              jint mode)
{
  const jchar *s=(*env)->GetStringCritical(env, pathname, NULL);
  if (NULL==s)
    {
      throw_new_exception(env, "implOpen: GetStringCritical returned null");
      return -1;
    }

  int rv=ssys_shmem_open((const char*)s, flags, mode);
  (*env)->ReleaseStringCritical(env, pathname, s);
  return rv;
}

JNIEXPORT jint JNICALL
Java_com_ssys_io_SharedMemoryChannel_implClose(JNIEnv *env, jobject this,
                                               jint md)
{
  return ssys_shmem_close(md);
}

JNIEXPORT jint JNICALL
Java_com_ssys_io_SharedMemoryChannel_implWrite(JNIEnv *env, jobject this, jint md,
                                           jbyteArray buf, jint len, jint off)
{
  jbyte *b=(*env)->GetByteArrayElements(env, buf, NULL);
  if (NULL==b)
    {
      throw_new_exception(env, "implWrite: GetByteArrayElements returned null");
      return -1;
    }

  int rv=ssys_shmem_write(md, (char*)(b+off), len);
  (*env)->ReleaseByteArrayElements(env, buf, b, JNI_ABORT); /* don't copy */

  return rv;
}

JNIEXPORT jint JNICALL
Java_com_ssys_io_SharedMemoryChannel_implRead(JNIEnv *env, jobject this, jint md,
                                          jbyteArray buf, jint len, jint off)
{
  jbyte *b=(*env)->GetByteArrayElements(env, buf, NULL);
  if (NULL==b)
    {
      throw_new_exception(env, "implRead: GetByteArrayElements returned null");
      return -1;
    }

  int rv=ssys_shmem_read(md, (char*)(b+off), len);
  (*env)->ReleaseByteArrayElements(env, buf, b, 0); /* always copy back */

  return rv;
}

JNIEXPORT jint
JNICALL Java_com_ssys_io_SharedMemorySelector_getMaximumDescriptors(JNIEnv *env,
                                                                    jobject this)
{
  return (SSYS_SHMEM_DESC_MAX);
}

JNIEXPORT jint JNICALL
Java_com_ssys_io_SharedMemorySelector_poll(JNIEnv *env, jobject this,
                                           jintArray md,
                                           jintArray interestedOps,
                                           jintArray selectedOps, jint len,
                                           jlong timeout)
{
  int md_len=(*env)->GetArrayLength(env, md);
  if (SSYS_SHMEM_DESC_MAX<len
      || SSYS_SHMEM_DESC_MAX<md_len
      || md_len!=(*env)->GetArrayLength(env, interestedOps)
      || md_len!=(*env)->GetArrayLength(env, selectedOps))
    {
      throw_new_exception(env, "poll: bad array length");
      return -1;
    }

  jint *pmd=(*env)->GetIntArrayElements(env, md, NULL);
  if (NULL==pmd)
    {
      throw_new_exception(env, "poll: GetByteArrayElements returned null: md");
      return -1;
    }

  jint *pInterestedOps=(*env)->GetIntArrayElements(env, interestedOps, NULL);
  if (NULL==pInterestedOps)
    {
      throw_new_exception(env, "poll: GetByteArrayElements returned null: interestedOps");
      return -1;
    }

  jint *pSelectedOps=(*env)->GetIntArrayElements(env, selectedOps, NULL);
  if (NULL==pSelectedOps)
    {
      throw_new_exception(env, "poll: GetByteArrayElements returned null: selectedOps");
      return -1;
    }

  copy_md_to_pollfd(pmd, pInterestedOps, len);
  int num=ssys_shmem_poll(fds, len, timeout);
  if (num)
    copy_pollfd_to_md(pSelectedOps, len);

  (*env)->ReleaseIntArrayElements(env, selectedOps, pSelectedOps,
                                  num?0:JNI_ABORT);
  (*env)->ReleaseIntArrayElements(env, interestedOps, pInterestedOps,
                                  JNI_ABORT);
  (*env)->ReleaseIntArrayElements(env, md, pmd, JNI_ABORT);

  return num;
}

inline void
copy_pollfd_to_md(jint *ops, int len)
{
  int i;
  for (i=0; i<len; i++)
    {
      *ops=fds[i].revents;
      ops++;
    }
}

inline void
copy_md_to_pollfd(jint *pmd, jint *ops, int len)
{
  int i;
  for (i=0; i<len; i++)
    {
      fds[i].fd=*pmd;
      fds[i].events=*ops;
      pmd++;
      ops++;
    }
}
