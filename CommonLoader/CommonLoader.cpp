#include "CommonLoader.h"
#include "Logger.h"
#include "ManagedCommonLoader.h"
#include <sstream>

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

	bool sigFailed = ApplicationStore::GetState(CMN_LOADER_STATE_INIT_SIG_SCAN_FAILED);
	bool asmFailed = ApplicationStore::GetState(CMN_LOADER_STATE_INIT_ASSEMBLY_FAILED);
	bool errored = sigFailed || asmFailed;

	if (errored)
	{
		std::stringstream errStream{};

		if (sigFailed && asmFailed)
		{
			errStream << "One or more critical errors occurred upon initialisation.";
		}
		else if (sigFailed)
		{
			errStream << "Failed to find signatures.";
		}
		else if (asmFailed)
		{
			errStream << "Failed to compile assembly in one or more codes.";
		}

		errStream << std::endl << std::endl
			<< "This may lead to unstable behaviour and could potentially result in a game crash."
			<< std::endl << std::endl
			<< "Would you like to continue anyway?";

		int msgResult = MessageBoxA(nullptr, errStream.str().c_str(),
			"Error", MB_YESNO | MB_ICONERROR | MB_DEFBUTTON2);

		if (msgResult == IDNO)
		{
			exit(-1);
		}
	}
}

void CommonLoader::RaiseUpdates()
{
	ManagedCommonLoader::RaiseUpdates();
}

const CommonLoaderAPI* CommonLoader::GetAPI()
{
	return &api_table;
}
