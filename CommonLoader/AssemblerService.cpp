#include "pch.h"
#include "AssemblerService.h"
#include <keystone/keystone.h>
#include <capstone/capstone.h>

using namespace CommonLoader;

ks_engine* CommonLoader::AssemblerService::assembler_instance = nullptr;
csh CommonLoader::AssemblerService::disassembler_instance = 0;
cs_err CommonLoader::AssemblerService::disassembler_error = cs_err::CS_ERR_OK;

void CommonLoader::AssemblerService::Init()
{
	ks_open(KS_ARCH_X86, ASSEMBLER_MODE, (ks_engine**)&assembler_instance);

	cs_opt_mem setup;

	setup.malloc = malloc;
	setup.calloc = calloc;
	setup.realloc = realloc;
	setup.free = free;
	setup.vsnprintf = vsnprintf;

	cs_option(disassembler_instance, CS_OPT_MEM, (size_t)&setup);

	disassembler_error = cs_open(CS_ARCH_X86, DISASSEMBLER_MODE, &disassembler_instance);
}

AssemblerResult* CommonLoader::AssemblerService::CompileAssembly(const char* source)
{
	AssemblerResult* result = new AssemblerResult();

	ks_asm(assembler_instance, source, 0, &result->data, &result->length, &result->instruction_count);

	return result;
}

CommonLoader::AssemblerResult::~AssemblerResult()
{
	ks_free(data);
}