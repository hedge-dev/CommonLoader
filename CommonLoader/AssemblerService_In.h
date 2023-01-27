#pragma once

#include <keystone/keystone.h>
#include <capstone/capstone.h>
#include <unordered_map>
#include <string_view>

#ifdef _WIN64
#define ASSEMBLER_MODE KS_MODE_64
#define DISASSEMBLER_MODE CS_MODE_64
#else
#define ASSEMBLER_MODE KS_MODE_32
#define DISASSEMBLER_MODE CS_MODE_32
#endif

namespace CommonLoader
{
	class AssemblerServiceImpl
	{
	public:
		static ks_engine* assembler_instance;
		static csh disassembler_instance;
		static cs_err disassembler_error;
		static std::unordered_map<uint32_t, uint64_t> symbol_map;

		static csh GetDisassembler() {
			return disassembler_instance;
		}

		inline static void Init()
		{
			ks_open(KS_ARCH_X86, ASSEMBLER_MODE, &assembler_instance);
			ks_option(assembler_instance, KS_OPT_SYNTAX, KS_OPT_SYNTAX_INTEL);
			ks_option(assembler_instance, KS_OPT_SYM_RESOLVER, reinterpret_cast<size_t>(ResolveSymbol));

			cs_opt_mem setup;

			setup.malloc = malloc;
			setup.calloc = calloc;
			setup.realloc = realloc;
			setup.free = free;
			setup.vsnprintf = vsnprintf;

			cs_option(disassembler_instance, CS_OPT_MEM, (size_t)&setup);

			disassembler_error = cs_open(CS_ARCH_X86, DISASSEMBLER_MODE, &disassembler_instance);
		}

		static bool ResolveSymbol(const char* symbol, uint64_t* value);
	};
}