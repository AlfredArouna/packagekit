/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2008-2009 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2008 Red Hat, Inc.
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <glib/gi18n-lib.h>
#include <string.h>
#include <gio/gdesktopappinfo.h>
#include <pango/pangocairo.h>
#include <dbus/dbus-glib.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "pk-main.h"
#include "pk-plugin-install.h"

#define PK_PLUGIN_INSTALL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), PK_TYPE_PLUGIN_INSTALL, PkPluginInstallPrivate))

typedef enum {
	IN_PROGRESS, /* looking up package information */
	INSTALLED,   /* package installed */
	UPGRADABLE,  /* package installed, newer version available */
	AVAILABLE,   /* package not installed, version available */
	UNAVAILABLE, /* package not installed or available */
	INSTALLING   /* currently installing a new version */
} PkPluginInstallPackageStatus;

struct PkPluginInstallPrivate
{
	PkPluginInstallPackageStatus	 status;
	gchar			*available_version;
	gchar			*available_package_name;
	gchar			*installed_version;
	gchar			*installed_package_name;
	GAppInfo		*app_info;
	gchar			*display_name;
	gchar			**package_names;
	PangoLayout		*pango_layout;
	PkClientPool		*client_pool;
	DBusGProxy		*install_package_proxy;
	DBusGProxyCall		*install_package_call;
};

G_DEFINE_TYPE (PkPluginInstall, pk_plugin_install, PK_TYPE_PLUGIN)

/**
 * pk_plugin_install_clear_layout:
 **/
static void
pk_plugin_install_clear_layout (PkPluginInstall *self)
{
	pk_debug ("clearing layout");

	if (self->priv->pango_layout) {
		g_object_unref (self->priv->pango_layout);
		self->priv->pango_layout = NULL;
	}
}

/**
 * pk_plugin_install_refresh:
 **/
static void
pk_plugin_install_refresh (PkPluginInstall *self)
{
	pk_plugin_request_refresh (PK_PLUGIN (self));
}

/**
 * pk_plugin_install_set_status:
 **/
static void
pk_plugin_install_set_status (PkPluginInstall *self, PkPluginInstallPackageStatus status)
{
	if (self->priv->status != status) {
		pk_debug ("setting status %u", status);
		self->priv->status = status;
	}
}

/**
 * pk_plugin_install_set_available_version:
 **/
static void
pk_plugin_install_set_available_version (PkPluginInstall *self, const gchar *version)
{
	pk_debug ("setting available version: %s", version);

	g_free (self->priv->available_version);
	self->priv->available_version = g_strdup (version);
}

/**
 * pk_plugin_install_set_available_package_name:
 **/
static void
pk_plugin_install_set_available_package_name (PkPluginInstall *self, const gchar *name)
{
	pk_debug ("setting available package name: %s", name);

	g_free (self->priv->available_package_name);
	self->priv->available_package_name = g_strdup (name);
}

/**
 * pk_plugin_install_set_installed_package_name:
 **/
static void
pk_plugin_install_set_installed_package_name (PkPluginInstall *self, const gchar *name)
{
	pk_debug ("setting installed package name: %s", name);

	g_free (self->priv->installed_package_name);
	self->priv->installed_package_name = g_strdup (name);
}

/**
 * pk_plugin_install_set_installed_version:
 **/
static void
pk_plugin_install_set_installed_version (PkPluginInstall *self, const gchar *version)
{
	pk_debug ("setting installed version: %s", version);

	g_free (self->priv->installed_version);
	self->priv->installed_version = g_strdup (version);
}

/**
 * pk_plugin_install_get_best_desktop_file:
 **/
