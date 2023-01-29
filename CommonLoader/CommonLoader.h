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