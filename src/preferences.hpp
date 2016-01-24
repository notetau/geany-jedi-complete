/*
 * preferences.hpp - a Geany plugin to provide code completion (Python) using jedi
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

struct JediCompletePluginPref
{
	bool start_completion_with_dot;
	int row_text_max;
	int suggestion_window_height_max;
	int page_up_down_skip_amount;
	std::string python_path;
	int jedi_server_port;
	std::string server_script_dir; // hidden preference

	static JediCompletePluginPref* instance()
	{
		static JediCompletePluginPref instance_;
		return &instance_;
	}

   private:
	JediCompletePluginPref() = default;
	explicit JediCompletePluginPref(JediCompletePluginPref const&) = delete;
	JediCompletePluginPref& operator=(JediCompletePluginPref const&) = delete;
	explicit JediCompletePluginPref(JediCompletePluginPref&&) = delete;
	JediCompletePluginPref& operator=(JediCompletePluginPref&&) = delete;

   public:
	~JediCompletePluginPref() = default;
};
