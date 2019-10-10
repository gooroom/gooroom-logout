/*
 * Copyright (c) 2015-2019 Gooroom <gooroom@gooroom.kr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "logout-dialog.h"


static gboolean
on_logout_dialog_show_idle (gpointer data)
{
	logout_dialog_show ();

	gtk_main_quit ();

	return FALSE;
}

int
main (int argc, char **argv)
{
	GtkCssProvider *provider;

	/* Initialize i18n */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);

	provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_resource (provider, "/kr/gooroom/logout/theme.css");
	gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
			GTK_STYLE_PROVIDER (provider),
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref (provider);

	g_timeout_add (10, (GSourceFunc)on_logout_dialog_show_idle, NULL);

	gtk_main ();

	return 0;
}
