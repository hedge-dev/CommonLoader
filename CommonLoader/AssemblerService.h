#pragma once
#include "CommonLoaderAPI.h"

typedef size_t csh;
namespace CommonLoader
{
	class AssemblerServiceImpl;

	class AssemblerService
	{
	public:
		static void Init();
		static csh GetDisassembler();
		static bool RemoveSymbol(const char* name);
		static void SetSymbol(const char* name, uint64_t value);
		static bool GetSymbol(const char* name, uint64_t& value);
		static AssemblerResult* CompileAssembly(const char* source, uint64_t base = 0);
	};
}