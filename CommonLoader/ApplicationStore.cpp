#pragma unmanaged

#include "ApplicationStore.h"
#include "CommonLoaderAPI.h"
#include <string>
#include <shlwapi.h>
#include <ShlObj.h>
#include <filesystem>
#include <unordered_map>
#include <Psapi.h>

ModuleInfo module_info{ nullptr, 0, nullptr };
uint32_t app_uid;
std::wstring app_storage_path{};
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> app_config{};
size_t states[CMN_LOADER_STATE_MAX + 1]{};

namespace CommonLoader
{
	void ApplicationStore::SetState(size_t state, size_t value)
	{
		if (state > CMN_LOADER_STATE_MAX)
			return;

		states[state] = value;
	}

	void ApplicationStore::SetStateFlag(size_t state, size_t flag, bool set)
	{
		if (state > CMN_LOADER_STATE_MAX)
			return;

		if (set)
		{
			states[state] |= flag;
		}
		else
		{
			states[state] &= ~flag;
		}
	}

	size_t ApplicationStore::GetState(size_t state)
	{
		if (state > CMN_LOADER_STATE_MAX)
			return CMN_LOADER_STATE_INVALID;

		return states[state];
	}

	void ApplicationStore::Init()
	{
		app_config = {};
		wchar_t szPath[2048];
		const auto& module = GetModule();

		const size_t headerSize = ((IMAGE_DOS_HEADER*)module.base)->e_lfanew + sizeof(IMAGE_NT_HEADERS);
		app_uid = XXH32(module.base, headerSize, 0);
		if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
		{
			PathAppendW(szPath, L"\\NeverFinishAnything\\CommonLoader\\.storage\\");
			SHCreateDirectoryExW(NULL, szPath, NULL);
			app_storage_path = szPath;

			ZeroMemory(szPath, sizeof(szPath));
			_ui64tow(app_uid, szPath, 16);

			app_storage_path += szPath;
			app_storage_path += L".ini";
			if (!Load(app_storage_path))
			{
				GetModuleFileNameW(NULL, szPath, sizeof(szPath));
				std::wstring name = PathFindFileNameW(szPath);
				SetOption("Name", std::string(name.begin(), name.end()));
				Save();
			}
		}
	}

	const ModuleInfo& ApplicationStore::GetModule()
	{
		if (module_info.size)
			return module_info;

		ZeroMemory(&module_info, sizeof(MODULEINFO));
		GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), reinterpret_cast<MODULEINFO*>(&module_info), sizeof(MODULEINFO));

		return module_info;
	}

	uint32_t ApplicationStore::GetUID()
	{
		return app_uid;
	}

	const std::wstring& ApplicationStore::GetStoragePath()
	{
		return app_storage_path;
	}

	bool ApplicationStore::Load(const std::wstring& path, bool append)
	{
		if (!append)
		{
			app_config.clear();
		}

		HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
		if (!hFile)
		{
			return false;
		}

		DWORD dwFileSize = GetFileSize(hFile, NULL);
		char* buffer = new char[dwFileSize + 1];
		if (!ReadFile(hFile, buffer, dwFileSize, &dwFileSize, NULL))
		{
			CloseHandle(hFile);
			return false;
		}
		CloseHandle(hFile);

		ini_t* ini = ini_load(buffer, nullptr);
		delete[] buffer;
		if (!ini)
		{
			return false;
		}
		
		for (size_t i = 0; i < ini_section_count(ini); i++)
		{
			const char* section = ini_section_name(ini, i);
			std::unordered_map<std::string, std::string> section_map{};
			for (size_t j = 0; j < ini_property_count(ini, i); j++)
			{
				const char* key = ini_property_name(ini, i, j);
				const char* value = ini_property_value(ini, i, j);
				section_map[key] = value;
			}
			app_config[section] = section_map;
		}

		ini_destroy(ini);
		return true;
	}

	bool ApplicationStore::Save()
	{
		return Save(GetStoragePath());
	}

	bool ApplicationStore::Save(const std::wstring& path)
	{
		HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (!hFile)
		{
			return false;
		}

		ini_t* ini = ini_create(nullptr);

		for (auto& section : app_config)
		{
			const int sid = ini_section_add(ini, section.first.c_str(), section.first.length());
			for (auto& property : section.second)
			{
				ini_property_add(ini, sid, property.first.c_str(), property.first.length(), property.second.c_str(), property.second.length());
			}
		}

		const int bufSize = ini_save(ini, nullptr, 0);
		char* buffer = new char[bufSize + 1];

		ini_save(ini, buffer, bufSize + 1);
		ini_destroy(ini);

		if (!WriteFile(hFile, buffer, bufSize - 1, nullptr, nullptr))
		{
			CloseHandle(hFile);
			delete[] buffer;
			return false;
		}

		CloseHandle(hFile);
		delete[] buffer;
		return true;
	}

	bool ApplicationStore::GetOption(const std::string& key, std::string& out)
	{
		return GetOption(DEFAULT_SECTION, key, out);
	}

	bool ApplicationStore::GetOption(const std::string& section, const std::string& key, std::string& out)
	{
		if (app_config.find(section) == app_config.end())
		{
			return false;
		}

		const auto result = app_config[section].find(key);
		if (result == app_config[section].end())
		{
			return false;
		}

		out = result->second;
		return true;
	}

	bool ApplicationStore::GetOptions(const std::string& section, std::vector<std::pair<const char*, const char*>>& values)
	{
		if (app_config.find(section) == app_config.end())
		{
			return false;
		}

		const auto& s = app_config[section];
		values.reserve(s.size());
		for (const auto& item : s)
		{
			values.emplace_back(item.first.c_str(), item.second.c_str());
		}

		return true;
	}

	void ApplicationStore::SetOption(const std::string& key, const std::string& value)
	{
		SetOption("Application", key, value);
	}

	void ApplicationStore::SetOption(const std::string& section, const std::string& key, const std::string& value)
	{
		if (app_config.find(section) == app_config.end())
		{
			app_config.insert({ section, std::unordered_map<std::string, std::string>() });
		}

		app_config[section].insert_or_assign(key, value);
	}

	void ApplicationStore::ClearSection(const std::string& section)
	{
		app_config.erase(section);
	}
}
