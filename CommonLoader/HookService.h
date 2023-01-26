#pragma once

#ifndef _WIN64
#define MIN_HOOK_LENGTH 5
#else
#define MIN_HOOK_LENGTH 14
#endif


namespace CommonLoader
{
	enum EHookBehavior
	{
		eHookBehavior_Before,
		eHookBehavior_After,
		eHookBehavior_Replace
	};
	
	enum EHookParameter
	{
		eHookParameter_Jump = 0,
		eHookParameter_Call = 1,
	};

	class HookService
	{
	public:
		static bool WriteASMHook(const char* source, size_t address, int behavior = eHookBehavior_After, int parameter = eHookParameter_Jump);
		static unsigned int NopInstructions(size_t address, unsigned int count);
	};
}
