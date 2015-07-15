lib_LTLIBRARIES = libsshmem.la
libsshmem_la_SOURCES = sshmem_api.c ring.c jni_sshmem_api.c rcommon.c
libsshmem_la_CPPFLAGS = -I$(top_srcdir)/include -I/usr/local/jdk/include \
	-I/usr/local/jdk/include/linux \
	-I/usr/lib/jvm/java-7-openjdk-amd64/include
libsshmem_la_LDFLAGS = -lrt
