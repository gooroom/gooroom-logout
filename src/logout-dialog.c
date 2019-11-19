/* 
 * Copyright (C) 2015-2019 Gooroom <gooroom@gooroom.kr>
 * Copyright (c) 2004-2006 Benedikt Meurer <benny@xfce.org>
 * Copyright (c) 2011      Nick Schermer <nick@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 * Parts of this file where taken from gnome-session/logout.c, which
 * was written by Owen Taylor <otaylor@redhat.com>.
 */

#include "logout-dialog.h"

#include <gtk/gtk.h>

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <cairo-xlib.h>

#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n.h>

typedef struct _LogoutDialogPrivate LogoutDialogPrivate;

struct _LogoutDialog {
	GtkDialog parent;

	LogoutDialogPrivate *priv;
};

struct _LogoutDialogClass {
	GtkDialogClass parent_class;
};

struct _LogoutDialogPrivate {
	GtkWidget *img_logo;
	GtkWidget *box_button;
};

enum {
	SYSTEM_LOGOUT = 0,
	SYSTEM_HIBERNATE,
	SYSTEM_SUSPEND,
	SYSTEM_RESTART,
	SYSTEM_SHUTDOWN,
	SYSTEM_CANCEL
};

enum {
	LOGOUT_NORMAL = 0,
	LOGOUT_NO_CONFIRMATION,
	LOGOUT_FORCE
};

static const struct {
	gint id;
	const char *label;
	const char *icon_name;
	const char *desc;
} DATA[] = {
	{ SYSTEM_LOGOUT,
      N_("_Log Out"),
      "system-log-out-symbolic",
      N_("Close all programs and log out.")
	},

	{ SYSTEM_HIBERNATE,
      N_("_Hibernate"),
      "system-hibernate-symbolic",
      N_("Save user sessions in memory and put the computer into sleep state.")
	},

	{ SYSTEM_SUSPEND,
      N_("Sus_pend"),
      "system-suspend-symbolic",
      N_("Save user sessions in hard disk and turn off the computer.")
	},

	{ SYSTEM_RESTART,
      N_("_Restart"),
      "system-restart-symbolic",
      N_("Shut down and automatically restart the computer?")
	},

	{ SYSTEM_SHUTDOWN,
      N_("Shut _Down"),
      "system-shutdown-symbolic",
      N_("Close all programs and turn off the computer.")
	},

	{ SYSTEM_CANCEL,
      N_("_Cancel"),
      "application-exit-symbolic",
      N_("Close this dialog.")
	},

	{ -1, NULL, NULL, NULL }
};



G_DEFINE_TYPE_WITH_PRIVATE (LogoutDialog, logout_dialog, GTK_TYPE_DIALOG)


/* Copied from xfce4-session/xfce4-session/xfsm-fadeout.c:
 * xfsm_x11_fadeout_new_window () */
