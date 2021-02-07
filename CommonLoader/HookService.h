#pragma once

#ifndef _WIN64
#define MIN_HOOK_LENGTH 5
#else
#define MIN_HOOK_LENGTH 14
#endif


namespace CommonLoader
{
	enum HookBehavior
	{
		Before,
		After,
		Replace
	};
	
	enum HookParameter
	{
		Jump = 0,
		Call = 1,
	};

	class HookService
	{
	public:
		static void WriteASMHook(const char* source, size_t address, int behavior = HookBehavior::After, int parameter = HookParameter::Jump);
		static unsigned int NopInstructions(size_t address, unsigned int count);
	};
}
