/*
 * completion.cpp - a Geany plugin to provide code completion (Python) using jedi
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

#include "completion.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <cstring>
#include <map>
#include <curl/curl.h>
#include <gtk/gtk.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

namespace geanycc
{

class PythonCodeCompletion::CodeCompletionImpl
{
	std::map<std::string, CompleteResultType> type_map;
	std::string activate_path;

	GError* spawn_error = NULL;
	std::string port_str = "8080";

	GPid serve_pid;
public:
	CodeCompletionImpl() :
	type_map({
		{"function",  COMPLETE_RESULT_FUNCTION  },
		{"class",     COMPLETE_RESULT_CLASS     },
 		{"statement", COMPLETE_RESULT_VAR       },
 		{"instance",  COMPLETE_RESULT_VAR       },
 		{"param",     COMPLETE_RESULT_VAR       },
 		{"module",    COMPLETE_RESULT_NAMESPACE },
 		{"import",    COMPLETE_RESULT_NAMESPACE },
		})
	{
		std::cout<<"created jedicomplete"<<std::endl;
		serve_pid = 0;
	}

	~CodeCompletionImpl()
	{
		clean_jedi_server();
		std::cout<<"destroying jedicomplete"<<std::endl;
	}

	void set_option(std::vector<std::string>& options)
	{
		clean_jedi_server();
		// we expect options ...
		// options[0] = python path
		// options[1] = directory puts serverscript (jediserver.py)
		// options[2] = server port
		if (options.size() != 3) {
			std::cerr<<"geany-jedi-complete: invalid options"<<std::endl;
			return;
		}

		const char* python_path = options[0].c_str();
		std::string dir = options[1];
		std::string script_path = dir + "/jediserver.py";
		if (!check_and_set_jediserver_script(script_path.c_str())) {
			return; // happen critical error
		}
		port_str = options[2];

		std::cout<<"option "<<python_path<<std::endl;
		std::cout<<"option "<<dir<<std::endl;
		std::cout<<"option "<<port_str<<std::endl;

		serve_pid = create_jedi_server(python_path, script_path.c_str());
		std::cout<<"start jedi server pid:"<< serve_pid <<std::endl;
	}

	bool check_and_set_jediserver_script(const char* script_path)
	{
		printf("test jediserver exists\n");
		if (!g_file_test(script_path, G_FILE_TEST_EXISTS)) {
			printf("not found jediserver.py\n");
			// now, jediserver.py doesn't exist, creating it
			// check directory exist
			gchar* dirname = g_path_get_dirname(script_path);
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
			if (!g_file_set_contents(script_path,
			                         (gchar*)jediserver_py, jediserver_py_len, &error)) {
				g_critical("Failed to save jediserver.py: %s", error->message);
				g_error_free(error);
				return false;
			}
		}
		return true;
	}
	static void spawn_pre_action(gpointer) {}

	static void watch_child(GPid pid, gint status, gpointer self_)
	{
		printf("closing child\n");
		g_spawn_close_pid(pid);
		auto self = (CodeCompletionImpl*)self_;
		if (self->serve_pid == pid) { // if new serve_pid set, keep it,
			self->serve_pid = 0;      // otherwise empty(0) set.
		}
	}

	GPid create_jedi_server(const char* python_path, const char* script_path)
	{
		const char* port = port_str.c_str();
		GPid pid;
		gchar* argv[] = {(char*)python_path, (char*)script_path, (char*)port, NULL};
		gchar* envp[] = {NULL};
		if (!g_spawn_async("/", argv, envp, G_SPAWN_DO_NOT_REAP_CHILD,
					  spawn_pre_action, NULL, &pid, &spawn_error)) {
			printf("fail to create jedi server :%s\n", spawn_error->message);
			g_error_free(spawn_error);
			spawn_error = NULL;
			return 0;
		}
		g_child_watch_add(pid, watch_child, this);
		return pid;
	}

	void clean_jedi_server()
	{
		if (serve_pid != 0) {
			printf("killing child\n");
			//POSIX, GPid == pid
			kill(serve_pid , SIGINT); // raise KeyboardInterrupt()
			//kill(serve_pid , SIGKILL);
			//Windows, GPid == HANDLE
			// TerminateProcess(id, 0);
		}
	}

	static size_t write_data_callback(char* buffer, size_t size, size_t nmemb, std::string* content)
	{
		int segsize = size * nmemb;
		content->append(buffer, segsize);
		return segsize;
	}

	void complete(CodeCompletionResults& result,
		const char* filename, const char* content, int line, int col, int flag)
	{
		result.clear();
		if(serve_pid == 0) { return; }
		printf("server process id %d\n", serve_pid);
		std::string post_data = "";
		char buf[1024];
		sprintf(buf, "%d,%d,%s\r\n", line, col-1, filename); // jedi-column start index is 0
		printf("complete:%s\n", buf);
		post_data += buf;
		post_data += content;
		// setup http request to complete using cURL
		CURL* curl = curl_easy_init();
		if (!curl) {
			printf("couldn't init curl\n");
			return;
		}

		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: text/plain");

		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		char url_str_buf[256];
		sprintf(url_str_buf, "http://localhost:%s/complete", port_str.c_str());
		std::cout<<"send url "<<url_str_buf<<std::endl;
		curl_easy_setopt(curl, CURLOPT_URL, url_str_buf);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.length());

		std::string result_str;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (std::string*)&result_str);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_callback);

		CURLcode ret = curl_easy_perform(curl);
		printf("server ret = %d\n", ret);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		if(ret != 0) {
			return;
		}

		// parse completion result
		std::istringstream stream(result_str);
		std::string temp;
		while(std::getline(stream, temp)) {
			if(temp[temp.length()-1] == '\r') { temp[temp.length()-1] = '\0'; }

			std::istringstream stream2(temp);
			std::string arg[3];
			int arg_count = 0;
			// receive $-separated values such as "type$typed_text$arguement"
			for(int i=0; i<3; i++) { std::getline(stream2, arg[i], '$'); };
			if(!arg[1].empty()) {
				CompleteResultRow row;
				row.type = type_map[arg[0]];
				row.availability = COMPLETE_RESULT_AVAIL_AVAIL;
				row.return_type = "?";
				row.typed_text  = std::move(arg[1]);
				row.arguments   = std::move(arg[2]);
				row.signature   = row.typed_text;
				row.signature  += " ";
				row.signature  += row.arguments;
				result.push_back(row);
			}
		}
		printf("complete %d\n", result.size());
	}

private:
	CodeCompletionImpl(const CodeCompletionImpl&);
	void operator=(const CodeCompletionImpl&);
};


// PythonCodeCompletion //////////////////////////////////////////////////////////////
PythonCodeCompletion::PythonCodeCompletion() : pimpl(new CodeCompletionImpl()) {}
PythonCodeCompletion::~PythonCodeCompletion() { delete pimpl; }

void PythonCodeCompletion::set_option(std::vector<std::string>& options)
{
	pimpl->set_option(options);
}

void PythonCodeCompletion::complete(CodeCompletionResults& result,
	const char* filename, const char* content, int line, int col, int flag)
{
	pimpl->complete(result, filename, content, line, col, flag);
}

}
