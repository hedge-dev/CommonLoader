#include "pch.h"
#include "CommonLoader.h"
#include "Logger.h"
#include "ManagedCommonLoader.h"
#include <sstream>
#include "clrhost/clrhost.h"

#define COMMONLOADER_DOMAIN "CommonLoader"
bool is_init{};
std::unique_ptr<clrhost::clr_context> CommonLoader::clr{};
static CommonLoader::NativeContext nativeContext
{
	.api = &CommonLoader::api_table,
	.logCallback = [](size_t level, const wchar_t* message)
	{
		Logger::Log((std::wstring_view)message, (Logger::LogType)level);
	}
};

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

	ManagedInvoke<"Initialize">(&nativeContext);
}

bool CommonLoader::LoadAssembly(const std::filesystem::path& path)
{
	Init();
	return ManagedInvoke<"LoadFile", bool>(path.c_str());
}

const Code_t** CommonLoader::GetCodes(size_t& outNumCodes)
{
	outNumCodes = nativeContext.numCodes;
	return nativeContext.codes;
}

const Code_t* CommonLoader::FindCode(const char* id)
{
	size_t numCodes = nativeContext.numCodes;
	auto* codes = nativeContext.codes;
	for (size_t i = 0; i < numCodes; i++)
	{
		if (strcmp(id, codes[i]->ID) == 0 || strcmp(id, codes[i]->FullName) == 0)
		{
			return codes[i];
		}
	}

	return nullptr;
}

bool CommonLoader::DisableCode(const Code_t* code)
{
	return ManagedInvoke<"DisableCode", bool>(code);
}

void CommonLoader::RaiseInitializers()
{
	ManagedInvoke<"RaiseInitializers">();

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
	ManagedInvoke<"RaiseUpdates">();
}

const CommonLoaderAPI* CommonLoader::GetAPI()
{
	return &api_table;
}