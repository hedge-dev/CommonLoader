#pragma once
#include "AssemblerService.h"
#include "ApplicationStore.h"
#include "clrhost/clrhost.h"

namespace CommonLoader
{
	class ManagedCommonLoader
	{
	public:
		static bool LoadAssembly(const std::filesystem::path& path);
		static bool DisableCode(const Code_t* code);
		static void RaiseInitializers();
		static void RaiseUpdates();
		static const Code_t** GetCodes(size_t& outNumCodes);
	};

	template<typename TSignature>
	typename std::enable_if<std::is_function_v<TSignature>, typename clrhost::FunctionPtr<TSignature>::StdCallFunction_t*>::type
		CreateDelegate(const char* typeName, const char* methodName)
	{
		return ::CommonLoader::clr->template CreateDelegate<TSignature>("CommonLoader.Managed", typeName, methodName);
	}

	template<typename TSignature>
	typename std::enable_if<std::is_function_v<TSignature>, typename clrhost::FunctionPtr<TSignature>::StdCallFunction_t*>::type
		CreateDelegate(const char* methodName)
	{
		return CreateDelegate<TSignature>("CommonLoader.Managed.CodeLoader", methodName);
	}

	struct NativeContext
	{
		const CommonLoaderAPI* api{};
		clrhost::FunctionPtr<void(size_t, const wchar_t*)>::StdCallFunction_t* logCallback{};
	};
}