static gchar *
pk_plugin_install_get_best_desktop_file (PkPluginInstall *self)
{
	GPtrArray *array = NULL;
	PkDesktop *desktop;
	GError *error = NULL;
	gboolean ret;
	gchar *data = NULL;
	const gchar *package;

	/* open desktop database */
	desktop = pk_desktop_new ();
	ret = pk_desktop_open_database (desktop, &error);
	if (!ret) {
		pk_warning ("failed to open database: %s", error->message);
		g_error_free (error);
		goto out;
	}

	/* get files */
	package = self->priv->installed_package_name;
	if (package == NULL) {
		pk_warning ("installed_package_name NULL so cannot get desktop file");
		goto out;
	}
	array = pk_desktop_get_shown_for_package (desktop, package, &error);
	if (array == NULL) {
		pk_debug ("no data: %s", error->message);
		g_error_free (error);
		goto out;
	}
	if (array->len == 0) {
		pk_debug ("no matches for %s", package);
		goto out;
	}

	/* just use the first entry */
	data = g_strdup (g_ptr_array_index (array, 0));
out:
	if (array != NULL) {
		g_ptr_array_foreach (array, (GFunc) g_free, NULL);
		g_ptr_array_free (array, TRUE);
	}
	g_object_unref (desktop);
	return data;
}

/**
 * pk_plugin_install_package_cb:
 **/
static void
pk_plugin_install_package_cb (PkClient *client, const PkPackageObj *obj, PkPluginInstall *self)
{
	gchar *filename;

	/* if we didn't use displayname, use the summary */
	if (self->priv->display_name == NULL)
		self->priv->display_name = g_strdup (obj->summary);

	/* parse the data */
	if (obj->info == PK_INFO_ENUM_AVAILABLE) {
		if (self->priv->status == IN_PROGRESS)
			pk_plugin_install_set_status (self, AVAILABLE);
		else if (self->priv->status == INSTALLED)
			pk_plugin_install_set_status (self, UPGRADABLE);
		pk_plugin_install_set_available_version (self, obj->id->version);
		pk_plugin_install_set_available_package_name (self, obj->id->name);

#if 0
		/* if we have data from the repo, override the user:
		 *  * we don't want the remote site pretending to install another package
		 *  * it might be localised if the backend supports it */
		if (obj->summary != NULL && obj->summary[0] != '\0')
			self->priv->display_name = g_strdup (obj->summary);
#endif

		pk_plugin_install_clear_layout (self);
		pk_plugin_install_refresh (self);

	} else if (obj->info == PK_INFO_ENUM_INSTALLED) {
		if (self->priv->status == IN_PROGRESS)
			pk_plugin_install_set_status (self, INSTALLED);
		else if (self->priv->status == AVAILABLE)
			pk_plugin_install_set_status (self, UPGRADABLE);
		pk_plugin_install_set_installed_version (self, obj->id->version);
		pk_plugin_install_set_installed_package_name (self, obj->id->name);

		/* get desktop file information */
		filename = pk_plugin_install_get_best_desktop_file (self);
		if (filename != NULL) {
			self->priv->app_info = G_APP_INFO (g_desktop_app_info_new_from_filename (filename));
#if 0
			/* override, as this will have translation */
			self->priv->display_name = g_strdup (g_app_info_get_name (self->priv->app_info));
#endif
		}
		g_free (filename);

		if (self->priv->app_info != 0)
			pk_plugin_install_set_status (self, INSTALLED);

		pk_plugin_install_clear_layout (self);
		pk_plugin_install_refresh (self);
	}
}

/**
 * pk_plugin_install_error_code_cb:
 **/
static void
pk_plugin_install_error_code_cb (PkClient *client, PkErrorCodeEnum code, const gchar *details, PkPluginInstall *self)
{
	pk_warning ("Error getting data from PackageKit: %s\n", details);

	if (self->priv->status == IN_PROGRESS) {
		pk_plugin_install_set_status (self, UNAVAILABLE);
		pk_plugin_install_clear_layout (self);
		pk_plugin_install_refresh (self);
	}
}

/**
 * pk_plugin_install_finished_cb:
 **/
static void
pk_plugin_install_finished_cb (PkClient *client, PkExitEnum exit, guint runtime, PkPluginInstall *self)
{
	if (self->priv->status == IN_PROGRESS) {
		pk_plugin_install_set_status (self, UNAVAILABLE);
		pk_plugin_install_clear_layout (self);
		pk_plugin_install_refresh (self);
	}
}

