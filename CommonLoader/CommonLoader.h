#pragma once

namespace CommonLoader 
{
	static bool InitializeAssemblyLoader(const char* path);
	static void RaiseInitializers();
	static void RaiseUpdates();
}