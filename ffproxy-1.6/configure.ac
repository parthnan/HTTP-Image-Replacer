# Process this file with autoconf to produce a configure script.
#
# $Id: configure.ac,v 1.4 2005/01/05 15:11:06 niklas Exp niklas $
#

AC_PREREQ(2.59)
AC_INIT([ffproxy],[1.6],[niklas@noxa.de])
AC_REVISION($Revision: 1.4 $)
AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_HEADERS([configure.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(socket, connect)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([dup2 gethostbyname inet_ntoa memset regcomp select socket strchr strncasecmp daemon])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
