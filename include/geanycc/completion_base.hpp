/*
 * completion.hpp - a core code of Geany completion plugins
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

namespace geanycc
{
enum CompleteResultType {
	COMPLETE_RESULT_VAR,
	COMPLETE_RESULT_FUNCTION,
	COMPLETE_RESULT_CLASS,
	COMPLETE_RESULT_METHOD,
	COMPLETE_RESULT_MEMBER,
	COMPLETE_RESULT_STRUCT,
	COMPLETE_RESULT_NAMESPACE,
	COMPLETE_RESULT_MACRO,
	COMPLETE_RESULT_OTHER,
	COMPLETE_RESULT_NONE,
};

enum CompleteResultAvailability {
	COMPLETE_RESULT_AVAIL_AVAIL,
	COMPLETE_RESULT_AVAIL_DEPRECATED,
	COMPLETE_RESULT_AVAIL_NOTAVAIL,
	COMPLETE_RESULT_AVAIL_NOTACCESS
};

struct CompleteResultRow
{
	CompleteResultType type;
	CompleteResultAvailability availability;
	std::string typed_text;
	std::string return_type;
	std::string arguments;
	std::string signature;
	CompleteResultRow()
	{
		typed_text.reserve(100);
		return_type.reserve(100);
		arguments.reserve(200);
		signature.reserve(400);
	}
};

typedef std::vector<CompleteResultRow> CodeCompletionResults;

class CodeCompletionBase
{
   public:
	CodeCompletionBase() {}
	virtual ~CodeCompletionBase() {}
	virtual void set_option(std::vector<std::string>& options) = 0;
	virtual void complete(CodeCompletionResults& result, const char* filename, const char* content,
	                      int line, int col, int flag = 0) = 0;
};
}
