/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 * save-session.c - Small program to talk to session manager.

   Copyright (C) 1998 Tom Tromey
   Copyright (C) 2008 Red Hat, Inc.
   Copyright (C) 2018-2019 Gooroom <gooroom@gooroom.kr>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#include <locale.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>


static gboolean opt_logout    = FALSE;
static gboolean opt_poweroff  = FALSE;
static gboolean opt_reboot    = FALSE;
static gboolean opt_hibernate = FALSE;
static gboolean opt_suspend   = FALSE;
static gint     opt_delay     = 0;

static GOptionEntry options[] = 
{
	{ "logout",    'l', 0, G_OPTION_ARG_NONE, &opt_logout,    NULL, NULL },
	{ "poweroff",  'p', 0, G_OPTION_ARG_NONE, &opt_poweroff,  NULL, NULL },
	{ "reboot",    'r', 0, G_OPTION_ARG_NONE, &opt_reboot,    NULL, NULL },
	{ "hibernate", 'h', 0, G_OPTION_ARG_NONE, &opt_hibernate, NULL, NULL },
	{ "suspend",   's', 0, G_OPTION_ARG_NONE, &opt_suspend,   NULL, NULL },
	{ "delay",     'd', 0, G_OPTION_ARG_INT,  &opt_delay,     NULL, NULL },
	{NULL}
};

enum {
	GSM_LOGOUT_MODE_NORMAL = 0,
	GSM_LOGOUT_MODE_NO_CONFIRMATION,
	GSM_LOGOUT_MODE_FORCE
};

typedef struct _Data {
    const char *function;
    const char *error_message;
} Data;

static GMainLoop *loop = NULL;



static void
display_error (const char *message)
{
        g_printerr ("%s\n", message);
}

static GDBusProxy *
sm_proxy_get (void)
{
	GDBusProxy *proxy;

	proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                           G_DBUS_PROXY_FLAGS_NONE,
                                           NULL, 
                                           "org.gnome.SessionManager",
                                           "/org/gnome/SessionManager",
                                           "org.gnome.SessionManager",
                                           NULL,
                                           NULL);

	return proxy;
}

static GDBusProxy *
login1_proxy_get (void)
{
	GDBusProxy *proxy;

	proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                           G_DBUS_PROXY_FLAGS_NONE,
                                           NULL, 
                                           "org.freedesktop.login1",
                                           "/org/freedesktop/login1",
                                           "org.freedesktop.login1.Manager",
                                           NULL,
                                           NULL);

    
	return proxy;
}

static gboolean
do_logout_idle (gpointer user_data)
{
	GVariant   *reply;
	GError     *error;
	GDBusProxy *proxy;

	proxy = sm_proxy_get ();
	if (proxy == NULL)
		goto done;

	error = NULL;
	reply = g_dbus_proxy_call_sync (proxy,
                                    "Logout",
                                    g_variant_new ("(u)", GSM_LOGOUT_MODE_NO_CONFIRMATION),
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1, NULL, &error);

	if (error != NULL) {
		g_warning ("Failed to call logout: %s", error->message);
		g_error_free (error);
	} else {
		g_variant_unref (reply);
	}

	g_clear_object (&proxy);

done:
	if (loop)
		g_main_loop_quit (loop);

	return FALSE;
}

static gboolean
do_endsession_idle (gpointer user_data)
{
	GVariant   *reply;
	GError     *error;
    GDBusProxy *proxy;

	Data *data = (Data *)user_data;

	if (!data || !data->function)
		goto done;

	proxy = login1_proxy_get ();
	if (proxy == NULL)
		goto done;

	error = NULL;
	reply = g_dbus_proxy_call_sync (proxy,
                                    data->function,
                                    g_variant_new ("(b)", TRUE),
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1, NULL, &error);

	if (error != NULL) {
		if (data->error_message)
			g_warning ("%s: %s", data->error_message, error->message);
		else
			g_warning ("%s", error->message);
		g_error_free (error);
	} else {
		g_variant_unref (reply);
	}

	g_clear_object (&proxy);

	g_free (data);

done:
	if (loop)
		g_main_loop_quit (loop);

	return FALSE;
}

int
main (int argc, char *argv[])
{
	GError *error;
	GOptionContext *ctx;
	int conflicting_options;

	error = NULL;
	ctx = g_option_context_new ("");
	g_option_context_add_main_entries (ctx, options, GETTEXT_PACKAGE);
	if (! g_option_context_parse (ctx, &argc, &argv, &error)) {
		g_warning ("Unable to start: %s", error->message);
		g_error_free (error);
		exit (1);
	}
	g_option_context_free (ctx);

	conflicting_options = 0;
	if (opt_logout)
		conflicting_options++;
	if (opt_poweroff)
		conflicting_options++;
	if (opt_reboot)
		conflicting_options++;
	if (opt_hibernate)
		conflicting_options++;
	if (opt_suspend)
		conflicting_options++;

	if (conflicting_options > 1) {
		display_error ("Program called with conflicting options");
		exit (1);
	}

	if (opt_logout) {
		if (opt_delay > 0) {
			g_timeout_add (opt_delay, (GSourceFunc)do_logout_idle, NULL);
			loop = g_main_loop_new (NULL, FALSE);

			g_main_loop_run (loop);
			g_main_loop_unref (loop);
		} else {
			do_logout_idle (NULL);
		}

		return 0;
	} 

	Data *data = g_new0 (Data, 1);

	if (opt_poweroff) {
		data->function = "PowerOff";
		data->error_message = "Failed to call shutdown";
	} else if (opt_reboot) {
		data->function = "Reboot";
		data->error_message = "Failed to call reboot";
	} else if (opt_hibernate) {
		data->function = "Hibernate";
		data->error_message = "Failed to call hibernate";
	} else if (opt_suspend) {
		data->function = "Suspend";
		data->error_message = "Failed to call suspend";
	} else {
		data->function = NULL;
		data->error_message = NULL;
	}

	if (opt_poweroff || opt_reboot || opt_suspend || opt_hibernate) {
		if (opt_delay > 0) {
			g_timeout_add (opt_delay, (GSourceFunc)do_endsession_idle ,data);

			loop = g_main_loop_new (NULL, FALSE);

			g_main_loop_run (loop);
			g_main_loop_unref (loop);
		} else {
			do_endsession_idle (data);
		}
	}

	return 0;
}
