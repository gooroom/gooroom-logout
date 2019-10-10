AM_CPPFLAGS = \
	-I$(top_srcdir)	\
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\"$(datadir)\"    \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = gooroom-logout

BUILT_SOURCES = \
	logout-dialog-resources.c \
	logout-dialog-resources.h

gooroom_logout_SOURCES = \
	$(BUILT_SOURCES) \
	logout-dialog.h	\
	logout-dialog.c	\
	main.c

gooroom_logout_CFLAGS = \
	$(X11_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GLIBS_CFLAGS)	\
	$(PLATFORM_CFLAGS)

#	-Wall \
#	-Wl,-z,defs \
#	-Wl,--as-needed
#	-Wl,-Bsymbolic-functions \
#	-Werror -Wno-error=deprecated-declarations

gooroom_logout_LDADD = \
	$(X11_LIBS) \
	$(GTK_LIBS) \
	$(GLIB_LIBS)

gooroom_logout_LDFLAGS = \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

resource_files = $(shell glib-compile-resources --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/gresource.xml)
logout-dialog-resources.c: gresource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name logout_dialog $<
logout-dialog-resources.h: gresource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name logout_dialog $<

CLEANFILES = \
	logout-dialog-resources.c \
	logout-dialog-resources.h \
    $(NULL)

