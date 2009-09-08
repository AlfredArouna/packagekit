/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007-2009 Richard Hughes <richard@hughsie.com>
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

#include "config.h"

//#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib/gi18n.h>
#include <packagekit-glib2/packagekit.h>
#include <packagekit-glib2/packagekit-private.h>
#include <sys/types.h>
#include <pwd.h>
#include <locale.h>

#include "egg-debug.h"

#define PK_EXIT_CODE_SYNTAX_INVALID	3
#define PK_EXIT_CODE_FILE_NOT_FOUND	4

static GMainLoop *loop = NULL;
static PkBitfield roles;
static gboolean is_console = FALSE;
static gboolean nowait = FALSE;
static PkControl *control = NULL;
static PkTaskText *task = NULL;
static PkProgressBar *progressbar = NULL;
static GCancellable *cancellable = NULL;

/**
 * pk_strpad:
 * @data: the input string
 * @length: the desired length of the output string, with padding
 *
 * Returns the text padded to a length with spaces. If the string is
 * longer than length then a longer string is returned.
 *
 * Return value: The padded string
 **/
static gchar *
pk_strpad (const gchar *data, guint length)
{
	gint size;
	guint data_len;
	gchar *text;
	gchar *padding;

	if (data == NULL)
		return g_strnfill (length, ' ');

	/* ITS4: ignore, only used for formatting */
	data_len = strlen (data);

	/* calculate */
	size = (length - data_len);
	if (size <= 0)
		return g_strdup (data);

	padding = g_strnfill (size, ' ');
	text = g_strdup_printf ("%s%s", data, padding);
	g_free (padding);
	return text;
}

/**
 * pk_console_package_cb:
 **/
static void
pk_console_package_cb (const PkItemPackage *obj, gpointer data)
{
	gchar *package = NULL;
	gchar *package_pad = NULL;
	gchar *info_pad = NULL;
	gchar **split = NULL;

	/* ignore finished */
	if (obj->info_enum == PK_INFO_ENUM_FINISHED)
		goto out;

	/* split */
	split = g_strsplit (obj->package_id, ";", -1);
	if (g_strv_length (split) != 4)
		goto out;

	/* make these all the same length */
	info_pad = pk_strpad (pk_info_enum_to_text (obj->info_enum), 12);

	/* create printable */
	package = pk_package_id_to_printable (obj->package_id);

	/* don't pretty print */
	if (!is_console) {
		g_print ("%s %s\n", info_pad, package);
		goto out;
	}

	/* pad the name-version */
	package_pad = pk_strpad (package, 40);
	g_print ("%s\t%s\t%s\n", info_pad, package_pad, obj->summary);
out:
	/* free all the data */
	g_free (package);
	g_free (package_pad);
	g_free (info_pad);
	g_strfreev (split);
}

/**
 * pk_console_transaction_cb:
 **/
static void
pk_console_transaction_cb (const PkItemTransaction *obj, gpointer user_data)
{
	struct passwd *pw;
	const gchar *role_text;
	gchar **lines;
	gchar **parts;
	guint i, lines_len;
	gchar *package = NULL;

	role_text = pk_role_enum_to_text (obj->role);
	/* TRANSLATORS: this is an atomic transaction */
	g_print ("%s: %s\n", _("Transaction"), obj->tid);
	/* TRANSLATORS: this is the time the transaction was started in system timezone */
	g_print (" %s: %s\n", _("System time"), obj->timespec);
	/* TRANSLATORS: this is if the transaction succeeded or not */
	g_print (" %s: %s\n", _("Succeeded"), obj->timespec ? _("True") : _("False"));
	/* TRANSLATORS: this is the transactions role, e.g. "update-system" */
	g_print (" %s: %s\n", _("Role"), role_text);

	/* only print if not null */
	if (obj->duration > 0) {
		/* TRANSLATORS: this is The duration of the transaction */
		g_print (" %s: %i %s\n", _("Duration"), obj->duration, _("(seconds)"));
	}

	/* TRANSLATORS: this is The command line used to do the action */
	g_print (" %s: %s\n", _("Command line"), obj->cmdline);
	/* TRANSLATORS: this is the user ID of the user that started the action */
	g_print (" %s: %i\n", _("User ID"), obj->uid);

	/* query real name */
	pw = getpwuid (obj->uid);
	if (pw != NULL) {
		if (pw->pw_name != NULL) {
			/* TRANSLATORS: this is the username, e.g. hughsie */
			g_print (" %s: %s\n", _("Username"), pw->pw_name);
		}
		if (pw->pw_gecos != NULL) {
			/* TRANSLATORS: this is the users real name, e.g. "Richard Hughes" */
			g_print (" %s: %s\n", _("Real name"), pw->pw_gecos);
		}
	}

	/* TRANSLATORS: these are packages touched by the transaction */
	lines = g_strsplit (obj->data, "\n", -1);
	lines_len = g_strv_length (lines);
	if (lines_len > 0)
		g_print (" %s\n", _("Affected packages:"));
	else
		g_print (" %s\n", _("Affected packages: None"));
	for (i=0; i<lines_len; i++) {
		parts = g_strsplit (lines[i], "\t", 3);

		/* create printable */
		package = pk_package_id_to_printable (parts[1]);
		g_print (" - %s %s\n", parts[0], package);
		g_free (package);
		g_strfreev (parts);
	}
	g_strfreev (lines);
}

/**
 * pk_console_distro_upgrade_cb:
 **/
static void
pk_console_distro_upgrade_cb (const PkItemDistroUpgrade *obj, gpointer user_data)
{
	/* TRANSLATORS: this is the distro, e.g. Fedora 10 */
	g_print ("%s: %s\n", _("Distribution"), obj->name);
	/* TRANSLATORS: this is type of update, stable or testing */
	g_print (" %s: %s\n", _("Type"), pk_update_state_enum_to_text (obj->state));
	/* TRANSLATORS: this is any summary text describing the upgrade */
	g_print (" %s: %s\n", _("Summary"), obj->summary);
}

/**
 * pk_console_category_cb:
 **/
static void
pk_console_category_cb (const PkItemCategory *obj, gpointer user_data)
{
	/* TRANSLATORS: this is the group category name */
	g_print ("%s: %s\n", _("Category"), obj->name);
	/* TRANSLATORS: this is group identifier */
	g_print (" %s: %s\n", _("ID"), obj->cat_id);
	if (obj->parent_id != NULL) {
		/* TRANSLATORS: this is the parent group */
		g_print (" %s: %s\n", _("Parent"), obj->parent_id);
	}
	/* TRANSLATORS: this is the name of the parent group */
	g_print (" %s: %s\n", _("Name"), obj->name);
	if (obj->summary != NULL) {
		/* TRANSLATORS: this is the summary of the group */
		g_print (" %s: %s\n", _("Summary"), obj->summary);
	}
	/* TRANSLATORS: this is preferred icon for the group */
	g_print (" %s: %s\n", _("Icon"), obj->icon);
}

