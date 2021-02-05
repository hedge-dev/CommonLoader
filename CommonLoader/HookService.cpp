#include "pch.h"
#include "HookService.h"
#include "AssemblerService.h"
#include <Windows.h>

#define CALCULATE_JUMP(destination, target) (size_t)target - (size_t)(destination + 4)

void CommonLoader::HookService::WriteASMHook(const char* source, size_t address, int behavior, int parameter)
{
	csh disasm = AssemblerService::GetDisassembler();
	cs_insn* insn = nullptr;

	const size_t count = cs_disasm(disasm, reinterpret_cast<uint8_t*>(address), 64, address, 0, &insn);

	size_t hookLen = 0;

	for (size_t i = 0; i < count; i++)
	{
		hookLen += insn[i].size;

		if (hookLen >= MIN_HOOK_LENGTH)
			break;
	}

	cs_free(insn, count);

	AssemblerResult* compiledCode = AssemblerService::CompileAssembly(source);

	void* hookPtr = _aligned_malloc(compiledCode->length + hookLen + MIN_HOOK_LENGTH, alignof(void*));
	size_t pos = reinterpret_cast<size_t>(hookPtr);

	switch (behavior)
	{
	case HookBehavior::Before:
		memcpy_s(reinterpret_cast<void*>(pos), compiledCode->length, compiledCode->data, compiledCode->length);
		pos += compiledCode->length;
		memcpy_s(reinterpret_cast<void*>(pos), hookLen, reinterpret_cast<void*>(address), hookLen);
		pos += hookLen;
		break;

	case HookBehavior::After:
		memcpy_s(reinterpret_cast<void*>(pos), hookLen, reinterpret_cast<void*>(address), hookLen);
		pos += hookLen;
		memcpy_s(reinterpret_cast<void*>(pos), compiledCode->length, compiledCode->data, compiledCode->length);
		pos += compiledCode->length;
		break;

	case HookBehavior::Replace:
		memcpy_s(reinterpret_cast<void*>(pos), compiledCode->length, compiledCode->data, compiledCode->length);
		pos += compiledCode->length;
		break;

	default:
	{
		_aligned_free(hookPtr);
		return;
	}
}

#ifndef WIN64
	if (parameter == Jump)
	{
		*reinterpret_cast<char*>(pos) = 0xE9;
		pos++;
		*reinterpret_cast<size_t*>(pos) = CALCULATE_JUMP(pos, address + hookLen);
		pos += sizeof(size_t);
	}
	else
	{
		*reinterpret_cast<char*>(pos) = 0xC3;
		pos += sizeof(size_t) + 1;
	}
#else
	if (parameter == Jump)
	{
		*reinterpret_cast<char*>(pos) = 0xFF;
		*(reinterpret_cast<char*>(pos) + 1) = 0x25;
		*(reinterpret_cast<char*>(pos) + 2) = 0x00;
		*(reinterpret_cast<char*>(pos) + 3) = 0x00;
		*(reinterpret_cast<char*>(pos) + 4) = 0x00;
		*(reinterpret_cast<char*>(pos) + 5) = 0x00;
		*reinterpret_cast<size_t*>(reinterpret_cast<char*>(pos) + 6) = address + hookLen;
	}
	else
	{
		*reinterpret_cast<char*>(address) = 0xC3;
	}
#endif

	unsigned long oldProtect;
	VirtualProtect(reinterpret_cast<void*>(address), hookLen, PAGE_READWRITE, &oldProtect);

	for (char* ptr = reinterpret_cast<char*>(address); ptr < reinterpret_cast<char*>(address + hookLen); ptr++)
	{
		*ptr = 0x90;
	}

#ifndef WIN64
	*reinterpret_cast<char*>(address) = parameter == Jump ? 0xE9 : 0xE8;
	*reinterpret_cast<unsigned*>(reinterpret_cast<char*>(address) + 1) = CALCULATE_JUMP(reinterpret_cast<char*>(address) + 1, hookPtr);
#else
	* reinterpret_cast<char*>(address) = 0xFF;
	*(reinterpret_cast<char*>(address) + 1) = parameter == Jump ? 0x25 : 0x15;
	*(reinterpret_cast<char*>(address) + 2) = 0x00;
	*(reinterpret_cast<char*>(address) + 3) = 0x00;
	*(reinterpret_cast<char*>(address) + 4) = 0x00;
	*(reinterpret_cast<char*>(address) + 5) = 0x00;
	*reinterpret_cast<size_t*>(reinterpret_cast<char*>(address) + 6) = reinterpret_cast<size_t>(hookPtr);
#endif

	VirtualProtect(reinterpret_cast<void*>(address), hookLen, oldProtect, &oldProtect);
	VirtualProtect(hookPtr, compiledCode->length, PAGE_EXECUTE_READWRITE, &oldProtect);

	delete compiledCode;
}