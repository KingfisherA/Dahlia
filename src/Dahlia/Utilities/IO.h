#pragma once

#include "Dahlia/Dahlia-API.h"
#include <string>
#include <string_view>

namespace DH::IO
{
	DH_API std::string ReadTextFile(const std::string_view filePath);

	namespace Dir
	{
		DH_API std::string_view GetDirectoryFromPath(const std::string_view path);
	}
}