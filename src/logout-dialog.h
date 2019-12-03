/* 
 * Copyright (C) 2018-2019 Gooroom <gooroom@gooroom.kr>
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
 */

#ifndef __LOGOUT_DIALOG_H__
#define __LOGOUT_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _LogoutDialog      LogoutDialog;
typedef struct _LogoutDialogClass LogoutDialogClass;

#define DIALOG_TYPE_LOGOUT            (logout_dialog_get_type ())
#define LOGOUT_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DIALOG_TYPE_LOGOUT, LogoutDialog))
#define LOGOUT_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DIALOG_TYPE_LOGOUT, LogoutDialogClass))
#define DIALOG_IS_LOGOUT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DIALOG_TYPE_LOGOUT))
#define DIALOG_IS_LOGOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DIALOG_TYPE_LOGOUT))
#define LOGOUT_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DIALOG_TYPE_LOGOUT, LogoutDialogClass))

GType         logout_dialog_get_type (void) G_GNUC_CONST;

void          logout_dialog_show     (void);

G_END_DECLS

#endif
