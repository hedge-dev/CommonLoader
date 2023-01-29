#pragma once
#include <string>
#include <vector>

// Don't want to include psapi.h all the time
struct ModuleInfo
{
	void* base;
	uint32_t size;
	void* entry_point;
};

namespace CommonLoader
{
	class ApplicationStore
	{
	public:
		static constexpr const char* DEFAULT_SECTION = "Application";
		
		static void SetState(size_t state, size_t value);
		static void SetStateFlag(size_t state, size_t flag, bool set);
		static size_t GetState(size_t state);
		static void Init();
		static const ModuleInfo& GetModule();
		static uint32_t GetUID();
		static const std::wstring& GetStoragePath();
		static bool Load(const std::wstring& path, bool append = false);
		static bool Save();
		static bool Save(const std::wstring& path);

		static bool GetOption(const std::string& key, std::string& out);
		static bool GetOption(const std::string& section, const std::string& key, std::string& out);
		static bool GetOptions(const std::string& section, std::vector<std::pair<const char*, const char*>>& values);

		static void SetOption(const std::string& key, const std::string& value);
		static void SetOption(const std::string& section, const std::string& key, const std::string& value);

		static void ClearSection(const std::string& section);
	};
}
