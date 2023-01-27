#include "AssemblerService.h"
#include "AssemblerService_In.h"

using namespace CommonLoader;

ks_engine* AssemblerServiceImpl::assembler_instance = nullptr;
csh AssemblerServiceImpl::disassembler_instance = 0;
cs_err AssemblerServiceImpl::disassembler_error = CS_ERR_OK;
std::unordered_map<uint32_t, uint64_t> AssemblerServiceImpl::symbol_map = {};

uint32_t MakeStringHash(const char* str)
{
	if (str == nullptr)
	{
		return 0;
	}

	return XXH32(str, strlen(str), 0);
}

void AssemblerService::Init()
{
	AssemblerServiceImpl::Init();
}

csh AssemblerService::GetDisassembler()
{
	return AssemblerServiceImpl::disassembler_instance;
}

bool AssemblerService::RemoveSymbol(const char* name)
{
	return AssemblerServiceImpl::symbol_map.erase(MakeStringHash(name));;
}

void AssemblerService::SetSymbol(const char* name, uint64_t value)
{
	AssemblerServiceImpl::symbol_map.insert_or_assign(MakeStringHash(name), value);
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
	const auto& it = symbol_map.find(MakeStringHash(symbol));
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