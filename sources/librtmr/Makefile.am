## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

lib_LTLIBRARIES = librtmr.la
librtmr_la_LDFLAGS = -version-info 1:0:0
librtmr_la_LIBADD        = -lm

librtmr_la_SOURCES = src/client-lib/client.c \
			src/common/entities.c \
			src/common/errors.c \
			src/common/utilities.c \
			src/common/netcommon.c

library_includedir=$(includedir)/rtmr
library_include_HEADERS=src/client-lib/client.h
			src/common/entities.h \
			src/common/errors.h \
			src/common/utilities.h \
			src/common/netcommon.h \
			src/common/constants.h
