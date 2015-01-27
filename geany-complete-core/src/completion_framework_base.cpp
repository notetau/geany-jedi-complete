/*
 * completion_framework_base.cpp - a core code of Geany completion plugins
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

#include <geanycc/completion_framework_base.hpp>

namespace geanycc
{

void CompletionFrameworkBase::set_completion_option(std::vector<std::string>& options)
{
	if (completion) {
		completion->set_option(options);
	}
}

void CompletionFrameworkBase::complete_async(const char* filename, const char* content, int line,
                                            int col, int flag)
{
	if (completion) {
		completion->complete_async(filename, content, line, col, flag);
	}
}

bool CompletionFrameworkBase::try_get_completion_results(CodeCompletionResults& result)
{
	if (completion) {
		return completion->try_get_results(result);
	} else {
		return false;
	}
}

std::string CompletionFrameworkBase::get_config_file()
{
	std::string config_file = geany_data->app->configdir;
	config_file += G_DIR_SEPARATOR_S "plugins" G_DIR_SEPARATOR_S;
	config_file += get_plugin_name();
	config_file += G_DIR_SEPARATOR_S "config.conf";
	return config_file;
}

}
