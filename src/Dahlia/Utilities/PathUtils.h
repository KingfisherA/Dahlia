#pragma once

#include "Dahlia/Dahlia-API.h"
#include "boost/filesystem.hpp"

namespace DH
{
	class DH_API  PathUtils
	{
	public:
		static void Init()
		{
			boost::filesystem::initial_path();
		}

		static boost::filesystem::path GetAbsolute(const std::string_view relative)
		{
			boost::filesystem::path exeDir = boost::filesystem::initial_path();
			exeDir.append(relative);
			return exeDir;
		}

		// The Assets folder, relative to the initial working directory
		static boost::filesystem::path GetAssetPath()
		{
			return GetAbsolute("Assets");
		}

		static boost::filesystem::path GetAssetPath(const std::string_view relativeAsset)
		{
			return GetAssetPath().append(relativeAsset);
		}
	};
}
