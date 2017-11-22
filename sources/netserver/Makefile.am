## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
# AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CFLAGS = -I/usr/include/postgresql -I/usr/include/pgsql

sysconf_DATA = src/netserver/conf/netserver.conf

bin_PROGRAMS = netserver

netserver_LDADD = -lpq -lm

netserver_SOURCES = src/netserver/main.c \
			src/netserver/serveur.c \
			src/common/entities.c \
			src/common/errors.c \
			src/common/utilities.c \
			src/common/netcommon.c \
			src/common/postgres/postgres-client-library.c

