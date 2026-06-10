#ifdef GOKNAR_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <string>

extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace
{
	constexpr wchar_t GPU_PREFERENCE_REGISTRY_KEY[] = L"Software\\Microsoft\\DirectX\\UserGpuPreferences";
	constexpr wchar_t GPU_PREFERENCE_TOKEN[] = L"GpuPreference=";
	constexpr wchar_t HIGH_PERFORMANCE_GPU_PREFERENCE[] = L"GpuPreference=2;";
	constexpr wchar_t RELAUNCHED_ENVIRONMENT_VARIABLE[] = L"GOKNAR_HIGH_PERFORMANCE_GPU_RELAUNCHED";

	std::wstring GetExecutablePath()
	{
		std::wstring path(MAX_PATH, L'\0');

		while (true)
		{
			const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
			if (length == 0)
			{
				return {};
			}

			if (static_cast<size_t>(length) < path.size() - 1)
			{
				path.resize(length);
				return path;
			}

			path.resize(path.size() * 2);
		}
	}

	std::wstring ReadRegistryString(HKEY key, const std::wstring& valueName)
	{
		DWORD type = 0;
		DWORD sizeInBytes = 0;
		LSTATUS result = RegQueryValueExW(key, valueName.c_str(), nullptr, &type, nullptr, &sizeInBytes);
		if (result != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ) || sizeInBytes == 0)
		{
			return {};
		}

		std::wstring value(sizeInBytes / sizeof(wchar_t), L'\0');
		result = RegQueryValueExW(key, valueName.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(value.data()), &sizeInBytes);
		if (result != ERROR_SUCCESS)
		{
			return {};
		}

		while (!value.empty() && value.back() == L'\0')
		{
			value.pop_back();
		}

		return value;
	}

	std::wstring WithHighPerformanceGpuPreference(std::wstring value)
	{
		const size_t preferenceBegin = value.find(GPU_PREFERENCE_TOKEN);
		if (preferenceBegin != std::wstring::npos)
		{
			const size_t preferenceEnd = value.find(L';', preferenceBegin);
			const size_t characterCount = preferenceEnd == std::wstring::npos
				? value.length() - preferenceBegin
				: preferenceEnd - preferenceBegin + 1;

			value.replace(preferenceBegin, characterCount, HIGH_PERFORMANCE_GPU_PREFERENCE);
			return value;
		}

		if (!value.empty() && value.back() != L';')
		{
			value.push_back(L';');
		}

		value += HIGH_PERFORMANCE_GPU_PREFERENCE;
		return value;
	}

	bool EnsureHighPerformanceGpuPreference(const std::wstring& executablePath)
	{
		HKEY key = nullptr;
		LSTATUS result = RegCreateKeyExW(
			HKEY_CURRENT_USER,
			GPU_PREFERENCE_REGISTRY_KEY,
			0,
			nullptr,
			0,
			KEY_READ | KEY_SET_VALUE,
			nullptr,
			&key,
			nullptr);

		if (result != ERROR_SUCCESS)
		{
			return false;
		}

		const std::wstring currentValue = ReadRegistryString(key, executablePath);
		const std::wstring desiredValue = WithHighPerformanceGpuPreference(currentValue);
		if (currentValue == desiredValue)
		{
			RegCloseKey(key);
			return false;
		}

		result = RegSetValueExW(
			key,
			executablePath.c_str(),
			0,
			REG_SZ,
			reinterpret_cast<const BYTE*>(desiredValue.c_str()),
			static_cast<DWORD>((desiredValue.length() + 1) * sizeof(wchar_t)));

		RegCloseKey(key);
		return result == ERROR_SUCCESS;
	}

	bool WasAlreadyRelaunched()
	{
		wchar_t value[2]{};
		return GetEnvironmentVariableW(RELAUNCHED_ENVIRONMENT_VARIABLE, value, static_cast<DWORD>(sizeof(value) / sizeof(value[0]))) > 0;
	}

	void RelaunchWithUpdatedGpuPreference(const std::wstring& executablePath)
	{
		if (WasAlreadyRelaunched() || IsDebuggerPresent())
		{
			return;
		}

		SetEnvironmentVariableW(RELAUNCHED_ENVIRONMENT_VARIABLE, L"1");

		std::wstring commandLine = GetCommandLineW();
		STARTUPINFOW startupInfo{};
		startupInfo.cb = sizeof(startupInfo);

		PROCESS_INFORMATION processInformation{};
		const BOOL created = CreateProcessW(
			executablePath.c_str(),
			commandLine.data(),
			nullptr,
			nullptr,
			FALSE,
			0,
			nullptr,
			nullptr,
			&startupInfo,
			&processInformation);

		if (!created)
		{
			SetEnvironmentVariableW(RELAUNCHED_ENVIRONMENT_VARIABLE, nullptr);
			return;
		}

		CloseHandle(processInformation.hThread);
		CloseHandle(processInformation.hProcess);
		ExitProcess(0);
	}

	struct HighPerformanceGpuPreferenceInitializer
	{
		HighPerformanceGpuPreferenceInitializer()
		{
			// Windows and driver GPU choices are launch-time decisions, so apply the OS preference before engine startup.
			const std::wstring executablePath = GetExecutablePath();
			if (!executablePath.empty() && EnsureHighPerformanceGpuPreference(executablePath))
			{
				RelaunchWithUpdatedGpuPreference(executablePath);
			}
		}
	};

	HighPerformanceGpuPreferenceInitializer highPerformanceGpuPreferenceInitializer;
}

#endif
