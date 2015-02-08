#pragma once

#include <string>
#include <vector>
#include <geanyplugin.h>

namespace geanycc
{
    namespace util
    {
	// get/set convert function std::vector and keyfile string list
	std::vector<std::string>
	get_vector_from_keyfile_stringlist(GKeyFile* keyfile,
	                                   const char* group, const char* key, GError* error);

	void set_keyfile_stringlist_by_vector(GKeyFile* keyfile,
	                                      const char* group, const char* key,
		                              std::vector<std::string>& value);

	// save keyfile to path
	void save_keyfile(GKeyFile* keyfile, const char* path);
	}
}
