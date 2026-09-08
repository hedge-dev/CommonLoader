#include "pch.h"
#include "clrhost/clrhost.h"
#include "ManagedCommonLoader.h"

namespace CommonLoader
{
    bool ManagedCommonLoader::LoadAssembly(const std::filesystem::path& path)
    {
        static auto loadFunction = CreateDelegate<size_t(const wchar_t*)>("LoadFile");
        return loadFunction(path.c_str());
    }

    bool ManagedCommonLoader::DisableCode(const Code_t* code)
    {
        static auto disableFunction = CreateDelegate<bool(const Code_t*)>("DisableCode");
        return disableFunction(code);
    }

    void ManagedCommonLoader::RaiseInitializers()
    {
        static auto raiseInitFunction = CreateDelegate<void()>("RaiseInitializers");
        raiseInitFunction();
    }

    void ManagedCommonLoader::RaiseUpdates()
    {
        static auto raiseUpdatesFunction = CreateDelegate<void()>("RaiseUpdates");
        raiseUpdatesFunction();
    }

    const Code_t** ManagedCommonLoader::GetCodes(size_t& outNumCodes)
    {
        static auto getNumCodes = CreateDelegate<size_t()>("GetNumCodes");
        static auto getCodes = CreateDelegate<const Code_t**()>("GetCodesPtr");

        outNumCodes = getNumCodes();
        return getCodes();
    }
}