/**
 * pk_plugin_install_recheck:
 **/
static void
pk_plugin_install_recheck (PkPluginInstall *self)
{
	guint i;
	const gchar *data;
	gchar **package_ids;
	GError *error = NULL;
	PkClient *client;
	gboolean ret;

	self->priv->status = IN_PROGRESS;
	pk_plugin_install_set_available_version (self, NULL);
	pk_plugin_install_set_available_package_name (self, NULL);
	pk_plugin_install_set_installed_version (self, NULL);
	pk_plugin_install_set_installed_package_name (self, NULL);

	/* get data, if if does not exist */
	if (self->priv->package_names == NULL) {
		data = pk_plugin_get_data (PK_PLUGIN (self), "displayname");
		self->priv->display_name = g_strdup (data);
		data = pk_plugin_get_data (PK_PLUGIN (self), "packagenames");
		self->priv->package_names = g_strsplit (data, " ", -1);
	}

	for (i=0; self->priv->package_names[i] != NULL; i++) {
		package_ids = pk_package_ids_from_id (self->priv->package_names[i]);
		client = pk_client_pool_create (self->priv->client_pool);

		/* do async resolve */
		ret = pk_client_resolve (client, PK_FILTER_ENUM_NONE, package_ids, &error);
		if (!ret) {
			pk_warning ("%s", error->message);
			g_clear_error (&error);
			pk_client_pool_remove (self->priv->client_pool, client);
		}
		g_strfreev (package_ids);
		g_object_unref (client);
	}

	if (pk_client_pool_get_size (self->priv->client_pool) == 0 && self->priv->status == IN_PROGRESS) {
		pk_plugin_install_set_status (self, UNAVAILABLE);
		pk_plugin_install_clear_layout (self);
		pk_plugin_install_refresh (self);
	}
}

/**
 * pk_plugin_install_append_markup:
 **/
static void
pk_plugin_install_append_markup (GString *str, const gchar *format, ...)
{
	va_list vap;
	gchar *tmp;

	va_start (vap, format);
	tmp = g_markup_vprintf_escaped (format, vap);
	va_end (vap);

	g_string_append (str, tmp);
	g_free (tmp);
}

/**
 * pk_plugin_install_rgba_from_gdk_color:
 **/
static guint32
pk_plugin_install_rgba_from_gdk_color (GdkColor *color)
{
	return (((color->red >> 8) << 24) |
		((color->green >> 8) << 16) |
		 ((color->blue >> 8) << 8) |
		  0xff);
}

/**
 * pk_plugin_install_set_source_from_rgba:
 **/
static void
pk_plugin_install_set_source_from_rgba (cairo_t *cr, guint32 rgba)
{
	cairo_set_source_rgba (cr,
			      ((rgba & 0xff000000) >> 24) / 255.,
			      ((rgba & 0x00ff0000) >> 16) / 255.,
			      ((rgba & 0x0000ff00) >> 8) / 255.,
			      (rgba & 0x000000ff) / 255.);
}

/**
 * pk_plugin_install_get_style:
 *
 * Retrieve the system colors and fonts.
 * This looks incredibly expensive .... to create a GtkWindow for
 * every expose ... but actually it's only moderately expensive;
 * Creating a GtkWindow is just normal GObject creation overhead --
 * the extra expense beyond that will come when we actually create
 * the window.
 **/
