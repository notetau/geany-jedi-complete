/*
 * server.cpp - a Geany plugin to provide code completion (Python) using jedi
 *
 * Copyright (C) 2015 Noto, Yuta <nonotetau(at)gmail(dot)com>
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

#include "server.hpp"

#include <gtk/gtk.h>
#include <geanycc/geanycc.hpp>

namespace geanycc
{
	namespace python
	{
		bool check_and_install_jediserver_script(const char* path)
		{
			if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
				printf("not found jediserver.py... try to install\n");
				// now, jediserver.py doesn't exist, creating it
				// check directory exist
				gchar* dirname = g_path_get_dirname(path);
				gint mkdir_ret = g_mkdir_with_parents(dirname, 0700);
				g_free(dirname);
				if (mkdir_ret == -1) {
					g_critical("Failed to create directory \"%s\"", dirname);
					return false;
				}
				// make jediserver.py
				// define jediserver_py, jediserver_py_len
				#include "data/jediserver_py.hpp"
				GError* error = NULL;
				if (!g_file_set_contents(path, (gchar*)jediserver_py, jediserver_py_len, &error)) {
					g_critical("Failed to save jediserver.py: %s", error->message);
					g_error_free(error);
					return false;
				}
			}
			return true;
		}
	}
}
