#include <pch.h>
#include <iostream>
#include <metahost.h>
#include <mscoree.h>
#include <vector>
#include <unordered_set>
#include "coreclrhost.h"
#include "clrhost.h"
#include "mscorlib.tlh"

#define DOTNET_DESKTOP_RUNTIME "Microsoft.WindowsDesktop.App"

size_t FindDiskAddress(const void* mem)
{
    HMODULE module{};
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)mem, &module)) return -1;

    auto* dosHeader = (PIMAGE_DOS_HEADER)module;
    auto* ntHeader = (IMAGE_NT_HEADERS*)(((char*)dosHeader) + dosHeader->e_lfanew);

    auto numSections = ntHeader->FileHeader.NumberOfSections;
    auto* sections = (IMAGE_SECTION_HEADER*)(((char*)&ntHeader->OptionalHeader) + ntHeader->FileHeader.SizeOfOptionalHeader);

    size_t memRva = (size_t)mem - (size_t)module;
    auto* containingSection = sections;

    for (size_t i = 0; i < numSections; i++)
    {
        auto& section = sections[i];
        if (section.VirtualAddress < memRva)
        {
            containingSection = &section;
        }
        else
        {
            break;
        }
    }

    return containingSection->PointerToRawData + (memRva - containingSection->VirtualAddress);
}

typedef int LoadAssemblyBytes_t(const uint8_t* data, size_t len, const uint8_t* symbols, size_t symbolsLen, void* loadContext, void* reserved);

void report_clr_error(const char* msg)
{
    printf(msg);
}

//int main()
//{
//    auto host = clrhost::clr_context::Initialize("CLRHOST");
//
//    auto say = host->CreateDelegate<void(const char*)>("ClrHost.Managed", "ClrHost.Managed.Test", "Say");
//    say("Hello World!");
//
//    return 0;
//}

bool clrhost::clr_context_coreclr::Initialize(const char* domainName)
{
    memset(&runtimeContract, 0, sizeof(runtimeContract));
    runtimeContract.size = sizeof(runtimeContract);
    runtimeContract.bundle_probe = &BundleProbe;

    auto dotnetRoot = dotnet::FindRoot();
    if (!dotnetRoot.has_value()) return false;

    auto runtimes = dotnet::FindRuntimes(dotnetRoot.value());
    dotnet::RuntimeInfo latestDesktop;

    for (const auto& runtime : runtimes)
    {
        if (runtime.name == DOTNET_DESKTOP_RUNTIME && runtime.version > latestDesktop.version)
        {
            latestDesktop = runtime;
        }
    }

    if (latestDesktop.path.empty() || latestDesktop.version.major < 5)
    {
        return false;
    }

    auto frameworks = latestDesktop.GetFrameworks();

    for (auto& fx : frameworks)
    {
        auto found = false;
        for (const auto& runtime : runtimes)
        {
            if (runtime.name == fx.name && runtime.version == fx.version)
            {
                fx = runtime;
                found = true;
                break;
            }
        }

        // Log something if not found
        if (!found)
        {
            return false;
        }
    }

    // Maybe update these to inplace_vector whenever C++26 rolls around
    std::array<const char*, 10> runtimeKeys;
    std::array<const char*, 10> runtimeValues;
    size_t runtimePropertyCount{};

    auto setRuntimeProperty = [&](const char* key, const char* value)
    {
        auto propIndex = runtimePropertyCount++;
        assert(runtimePropertyCount < runtimeKeys.size() && runtimePropertyCount < runtimeValues.size());

        runtimeKeys[propIndex] = key;
        runtimeValues[propIndex] = value;
    };

    frameworks.push_back(latestDesktop);

    std::filesystem::path coreclrPath{};
    std::string tpaList{};
    std::string nativeDllPathList{};
    std::string coreLibPath{};

    for (const auto& fx : frameworks)
    {
        auto coreclr = fx.path / "coreclr.dll";
        if (std::filesystem::exists(coreclr))
        {
            coreclrPath = coreclr;
            coreLibPath = fx.path.string();
        }

        nativeDllPathList += fx.path.string();
        nativeDllPathList += ';';

        for (const auto& file : std::filesystem::directory_iterator{ fx.path })
        {
            if (file.is_directory() || file.path().extension() != ".dll") continue;

            tpaList += file.path().string();
            tpaList += ';';
        }
    }

    auto* coreclrHandle = LoadLibraryW(coreclrPath.c_str());
    clr = dotnet::coreclr(coreclrHandle);
    if (!clr.module) return false;

    char nameBuffer[2048];
    HMODULE selfHandle{};
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)&BundleProbe, &selfHandle);
    GetModuleFileNameA(selfHandle, nameBuffer, sizeof(nameBuffer));

    char bundleProbeBuf[64];
    sprintf_s(bundleProbeBuf, "0x%p", &BundleProbe);

    char runtimeContractBuf[64];
    sprintf_s(runtimeContractBuf, "0x%p", &runtimeContract);

    setRuntimeProperty(HOST_PROPERTY_TRUSTED_PLATFORM_ASSEMBLIES, tpaList.c_str());
    setRuntimeProperty(HOST_PROPERTY_NATIVE_DLL_SEARCH_DIRECTORIES, nativeDllPathList.c_str());
    setRuntimeProperty(HOST_PROPERTY_SYSTEM_CORELIB_DIRECTORY, coreLibPath.c_str());
    setRuntimeProperty(HOST_PROPERTY_RUNTIME_CONTRACT, runtimeContractBuf);

    // Use BUNDLE_PROBE as that's the oldest supported method
    // of loading internal assemblies
    setRuntimeProperty(HOST_PROPERTY_BUNDLE_PROBE, bundleProbeBuf);

    clr.initialize(nameBuffer, domainName, runtimePropertyCount, runtimeKeys.data(), runtimeValues.data(), &hostHandle, &domainID);
    clr.set_error_writer(report_clr_error);

    return true;
}