/**
 * pk_console_update_detail_cb:
 **/
static void
pk_console_update_detail_cb (const PkItemUpdateDetail *detail, gpointer data)
{
	gchar *issued;
	gchar *updated;
	gchar *package = NULL;

	/* TRANSLATORS: this is a header for the package that can be updated */
	g_print ("%s\n", _("Details about the update:"));

	/* create printable */
	package = pk_package_id_to_printable (detail->package_id);

	/* TRANSLATORS: details about the update, package name and version */
	g_print (" %s: %s\n", _("Package"), package);
	if (detail->updates != NULL) {
		/* TRANSLATORS: details about the update, any packages that this update updates */
		g_print (" %s: %s\n", _("Updates"), detail->updates);
	}
	if (detail->obsoletes != NULL) {
		/* TRANSLATORS: details about the update, any packages that this update obsoletes */
		g_print (" %s: %s\n", _("Obsoletes"), detail->obsoletes);
	}
	if (detail->vendor_url != NULL) {
		/* TRANSLATORS: details about the update, the vendor URLs */
		g_print (" %s: %s\n", _("Vendor"), detail->vendor_url);
	}
	if (detail->bugzilla_url != NULL) {
		/* TRANSLATORS: details about the update, the bugzilla URLs */
		g_print (" %s: %s\n", _("Bugzilla"), detail->bugzilla_url);
	}
	if (detail->cve_url != NULL) {
		/* TRANSLATORS: details about the update, the CVE URLs */
		g_print (" %s: %s\n", _("CVE"), detail->cve_url);
	}
	if (detail->restart_enum != PK_RESTART_ENUM_NONE) {
		/* TRANSLATORS: details about the update, if the package requires a restart */
		g_print (" %s: %s\n", _("Restart"), pk_restart_enum_to_text (detail->restart_enum));
	}
	if (detail->update_text != NULL) {
		/* TRANSLATORS: details about the update, any description of the update */
		g_print (" %s: %s\n", _("Update text"), detail->update_text);
	}
	if (detail->changelog != NULL) {
		/* TRANSLATORS: details about the update, the changelog for the package */
		g_print (" %s: %s\n", _("Changes"), detail->changelog);
	}
	if (detail->state_enum != PK_UPDATE_STATE_ENUM_UNKNOWN) {
		/* TRANSLATORS: details about the update, the ongoing state of the update */
		g_print (" %s: %s\n", _("State"), pk_update_state_enum_to_text (detail->state_enum));
	}
	issued = pk_iso8601_from_date (detail->issued);
	if (issued != NULL) {
		/* TRANSLATORS: details about the update, date the update was issued */
		g_print (" %s: %s\n", _("Issued"), issued);
	}
	updated = pk_iso8601_from_date (detail->updated);
	if (updated != NULL) {
		/* TRANSLATORS: details about the update, date the update was updated */
		g_print (" %s: %s\n", _("Updated"), updated);
	}
	g_free (issued);
	g_free (updated);
	g_free (package);
}

/**
 * pk_console_repo_detail_cb:
 **/
static void
pk_console_repo_detail_cb (const PkItemRepoDetail *obj, gpointer data)
{
	gchar *enabled_pad;
	gchar *repo_pad;

	if (obj->enabled) {
		/* TRANSLATORS: if the repo is enabled */
		enabled_pad = pk_strpad (_("Enabled"), 10);
	} else {
		/* TRANSLATORS: if the repo is disabled */
		enabled_pad = pk_strpad (_("Disabled"), 10);
	}

	repo_pad = pk_strpad (obj->repo_id, 25);
	g_print (" %s %s %s\n", enabled_pad, repo_pad, obj->description);
	g_free (enabled_pad);
	g_free (repo_pad);
}

/**
 * pk_console_require_restart_cb:
 **/
static void
pk_console_require_restart_cb (const PkItemRequireRestart *obj, gpointer data)
{
	gchar *package = NULL;

	/* create printable */
	package = pk_package_id_to_printable (obj->package_id);

	if (obj->restart == PK_RESTART_ENUM_SYSTEM) {
		/* TRANSLATORS: a package requires the system to be restarted */
		g_print ("%s %s\n", _("System restart required by:"), package);
	} else if (obj->restart == PK_RESTART_ENUM_SESSION) {
		/* TRANSLATORS: a package requires the session to be restarted */
		g_print ("%s %s\n", _("Session restart required:"), package);
	} else if (obj->restart == PK_RESTART_ENUM_SECURITY_SYSTEM) {
		/* TRANSLATORS: a package requires the system to be restarted due to a security update*/
		g_print ("%s %s\n", _("System restart (security) required by:"), package);
	} else if (obj->restart == PK_RESTART_ENUM_SECURITY_SESSION) {
		/* TRANSLATORS: a package requires the session to be restarted due to a security update */
		g_print ("%s %s\n", _("Session restart (security) required:"), package);
	} else if (obj->restart == PK_RESTART_ENUM_APPLICATION) {
		/* TRANSLATORS: a package requires the application to be restarted */
		g_print ("%s %s\n", _("Application restart required by:"), package);
	}
	g_free (package);
}

/**
 * pk_console_details_cb:
 **/
static void
pk_console_details_cb (const PkItemDetails *obj, gpointer data)
{
	gchar *package = NULL;

	/* create printable */
	package = pk_package_id_to_printable (obj->package_id);

	/* TRANSLATORS: This a list of details about the package */
	g_print ("%s\n", _("Package description"));
	g_print ("  package:     %s\n", package);
	g_print ("  license:     %s\n", obj->license);
	g_print ("  group:       %s\n", pk_group_enum_to_text (obj->group_enum));
	g_print ("  description: %s\n", obj->description);
	g_print ("  size:        %lu bytes\n", (long unsigned int) obj->size);
	g_print ("  url:         %s\n", obj->url);

	g_free (package);
}

/**
 * pk_console_message_cb:
 **/
static void
pk_console_message_cb (const PkItemMessage *obj, gpointer data)
{
	/* TRANSLATORS: This a message (like a little note that may be of interest) from the transaction */
	g_print ("%s %s: %s\n", _("Message:"), pk_message_enum_to_text (obj->message), obj->details);
}

