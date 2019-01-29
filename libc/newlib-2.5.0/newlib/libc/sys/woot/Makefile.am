AUTOMAKE_OPTIONS = cygnus
INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)
AM_CCASFLAGS = $(INCLUDES)

GENERAL_SOURCES = syscalls.c crt0.c

if USE_LIBTOOL
ADD_OBJS = syscalls.lo
noinst_LTLIBRARIES = libwoot.la
libwoot_la_SOURCES = $(GENERAL_SOURCES)
libwoot_la_LIBADD = $(ADD_OBJS)
libwoot_la_DEPENDENCIES = $(ADD_OBJS) crt0.o
LIBC_COMPILE = $(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
ADD_OBJS = syscalls.o
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(ADD_OBJS)
lib_a_DEPENDENCIES = $(ADD_OBJS) crt0.o
lib_a_CFLAGS = $(AM_CFLAGS)
LIBC_COMPILE = $(COMPILE)
noinst_DATA =
endif # USE_LIBTOOL

objectlist.awk.in: $(noinst_LTLIBRARIES)
	-rm -f objectlist.awk.in
	for i in `ls *.lo` ; \
	do \
		echo $$i `pwd`/$$i >> objectlist.awk.in ; \
	done

all: crt0.o

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
