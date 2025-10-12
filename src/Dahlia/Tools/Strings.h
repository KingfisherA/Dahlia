#pragma once

#include "Dahlia/Dahlia-API.h"
#include <vector>
#include <string>
#include <string_view>

namespace DH::Strings
{
	DH_API bool Replace(std::string &str, const std::string_view from, const std::string_view to);

	DH_API std::vector<std::string> Split(const std::string_view s,
								   const char seperator);

	DH_API std::vector<std::string> Split(const std::string_view s,
								   const std::string_view seperator);

	DH_API void FindAll(const std::string_view str, const std::string_view searchPhrase, std::vector<size_t> &results);
}