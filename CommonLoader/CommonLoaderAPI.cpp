#pragma unmanaged

#include "pch.h"
#include "CommonLoaderAPI.h"
#include "CommonLoader.h"
#include "SigScanner.h"
#include "ApplicationStore.h"
#include "AssemblerService.h"

int CMN_LOADER_API GetVersionImpl()
{
	return CMN_LOADER_API_VERSION;
}

void* CMN_LOADER_API ScanSignatureImpl(const char* pattern, const char* mask)
{
	return CommonLoader::Scan(pattern, mask);
}

void* CMN_LOADER_API ScanSignatureExImpl(const char* pattern, const char* mask, void* begin, size_t size)
{
	return CommonLoader::Scan(pattern, mask, strlen(mask), begin, size);
}

int CMN_LOADER_API GetApplicationOptionExImpl(const char* section, const char* key, char* out, size_t out_size)
{
	if (section == nullptr)
	{
		section = CommonLoader::ApplicationStore::DEFAULT_SECTION;
	}

	std::string out_value{};
	if (CommonLoader::ApplicationStore::GetOption(section, key, out_value))
	{
		if (out == nullptr)
		{
			return out_value.size();
		}

		memcpy(out, out_value.c_str(), std::min(out_value.size(), out_size));
		return out_value.size();
	}

	return -1;
}

int CMN_LOADER_API GetApplicationOptionImpl(const char* key, char* out, size_t out_size)
{
	return GetApplicationOptionExImpl(nullptr, key, out, out_size);
}

void CMN_LOADER_API SaveApplicationOptionsImpl()
{
	CommonLoader::ApplicationStore::Save();
}

AssemblerResult* CMN_LOADER_API CompileAssemblyImpl(const char* code)
{
	if (code == nullptr)
	{
		return nullptr;
	}

	return CommonLoader::AssemblerService::CompileAssembly(code);
}

void CMN_LOADER_API FreeAssemblerObjectImpl(AssemblerResult* arg)
{
	if (arg == nullptr)
	{
		return;
	}

	delete arg;
}

void CMN_LOADER_API SetAssemblerSymbolImpl(const char* name, uint64_t value)
{
	if (name == nullptr)
	{
		return;
	}

	CommonLoader::AssemblerService::SetSymbol(name, value);
}

bool CMN_LOADER_API GetAssemblerSymbolImpl(const char* name, uint64_t* out)
{
	if (name == nullptr || out == nullptr)
	{
		return false;
	}

	return CommonLoader::AssemblerService::GetSymbol(name, *out);
}

bool CMN_LOADER_API RemoveAssemblerSymbolImpl(const char* name)
{
	if (name == nullptr)
	{
		return false;
	}

	return CommonLoader::AssemblerService::RemoveSymbol(name);
}

void CMN_LOADER_API SetStateImpl(size_t state, size_t value)
{
	CommonLoader::ApplicationStore::SetState(state, value);
}

void CMN_LOADER_API SetStateFlagImpl(size_t state, size_t flag, bool set)
{
	CommonLoader::ApplicationStore::SetStateFlag(state, flag, set);
}

size_t CMN_LOADER_API GetStateImpl(size_t state)
{
	return CommonLoader::ApplicationStore::GetState(state);
}

namespace CommonLoader
{
	const CommonLoaderAPI api_table
	{
		GetVersionImpl,
		ScanSignatureImpl,
		ScanSignatureExImpl,
		GetApplicationOptionImpl,
		GetApplicationOptionExImpl,
		SaveApplicationOptionsImpl,
		CompileAssemblyImpl,
		FreeAssemblerObjectImpl,
		SetAssemblerSymbolImpl,
		GetAssemblerSymbolImpl,
		RemoveAssemblerSymbolImpl,
		SetStateImpl,
		SetStateFlagImpl,
		GetStateImpl
	};
}