static Window
x11_fadeout_new_window (GdkDisplay *display, GdkScreen *screen)
{
	XSetWindowAttributes  attr;
	Display              *xdisplay;
	Window                xwindow;
	GdkWindow            *root;
	GdkCursor            *cursor;
	cairo_t              *cr;
	gint                  width;
	gint                  height;
	GdkPixbuf            *root_pixbuf;
	cairo_surface_t      *surface;
	gulong                mask = 0;
	gulong                opacity;
	gboolean              composited;

	xdisplay = gdk_x11_display_get_xdisplay (display);
	root = gdk_screen_get_root_window (screen);

	width = gdk_window_get_width (root);
	height = gdk_window_get_height (root);

	composited = gdk_screen_is_composited (screen)
		&& gdk_screen_get_rgba_visual (screen) != NULL;

	cursor = gdk_cursor_new_for_display (display, GDK_WATCH);

	if (!composited) {
		/* create a copy of root window before showing the fadeout */
		root_pixbuf = gdk_pixbuf_get_from_window (root, 0, 0, width, height);
	}

	attr.cursor = gdk_x11_cursor_get_xcursor (cursor);
	mask |= CWCursor;

	attr.override_redirect = TRUE;
	mask |= CWOverrideRedirect;

	attr.background_pixel = BlackPixel (xdisplay, gdk_x11_screen_get_screen_number (screen));
	mask |= CWBackPixel;

	xwindow = XCreateWindow (xdisplay, gdk_x11_window_get_xid (root),
			0, 0, width, height, 0, CopyFromParent,
			InputOutput, CopyFromParent, mask, &attr);

	g_object_unref (cursor);

	if (composited) {
		/* apply transparency before map */
		opacity = 0.2 * 0xffffffff;
		XChangeProperty (xdisplay, xwindow,
				gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_WINDOW_OPACITY"),
				XA_CARDINAL, 32, PropModeReplace, (guchar *)&opacity, 1);
	}

	XMapWindow (xdisplay, xwindow);

	if (!composited) {
		/* create background for window */
		surface = cairo_xlib_surface_create (xdisplay, xwindow,
				gdk_x11_visual_get_xvisual (gdk_screen_get_system_visual (screen)),
				0, 0);
		cairo_xlib_surface_set_size (surface, width, height);
		cr = cairo_create (surface);

		/* draw the copy of the root window */
		gdk_cairo_set_source_pixbuf (cr, root_pixbuf, 0, 0);
		cairo_paint (cr);
		g_object_unref (root_pixbuf);

		/* draw black transparent layer */
		cairo_set_source_rgba (cr, 0, 0, 0, 0.5);
		cairo_paint (cr);
		cairo_destroy (cr);
		cairo_surface_destroy (surface);
	}

	return xwindow;
}

static GList *
fadeout_window_show (GdkDisplay *display)
{
	GdkScreen *screen;
	Window     xwindow;
	GList     *xwindows = NULL;

	screen = gdk_display_get_default_screen (display);
	xwindow = x11_fadeout_new_window (display, screen);
	xwindows = g_list_prepend (xwindows, GINT_TO_POINTER (xwindow));

	return xwindows;
}

static void
fadeout_window_hide (GList *xwindows, GdkDisplay *display)
{
	GList   *l = NULL;
	Display *xdisplay;

	xdisplay = gdk_x11_display_get_xdisplay (display);

	for (l = xwindows; l; l = l->next) {
		Window xwindow = GPOINTER_TO_INT (l->data);
		XDestroyWindow (xdisplay, xwindow);
	}
}

static GDBusProxy *
login1_proxy_get (void)
{
	GDBusProxy *proxy = NULL;

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
is_function_available (const char *function)
{
	if (!g_str_equal (function, "CanReboot") &&
	    !g_str_equal (function, "CanPowerOff") &&
	    !g_str_equal (function, "CanSuspend") &&
        !g_str_equal (function, "CanHibernate")) {
		return FALSE;
	}

	gboolean result = FALSE;
	GDBusProxy *proxy = login1_proxy_get ();
	if (proxy) {
		GVariant *r = NULL;
		gchar *string = NULL;

		r = g_dbus_proxy_call_sync (proxy,
				function,
				NULL,
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL,
				NULL);

		if (r) {
			if (g_variant_is_of_type (r, G_VARIANT_TYPE ("(s)"))) {
				g_variant_get (r, "(&s)", &string);
				result = g_str_equal (string, "yes");
			}

			g_variant_unref (r);
		}
	}

	return result;
}

static gboolean
end_session (const gchar *function)
{
	if (!g_str_equal (function, "reboot") &&
        !g_str_equal (function, "suspend") &&
        !g_str_equal (function, "poweroff") &&
        !g_str_equal (function, "hibernate")) {
		return FALSE;
	}

	gboolean ret = TRUE;
	GError *error = NULL;
	gchar *pkexec = g_find_program_in_path ("pkexec");
    if (pkexec) {
		gchar *cmdline = g_strdup_printf ("%s /bin/systemctl %s", pkexec, function);
		g_spawn_command_line_async (cmdline, &error);
		if (error) {
			g_warning ("Failed to execute function: %s", error->message);
			g_error_free (error);
			ret = FALSE;
		}
		g_free (cmdline);
    }
    g_free (pkexec);

	return ret;

#if 0
	if (!g_str_equal (function, "PowerOff") &&
        !g_str_equal (function, "Reboot") &&
        !g_str_equal (function, "Hibernate") &&
        !g_str_equal (function, "Suspend")) {
		return FALSE;
	}

	GDBusProxy *proxy = login1_proxy_get ();
	if (proxy) {
		GError *error = NULL;
		GVariant *parameters = NULL;

		parameters = g_variant_new ("(b)", FALSE);

		g_dbus_proxy_call_sync (proxy,
				function,
				parameters,
				G_DBUS_CALL_FLAGS_NONE,
				-1,
				NULL,
				&error);

		if (error) {
			g_warning ("Failed to call %s: %s", function, error->message);
			g_error_free (error);
		}

		if (parameters)
			g_variant_unref (parameters);

		g_clear_object (&proxy);
	}

	return TRUE;
#endif
}

static gboolean
do_logout_idle (gpointer data)
{
	g_spawn_command_line_async ("/usr/bin/gnome-session-quit --force", NULL);

	return FALSE;
}

static void
on_system_command_button_clicked (GtkWidget *button, gpointer data)
{
	LogoutDialog *dialog = LOGOUT_DIALOG (data);
	LogoutDialogPrivate *priv = dialog->priv;

	gint id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "id"));

	switch (id)
	{
		case SYSTEM_LOGOUT:
		{
			do_logout_idle (NULL);
			gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
			break;
		}

		case SYSTEM_SUSPEND:
		{
			if (end_session ("suspend"))
				gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
			break;
		}

		case SYSTEM_HIBERNATE:
		{
			if (end_session ("hibernate"))
				gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
			break;
		}

		case SYSTEM_RESTART:
		{
			if (end_session ("reboot"))
				gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
			break;
		}

		case SYSTEM_SHUTDOWN:
		{
			if (end_session ("poweroff"))
				gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
			break;
		}

		case SYSTEM_CANCEL:
		{
			gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
			break;
		}

		default:
			return;
	}
}

