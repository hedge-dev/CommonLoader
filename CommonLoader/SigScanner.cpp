#include "pch.h"
#include "SigScanner.h"
#include "Windows.h"
#include <Psapi.h>

MODULEINFO module_info{nullptr, 0, nullptr};

const MODULEINFO& get_module()
{
    if (module_info.SizeOfImage)
        return module_info;

    ZeroMemory(&module_info, sizeof(MODULEINFO));
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &module_info, sizeof(MODULEINFO));

    return module_info;
}

void* CommonLoader::Scan(const char* p_pattern, const char* p_mask)
{
    const MODULEINFO& moduleInfo = get_module();
    const size_t length = strlen(p_mask);

    for (size_t i = 0; i < moduleInfo.SizeOfImage; i++)
    {
        char* memory = (char*)moduleInfo.lpBaseOfDll + i;

        size_t j;
        for (j = 0; j < length; j++)
        {
            if (p_mask[j] != '?' && p_pattern[j] != memory[j])
                break;
        }

        if (j == length)
            return memory;
    }

    return nullptr;
}
