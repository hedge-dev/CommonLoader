#pragma once
#include <string>

struct _MODULEINFO;

namespace CommonLoader
{
	class ApplicationStore
	{
	public:
		static void Init();
		static const _MODULEINFO& GetModule();
		static uint32_t GetUID();
		static const std::wstring& GetStoragePath();
		static bool Load(const std::wstring& path, bool append = false);
		static bool Save();
		static bool Save(const std::wstring& path);

		static bool GetOption(const std::string& key, std::string& out);
		static bool GetOption(const std::string& section, const std::string& key, std::string& out);

		static void SetOption(const std::string& key, const std::string& value);
		static void SetOption(const std::string& section, const std::string& key, const std::string& value);

		static void ClearSection(const std::string& section);
	};
}
