#ifndef __RINGTEST_H__
#define __RINGTEST_H__

#define RING_ELEMENTS 128
#define RING_HEADER_SIZE 64
#define RING_ELEMENT_SIZE (RING_HEADER_SIZE+1024)
#define READ_WRITE_SAMPLES 1000000
#define READ_WRITE_PAYLOAD_SIZE 1024

void test_init(int argc, char **argv);

#endif /* #ifndef __RINGTEST_H__ */