/**
 * pk_console_files_cb:
 **/
static void
pk_console_files_cb (PkItemFiles *obj, gpointer data)
{
	guint i;

	/* empty */
	if (obj->files == NULL || obj->files[0] == NULL) {
		/* TRANSLATORS: This where the package has no files */
		g_print ("%s\n", _("No files"));
		return;
	}

	/* TRANSLATORS: This a list files contained in the package */
	g_print ("%s\n", _("Package files"));
	for (i=0; obj->files[i] != NULL; i++) {
		g_print ("  %s\n", obj->files[i]);
	}
}

/**
 * pk_console_progress_cb:
 **/
static void
pk_console_progress_cb (PkProgress *progress, PkProgressType type, gpointer data)
{
	gint percentage;
	PkStatusEnum status;

	/* percentage */
	if (type == PK_PROGRESS_TYPE_PERCENTAGE) {
		g_object_get (progress,
			      "percentage", &percentage,
			      NULL);
		pk_progress_bar_set_percentage (progressbar, percentage);
	}

	/* status */
	if (type == PK_PROGRESS_TYPE_STATUS) {
		g_object_get (progress,
			      "status", &status,
			      NULL);
		if (status == PK_STATUS_ENUM_FINISHED)
			return;
		/* TODO: translate */
		pk_progress_bar_start (progressbar, pk_status_enum_to_text (status));
	}
}

/**
 * pk_console_finished_cb:
 **/
static void
pk_console_finished_cb (GObject *object, GAsyncResult *res, gpointer data)
{
	const PkItemErrorCode *error_item;
	PkResults *results;
	GError *error = NULL;
	GPtrArray *array;
	PkExitEnum exit_enum;
	PkRestartEnum restart;
	PkRoleEnum role;

	/* no more progress */
	pk_progress_bar_end (progressbar);

	/* get the results */
	results = pk_client_generic_finish (PK_CLIENT(task), res, &error);
	if (results == NULL) {
		g_print ("Failed to complete: %s\n", error->message);
		g_error_free (error);
		goto out;
	}

	/* get the role */
	g_object_get (G_OBJECT(results), "role", &role, NULL);

	exit_enum = pk_results_get_exit_code (results);
//	if (exit_enum != PK_EXIT_ENUM_CANCELLED)
//		egg_test_failed (test, "failed to cancel search: %s", pk_exit_enum_to_text (exit_enum));

	/* check error code */
	error_item = pk_results_get_error_code (results);
//	if (error_item->code != PK_ERROR_ENUM_TRANSACTION_CANCELLED)
//		egg_test_failed (test, "failed to get error code: %i", error_item->code);
//	if (g_strcmp0 (error_item->details, "The task was stopped successfully") != 0)
//		egg_test_failed (test, "failed to get error message: %s", error_item->details);

	/* package */
	if (role != PK_ROLE_ENUM_INSTALL_PACKAGES &&
	    role != PK_ROLE_ENUM_UPDATE_PACKAGES &&
	    role != PK_ROLE_ENUM_UPDATE_SYSTEM &&
	    role != PK_ROLE_ENUM_REMOVE_PACKAGES) {
		array = pk_results_get_package_array (results);
		g_ptr_array_foreach (array, (GFunc) pk_console_package_cb, NULL);
		g_ptr_array_unref (array);
	}

	/* transaction */
	array = pk_results_get_transaction_array (results);
	g_ptr_array_foreach (array, (GFunc) pk_console_transaction_cb, NULL);
	g_ptr_array_unref (array);

	/* distro_upgrade */
	array = pk_results_get_distro_upgrade_array (results);
	g_ptr_array_foreach (array, (GFunc) pk_console_distro_upgrade_cb, NULL);
	g_ptr_array_unref (array);

	/* category */
	array = pk_results_get_category_array (results);
	g_ptr_array_foreach (array, (GFunc) pk_console_category_cb, NULL);
	g_ptr_array_unref (array);

	/* update_detail */
	array = pk_results_get_update_detail_array (results);
	g_ptr_array_foreach (array, (GFunc) pk_console_update_detail_cb, NULL);
	g_ptr_array_unref (array);

	/* repo_detail */
	array = pk_results_get_repo_detail_array (results);
	g_ptr_array_foreach (array, (GFunc) pk_console_repo_detail_cb, NULL);
	g_ptr_array_unref (array);

	/* require_restart */
	array = pk_results_get_require_restart_array (results);
	g_ptr_array_foreach (array, (GFunc) pk_console_require_restart_cb, NULL);
	g_ptr_array_unref (array);

	/* details */
	array = pk_results_get_details_array (results);
	g_ptr_array_foreach (array, (GFunc) pk_console_details_cb, NULL);
	g_ptr_array_unref (array);

	/* message */
	array = pk_results_get_message_array (results);
	g_ptr_array_foreach (array, (GFunc) pk_console_message_cb, NULL);
	g_ptr_array_unref (array);

	/* don't print files if we are DownloadPackages */
	if (role != PK_ROLE_ENUM_DOWNLOAD_PACKAGES) {
		array = pk_results_get_files_array (results);
		g_ptr_array_foreach (array, (GFunc) pk_console_files_cb, NULL);
		g_ptr_array_unref (array);
	}

	/* is there any restart to notify the user? */
	restart = pk_results_get_require_restart_worst (results);
	if (restart == PK_RESTART_ENUM_SYSTEM) {
		/* TRANSLATORS: a package needs to restart their system */
		g_print ("%s\n", _("Please restart the computer to complete the update."));
	} else if (restart == PK_RESTART_ENUM_SESSION) {
		/* TRANSLATORS: a package needs to restart the session */
		g_print ("%s\n", _("Please logout and login to complete the update."));
	} else if (restart == PK_RESTART_ENUM_SECURITY_SYSTEM) {
		/* TRANSLATORS: a package needs to restart their system (due to security) */
		g_print ("%s\n", _("Please restart the computer to complete the update as important security updates have been installed."));
	} else if (restart == PK_RESTART_ENUM_SECURITY_SESSION) {
		/* TRANSLATORS: a package needs to restart the session (due to security) */
		g_print ("%s\n", _("Please logout and login to complete the update as important security updates have been installed."));
	}
out:
	if (results != NULL)
		g_object_unref (results);
	g_main_loop_quit (loop);
}

/**
 * pk_console_install_packages:
 **/
