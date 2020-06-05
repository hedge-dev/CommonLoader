#include "pch.h"
#include "ManagedCommonLoader.h"
#include "AssemblyLoader.h"
#include "CommonLoader.h"

void CommonLoader::CommonLoader::InitializeAssemblyLoader(const char* path)
{
	ManagedCommonLoader::InitializeLoader(path);
}

void CommonLoader::CommonLoader::RaiseInitializers()
{
	ManagedCommonLoader::RaiseInitializers();
}

void CommonLoader::CommonLoader::RaiseUpdates()
{
	ManagedCommonLoader::RaiseUpdates();
}