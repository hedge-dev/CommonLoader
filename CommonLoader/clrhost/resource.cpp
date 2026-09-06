#include "pch.h"
#include "resource.h"
#include <unordered_map>
#include <array>

CLRHOST_DECLARE_RESOURCE(ClrHost_Managed_dll);
CLRHOST_DECLARE_RESOURCE(CommonLoader_Managed_dll);

namespace clrhost
{
    const static std::array resources = 
    {
        &CLRHOST_RESOURCE(ClrHost_Managed_dll),
        &CLRHOST_RESOURCE(CommonLoader_Managed_dll)
    };

    resource::resource(std::string_view name, const void* ptr, size_t size) : name(name), ptr(ptr), size(size)
    {

    }

    const resource* resource::find(std::string_view name)
    {
        for (auto res : resources)
        {
            if (res->name == name)
            {
                return res;
            }
        }

        return nullptr;
    }
}