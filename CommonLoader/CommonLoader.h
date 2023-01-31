#pragma once

struct CommonLoaderAPI;
namespace CommonLoader 
{
	extern const CommonLoaderAPI api_table;

	void Init();
	bool LoadAssembly(const char* path);
	void RaiseInitializers();
	void RaiseUpdates();
	const CommonLoaderAPI* GetAPI();
}

#define CMN_LOADER_DEFINE_API_EXPORT extern "C" __declspec(dllexport) const CommonLoaderAPI* __cdecl CommonLoader_GetAPIPointer() { return CommonLoader::GetAPI(); }