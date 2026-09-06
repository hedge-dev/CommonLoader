#pragma once
#include "dotnet.h"
#include <wrl/client.h>

namespace mscorlib
{
    struct _AppDomain;
}

namespace clrhost
{
    using namespace Microsoft::WRL;
    class clr_context;

    template<typename TReturn, typename... TArgs>
    struct FunctionPtr;

    template<typename TReturn, typename... TArgs>
    struct FunctionPtr<TReturn(TArgs...)>
    {
        typedef TReturn Function_t(TArgs...);
        typedef TReturn __stdcall StdCallFunction_t(TArgs...);
        Function_t* ptr{};

        FunctionPtr(Function_t* ptr) : ptr((Function_t*)ptr)
        {

        }

        TReturn operator()(TArgs... args) const
        {
            return ptr(args...);
        }

        operator Function_t* () const
        {
            return ptr;
        }
    };

    enum clr_init_flags
    {
        ClrInit_TryCoreCLR = 1,
        ClrInit_TryFramework = 2,
        ClrInit_TryAll = ClrInit_TryCoreCLR | ClrInit_TryFramework
    };

    class clr_context
    {
    public:
        static std::unique_ptr<clr_context> Initialize(const char* domainName, clr_init_flags flags = ClrInit_TryAll);
        virtual void* CreateDelegate(const char* assemblyName, const char* typeName, const char* methodName) = 0;

        template<typename TSignature>
        typename std::enable_if<std::is_function_v<TSignature>, typename FunctionPtr<TSignature>::StdCallFunction_t*>::type
            CreateDelegate(const char* assemblyName, const char* typeName, const char* methodName)
        {
            return (typename FunctionPtr<TSignature>::StdCallFunction_t*)CreateDelegate(assemblyName, typeName, methodName);
        }
    };

    class clr_context_coreclr final : public clr_context
    {
    public:
        dotnet::coreclr clr{};
        void* hostHandle{};
        uint32_t domainID{};
        host_runtime_contract runtimeContract{};

        bool Initialize(const char* domainName);
        void* CreateDelegate(const char* assemblyName, const char* typeName, const char* methodName) override;
        static bool CORECLR_CALLING_CONVENTION BundleProbe(const char* pathRaw, int64_t* offset, int64_t* size, int64_t* compressedSize);

        ~clr_context_coreclr()
        {
            if (hostHandle)
            {
                clr.shutdown(hostHandle, domainID);
                hostHandle = nullptr;
                domainID = 0;
            }
        }
    };

    class clr_context_framework final : public clr_context
    {
    public:
        typedef bool __stdcall SetAssemblyResolve_t(const char*, const void*&, size_t&);
        typedef void* __stdcall CreateDelegate_t(const char*, const char*, const char*);

        // Essentially a constant, this version is fixed enough to be put on
        // all .NET Framework assemblies in their Metadata's RuntimeVersion
        inline static constexpr const wchar_t* FrameworkVersion = L"v4.0.30319";
        inline static constexpr const char* ManagedShim = "ClrHost.Managed.dll";

        ComPtr<ICLRMetaHost> metaHost{};
        ComPtr<ICLRRuntimeInfo> runtime{};
        ComPtr<ICorRuntimeHost> host{};
        ComPtr<mscorlib::_AppDomain> appdomain{};
        CreateDelegate_t* createDelegate{};

        bool Initialize(const char* domainName);
        inline void* CreateDelegate(const char* assemblyName, const char* typeName, const char* methodName) override
        {
            if (!createDelegate) return nullptr;

            return createDelegate(assemblyName, typeName, methodName);
        }

        static bool __stdcall AssemblyResolve(const char*, const void*&, size_t&);
    };
}