static void
logout_dialog_init (LogoutDialog *dialog)
{
	GtkWidget *hbox, *content_area;
	LogoutDialogPrivate *priv;

	priv = dialog->priv = logout_dialog_get_instance_private (dialog);

	gtk_widget_init_template (GTK_WIDGET (dialog));

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_container_set_border_width (GTK_CONTAINER (content_area), 0);
	gtk_box_set_spacing (GTK_BOX (content_area), 0);
	gtk_widget_hide (gtk_dialog_get_action_area (GTK_DIALOG (dialog)));
G_GNUC_END_IGNORE_DEPRECATIONS

	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_resource_at_scale ("/kr/gooroom/logout/logo.svg", 160, -1, TRUE, NULL);
	if (pixbuf) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (priv->img_logo), pixbuf);
		g_object_unref (pixbuf);
	}

	gint i;
	for (i = 0; DATA[i].id != -1; i++ ) {
		GtkWidget *button = NULL;
		if (DATA[i].id == SYSTEM_RESTART) {
			if (is_function_available ("CanReboot"))
				button = gtk_button_new ();
		} else if (DATA[i].id == SYSTEM_SHUTDOWN) {
			if (is_function_available ("CanPowerOff"))
				button = gtk_button_new ();
		} else if (DATA[i].id == SYSTEM_SUSPEND) {
			if (is_function_available ("CanSuspend"))
				button = gtk_button_new ();
		} else if (DATA[i].id == SYSTEM_HIBERNATE) {
			if (is_function_available ("CanHibernate"))
				button = gtk_button_new ();
		} else {
			button = gtk_button_new ();
		}
		if (!button) continue;

		gtk_widget_set_can_focus (button, FALSE);
		gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
		if (DATA[i].id == SYSTEM_CANCEL) {
			gtk_widget_set_name (button, "logout-dialog-button-last");
		} else {
			gtk_widget_set_name (button, "logout-dialog-button");
		}
		g_object_set_data (G_OBJECT (button), "id", GINT_TO_POINTER (DATA[i].id));
		gtk_box_pack_start (GTK_BOX (priv->box_button), button, TRUE, FALSE, 0);

		GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
		gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
		gtk_container_add (GTK_CONTAINER (button), hbox);

		GtkWidget *icon = gtk_image_new_from_icon_name (DATA[i].icon_name, GTK_ICON_SIZE_BUTTON);
		gtk_image_set_pixel_size (GTK_IMAGE (icon), 32);
		gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

		GtkWidget *label = gtk_label_new_with_mnemonic (_(DATA[i].label));
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

		gtk_widget_show_all (button);

		g_signal_connect (G_OBJECT (button), "clicked",
				G_CALLBACK (on_system_command_button_clicked), dialog);
	}
}