void* clrhost::clr_context_coreclr::CreateDelegate(const char* assemblyName, const char* typeName, const char* methodName)
{
    if (!hostHandle) return nullptr;
    
    void* delegate{};
    clr.create_delegate(hostHandle, domainID, assemblyName, typeName, methodName, &delegate);

    return delegate;
}

bool CORECLR_CALLING_CONVENTION clrhost::clr_context_coreclr::BundleProbe(const char* pathRaw, int64_t* offset, int64_t* size, int64_t* compressedSize)
{
    std::string_view path = pathRaw;

    auto* res = clrhost::resource::find(path);

    if (res != nullptr)
    {
        size_t diskAddress = FindDiskAddress(res->ptr);
        *offset = diskAddress;
        *size = res->size;
        *compressedSize = 0;

        return true;
    }

    return false;
}

bool clrhost::clr_context_framework::Initialize(const char* domainName)
{
    auto mscoree = LoadLibraryW(L"mscoree.dll");
    if (!mscoree) return false;

    auto clrCreateInstance = (decltype(CLRCreateInstance)*)GetProcAddress(mscoree, "CLRCreateInstance");
    if (!clrCreateInstance) return false;

    clrCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&metaHost));
    metaHost->GetRuntime(FrameworkVersion, IID_PPV_ARGS(&runtime));

    runtime->GetInterface(CLSID_CorRuntimeHost, IID_PPV_ARGS(&host));
    host->Start();

    std::string_view domainView{ domainName };
    std::wstring domainNameWide{ domainView.begin(), domainView.end() };

    ComPtr<IUnknown> domainUnknown{};

    host->CreateDomain(domainNameWide.c_str(), nullptr, &domainUnknown);
    domainUnknown->QueryInterface(IID_PPV_ARGS(&appdomain));

    auto* shimResource = clrhost::resource::find(ManagedShim);

    auto* fileData = SafeArrayCreateVector(VT_UI1, 0, shimResource->size);
    void* pFileData{};
    SafeArrayAccessData(fileData, &pFileData);
    memcpy(pFileData, shimResource->ptr, shimResource->size);
    SafeArrayUnaccessData(fileData);

    ComPtr<mscorlib::_Assembly> loaderAssembly{};
    HRESULT err = appdomain->Load_3(fileData, &loaderAssembly);

    SafeArrayDestroy(fileData);

    ComPtr<mscorlib::_Type> memMarshalType{};
    loaderAssembly->GetType_2(SysAllocString(L"ClrHost.Managed.MemoryMarshal"), &memMarshalType);

    ComPtr<mscorlib::_MethodInfo> createDelegateMethod{};
    memMarshalType->GetMethod_2(SysAllocString(L"GetCreateDelegatePtr"), 
        (mscorlib::BindingFlags)(mscorlib::BindingFlags_Static | mscorlib::BindingFlags_Public), &createDelegateMethod);

    VARIANT createDelegateV{};
    createDelegateMethod->Invoke_3(VARIANT{ VT_NULL }, nullptr, &createDelegateV);

    createDelegate = (CreateDelegate_t*)createDelegateV.byref;

    auto setResolve = clr_context::CreateDelegate<void(SetAssemblyResolve_t*)>("ClrHost.Managed", "ClrHost.Managed.MemoryMarshal", "SetAssemblyResolve");
    if (!setResolve) return false;

    setResolve(&AssemblyResolve);

    return true;
}

bool __stdcall clrhost::clr_context_framework::AssemblyResolve(const char* name, const void*& outData, size_t& outSize)
{
    auto* resource = clrhost::resource::find(name);
    if (resource)
    {
        outData = resource->ptr;
        outSize = resource->size;
        return true;
    }

    return false;
}

std::unique_ptr<clrhost::clr_context> clrhost::clr_context::Initialize(const char* domainName, clr_init_flags flags)
{
    if (flags & ClrInit_TryCoreCLR)
    {
        auto coreclr = std::make_unique<clr_context_coreclr>();
        if (coreclr->Initialize(domainName)) return std::move(coreclr);
    }

    if (flags & ClrInit_TryFramework)
    {
        auto framework = std::make_unique<clr_context_framework>();
        if (framework->Initialize(domainName)) return std::move(framework);
    }

    return nullptr;
}
