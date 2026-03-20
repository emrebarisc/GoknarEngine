#include "pch.h"
#include "ConfigManager.h"

#include <fstream>
#include <algorithm>
#include <cctype>

std::string ConfigManager::Trim(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) return "";
	
	size_t last = str.find_last_not_of(" \t\r\n");
	return str.substr(first, (last - first + 1));
}

bool ConfigManager::ReadFile(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		return false;
	}

	std::string line;
	std::string currentSection = "Default";

	while (std::getline(file, line))
	{
		line = Trim(line);

		if (line.empty() || line[0] == ';' || line[0] == '#')
		{
			continue;
		}

		if (line.front() == '[' && line.back() == ']')
		{
			currentSection = line.substr(1, line.size() - 2);
			currentSection = Trim(currentSection);
		}
		else
		{
			size_t delimiterPos = line.find('=');
			if (delimiterPos != std::string::npos)
			{
				std::string key = Trim(line.substr(0, delimiterPos));
				std::string value = Trim(line.substr(delimiterPos + 1));
				
				data_[currentSection][key] = value;
			}
		}
	}

	file.close();
	return true;
}

std::string ConfigManager::GetString(const std::string& section, const std::string& key, const std::string& defaultValue) const
{
	auto secIt = data_.find(section);
	if (secIt != data_.end())
	{
		auto keyIt = secIt->second.find(key);
		if (keyIt != secIt->second.end())
		{
			return keyIt->second;
		}
	}
	return defaultValue;
}

int ConfigManager::GetInt(const std::string& section, const std::string& key, int defaultValue) const
{
	std::string val = GetString(section, key);
	if (!val.empty())
	{
		try { return std::stoi(val); } catch (...) {}
	}
	return defaultValue;
}

float ConfigManager::GetFloat(const std::string& section, const std::string& key, float defaultValue) const
{
	std::string val = GetString(section, key);
	if (!val.empty())
	{
		try { return std::stof(val); } catch (...) {}
	}
	return defaultValue;
}

bool ConfigManager::GetBool(const std::string& section, const std::string& key, bool defaultValue) const
{
	std::string val = GetString(section, key);
	if (!val.empty())
	{
		std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) { return std::tolower(c); });
		
		if (val == "true" || val == "1" || val == "yes") return true;
		if (val == "false" || val == "0" || val == "no") return false;
	}
	return defaultValue;
}