#pragma once

#include <keystone/keystone.h>
#include <capstone/capstone.h>

#ifdef _WIN64
#define ASSEMBLER_MODE KS_MODE_64
#define DISASSEMBLER_MODE CS_MODE_64
#else
#define ASSEMBLER_MODE KS_MODE_32
#define DISASSEMBLER_MODE CS_MODE_32
#endif

namespace CommonLoader 
{
	class AssemblerResult
	{
	public:
		size_t instruction_count;
		size_t length;
		unsigned char* data;

		~AssemblerResult();
	};

	class AssemblerService
	{
	protected:
		static ks_engine* assembler_instance;
		static csh disassembler_instance;
		static cs_err disassembler_error;

	public:
		static void Init();
		static AssemblerResult* CompileAssembly(const char* source);
		static csh GetDisassembler() {
			return disassembler_instance;
		}
	};
}