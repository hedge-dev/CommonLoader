#pragma once

namespace CommonLoader 
{
	void Init();
	bool LoadAssembly(const char* path);
	void RaiseInitializers();
	void RaiseUpdates();
}