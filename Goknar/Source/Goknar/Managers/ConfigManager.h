#ifndef __CONFIGMANAGER_H__
#define __CONFIGMANAGER_H__

#include "Goknar/Core.h"

#include <string>
#include <unordered_map>

class GOKNAR_API ConfigManager
{
public:
	ConfigManager() = default;
	~ConfigManager() = default;

	bool ReadFile(const std::string& filePath);

	std::string GetString(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
	int GetInt(const std::string& section, const std::string& key, int defaultValue = 0) const;
	float GetFloat(const std::string& section, const std::string& key, float defaultValue = 0.0f) const;
	bool GetBool(const std::string& section, const std::string& key, bool defaultValue = false) const;

private:
	static std::string Trim(const std::string& str);

	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data_;
};

#endif