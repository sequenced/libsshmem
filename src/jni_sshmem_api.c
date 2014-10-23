#include <jni.h>
#include <jni_sshmem_api.h>
#include <sshmem_api.h>

mode_t mode=0;
int elements=64;
int element_size=2048;
int header_size=64;

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
Java_mxg_io_SharedMemoryChannel_open(JNIEnv *env, jobject this,
                                     jstring pathname, jint flags, jint mode)
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
Java_mxg_io_SharedMemoryChannel_close(JNIEnv *env, jobject this, jint md)
{
  return ssys_shmem_close(md);
}

JNIEXPORT jint JNICALL
Java_mxg_io_SharedMemoryChannel_write(JNIEnv *env, jobject this, jint md,
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
Java_mxg_io_SharedMemoryChannel_read(JNIEnv *env, jobject this, jint md,
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
