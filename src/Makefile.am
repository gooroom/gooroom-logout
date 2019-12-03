AM_CPPFLAGS = \
	-I$(top_srcdir)	\
	-DGNOMELOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\"$(datadir)\"    \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = gooroom-logout gooroom-logout-command

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
	$(GLIB_CFLAGS)	\
	$(PLATFORM_CFLAGS)

gooroom_logout_LDADD = \
	$(X11_LIBS) \
	$(GTK_LIBS) \
	$(GLIB_LIBS)

gooroom_logout_LDFLAGS = \
	-no-undefined \
	$(PLATFORM_LDFLAGS)



gooroom_logout_command_SOURCES = \
	gooroom-logout-command.c

gooroom_logout_command_CFLAGS = \
	$(GIO_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(PLATFORM_CFLAGS)

gooroom_logout_command_LDADD = \
	$(GIO_LIBS)	\
	$(GLIB_LIBS)

gooroom_logout_command_LDFLAGS = \
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