static gboolean
pk_console_install_packages (gchar **packages, GError **error)
{
	gboolean ret = TRUE;
	gchar **package_ids;
	GError *error_local = NULL;

	package_ids = pk_console_resolve_packages (PK_CLIENT(task), pk_bitfield_value (PK_FILTER_ENUM_NOT_INSTALLED), packages, &error_local);
	if (package_ids == NULL) {
		/* TRANSLATORS: There was an error getting the list of files for the package. The detailed error follows */
		*error = g_error_new (1, 0, _("This tool could not find the available package: %s"), error_local->message);
		g_error_free (error_local);
		ret = FALSE;
		goto out;
	}

	/* do the async action */
	pk_task_install_packages_async (PK_TASK(task), package_ids, cancellable,
				        (PkProgressCallback) pk_console_progress_cb, NULL,
				        (GAsyncReadyCallback) pk_console_finished_cb, NULL);
out:
	g_strfreev (package_ids);
	return ret;
}

/**
 * pk_console_remove_packages:
 **/
static gboolean
pk_console_remove_packages (gchar **packages, GError **error)
{
	gboolean ret = TRUE;
	gchar **package_ids;
	GError *error_local = NULL;

	package_ids = pk_console_resolve_packages (PK_CLIENT(task), pk_bitfield_value (PK_FILTER_ENUM_INSTALLED), packages, &error_local);
	if (package_ids == NULL) {
		/* TRANSLATORS: There was an error getting the list of files for the package. The detailed error follows */
		*error = g_error_new (1, 0, _("This tool could not find the installed package: %s"), error_local->message);
		g_error_free (error_local);
		ret = FALSE;
		goto out;
	}

	/* do the async action */
	pk_task_remove_packages_async (PK_TASK(task), package_ids, FALSE, FALSE, cancellable,
				       (PkProgressCallback) pk_console_progress_cb, NULL,
				       (GAsyncReadyCallback) pk_console_finished_cb, NULL);
out:
	g_strfreev (package_ids);
	return ret;
}

/**
 * pk_console_download_packages:
 **/
static gboolean
pk_console_download_packages (gchar **packages, const gchar *directory, GError **error)
{
	gboolean ret = TRUE;
	gchar **package_ids;
	GError *error_local = NULL;

	package_ids = pk_console_resolve_packages (PK_CLIENT(task), pk_bitfield_value (PK_FILTER_ENUM_NONE), packages, &error_local);
	if (package_ids == NULL) {
		/* TRANSLATORS: There was an error getting the list of files for the package. The detailed error follows */
		*error = g_error_new (1, 0, _("This tool could not find the package: %s"), error_local->message);
		g_error_free (error_local);
		ret = FALSE;
		goto out;
	}

	/* do the async action */
	pk_client_download_packages_async (PK_CLIENT(task), package_ids, directory, cancellable,
				           (PkProgressCallback) pk_console_progress_cb, NULL,
				           (GAsyncReadyCallback) pk_console_finished_cb, NULL);
out:
	g_strfreev (package_ids);
	return ret;
}

/**
 * pk_console_update_packages:
 **/
static gboolean
pk_console_update_packages (gchar **packages, GError **error)
{
	gboolean ret = TRUE;
	gchar **package_ids;
	GError *error_local = NULL;

	package_ids = pk_console_resolve_packages (PK_CLIENT(task), pk_bitfield_value (PK_FILTER_ENUM_NOT_INSTALLED), packages, &error_local);
	if (package_ids == NULL) {
		/* TRANSLATORS: There was an error getting the list of files for the package. The detailed error follows */
		*error = g_error_new (1, 0, _("This tool could not find the package: %s"), error_local->message);
		g_error_free (error_local);
		ret = FALSE;
		goto out;
	}

	/* do the async action */
	pk_task_update_packages_async (PK_TASK(task), package_ids, cancellable,
				       (PkProgressCallback) pk_console_progress_cb, NULL,
				       (GAsyncReadyCallback) pk_console_finished_cb, NULL);
out:
	g_strfreev (package_ids);
	return ret;
}

/**
 * pk_console_get_requires:
 **/
static gboolean
pk_console_get_requires (PkBitfield filters, gchar **packages, GError **error)
{
	gboolean ret = TRUE;
	gchar **package_ids = NULL;
	GError *error_local = NULL;

	package_ids = pk_console_resolve_packages (PK_CLIENT(task), pk_bitfield_value (PK_FILTER_ENUM_NONE), packages, &error_local);
	if (package_ids == NULL) {
		/* TRANSLATORS: There was an error getting the list of files for the package. The detailed error follows */
		*error = g_error_new (1, 0, _("This tool could not find all the packages: %s"), error_local->message);
		g_error_free (error_local);
		ret = FALSE;
		goto out;
	}

	/* do the async action */
	pk_client_get_requires_async (PK_CLIENT(task), filters, package_ids, TRUE, cancellable,
				      (PkProgressCallback) pk_console_progress_cb, NULL,
				      (GAsyncReadyCallback) pk_console_finished_cb, NULL);
out:
	g_strfreev (package_ids);
	return ret;
}

/**
 * pk_console_get_depends:
 **/
static gboolean
pk_console_get_depends (PkBitfield filters, gchar **packages, GError **error)
{
	gboolean ret = TRUE;
	gchar **package_ids = NULL;
	GError *error_local = NULL;

	package_ids = pk_console_resolve_packages (PK_CLIENT(task), pk_bitfield_value (PK_FILTER_ENUM_NONE), packages, &error_local);
	if (package_ids == NULL) {
		/* TRANSLATORS: There was an error getting the dependencies for the package. The detailed error follows */
		*error = g_error_new (1, 0, _("This tool could not find all the packages: %s"), error_local->message);
		g_error_free (error_local);
		ret = FALSE;
		goto out;
	}

	/* do the async action */
	pk_client_get_depends_async (PK_CLIENT(task), filters, package_ids, FALSE, cancellable,
				     (PkProgressCallback) pk_console_progress_cb, NULL,
				     (GAsyncReadyCallback) pk_console_finished_cb, NULL);
out:
	g_strfreev (package_ids);
	return ret;
}

/**
 * pk_console_get_details:
 **/
static gboolean
pk_console_get_details (gchar **packages, GError **error)
{
	gboolean ret = TRUE;
	gchar **package_ids = NULL;
	GError *error_local = NULL;

	package_ids = pk_console_resolve_packages (PK_CLIENT(task), pk_bitfield_value (PK_FILTER_ENUM_NONE), packages, &error_local);
	if (package_ids == NULL) {
		/* TRANSLATORS: There was an error getting the details about the package. The detailed error follows */
		*error = g_error_new (1, 0, _("This tool could not find all the packages: %s"), error_local->message);
		g_error_free (error_local);
		ret = FALSE;
		goto out;
	}

	/* do the async action */
	pk_client_get_details_async (PK_CLIENT(task), package_ids, cancellable,
				     (PkProgressCallback) pk_console_progress_cb, NULL,
				     (GAsyncReadyCallback) pk_console_finished_cb, NULL);
out:
	g_strfreev (package_ids);
	return ret;
}