static void
logout_dialog_class_init (LogoutDialogClass *class)
{
	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
			"/kr/gooroom/logout/logout-dialog.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), LogoutDialog, img_logo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), LogoutDialog, box_button);
}

static void
logout_dialog_grab_callback (GdkSeat   *seat,
                             GdkWindow *window,
                             gpointer   user_data)
{
	/* ensure window is mapped to avoid unsuccessful grabs */
	if (!gdk_window_is_visible (window))
		gdk_window_show (window);
}

/* Copied from xfce4-session/xfce4-session/xfsm-logout-dialog.c:
 * xfsm_logout_dialog_run () */
static gint
logout_dialog_run (GtkWidget *dialog, gboolean grab_input)
{
	GdkWindow *window;
	gint       ret;
	GdkDevice *device;
	GdkSeat   *seat;

	if (grab_input) {
		gtk_widget_show_now (dialog);
		window = gtk_widget_get_window (dialog);

		device = gtk_get_current_event_device ();
		seat = device != NULL
			? gdk_device_get_seat (device)
			: gdk_display_get_default_seat (gtk_widget_get_display (dialog));

		if (gdk_seat_grab (seat, window,
					GDK_SEAT_CAPABILITY_KEYBOARD,
					FALSE, NULL, NULL,
					logout_dialog_grab_callback,
					NULL) != GDK_GRAB_SUCCESS)
		{
			g_critical ("Failed to grab the keyboard for logout window");
		}

		/* force input to the dialog */
		XSetInputFocus (gdk_x11_get_default_xdisplay (),
				GDK_WINDOW_XID (window),
				RevertToParent, CurrentTime);
	}

	ret = gtk_dialog_run (GTK_DIALOG (dialog));

	if (grab_input)
		gdk_seat_ungrab (seat);

	return ret;
}

/* Copied from xfce4-session/xfce4-session/xfsm-logout-dialog.c:
 * xfsm_logout_dialog () */
void
logout_dialog_show (void)
{
	gint              result;
	GtkWidget        *hidden;
	GtkWidget        *dialog;
	GdkScreen        *screen;
	GList            *xwindows;
	GdkDevice        *device;
	GdkSeat          *seat;
	gint              grab_count = 0;

	screen = gdk_screen_get_default ();
	hidden = gtk_invisible_new_for_screen (screen);
	gtk_widget_show (hidden);

	/* wait until we can grab the keyboard, we need this for
	 * the dialog when running it */
	for (;;) {
		device = gtk_get_current_event_device ();
		seat = device != NULL
			? gdk_device_get_seat (device)
			: gdk_display_get_default_seat (gtk_widget_get_display (hidden));

		if (gdk_seat_grab (seat, gtk_widget_get_window (hidden),
					GDK_SEAT_CAPABILITY_KEYBOARD,
					FALSE, NULL, NULL,
					logout_dialog_grab_callback,
					NULL) == GDK_GRAB_SUCCESS)
		{
			gdk_seat_ungrab (seat);
			break;
		}

		if (grab_count++ >= 40) {
			g_critical ("Failed to grab the keyboard for logout window");
			break;
		}

		g_usleep (G_USEC_PER_SEC / 20);
	}

	/* display fadeout */
	xwindows = fadeout_window_show (gdk_screen_get_display (screen));

	dialog = g_object_new (DIALOG_TYPE_LOGOUT,
                           "type", GTK_WINDOW_POPUP,
			               "screen", screen, NULL);

	gtk_widget_realize (dialog);

	gdk_window_set_override_redirect (gtk_widget_get_window (dialog), TRUE);
	gdk_window_raise (gtk_widget_get_window (dialog));
	gtk_widget_destroy (hidden);

	result = logout_dialog_run (dialog, TRUE);

	fadeout_window_hide (xwindows, gdk_screen_get_display (screen));
	g_list_free (xwindows);

	gtk_widget_destroy (dialog);
}
