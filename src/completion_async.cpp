/*
 * completion_async.cpp - a core code of Geany completion plugins
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

#include <geanycc/completion_async.hpp>
#include <geanycc/completion_base.hpp>
#include <thread>
#include <mutex>
#include <list>
#include <algorithm>
#include <memory>
#include <future>
#include <iostream>
#include <chrono>

namespace geanycc
{
class CodeCompletionAsyncWrapper::CodeCompletionAsyncWrapperImpl
{
   public:
	CodeCompletionAsyncWrapperImpl(CodeCompletionBase* completion)
	{
		this->completion = completion;
	}

	~CodeCompletionAsyncWrapperImpl()
	{
		if (completion) {
			delete completion;
			completion = nullptr;
		}
	}

	void set_option(std::vector<std::string>& options)
	{
		std::lock_guard<std::mutex> lock(comp_mutex);
		completion->set_option(options);
	}
	void complete_async(const char* filename, const char* content, int line, int col, int flag)
	{
		std::string filename_(filename);
		std::string content_(content);

		std::shared_ptr<CodeCompletionResults> p(new CodeCompletionResults());

		std::future<std::shared_ptr<CodeCompletionResults> > f = std::async(
		    std::launch::async, [=](std::shared_ptr<CodeCompletionResults> results) {
			                        std::lock_guard<std::mutex> lock(comp_mutex);

			                        completion->complete(*results, filename_.c_str(),
			                                             content_.c_str(), line, col, flag);
			                        return results;
			                    },
		    p);

		// push front (stack)
		future_list.push_front(std::move(f));
	}

	bool try_get_results(CodeCompletionResults& results)
	{
		if (future_list.empty()) {
			return false;
		} else {
			bool ret = 0;
			auto iter = future_list.begin();

			if (iter->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
				results = *(iter->get());  // copy
				ret = true;
				future_list.erase(iter);

				future_list.remove_if([](std::future<std::shared_ptr<CodeCompletionResults> >& f) {
					return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
				});
			} else {
				ret = false;
			}
			return ret;
		}
	}

   private:
	CodeCompletionBase* completion = nullptr;

	std::list<std::future<std::shared_ptr<CodeCompletionResults> > > future_list;

	std::mutex comp_mutex;
};

CodeCompletionAsyncWrapper::CodeCompletionAsyncWrapper(CodeCompletionBase* completion)
    : pimpl(new CodeCompletionAsyncWrapperImpl(completion))
{
}

CodeCompletionAsyncWrapper::~CodeCompletionAsyncWrapper() { delete pimpl; }

void CodeCompletionAsyncWrapper::set_option(std::vector<std::string>& options)
{
	pimpl->set_option(options);
}

void CodeCompletionAsyncWrapper::complete_async(const char* filename, const char* content, int line,
                                                int col, int flag)
{
	pimpl->complete_async(filename, content, line, col, flag);
}

bool CodeCompletionAsyncWrapper::try_get_results(CodeCompletionResults& results)
{
	return pimpl->try_get_results(results);
}
}
