/*
 * completion_framework.cpp - a Geany plugin to provide code completion (Python) using jedi
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

#include "completion_framework.hpp"

#include "preferences.hpp"
#include "completion.hpp"

#include <SciLexer.h>

namespace geanycc
{

PythonCompletionFramework::PythonCompletionFramework()
{
	completion = new CodeCompletionAsyncWrapper(new PythonCodeCompletion());
}

PythonCompletionFramework::~PythonCompletionFramework()
{
	if (completion) {
		delete completion;
		completion = NULL;
	}
}

bool PythonCompletionFramework::check_filetype(GeanyFiletype* ft) const
{
	if (ft == NULL) { return false; }
	return (ft->id == GEANY_FILETYPES_PYTHON);
}

bool PythonCompletionFramework::check_trigger_char(GeanyEditor* editor)
{
	int pos = sci_get_current_position(editor->sci);
	if (pos < 2) {
		return false;
	}

	char c1 = sci_get_char_at(editor->sci, pos - 1);
	JediCompletePluginPref* pref = JediCompletePluginPref::instance();

	int style_id = sci_get_style_at(editor->sci, pos);
	switch (style_id) {
		case SCE_P_COMMENTLINE: case SCE_P_COMMENTBLOCK:
		case SCE_P_STRING:      case SCE_P_STRINGEOL:
			return false;
	}

	if (pref->start_completion_with_dot) {
		if (c1 == '.') {
			int c0_style_id = sci_get_style_at(editor->sci, pos - 1);
			if (c0_style_id == SCE_P_NUMBER) {
				return false;
			}
			/* TODO ignore 0 omitted floating number such as ".123" */
			return true;
		}
	}
	return false;
}

}