/**
 * pk_console_get_files:
 **/
static gboolean
pk_console_get_files (gchar **packages, GError **error)
{
	gboolean ret = TRUE;
	gchar **package_ids = NULL;
	GError *error_local = NULL;

	package_ids = pk_console_resolve_packages (PK_CLIENT(task), pk_bitfield_value (PK_FILTER_ENUM_NONE), packages, &error_local);
	if (package_ids == NULL) {
		/* TRANSLATORS: The package name was not found in any software sources. The detailed error follows */
		*error = g_error_new (1, 0, _("This tool could not find all the packages: %s"), error_local->message);
		g_error_free (error_local);
		ret = FALSE;
		goto out;
	}

	/* do the async action */
	pk_client_get_files_async (PK_CLIENT(task), package_ids, cancellable,
				   (PkProgressCallback) pk_console_progress_cb, NULL,
				   (GAsyncReadyCallback) pk_console_finished_cb, NULL);
out:
	g_strfreev (package_ids);
	return ret;
}

/**
 * pk_console_get_update_detail
 **/
static gboolean
pk_console_get_update_detail (gchar **packages, GError **error)
{
	gboolean ret = TRUE;
	gchar **package_ids = NULL;
	GError *error_local = NULL;

	package_ids = pk_console_resolve_packages (PK_CLIENT(task), pk_bitfield_value (PK_FILTER_ENUM_NOT_INSTALLED), packages, &error_local);
	if (package_ids == NULL) {
		/* TRANSLATORS: The package name was not found in any software sources. The detailed error follows */
		*error = g_error_new (1, 0, _("This tool could not find all the packages: %s"), error_local->message);
		g_error_free (error_local);
		ret = FALSE;
		goto out;
	}

	/* do the async action */
	pk_client_get_update_detail_async (PK_CLIENT(task), package_ids, cancellable,
					   (PkProgressCallback) pk_console_progress_cb, NULL,
					   (GAsyncReadyCallback) pk_console_finished_cb, NULL);
out:
	g_strfreev (package_ids);
	return ret;
}

/**
 * pk_connection_changed_cb:
 **/
static void
pk_connection_changed_cb (PkControl *control_, gboolean connected, gpointer data)
{
	/* if the daemon crashed, don't hang around */
	if (!connected) {
		/* TRANSLATORS: This is when the daemon crashed, and we are up shit creek without a paddle */
		g_print ("%s\n", _("The daemon crashed mid-transaction!"));
		_exit (2);
	}
}

/**
 * pk_console_sigint_cb:
 **/
static void
pk_console_sigint_cb (int sig)
{
	egg_debug ("Handling SIGINT");

	/* restore default ASAP, as the cancels might hang */
	signal (SIGINT, SIG_DFL);

	/* cancel any tasks still running */
	g_cancellable_cancel (cancellable);

	/* kill ourselves */
	egg_debug ("Retrying SIGINT");
	kill (getpid (), SIGINT);
}

/**
 * pk_console_get_summary:
 **/
static gchar *
pk_console_get_summary (void)
{
	GString *string;
	string = g_string_new ("");

	/* TRANSLATORS: This is the header to the --help menu */
	g_string_append_printf (string, "%s\n\n%s\n", _("PackageKit Console Interface"),
				/* these are commands we can use with pkcon */
				_("Subcommands:"));

	/* always */
	g_string_append_printf (string, "  %s\n", "get-actions");
	g_string_append_printf (string, "  %s\n", "get-groups");
	g_string_append_printf (string, "  %s\n", "get-filters");
	g_string_append_printf (string, "  %s\n", "get-transactions");
	g_string_append_printf (string, "  %s\n", "get-time");

	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_SEARCH_NAME) ||
	    pk_bitfield_contain (roles, PK_ROLE_ENUM_SEARCH_DETAILS) ||
	    pk_bitfield_contain (roles, PK_ROLE_ENUM_SEARCH_GROUP) ||
	    pk_bitfield_contain (roles, PK_ROLE_ENUM_SEARCH_FILE))
		g_string_append_printf (string, "  %s\n", "search [name|details|group|file] [data]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_INSTALL_PACKAGES))
		g_string_append_printf (string, "  %s\n", "install [packages]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_INSTALL_FILES))
		g_string_append_printf (string, "  %s\n", "install-local [files]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_DOWNLOAD_PACKAGES))
		g_string_append_printf (string, "  %s\n", "download [directory] [packages]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_INSTALL_SIGNATURE))
		g_string_append_printf (string, "  %s\n", "install-sig [type] [key_id] [package_id]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_REMOVE_PACKAGES))
		g_string_append_printf (string, "  %s\n", "remove [package]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_UPDATE_SYSTEM) ||
	    pk_bitfield_contain (roles, PK_ROLE_ENUM_UPDATE_PACKAGES))
		g_string_append_printf (string, "  %s\n", "update <package>");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_REFRESH_CACHE))
		g_string_append_printf (string, "  %s\n", "refresh");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_RESOLVE))
		g_string_append_printf (string, "  %s\n", "resolve [package]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_UPDATES))
		g_string_append_printf (string, "  %s\n", "get-updates");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_DEPENDS))
		g_string_append_printf (string, "  %s\n", "get-depends [package]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_REQUIRES))
		g_string_append_printf (string, "  %s\n", "get-requires [package]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_DETAILS))
		g_string_append_printf (string, "  %s\n", "get-details [package]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_DISTRO_UPGRADES))
		g_string_append_printf (string, "  %s\n", "get-distro-upgrades");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_FILES))
		g_string_append_printf (string, "  %s\n", "get-files [package]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_UPDATE_DETAIL))
		g_string_append_printf (string, "  %s\n", "get-update-detail [package]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_PACKAGES))
		g_string_append_printf (string, "  %s\n", "get-packages");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_ROLLBACK))
		g_string_append_printf (string, "  %s\n", "rollback");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_REPO_LIST))
		g_string_append_printf (string, "  %s\n", "repo-list");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_REPO_ENABLE))
		g_string_append_printf (string, "  %s\n", "repo-enable [repo_id]");
		g_string_append_printf (string, "  %s\n", "repo-disable [repo_id]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_REPO_SET_DATA))
		g_string_append_printf (string, "  %s\n", "repo-set-data [repo_id] [parameter] [value];");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_WHAT_PROVIDES))
		g_string_append_printf (string, "  %s\n", "what-provides [search]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_ACCEPT_EULA))
		g_string_append_printf (string, "  %s\n", "accept-eula [eula-id]");
	if (pk_bitfield_contain (roles, PK_ROLE_ENUM_GET_CATEGORIES))
		g_string_append_printf (string, "  %s\n", "get-categories");
	return g_string_free (string, FALSE);
}

