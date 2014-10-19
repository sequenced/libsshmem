TESTS = $(check_PROGRAMS)
check_PROGRAMS = rrd rwr
rrd_SOURCES = rrd.c ringtest.c ../src/rcommon.c
rrd_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
rrd_LDFLAGS = -L../src -lsshmem -lrt
rwr_SOURCES = rwr.c ringtest.c ../src/rcommon.c
rwr_CPPFLAGS = $(rrd_CPPFLAGS)
rwr_LDFLAGS = $(rrd_LDFLAGS)
