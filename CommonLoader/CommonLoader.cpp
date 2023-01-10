#include "ManagedCommonLoader.h"
#include "AssemblyLoader.h"
#include "CommonLoader.h"

bool CommonLoader::CommonLoader::InitializeAssemblyLoader(const char* path)
{
	return ManagedCommonLoader::InitializeLoader(path);
}

void CommonLoader::CommonLoader::RaiseInitializers()
{
	ManagedCommonLoader::RaiseInitializers();
}

void CommonLoader::CommonLoader::RaiseUpdates()
{
	ManagedCommonLoader::RaiseUpdates();
}