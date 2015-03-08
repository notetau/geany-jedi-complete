/*
 * completion_framework.hpp - a Geany plugin to provide code completion (Python) using jedi
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

#pragma once
#include <string>
#include <vector>

#include <geanycc/geanycc.hpp>

namespace geanycc
{

class PythonCompletionFramework : public CompletionFrameworkBase
{
   public:
	PythonCompletionFramework();
	virtual ~PythonCompletionFramework();

	const char* get_plugin_name() const { return _("jedi-complete"); }

	bool check_filetype(GeanyFiletype* ft) const;

	/// return true if typed . except for comments and strings, otherwise false.
	bool check_trigger_char(GeanyEditor* editor);

	GtkWidget* create_config_widget(GtkDialog* dialog);

	void load_preferences();
	void updated_preferences();
	void save_preferences();
};

}
