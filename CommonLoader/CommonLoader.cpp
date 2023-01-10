#include "ManagedCommonLoader.h"
#include "AssemblyLoader.h"
#include "CommonLoader.h"

bool CommonLoader::InitializeAssemblyLoader(const char* path)
{
	return ManagedCommonLoader::InitializeLoader(path);
}

void CommonLoader::RaiseInitializers()
{
	ManagedCommonLoader::RaiseInitializers();
}

void CommonLoader::RaiseUpdates()
{
	ManagedCommonLoader::RaiseUpdates();
}