/**
 * pk_console_get_time_since_action_cb:
 **/
static void
pk_console_get_time_since_action_cb (GObject *object, GAsyncResult *res, gpointer data)
{
	guint time_ms;
	GError *error = NULL;
//	PkControl *control = PK_CONTROL(object);

	/* get the results */
	time_ms = pk_control_get_time_since_action_finish (control, res, &error);
	if (time_ms == 0) {
		/* TRANSLATORS: we keep a database updated with the time that an action was last executed */
		g_print ("%s: %s\n", _("Failed to get the time since this action was last completed"), error->message);
		g_error_free (error);
		goto out;
	}
	g_print ("time is %is\n", time_ms);
out:
	g_main_loop_quit (loop);
}

/**
 * main:
 **/
int
main (int argc, char *argv[])
{
	gboolean ret;
	GError *error = NULL;
	gboolean verbose = FALSE;
	gboolean program_version = FALSE;
	GOptionContext *context;
	gchar *options_help;
	gchar *filter = NULL;
	gchar *summary = NULL;
	const gchar *mode;
	const gchar *value = NULL;
	const gchar *details = NULL;
	const gchar *parameter = NULL;
	PkBitfield groups;
	gchar *text;
	PkBitfield filters = 0;
	gint retval = EXIT_SUCCESS;

	const GOptionEntry options[] = {
		{ "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
			/* TRANSLATORS: command line argument, if we should show debugging information */
			_("Show extra debugging information"), NULL },
		{ "version", '\0', 0, G_OPTION_ARG_NONE, &program_version,
			/* TRANSLATORS: command line argument, just show the version string */
			_("Show the program version and exit"), NULL},
		{ "filter", '\0', 0, G_OPTION_ARG_STRING, &filter,
			/* TRANSLATORS: command line argument, use a filter to narrow down results */
			_("Set the filter, e.g. installed"), NULL},
		{ "nowait", 'n', 0, G_OPTION_ARG_NONE, &nowait,
			/* TRANSLATORS: command line argument, work asynchronously */
			_("Exit without waiting for actions to complete"), NULL},
		{ NULL}
	};

	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	if (! g_thread_supported ())
		g_thread_init (NULL);
	g_type_init ();

	/* do stuff on ctrl-c */
	signal (SIGINT, pk_console_sigint_cb);

	/* check if we are on console */
	if (isatty (fileno (stdout)) == 1)
		is_console = TRUE;

	/* we need the roles early, as we only show the user only what they can do */
	control = pk_control_new ();
	ret = pk_control_get_properties_sync (control, &error);
	if (!ret) {
		/* TRANSLATORS: we failed to contact the daemon */
		g_print ("%s: %s\n", _("Failed to contact PackageKit"), error->message);
		g_error_free (error);
		goto out_last;
	}

	/* get data */
	g_object_get (control,
		      "roles", &roles,
		      NULL);

	summary = pk_console_get_summary ();
	progressbar = pk_progress_bar_new ();
	pk_progress_bar_set_size (progressbar, 25);
	pk_progress_bar_set_padding (progressbar, 20);

	cancellable = g_cancellable_new ();
	context = g_option_context_new ("PackageKit Console Program");
	g_option_context_set_summary (context, summary) ;
	g_option_context_add_main_entries (context, options, NULL);
	g_option_context_parse (context, &argc, &argv, NULL);
	/* Save the usage string in case command parsing fails. */
	options_help = g_option_context_get_help (context, TRUE, NULL);
	g_option_context_free (context);

	/* we are now parsed */
	egg_debug_init (verbose);

	if (program_version) {
		g_print (VERSION "\n");
		goto out_last;
	}

	if (argc < 2) {
		g_print ("%s", options_help);
		retval = PK_EXIT_CODE_SYNTAX_INVALID;
		goto out_last;
	}

	loop = g_main_loop_new (NULL, FALSE);

	/* watch when the daemon aborts */
	g_signal_connect (control, "connection-changed",
			  G_CALLBACK (pk_connection_changed_cb), loop);

	/* create transactions */
	task = pk_task_text_new ();

	/* check filter */
	if (filter != NULL) {
		filters = pk_filter_bitfield_from_text (filter);
		if (filters == 0) {
			/* TRANSLATORS: The user specified an incorrect filter */
			error = g_error_new (1, 0, "%s: %s", _("The filter specified was invalid"), filter);
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
	}
	egg_debug ("filter=%s, filters=%" PK_BITFIELD_FORMAT, filter, filters);

	mode = argv[1];
	if (argc > 2)
		value = argv[2];
	if (argc > 3)
		details = argv[3];
	if (argc > 4)
		parameter = argv[4];

	/* parse the big list */
	if (strcmp (mode, "search") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: a search type can be name, details, file, etc */
			error = g_error_new (1, 0, "%s", _("A search type is required, e.g. name"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;

		} else if (strcmp (value, "name") == 0) {
			if (details == NULL) {
				/* TRANSLATORS: the user needs to provide a search term */
				error = g_error_new (1, 0, "%s", _("A search term is required"));
				retval = PK_EXIT_CODE_SYNTAX_INVALID;
				goto out;
			}
			/* fire off an async request */
			pk_client_search_name_async (PK_CLIENT(task), filters, details, cancellable,
						     (PkProgressCallback) pk_console_progress_cb, NULL,
						     (GAsyncReadyCallback) pk_console_finished_cb, NULL);

		} else if (strcmp (value, "details") == 0) {
			if (details == NULL) {
				/* TRANSLATORS: the user needs to provide a search term */
				error = g_error_new (1, 0, "%s", _("A search term is required"));
				retval = PK_EXIT_CODE_SYNTAX_INVALID;
				goto out;
			}
			/* fire off an async request */
			pk_client_search_details_async (PK_CLIENT(task), filters, details, cancellable,
						        (PkProgressCallback) pk_console_progress_cb, NULL,
						        (GAsyncReadyCallback) pk_console_finished_cb, NULL);

		} else if (strcmp (value, "group") == 0) {
			if (details == NULL) {
				/* TRANSLATORS: the user needs to provide a search term */
				error = g_error_new (1, 0, "%s", _("A search term is required"));
				retval = PK_EXIT_CODE_SYNTAX_INVALID;
				goto out;
			}
			/* fire off an async request */
			pk_client_search_group_async (PK_CLIENT(task), filters, details, cancellable,
						      (PkProgressCallback) pk_console_progress_cb, NULL,
						      (GAsyncReadyCallback) pk_console_finished_cb, NULL);

		} else if (strcmp (value, "file") == 0) {
			if (details == NULL) {
				/* TRANSLATORS: the user needs to provide a search term */
				error = g_error_new (1, 0, "%s", _("A search term is required"));
				retval = PK_EXIT_CODE_SYNTAX_INVALID;
				goto out;
			}
			/* fire off an async request */
			pk_client_search_file_async (PK_CLIENT(task), filters, details, cancellable,
						     (PkProgressCallback) pk_console_progress_cb, NULL,
						     (GAsyncReadyCallback) pk_console_finished_cb, NULL);
		} else {
			/* TRANSLATORS: the search type was provided, but invalid */
			error = g_error_new (1, 0, "%s", _("Invalid search type"));
		}

	} else if (strcmp (mode, "install") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: the user did not specify what they wanted to install */
			error = g_error_new (1, 0, "%s", _("A package name to install is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		nowait = !pk_console_install_packages (argv+2, &error);

	} else if (strcmp (mode, "install-local") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: the user did not specify what they wanted to install */
			error = g_error_new (1, 0, "%s", _("A filename to install is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_task_install_files_async (PK_TASK(task), argv+2, cancellable,
					     (PkProgressCallback) pk_console_progress_cb, NULL,
					     (GAsyncReadyCallback) pk_console_finished_cb, NULL);


	} else if (strcmp (mode, "install-sig") == 0) {
		if (value == NULL || details == NULL || parameter == NULL) {
			/* TRANSLATORS: geeky error, 99.9999% of users won't see this */
			error = g_error_new (1, 0, "%s", _("A type, key_id and package_id are required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_client_install_signature_async (PK_CLIENT(task), PK_SIGTYPE_ENUM_GPG, details, parameter, cancellable,
						   (PkProgressCallback) pk_console_progress_cb, NULL,
						   (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "remove") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: the user did not specify what they wanted to remove */
			error = g_error_new (1, 0, "%s", _("A package name to remove is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		nowait = !pk_console_remove_packages (argv+2, &error);

	} else if (strcmp (mode, "download") == 0) {
		if (value == NULL || details == NULL) {
			/* TRANSLATORS: the user did not specify anything about what to download or where */
			error = g_error_new (1, 0, "%s", _("A destination directory and the package names to download are required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		ret = g_file_test (value, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR);
		if (!ret) {
			/* TRANSLATORS: the directory does not exist, so we can't continue */
			error = g_error_new (1, 0, "%s: %s", _("Directory not found"), value);
			retval = PK_EXIT_CODE_FILE_NOT_FOUND;
			goto out;
		}
		nowait = !pk_console_download_packages (argv+3, value, &error);

	} else if (strcmp (mode, "accept-eula") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: geeky error, 99.9999% of users won't see this */
			error = g_error_new (1, 0, "%s", _("A licence identifier (eula-id) is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_client_accept_eula_async (PK_CLIENT(task), value, cancellable,
					     (PkProgressCallback) pk_console_progress_cb, NULL,
					     (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "rollback") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: geeky error, 99.9999% of users won't see this */
			error = g_error_new (1, 0, "%s", _("A transaction identifier (tid) is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_client_rollback_async (PK_CLIENT(task), value, cancellable,
					  (PkProgressCallback) pk_console_progress_cb, NULL,
					  (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "update") == 0) {
		if (value == NULL) {
			/* do the system update */
			pk_task_update_system_async (PK_TASK(task), cancellable,
						     (PkProgressCallback) pk_console_progress_cb, NULL,
						     (GAsyncReadyCallback) pk_console_finished_cb, NULL);
		} else {
			nowait = !pk_console_update_packages (argv+2, &error);
		}

	} else if (strcmp (mode, "resolve") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: The user did not specify a package name */
			error = g_error_new (1, 0, "%s", _("A package name to resolve is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_client_resolve_async (PK_CLIENT(task), filters, argv+2, cancellable,
				         (PkProgressCallback) pk_console_progress_cb, NULL,
					 (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "repo-enable") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: The user did not specify a repository (software source) name */
			error = g_error_new (1, 0, "%s", _("A repository name is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_client_repo_enable_async (PK_CLIENT(task), value, TRUE, cancellable,
					     (PkProgressCallback) pk_console_progress_cb, NULL,
					     (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "repo-disable") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: The user did not specify a repository (software source) name */
			error = g_error_new (1, 0, "%s", _("A repository name is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_client_repo_enable_async (PK_CLIENT(task), value, FALSE, cancellable,
					     (PkProgressCallback) pk_console_progress_cb, NULL,
					     (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "repo-set-data") == 0) {
		if (value == NULL || details == NULL || parameter == NULL) {
			/* TRANSLATORS: The user didn't provide any data */
			error = g_error_new (1, 0, "%s", _("A repo name, parameter and value are required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_client_repo_set_data_async (PK_CLIENT(task), value, details, parameter, cancellable,
					       (PkProgressCallback) pk_console_progress_cb, NULL,
					       (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "repo-list") == 0) {
		pk_client_get_repo_list_async (PK_CLIENT(task), filters, cancellable,
					       (PkProgressCallback) pk_console_progress_cb, NULL,
					       (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "get-time") == 0) {
		PkRoleEnum role;
		if (value == NULL) {
			/* TRANSLATORS: The user didn't specify what action to use */
			error = g_error_new (1, 0, "%s", _("An action, e.g. 'update-system' is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		role = pk_role_enum_from_text (value);
		if (role == PK_ROLE_ENUM_UNKNOWN) {
			/* TRANSLATORS: The user specified an invalid action */
			error = g_error_new (1, 0, "%s", _("A correct role is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_control_get_time_since_action_async (control, role, cancellable,
							(GAsyncReadyCallback) pk_console_get_time_since_action_cb, NULL);

	} else if (strcmp (mode, "get-depends") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: The user did not provide a package name */
			error = g_error_new (1, 0, "%s", _("A package name is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		nowait = !pk_console_get_depends (filters, argv+2, &error);

	} else if (strcmp (mode, "get-distro-upgrades") == 0) {
		pk_client_get_distro_upgrades_async (PK_CLIENT(task), cancellable,
						     (PkProgressCallback) pk_console_progress_cb, NULL,
						     (GAsyncReadyCallback) pk_console_finished_cb, NULL);


	} else if (strcmp (mode, "get-update-detail") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: The user did not provide a package name */
			error = g_error_new (1, 0, "%s", _("A package name is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		nowait = !pk_console_get_update_detail (argv+2, &error);

	} else if (strcmp (mode, "get-requires") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: The user did not provide a package name */
			error = g_error_new (1, 0, "%s", _("A package name is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		nowait = !pk_console_get_requires (filters, argv+2, &error);

	} else if (strcmp (mode, "what-provides") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: each package "provides" certain things, e.g. mime(gstreamer-decoder-mp3), the user didn't specify it */
			error = g_error_new (1, 0, "%s", _("A package provide string is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		pk_client_what_provides_async (PK_CLIENT(task), filters, PK_PROVIDES_ENUM_CODEC, value, cancellable,
					       (PkProgressCallback) pk_console_progress_cb, NULL,
					       (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "get-details") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: The user did not provide a package name */
			error = g_error_new (1, 0, "%s", _("A package name is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		nowait = !pk_console_get_details (argv+2, &error);

	} else if (strcmp (mode, "get-files") == 0) {
		if (value == NULL) {
			/* TRANSLATORS: The user did not provide a package name */
			error = g_error_new (1, 0, "%s", _("A package name is required"));
			retval = PK_EXIT_CODE_SYNTAX_INVALID;
			goto out;
		}
		nowait = !pk_console_get_files (argv+2, &error);

	} else if (strcmp (mode, "get-updates") == 0) {
		pk_client_get_updates_async (PK_CLIENT(task), filters, cancellable,
					     (PkProgressCallback) pk_console_progress_cb, NULL,
					     (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "get-categories") == 0) {
		pk_client_get_categories_async (PK_CLIENT(task), cancellable,
						(PkProgressCallback) pk_console_progress_cb, NULL,
						(GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "get-packages") == 0) {
		pk_client_get_packages_async (PK_CLIENT(task), filters, cancellable,
					      (PkProgressCallback) pk_console_progress_cb, NULL,
					      (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "get-roles") == 0) {
		text = pk_role_bitfield_to_text (roles);
		g_strdelimit (text, ";", '\n');
		g_print ("%s\n", text);
		g_free (text);
		nowait = TRUE;

	} else if (strcmp (mode, "get-filters") == 0) {
		g_object_get (control,
			      "filters", &filters,
			      NULL);
		text = pk_filter_bitfield_to_text (filters);
		g_strdelimit (text, ";", '\n');
		g_print ("%s\n", text);
		g_free (text);
		nowait = TRUE;

	} else if (strcmp (mode, "get-groups") == 0) {
		g_object_get (control,
			      "groups", &groups,
			      NULL);
		text = pk_group_bitfield_to_text (groups);
		g_strdelimit (text, ";", '\n');
		g_print ("%s\n", text);
		g_free (text);
		nowait = TRUE;

	} else if (strcmp (mode, "get-transactions") == 0) {
		pk_client_get_old_transactions_async (PK_CLIENT(task), 10, cancellable,
						      (PkProgressCallback) pk_console_progress_cb, NULL,
						      (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else if (strcmp (mode, "refresh") == 0) {
		pk_client_refresh_cache_async (PK_CLIENT(task), FALSE, cancellable,
					       (PkProgressCallback) pk_console_progress_cb, NULL,
					       (GAsyncReadyCallback) pk_console_finished_cb, NULL);

	} else {
		/* TRANSLATORS: The user tried to use an unsupported option on the command line */
		error = g_error_new (1, 0, _("Option '%s' is not supported"), mode);
	}

	/* do we wait for the method? */
	if (!nowait && error == NULL)
		g_main_loop_run (loop);

out:
	if (error != NULL) {
//			/* TRANSLATORS: User does not have permission to do this */
//			g_print ("%s\n", _("Incorrect privileges for this operation"));
		/* TRANSLATORS: Generic failure of what they asked to do */
		g_print ("%s:  %s\n", _("Command failed"), error->message);
		if (retval == EXIT_SUCCESS)
			retval = EXIT_FAILURE;
	}

	g_free (options_help);
	g_free (filter);
	g_free (summary);
	g_object_unref (progressbar);
	g_object_unref (control);
	g_object_unref (task);
	g_object_unref (cancellable);
out_last:
	return retval;
}

/***************************************************************************
 ***                          MAKE CHECK TESTS                           ***
 ***************************************************************************/
#ifdef EGG_TEST
#include "egg-test.h"

void
egg_test_console (EggTest *test)
{
	gchar *text_safe;

	if (!egg_test_start (test, "PkConsole"))
		return;

	/************************************************************
	 ****************         Padding          ******************
	 ************************************************************/
	egg_test_title (test, "pad smaller");
	text_safe = pk_strpad ("richard", 10);
	if (g_strcmp0 (text_safe, "richard   ") == 0)
		egg_test_success (test, NULL);
	else
		egg_test_failed (test, "failed the padd '%s'", text_safe);
	g_free (text_safe);

	/************************************************************/
	egg_test_title (test, "pad NULL");
	text_safe = pk_strpad (NULL, 10);
	if (g_strcmp0 (text_safe, "          ") == 0)
		egg_test_success (test, NULL);
	else
		egg_test_failed (test, "failed the padd '%s'", text_safe);
	g_free (text_safe);

	/************************************************************/
	egg_test_title (test, "pad nothing");
	text_safe = pk_strpad ("", 10);
	if (g_strcmp0 (text_safe, "          ") == 0)
		egg_test_success (test, NULL);
	else
		egg_test_failed (test, "failed the padd '%s'", text_safe);
	g_free (text_safe);

	/************************************************************/
	egg_test_title (test, "pad over");
	text_safe = pk_strpad ("richardhughes", 10);
	if (g_strcmp0 (text_safe, "richardhughes") == 0)
		egg_test_success (test, NULL);
	else
		egg_test_failed (test, "failed the padd '%s'", text_safe);
	g_free (text_safe);

	/************************************************************/
	egg_test_title (test, "pad zero");
	text_safe = pk_strpad ("rich", 0);
	if (g_strcmp0 (text_safe, "rich") == 0)
		egg_test_success (test, NULL);
	else
		egg_test_failed (test, "failed the padd '%s'", text_safe);
	g_free (text_safe);
	egg_test_end (test);
}
#endif
