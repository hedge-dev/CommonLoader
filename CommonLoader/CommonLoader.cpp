#include "pch.h"
#include "CommonLoader.h"
#include "Logger.h"
#include "ManagedCommonLoader.h"
#include <sstream>
#include "clrhost/clrhost.h"

#define COMMONLOADER_DOMAIN "CommonLoader"
bool is_init{};
std::unique_ptr<clrhost::clr_context> CommonLoader::clr{};

void CommonLoader::Init()
{
	if (is_init)
	{
		return;
	}

	is_init = true;
	
	clr = std::move(clrhost::clr_context::Initialize(COMMONLOADER_DOMAIN));
	ApplicationStore::Init();
	AssemblerService::Init();
	InitSigScanner();

	auto providerInit = CreateDelegate<void(const NativeContext&)>("Initialize");
	providerInit(
		{
			.api = &api_table,
			.logCallback = [](size_t level, const wchar_t* message) 
			{
				Logger::Log((std::wstring_view)message, (Logger::LogType)level);
			}
		});
}

bool CommonLoader::LoadAssembly(const std::filesystem::path& path)
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