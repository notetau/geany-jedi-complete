/*
 * plugin_info.cpp - a Geany plugin to provide code completion (Python) using jedi
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

extern "C" {
PLUGIN_VERSION_CHECK(211)

PLUGIN_SET_INFO(
	_("jedi-complete"),
	_("python code completion by jedi"),
	_("0.01"),
	_("Noto, Yuta <nonotetau@gmail.com>")
)
}

#include "completion_framework.hpp"
namespace geanycc
{
CompletionFrameworkBase* create_lang_completion_framework()
{
	return new PythonCompletionFramework();
}
}
