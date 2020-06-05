#include "pch.h"
#include "HookService.h"
#include "AssemblerService.h"
#include <Windows.h>

#define CALCULATE_JUMP(destination, target) (size_t)target - (size_t)(destination + 4)

void CommonLoader::HookService::WriteASMHook(const char* source, size_t address, int behavior)
{
	csh disasm = AssemblerService::GetDisassembler();
	cs_insn* insn;

	size_t count = cs_disasm(disasm, (uint8_t*)address, 64, address, 0, &insn);
	
	size_t hookLen = 0;

	for (size_t i = 0; i < count; i++)
	{
		hookLen += insn[i].size;

		if (hookLen >= MIN_HOOK_LENGTH)
			break;
	}
	cs_free(insn, count);

	AssemblerResult* compiledCode = AssemblerService::CompileAssembly(source);

	void* hookPtr = _aligned_malloc(compiledCode->length + hookLen + 5, 4);
	size_t pos = (size_t)hookPtr;

	switch (behavior)
	{
	case HookBehavior::Before:
		memcpy_s((void*)pos, compiledCode->length, compiledCode->data, compiledCode->length);
		pos += compiledCode->length;
		memcpy_s((void*)pos, hookLen, (void*)address, hookLen);
		pos += hookLen;
		break;

	case HookBehavior::After:
		memcpy_s((void*)pos, hookLen, (void*)address, hookLen);
		pos += hookLen;
		memcpy_s((void*)pos, compiledCode->length, compiledCode->data, compiledCode->length);
		pos += compiledCode->length;
		break;

	case HookBehavior::Replace:
		memcpy_s((void*)pos, compiledCode->length, compiledCode->data, compiledCode->length);
		pos += compiledCode->length;
		break;
	}

#ifndef WIN64
	*((char*)pos) = 0xE9;
	pos++;
	*((size_t*)pos) = CALCULATE_JUMP(pos, address + hookLen);
	pos += sizeof(size_t);
#else
	*((char*)pos) = 0xFF;
	*((char*)pos + 1) = 0x25;
	*((char*)pos + 2) = 0x00;
	*((char*)pos + 3) = 0x00;
	*((char*)pos + 4) = 0x00;
	*((char*)pos + 5) = 0x00;
	*((size_t*)pos + 6) = address + hookLen;
#endif

	unsigned long oldProtect;
	VirtualProtect((void*)address, hookLen, PAGE_READWRITE, &oldProtect);
	
	for (char* ptr = (char*)address; ptr < (char*)(address + hookLen); ptr++)
	{
		*ptr = 0x90;
	}

#ifndef WIN64
	* ((char*)address) = 0xE9;
	*((size_t*)((char*)address + 1)) = CALCULATE_JUMP((char*)address + 1, hookPtr);
#else
	*((char*)address) = 0xFF;
	*((char*)address + 1) = 0x25;
	*((char*)address + 2) = 0x00;
	*((char*)address + 3) = 0x00;
	*((char*)address + 4) = 0x00;
	*((char*)address + 5) = 0x00;
	*((size_t*)address + 6) = (size_t)hookPtr;
#endif
	
	VirtualProtect((void*)address, hookLen, oldProtect, &oldProtect);

	delete compiledCode;
}