static void
pk_plugin_install_get_style (PangoFontDescription **font_desc, guint32 *foreground, guint32 *background, guint32 *link)
{
	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkStyle *style;
	GdkColor link_color = { 0, 0, 0, 0xeeee };
	GdkColor *tmp = NULL;

	gtk_widget_ensure_style (window);

	style = gtk_widget_get_style (window);
	*foreground = pk_plugin_install_rgba_from_gdk_color (&style->text[GTK_STATE_NORMAL]);
	*background = pk_plugin_install_rgba_from_gdk_color (&style->base[GTK_STATE_NORMAL]);

	gtk_widget_style_get (GTK_WIDGET (window), "link-color", &tmp, NULL);
	if (tmp != NULL) {
		link_color = *tmp;
		gdk_color_free (tmp);
	}

	*link = pk_plugin_install_rgba_from_gdk_color (&link_color);

	*font_desc = pango_font_description_copy (style->font_desc);

	gtk_widget_destroy (window);
}

/**
 * pk_plugin_install_ensure_layout:
 **/
static void
pk_plugin_install_ensure_layout (PkPluginInstall *self, cairo_t *cr, PangoFontDescription *font_desc, guint32 link_color)
{
	GString *markup = g_string_new (NULL);

	if (self->priv->pango_layout != NULL)
		return;

	self->priv->pango_layout = pango_cairo_create_layout (cr);
	pango_layout_set_font_description (self->priv->pango_layout, font_desc);

	/* WARNING: Any changes to what links are created here will require corresponding
	 * changes to the pk_plugin_install_button_release () method
	 */
	switch (self->priv->status) {
	case IN_PROGRESS:
		/* TRANSLATORS: when we are getting data from the daemon */
		pk_plugin_install_append_markup (markup, _("Getting package information..."));
		break;
	case INSTALLED:
		if (self->priv->app_info != 0) {
			pk_plugin_install_append_markup (markup, "<span color='#%06x' underline='single'>", link_color >> 8);
			/* TRANSLATORS: run an applicaiton */
			pk_plugin_install_append_markup (markup, _("Run %s"), self->priv->display_name);
			pk_plugin_install_append_markup (markup, "</span>");
		} else
			pk_plugin_install_append_markup (markup, "<big>%s</big>", self->priv->display_name);
		if (self->priv->installed_version != NULL)
			/* TRANSLATORS: show the installed version of a package */
			pk_plugin_install_append_markup (markup, "\n<small>%s: %s</small>", _("Installed version"), self->priv->installed_version);
		break;
	case UPGRADABLE:
		pk_plugin_install_append_markup (markup, "<big>%s</big>", self->priv->display_name);
		if (self->priv->app_info != 0) {
			if (self->priv->installed_version != NULL) {
				pk_plugin_install_append_markup (markup, "\n<span color='#%06x' underline='single'>", link_color >> 8);
				/* TRANSLATORS: run the application now */
				pk_plugin_install_append_markup (markup, _("Run version %s now"), self->priv->installed_version);
				pk_plugin_install_append_markup (markup, "</span>");
			} else {
				pk_plugin_install_append_markup (markup,
				              "\n<span color='#%06x' underline='single'>%s</span>",
					      /* TRANSLATORS: run the application now */
					      _("Run now"), link_color >> 8);
		        }
		}

		pk_plugin_install_append_markup (markup, "\n<span color='#%06x' underline='single'>", link_color >> 8);
		/* TRANSLATORS: update to a new version of the package */
		pk_plugin_install_append_markup (markup, _("Update to version %s"), self->priv->available_version);
		pk_plugin_install_append_markup (markup, "</span>");
		break;
	case AVAILABLE:
		pk_plugin_install_append_markup (markup, "<span color='#%06x' underline='single'>", link_color >> 8);
		/* TRANSLATORS: To install a package */
		pk_plugin_install_append_markup (markup, _("Install %s now"), self->priv->display_name);
		pk_plugin_install_append_markup (markup, "</span>");
		/* TRANSLATORS: the version of the package */
		pk_plugin_install_append_markup (markup, "\n<small>%s: %s</small>", _("Version"), self->priv->available_version);
		break;
	case UNAVAILABLE:
		pk_plugin_install_append_markup (markup, "<big>%s</big>", self->priv->display_name);
		/* TRANSLATORS: noting found, so can't install */
		pk_plugin_install_append_markup (markup, "\n<small>%s</small>", _("No packages found for your system"));
		break;
	case INSTALLING:
		pk_plugin_install_append_markup (markup, "<big>%s</big>", self->priv->display_name);
		/* TRANSLATORS: package is being installed */
		pk_plugin_install_append_markup (markup, "\n<small>%s</small>", _("Installing..."));
		break;
	}

	pango_layout_set_markup (self->priv->pango_layout, markup->str, -1);
	g_string_free (markup, TRUE);
}

