#include "AssemblerService.h"
#include "AssemblerService_In.h"
#include "Logger.h"
#include <sstream>

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

	ks_asm(AssemblerServiceImpl::assembler_instance, source, base,
		&result->data, &result->length, &result->instruction_count,
		(ks_err_context**)&result->errors, &result->errors_size);

	return result;
}

void AssemblerService::OnError(const char* message, const AssemblerResult* result)
{
	if (!result->errors_size)
	{
		return;
	}

	ApplicationStore::SetState(CMN_LOADER_STATE_INIT_ASSEMBLY_FAILED, 1);

	if (ApplicationStore::GetState(CMN_LOADER_STATE_DISABLE_LOGGING))
	{
		return;
	}

	if (message)
	{
		Logger::Error("{}", message);
	}

	for (size_t i = 0; i < result->errors_size; i++)
	{
		const AssemblerError& err = result->errors[i];

		Logger::Error("{}({},{}): error {}: '{}': {}",
			message ? "    " : "",
			err.line_num,
			err.line_char,
			err.err_num,
			err.line,
			ks_strerror((ks_err)err.err_num));
	}
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
	if (data)
	{
		ks_free(data);
	}

	if (errors)
	{
		delete errors;
	}
}