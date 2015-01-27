/*
 * completion.hpp - a Geany plugin to provide code completion (Python) using jedi
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
class PythonCodeCompletion : public CodeCompletionBase
{
   public:
	PythonCodeCompletion();
	~PythonCodeCompletion();
	void set_option(std::vector<std::string>& options);
	void complete(CodeCompletionResults& result, const char* filename, const char* content,
	              int line, int col, int flag = 0);

   private:
	PythonCodeCompletion(const PythonCodeCompletion&);
	void operator=(const PythonCodeCompletion&);

	class CodeCompletionImpl;
	CodeCompletionImpl* pimpl;
};
}
