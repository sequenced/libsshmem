#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([libsshmem], [0.2], [bugs@sequencedsystems.com])
AM_INIT_AUTOMAKE([-Wall gnu])
LT_PREREQ([2.2])
LT_INIT([dlopen])
AC_CONFIG_SRCDIR([src/jni_sshmem_api.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h string.h unistd.h poll.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([ftruncate memset])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 include/Makefile
		 test/Makefile])
AC_OUTPUT
