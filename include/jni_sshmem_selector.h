/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_ssys_io_Selector2 */

#ifndef _Included_com_ssys_io_Selector2
#define _Included_com_ssys_io_Selector2
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_ssys_io_Selector2
 * Method:    getMaximumDescriptors
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_ssys_io_Selector2_getMaximumDescriptors
  (JNIEnv *, jobject);

/*
 * Class:     com_ssys_io_Selector2
 * Method:    poll
 * Signature: ([I[I[IIJ)I
 */
JNIEXPORT jint JNICALL Java_com_ssys_io_Selector2_poll
  (JNIEnv *, jobject, jintArray, jintArray, jintArray, jint, jlong);

#ifdef __cplusplus
}
#endif
#endif
