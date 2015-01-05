#include <poll.h> /* for struct pollfd */
#include <jni.h>
#include <jni_sshmem_api.h>
#include <sshmem_api.h>

mode_t mode=0;
int elements=SSYS_SHMEM_ELEMENTS;
int element_size=SSYS_SHMEM_ELEMENT_SIZE;
int header_size=SSYS_SHMEM_HEADER_SIZE;
struct pollfd fds[SSYS_SHMEM_DESC_MAX];

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
Java_com_ssys_io_SharedMemoryChannel_open(JNIEnv *env, jobject this,
                                          jstring pathname, jint flags,
                                          jint mode)
{
  const jchar *s=(*env)->GetStringCritical(env, pathname, NULL);
  if (NULL==s)
    {
      throw_new_exception(env, "open: GetStringCritical returned null");
      return -1;
    }

  int rv=ssys_shmem_open((const char*)s, flags, mode);
  (*env)->ReleaseStringCritical(env, pathname, s);
  return rv;
}

JNIEXPORT jint JNICALL
Java_com_ssys_io_SharedMemoryChannel_close(JNIEnv *env, jobject this, jint md)
{
  return ssys_shmem_close(md);
}

JNIEXPORT jint JNICALL
Java_com_ssys_io_SharedMemoryChannel_write(JNIEnv *env, jobject this, jint md,
                                           jbyteArray buf, jint len, jint off)
{
  jbyte *b=(*env)->GetByteArrayElements(env, buf, NULL);
  if (NULL==b)
    {
      throw_new_exception(env, "write: GetByteArrayElements returned null");
      return -1;
    }

  int rv=ssys_shmem_write(md, (char*)(b+off), len);
  (*env)->ReleaseByteArrayElements(env, buf, b, JNI_ABORT); /* don't copy */

  return rv;
}

JNIEXPORT jint JNICALL
Java_com_ssys_io_SharedMemoryChannel_read(JNIEnv *env, jobject this, jint md,
                                          jbyteArray buf, jint len, jint off)
{
  jbyte *b=(*env)->GetByteArrayElements(env, buf, NULL);
  if (NULL==b)
    {
      throw_new_exception(env, "read: GetByteArrayElements returned null");
      return -1;
    }

  int rv=ssys_shmem_read(md, (char*)(b+off), len);
  (*env)->ReleaseByteArrayElements(env, buf, b, 0); /* always copy back */

  return rv;
}

JNIEXPORT jint JNICALL
Java_com_ssys_io_SharedMemoryChannel_poll(JNIEnv *env, jobject this,
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

  //TODO
  return -1;
}
