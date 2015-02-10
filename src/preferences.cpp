/*
 * preferences.cpp - a Geany plugin to provide code completion (Python) using jedi
 *
 * Copyright (C) 2014-2015 Noto, Yuta <nonotetau(at)gmail(dot)com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <geanycc/geanycc.hpp>
#include <geanycc/utils.hpp>
#include <geanyplugin.h>

#include <vector>
#include <string>
#include <sstream>

#include "completion_framework.hpp"

#include "preferences.hpp"

static struct PrefWidget
{
    GtkWidget* row_text_max_spinbtn;
    GtkWidget* swin_height_max_spinbtn;

    GtkWidget* start_with_dot;
    GtkEntryBuffer* pypath_buffer;
    GtkWidget* port_spinbtn;
} pref_widgets;

static void on_configure_response(GtkDialog* dialog, gint response, gpointer user_data)
{
    if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY) {
	g_print("geany complete: modified preferences\n");
	auto self = (geanycc::PythonCompletionFramework*)user_data;

	JediCompletePluginPref* pref = JediCompletePluginPref::instance();
	// suggestion window
	pref->row_text_max =
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pref_widgets.row_text_max_spinbtn));

	pref->suggestion_window_height_max =
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pref_widgets.swin_height_max_spinbtn));
	// python
	pref->start_completion_with_dot =
	    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_widgets.start_with_dot));

	pref->jedi_server_port =
	    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pref_widgets.port_spinbtn));

	pref->python_path = gtk_entry_buffer_get_text(pref_widgets.pypath_buffer);

	self->save_preferences();
	self->updated_preferences();
    }
}

static void on_click_file_choose_button(GtkButton* button, gpointer user_data)
{
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Select Python binary",
                                                    NULL,
						    GTK_FILE_CHOOSER_ACTION_OPEN,
						    _("_Cancel"),
						    GTK_RESPONSE_CANCEL,
						    _("_Open"),
						    GTK_RESPONSE_ACCEPT,
						    NULL);
    GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
	char* filename = gtk_file_chooser_get_filename(chooser);
	gtk_entry_buffer_set_text(pref_widgets.pypath_buffer, filename, -1);
	g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_click_exec_button(GtkButton* button, gpointer user_data)
{
}

#define GETOBJ(name) GTK_WIDGET(gtk_builder_get_object(builder, name))

namespace geanycc
{

GtkWidget* PythonCompletionFramework::create_config_widget(GtkDialog* dialog)
{
    g_debug("code complete: plugin_configure");
    JediCompletePluginPref* pref = JediCompletePluginPref::instance();

    GError* err = NULL;
    GtkBuilder* builder = gtk_builder_new();
    // here defines prefpy_ui, prefpy_ui_len
#include "data/prefpy_ui.hpp"
    gint ret = gtk_builder_add_from_string(builder, (gchar*)prefpy_ui, prefpy_ui_len, &err);
    if (err) {
	    printf("fail to load preference ui: %s\n", err->message);
	    GtkWidget* vbox = gtk_vbox_new(FALSE, 5);
	    return vbox;
    }

    // suggestion window
    pref_widgets.row_text_max_spinbtn = GETOBJ("spin_rowtextmax");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pref_widgets.row_text_max_spinbtn),
			      pref->row_text_max);

    pref_widgets.swin_height_max_spinbtn = GETOBJ("spin_sugwinheight");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pref_widgets.swin_height_max_spinbtn),
			      pref->suggestion_window_height_max);
    // python
    pref_widgets.start_with_dot = GETOBJ("cbtn_dot");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_widgets.start_with_dot),
				 pref->start_completion_with_dot);

    GtkWidget* pypath_entry = GETOBJ("te_pypath");
    pref_widgets.pypath_buffer = gtk_entry_get_buffer(GTK_ENTRY(pypath_entry));
    gtk_entry_set_text(GTK_ENTRY(pypath_entry), pref->python_path.c_str());

    GtkWidget* file_choose_button = GETOBJ("btn_filesel");
    g_signal_connect(file_choose_button, "clicked", G_CALLBACK(on_click_file_choose_button), NULL);

    pref_widgets.port_spinbtn = GETOBJ("spin_port");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pref_widgets.port_spinbtn),
			      pref->jedi_server_port);

    GtkWidget* reinstall_button = GETOBJ("btn_reinstall");
    g_signal_connect(reinstall_button, "clicked", G_CALLBACK(on_click_exec_button), NULL);

    g_signal_connect(dialog, "response", G_CALLBACK(on_configure_response), this);
    GtkWidget* vbox = GETOBJ("box_prefpy");
    return vbox;
}

void PythonCompletionFramework::load_preferences()
{
    JediCompletePluginPref* pref = JediCompletePluginPref::instance();

    std::string config_file = get_config_file();

    // Initialising options from config file
    GKeyFile* keyfile = g_key_file_new();
    if (g_key_file_load_from_file(keyfile, config_file.c_str(), G_KEY_FILE_NONE, NULL)) {
	const char* group = get_plugin_name();

	pref->row_text_max =
	    g_key_file_get_integer(keyfile, group, "maximum_char_in_row", NULL);
	pref->suggestion_window_height_max =
	    g_key_file_get_integer(keyfile, group, "maximum_sug_window_height", NULL);
	pref->start_completion_with_dot =
	    g_key_file_get_boolean(keyfile, group, "start_completion_with_dot", NULL);
	pref->jedi_server_port =
	    g_key_file_get_integer(keyfile, group, "server_port", NULL);
	gchar* temp;
	temp = g_key_file_get_string(keyfile, group, "python_path", NULL);
	pref->python_path = temp;
	g_free(temp);
    } else {
	pref->row_text_max = 120;
	pref->suggestion_window_height_max = 300;
	pref->start_completion_with_dot = true;
	pref->jedi_server_port = 8080;
	pref->python_path = "/usr/bin/python";
    }
    // hidden preference "server_script_dir"
    gchar* dirname = g_path_get_dirname(config_file.c_str());
    pref->server_script_dir = dirname;
    g_free(dirname);

    g_key_file_free(keyfile);

    this->updated_preferences();
}

void PythonCompletionFramework::save_preferences()
{
    std::string config_file = get_config_file();
    GKeyFile* keyfile = g_key_file_new();
    const char* group = get_plugin_name();
    JediCompletePluginPref* pref = JediCompletePluginPref::instance();

    g_key_file_set_integer(keyfile, group, "maximum_char_in_row", pref->row_text_max);
    g_key_file_set_integer(keyfile, group, "maximum_sug_window_height",
			   pref->suggestion_window_height_max);
    g_key_file_set_boolean(keyfile, group, "start_completion_with_dot",
			   pref->start_completion_with_dot);
    g_key_file_set_integer(keyfile, group, "server_port", pref->jedi_server_port);
    g_key_file_set_string(keyfile, group, "python_path", pref->python_path.c_str());

    geanycc::util::save_keyfile(keyfile, config_file.c_str());

    g_key_file_free(keyfile);
}

void PythonCompletionFramework::updated_preferences()
{
    JediCompletePluginPref* pref = JediCompletePluginPref::instance();
    //itoa
    char port_str[10] = {0};
    sprintf(port_str, "%d", pref->jedi_server_port);
    std::vector<std::string> options = {
	pref->python_path,
	pref->server_script_dir,
	std::string(port_str),
    };
    this->set_completion_option(options);
    if (this->suggestion_window) {
	    this->suggestion_window->set_max_char_in_row(pref->row_text_max);
	    this->suggestion_window->set_max_window_height(pref->suggestion_window_height_max);
    }
}
}