/**
 * pk_plugin_install_get_package_icon:
 **/
static gchar *
pk_plugin_install_get_package_icon (PkPluginInstall *self)
{
	gboolean ret;
	GKeyFile *file;
	gchar *data = NULL;
	const gchar *filename;

	/* do we have data? */
	if (self->priv->installed_package_name == NULL) {
		pk_debug ("installed_package_name NULL, so cannot get icon");
		goto out;
	}

	/* get data from the best file */
	file = g_key_file_new ();
	filename = pk_plugin_install_get_best_desktop_file (self);
	if (filename == NULL) {
		pk_debug ("no desktop file");
		goto out;
	}
	ret = g_key_file_load_from_file (file, filename, G_KEY_FILE_NONE, NULL);
	if (!ret) {
		pk_warning ("failed to open %s", filename);
		goto out;
	}
	data = g_key_file_get_string (file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);
	g_key_file_free (file);
out:
	return data;
}

/**
 * pk_plugin_install_start:
 **/
static gboolean
pk_plugin_install_start (PkPlugin *plugin)
{
	PkPluginInstall *self = PK_PLUGIN_INSTALL (plugin);
	pk_plugin_install_recheck (self);
	return TRUE;
}

/**
 * pk_plugin_install_draw:
 **/
static gboolean
pk_plugin_install_draw (PkPlugin *plugin, cairo_t *cr)
{
	guint32 foreground, background, link;
	PangoFontDescription *font_desc;
	guint x;
	guint y;
	guint width;
	guint height;
	const gchar *filename;
	GtkIconTheme *theme;
	GdkPixbuf *pixbuf;
	PkPluginInstall *self = PK_PLUGIN_INSTALL (plugin);

	/* get parameters */
	g_object_get (self,
		      "x", &x,
		      "y", &y,
		      "width", &width,
		      "height", &height,
		      NULL);

	pk_debug ("drawing on %ux%u (%ux%u)", x, y, width, height);

	/* get properties */
	pk_plugin_install_get_style (&font_desc, &foreground, &background, &link);

       /* fill background */
	pk_plugin_install_set_source_from_rgba (cr, background);
	cairo_rectangle (cr, x, y, width, height);
	cairo_fill (cr);

        /* grey outline */
	cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
	cairo_rectangle (cr, x + 0.5, y + 0.5, width - 1, height - 1);
	cairo_set_line_width (cr, 1);
	cairo_stroke (cr);

	/* get themed icon */
	filename = pk_plugin_install_get_package_icon (self);
	if (filename == NULL)
		filename = "package-x-generic";
	theme = gtk_icon_theme_get_default ();
	pixbuf = gtk_icon_theme_load_icon (theme, filename, 48, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
	if (pixbuf == NULL)
		goto skip;
	gdk_cairo_set_source_pixbuf (cr, pixbuf, x + PK_PLUGIN_INSTALL_MARGIN, y + PK_PLUGIN_INSTALL_MARGIN);
	cairo_rectangle (cr, x + PK_PLUGIN_INSTALL_MARGIN, y + PK_PLUGIN_INSTALL_MARGIN, 48, 48);
	cairo_fill (cr);
	g_object_unref (pixbuf);

skip:
	/* write text */
	pk_plugin_install_ensure_layout (self, cr, font_desc, link);
	cairo_move_to (cr, (x + PK_PLUGIN_INSTALL_MARGIN*2) + 48, y + PK_PLUGIN_INSTALL_MARGIN + PK_PLUGIN_INSTALL_MARGIN);
	pk_plugin_install_set_source_from_rgba (cr, foreground);
	pango_cairo_show_layout (cr, self->priv->pango_layout);
	return TRUE;
}

/**
 * pk_plugin_install_line_is_terminated:
 *
 * Cut and paste from pango-layout.c; determines if a layout iter is on
 * a line terminated by a real line break (rather than a line break from
 * wrapping). We use this to determine whether the empty run at the end
 * of a display line should be counted as a break between links or not.
 *
 * (Code in pango-layout.c is by me, Copyright Red Hat, and hereby relicensed
 * to the license of this file)
 **/
static gboolean
pk_plugin_install_line_is_terminated (PangoLayoutIter *iter)
{
	/* There is a real terminator at the end of each paragraph other
	 * than the last.
	 */
	PangoLayoutLine *line = pango_layout_iter_get_line (iter);
	GSList *lines = pango_layout_get_lines (pango_layout_iter_get_layout (iter));
	GSList *link = g_slist_find (lines, line);
	if (!link) {
		pk_warning ("Can't find line in layout line list");
		return FALSE;
	}

	if (link->next) {
		PangoLayoutLine *next_line = (PangoLayoutLine *)link->next->data;
		if (next_line->is_paragraph_start)
			return TRUE;
	}

	return FALSE;
}

/**
 * pk_plugin_install_get_link_index:
 *
 * This function takes an X,Y position and determines whether it is over one
 * of the underlined portions of the layout (a link). It works by iterating
 * through the runs of the layout (a run is a segment with a consistent
 * font and display attributes, more or less), and counting the underlined
 * segments that we see. A segment that is underlined could be broken up
 * into multiple runs if it is drawn with multiple fonts due to fonts
 * substitution, so we actually count non-underlined => underlined
 * transitions.
 **/
static gint
pk_plugin_install_get_link_index (PkPluginInstall *self, gint x, gint y)
{
	gint idx;
	gint trailing;
	PangoLayoutIter *iter;
	gint seen_links = 0;
	gboolean in_link = FALSE;
	gint result = -1;

	/* Coordinates are relative to origin of plugin (different from drawing) */

	if (!self->priv->pango_layout)
		return -1;

	x -= (PK_PLUGIN_INSTALL_MARGIN * 2) + 48;
	y -= (PK_PLUGIN_INSTALL_MARGIN * 2);

	if (!pango_layout_xy_to_index (self->priv->pango_layout, x * PANGO_SCALE, y * PANGO_SCALE, &idx, &trailing))
		return - 1;

	iter = pango_layout_get_iter (self->priv->pango_layout);
	while (TRUE) {
		PangoLayoutRun *run = pango_layout_iter_get_run (iter);
		if (run) {
			PangoItem *item = run->item;
			PangoUnderline uline = PANGO_UNDERLINE_NONE;
			GSList *l;

			for (l = item->analysis.extra_attrs; l; l = l->next) {
				PangoAttribute *attr = (PangoAttribute *)l->data;
				if (attr->klass->type == PANGO_ATTR_UNDERLINE) {
					uline = (PangoUnderline) ( (PangoAttrInt *)attr)->value;
				}
			}

			if (uline == PANGO_UNDERLINE_NONE)
				in_link = FALSE;
			else if (!in_link) {
				in_link = TRUE;
				seen_links++;
			}

			if (item->offset <= idx && idx < item->offset + item->length) {
				if (in_link)
					result = seen_links - 1;

				break;
			}
		} else {
			/* We have an empty run at the end of each line. A line break doesn't
			 * terminate the link, but a real newline does.
			 */
			if (pk_plugin_install_line_is_terminated (iter))
				in_link = FALSE;
		}

		if (!pango_layout_iter_next_run (iter))
			break;
	}

	pango_layout_iter_free (iter);

	return result;
}

/**
 * pk_plugin_install_method_finished_cb:
 **/
static void
pk_plugin_install_method_finished_cb (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
	PkPluginInstall *self = (PkPluginInstall *)user_data;
	GError *error = NULL;

	if (!dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID)) {
		pk_warning ("Error occurred during install: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (self->priv->install_package_proxy);
	self->priv->install_package_proxy = NULL;
	self->priv->install_package_call = NULL;

	pk_plugin_install_recheck (self);
}

/**
 * pk_plugin_install_install_package:
 **/
static void
pk_plugin_install_install_package (PkPluginInstall *self, Time event_time)
{
	GdkEvent *event;
	GdkWindow *window;
	guint xid = 0;
	gchar **packages;
	DBusGConnection *connection;

	if (self->priv->available_package_name == NULL) {
		pk_warning ("No available package to install");
		return;
	}

	if (self->priv->install_package_call != 0) {
		pk_warning ("Already installing package");
		return;
	}

	/* TODO: needs to be on Modify interface */
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, NULL);
	self->priv->install_package_proxy = dbus_g_proxy_new_for_name (connection,
							 "org.freedesktop.PackageKit",
							 "/org/freedesktop/PackageKit",
							 "org.freedesktop.PackageKit.Modify");

	/* will be NULL when activated not using a keyboard or a mouse */
	event = gtk_get_current_event ();
	if (event != NULL && event->any.window != NULL) {
		window = gdk_window_get_toplevel (event->any.window);
		xid = GDK_DRAWABLE_XID (window);
	}

	packages = g_strsplit (self->priv->available_package_name, ";", -1);
	self->priv->install_package_call =
		dbus_g_proxy_begin_call_with_timeout (self->priv->install_package_proxy,
						      "InstallPackageNames",
						      pk_plugin_install_method_finished_cb,
						      self,
						      (GDestroyNotify) 0,
						      24 * 60 * 1000 * 1000, /* one day */
						      G_TYPE_UINT, xid, /* xid */
						      G_TYPE_STRV, packages,
						      G_TYPE_STRING, "hide-finished",
						      G_TYPE_INVALID,
						      G_TYPE_INVALID);
	g_strfreev (packages);

	pk_plugin_install_set_status (self, INSTALLING);
	pk_plugin_install_clear_layout (self);
	pk_plugin_install_refresh (self);
}

/**
 * pk_plugin_install_get_server_timestamp:
 **/
static guint32
pk_plugin_install_get_server_timestamp ()
{
	GtkWidget *invisible = gtk_invisible_new ();
	GdkWindow *window;
	guint32 server_time;

	gtk_widget_realize (invisible);
	window = gtk_widget_get_window (invisible);
	server_time = gdk_x11_get_server_time (window);
	gtk_widget_destroy (invisible);
	return server_time;
}

/**
 * pk_plugin_install_run_application:
 **/
static void
pk_plugin_install_run_application (PkPluginInstall *self, Time event_time)
{
	GError *error = NULL;
	GdkAppLaunchContext *context;

	if (self->priv->app_info == 0) {
		pk_warning ("Didn't find application to launch");
		return;
	}

	if (event_time == 0)
		event_time = pk_plugin_install_get_server_timestamp ();

	context = gdk_app_launch_context_new ();
	gdk_app_launch_context_set_timestamp (context, event_time);
	if (!g_app_info_launch (self->priv->app_info, NULL, G_APP_LAUNCH_CONTEXT (context), &error)) {
		pk_warning ("%s\n", error->message);
		g_clear_error (&error);
		return;
	}

	if (context != NULL)
		g_object_unref (context);
}

/**
 * pk_plugin_install_button_release:
 **/
static gboolean
pk_plugin_install_button_release (PkPlugin *plugin, gint x, gint y, Time event_time)
{
	PkPluginInstall *self = PK_PLUGIN_INSTALL (plugin);
	gint idx = pk_plugin_install_get_link_index (self, x, y);
	if (idx < 0)
		return FALSE;

	switch (self->priv->status) {
	case IN_PROGRESS:
	case INSTALLING:
	case UNAVAILABLE:
		break;
	case INSTALLED:
		if (self->priv->app_info != NULL)
			pk_plugin_install_run_application (self, event_time);
		break;
	case UPGRADABLE:
		if (self->priv->app_info != NULL && idx == 0)
			pk_plugin_install_run_application (self, event_time);
		else
			pk_plugin_install_install_package (self, event_time);
		break;
	case AVAILABLE:
		if (self->priv->available_package_name != NULL)
			pk_plugin_install_install_package (self, event_time);
		break;
	}
	return TRUE;
}

/**
 * pk_plugin_install_finalize:
 **/
static void
pk_plugin_install_finalize (GObject *object)
{
	PkPluginInstall *self;
	g_return_if_fail (PK_IS_PLUGIN_INSTALL (object));
	self = PK_PLUGIN_INSTALL (object);

	pk_plugin_install_clear_layout (self);

	if (self->priv->app_info != NULL)
		g_object_unref (self->priv->app_info);

	if (self->priv->install_package_call != NULL) {
		dbus_g_proxy_cancel_call (self->priv->install_package_proxy, self->priv->install_package_call);
		g_object_unref (self->priv->install_package_proxy);
	}

	/* remove clients */
	g_object_unref (self->priv->client_pool);

	G_OBJECT_CLASS (pk_plugin_install_parent_class)->finalize (object);
}

/**
 * pk_plugin_install_class_init:
 **/
static void
pk_plugin_install_class_init (PkPluginInstallClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	PkPluginClass *plugin_class = PK_PLUGIN_CLASS (klass);

	object_class->finalize = pk_plugin_install_finalize;
	plugin_class->start = pk_plugin_install_start;
	plugin_class->draw = pk_plugin_install_draw;
	plugin_class->button_release = pk_plugin_install_button_release;

	g_type_class_add_private (klass, sizeof (PkPluginInstallPrivate));
}

/**
 * pk_plugin_install_init:
 **/
static void
pk_plugin_install_init (PkPluginInstall *self)
{
	self->priv = PK_PLUGIN_INSTALL_GET_PRIVATE (self);

	self->priv->status = IN_PROGRESS;
	self->priv->available_version = NULL;
	self->priv->available_package_name = NULL;
	self->priv->installed_version = NULL;
	self->priv->installed_package_name = NULL;
	self->priv->app_info = NULL;
	self->priv->display_name = NULL;
	self->priv->package_names = NULL;
	self->priv->pango_layout = NULL;
	self->priv->install_package_proxy = NULL;
	self->priv->install_package_call = NULL;

	/* use a client pool to do everything async */
	self->priv->client_pool = pk_client_pool_new ();
	pk_client_pool_connect (self->priv->client_pool, "package", G_CALLBACK (pk_plugin_install_package_cb), G_OBJECT (self));
	pk_client_pool_connect (self->priv->client_pool, "error-code", G_CALLBACK (pk_plugin_install_error_code_cb), G_OBJECT (self));
	pk_client_pool_connect (self->priv->client_pool, "finished", G_CALLBACK (pk_plugin_install_finished_cb), G_OBJECT (self));
}

/**
 * pk_plugin_install_new:
 * Return value: A new plugin_install class instance.
 **/
PkPluginInstall *
pk_plugin_install_new (void)
{
	PkPluginInstall *self;
	self = g_object_new (PK_TYPE_PLUGIN_INSTALL, NULL);
	return PK_PLUGIN_INSTALL (self);
}

/***************************************************************************
 ***                          MAKE CHECK TESTS                           ***
 ***************************************************************************/
#ifdef EGG_TEST
#include "egg-test.h"

void
egg_test_plugin_install (EggTest *test)
{
	PkPluginInstall *self;

	if (!egg_test_start (test, "PkPluginInstall"))
		return;

	/************************************************************/
	egg_test_title (test, "get an instance");
	self = pk_plugin_install_new ();
	if (self != NULL)
		egg_test_success (test, NULL);
	else
		egg_test_failed (test, NULL);

	g_object_unref (self);

	egg_test_end (test);
}
#endif
