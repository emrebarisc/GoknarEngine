#ifndef __CONTENTPATHUTILS_H__
#define __CONTENTPATHUTILS_H__

#include <filesystem>
#include <string>

#include "Goknar/Core.h"

namespace ContentPathUtils
{
	inline std::string NormalizePath(const std::string& path)
	{
		if (path.empty())
		{
			return "";
		}

		std::string normalizedPath = std::filesystem::path(path).lexically_normal().generic_string();
		while (normalizedPath.find("//") != std::string::npos)
		{
			normalizedPath.replace(normalizedPath.find("//"), 2, "/");
		}

		if (normalizedPath.rfind("./", 0) == 0)
		{
			normalizedPath = normalizedPath.substr(2);
		}

		return normalizedPath;
	}

	inline std::string EnsureTrailingSlash(const std::string& path)
	{
		if (path.empty() || path.back() == '/')
		{
			return path;
		}

		return path + "/";
	}

	inline bool StartsWith(const std::string& value, const std::string& prefix)
	{
		return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
	}

	inline std::string ToContentRelativePath(const std::string& path)
	{
		std::string normalizedPath = NormalizePath(path);
		if (normalizedPath.empty())
		{
			return normalizedPath;
		}

		const std::string normalizedContentDir = EnsureTrailingSlash(NormalizePath(ContentDir));
		if (!normalizedContentDir.empty() && StartsWith(normalizedPath, normalizedContentDir))
		{
			return normalizedPath.substr(normalizedContentDir.size());
		}

		constexpr const char* contentPrefix = "Content/";
		if (StartsWith(normalizedPath, contentPrefix))
		{
			return normalizedPath.substr(std::char_traits<char>::length(contentPrefix));
		}

		const size_t contentDirectoryPosition = normalizedPath.find("/Content/");
		if (contentDirectoryPosition != std::string::npos)
		{
			return normalizedPath.substr(contentDirectoryPosition + 9);
		}

		return normalizedPath;
	}

	inline std::string ToAbsoluteContentPath(const std::string& path)
	{
		return NormalizePath(EnsureTrailingSlash(NormalizePath(ContentDir)) + ToContentRelativePath(path));
	}
}

#endif
