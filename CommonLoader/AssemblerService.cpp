#include "AssemblerService.h"
#include "AssemblerService_In.h"

using namespace CommonLoader;

ks_engine* AssemblerServiceImpl::assembler_instance = nullptr;
csh AssemblerServiceImpl::disassembler_instance = 0;
cs_err AssemblerServiceImpl::disassembler_error = CS_ERR_OK;
std::unordered_map<std::string_view, uint64_t> AssemblerServiceImpl::symbol_map = {};

void AssemblerService::Init()
{
	AssemblerServiceImpl::Init();
}

csh AssemblerService::GetDisassembler()
{
	return AssemblerServiceImpl::disassembler_instance;
}

void AssemblerService::RemoveSymbol(const char* name)
{
	AssemblerServiceImpl::symbol_map.erase(name);
}

void AssemblerService::SetSymbol(const char* name, uint64_t value)
{
	AssemblerServiceImpl::symbol_map.insert_or_assign({ name }, value);
}

bool AssemblerService::GetSymbol(const char* name, uint64_t& value)
{
	return AssemblerServiceImpl::ResolveSymbol(name, &value);
}

AssemblerResult* AssemblerService::CompileAssembly(const char* source, uint64_t base)
{
	AssemblerResult* result = new AssemblerResult();

	if (ks_asm(AssemblerServiceImpl::assembler_instance, source, base, &result->data, &result->length, &result->instruction_count) == -1)
	{
		result->error_string = ks_strerror(ks_errno(AssemblerServiceImpl::assembler_instance));
	}

	return result;
}

bool AssemblerServiceImpl::ResolveSymbol(const char* symbol, uint64_t* value)
{
	const auto& it = symbol_map.find(symbol);
	if (it != symbol_map.end())
	{
		*value = it->second;
		return true;
	}
	
	return false;
}


AssemblerResult::~AssemblerResult()
{
	if (data != nullptr)
	{
		ks_free(data);
	}
}