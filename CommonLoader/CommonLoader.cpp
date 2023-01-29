#include "CommonLoader.h"
#include "ManagedCommonLoader.h"

#pragma unmanaged
bool is_init{};
void CommonLoader::Init()
{
	if (is_init)
	{
		return;
	}

	is_init = true;
	ApplicationStore::Init();
	AssemblerService::Init();
	InitSigScanner();
}
#pragma managed

bool CommonLoader::LoadAssembly(const char* path)
{
	Init();
	return ManagedCommonLoader::LoadAssembly(path);
}

void CommonLoader::RaiseInitializers()
{
	ManagedCommonLoader::RaiseInitializers();
}

void CommonLoader::RaiseUpdates()
{
	ManagedCommonLoader::RaiseUpdates();
}

const CommonLoaderAPI* CommonLoader::GetAPI()
{
	return &api_table;
}
