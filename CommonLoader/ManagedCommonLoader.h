#pragma once
#include "AssemblerService.h"
#include "ApplicationStore.h"
#include "clrhost/clrhost.h"

namespace CommonLoader
{
	template<std::size_t N>
	struct FixedString {
		char data[N]{};

		constexpr FixedString(const char(&str)[N]) {
			std::copy_n(str, N, data);
		}

		constexpr size_t size()
		{
			return N;
		}
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
		size_t numCodes{};
		const Code_t** codes{};
	};

	template<FixedString MethodName, typename TReturn, typename... TArgs>
	inline TReturn ManagedInvoke(TArgs... args)
	{
		static auto pImpl = CreateDelegate<TReturn(TArgs...)>(MethodName.data);
		if (std::is_same_v<TReturn, void>)
		{
			pImpl(args...);
		}
		else
		{
			return pImpl(args...);
		}
	}

	template<FixedString MethodName, typename... TArgs>
	inline void ManagedInvoke(TArgs... args)
	{
		ManagedInvoke<MethodName, void>(args...);
	}
}