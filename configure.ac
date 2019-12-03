dnl ***************************
dnl *** Initialize autoconf ***
dnl ***************************
AC_INIT([gooroom-logout], [0.1.0])
AC_PREREQ([2.60])

dnl ***************************
dnl *** Initialize automake ***
dnl ***************************
AM_INIT_AUTOMAKE([1.8 dist-xz no-dist-gzip foreign])
AC_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE()
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl *******************************
dnl *** Check for UNIX variants ***
dnl *******************************
AC_AIX()
AC_ISC_POSIX()
AC_MINIX()

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************
AC_PROG_CC()
AM_PROG_CC_C_O()
AC_PROG_LD()
AC_PROG_INSTALL()
IT_PROG_INTLTOOL([0.35.0])

dnl **************************
dnl *** Initialize libtool ***
dnl **************************
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl **********************************
dnl *** Check for standard headers ***
dnl **********************************
AC_HEADER_STDC()
AC_CHECK_HEADERS([stdlib.h string.h errno.h unistd.h])

#AC_CONFIG_MACRO_DIR([m4])

dnl ******************************
dnl *** Check for i18n support ***
dnl ******************************
GETTEXT_PACKAGE=gooroom-logout
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext Package])
AM_GLIB_GNU_GETTEXT

dnl ***********************************
dnl *** Check for required packages ***
dnl ***********************************
PKG_CHECK_MODULES(GTK, gtk+-3.0 >= 3.20.0)
PKG_CHECK_MODULES(GIO, gio-2.0 >= 2.54.1)
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.44.0)
PKG_CHECK_MODULES(X11, x11 >= 1.6.7)

dnl *********************************
dnl *** Substitute platform flags ***
dnl *********************************
AC_MSG_CHECKING([PLATFORM_CPPFLAGS])
AC_MSG_RESULT([$PLATFORM_CPPFLAGS])
AC_SUBST([PLATFORM_CPPFLAGS])
AC_MSG_CHECKING([PLATFORM_CFLAGS])
AC_MSG_RESULT([$PLATFORM_CFLAGS])
AC_SUBST([PLATFORM_CFLAGS])
AC_MSG_CHECKING([PLATFORM_LDFLAGS])
AC_MSG_RESULT([$PLATFORM_LDFLAGS])
AC_SUBST([PLATFORM_LDFLAGS])

AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
])
