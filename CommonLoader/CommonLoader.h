#pragma once
#include <filesystem>

struct CommonLoaderAPI;
namespace clrhost
{
	class clr_context;
}

namespace CommonLoader 
{
	extern const CommonLoaderAPI api_table;
	extern std::unique_ptr<clrhost::clr_context> clr;

	void Init();
	bool LoadAssembly(const std::filesystem::path& path);
	void RaiseInitializers();
	void RaiseUpdates();
	const CommonLoaderAPI* GetAPI();
}

#define CMN_LOADER_DEFINE_API_EXPORT extern "C" __declspec(dllexport) const CommonLoaderAPI* __cdecl CommonLoader_GetAPIPointer() { return CommonLoader::GetAPI(); }