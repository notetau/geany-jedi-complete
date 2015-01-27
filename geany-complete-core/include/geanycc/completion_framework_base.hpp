/*
 * completion_framework.hpp - a core code of Geany completion plugins
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

#include "cc_plugin.hpp"
#include "completion_base.hpp"
#include "completion_async.hpp"
#include "suggestion_window.hpp"

namespace geanycc
{

class CompletionFrameworkBase
{
   protected:
	CodeCompletionAsyncWrapper* completion = nullptr;
	SuggestionWindow* suggestion_window = nullptr;

   public:
	CompletionFrameworkBase() {}
	virtual ~CompletionFrameworkBase() {}

	virtual const char* get_plugin_name() const = 0;

	virtual bool check_filetype(GeanyFiletype* ft) const = 0;
	/**
	    return true if typed . -> :: except for comments and strings, otherwise false.
	 */
	virtual bool check_trigger_char(GeanyEditor* editor) = 0;

	virtual GtkWidget* create_config_widget(GtkDialog* dialog) = 0;

	virtual void load_preferences() = 0;
	virtual void updated_preferences() = 0;
	virtual void save_preferences() = 0;

	void set_completion_option(std::vector<std::string>& options);
	void complete_async(const char* filename, const char* content, int line, int col, int flag = 0);
	bool try_get_completion_results(CodeCompletionResults& result);

	void set_suggestion_window(SuggestionWindow* window) { suggestion_window = window; }

	std::string get_config_file();
};

}
