// This only exists to generated C# using clang -E CommonLoaderAPI.h

#define CMN_LOADER_STATE_SKIP_SIG_VALIDATION 0
#define CMN_LOADER_STATE_INIT_SIG_SCAN_FAILED 1
#define CMN_LOADER_STATE_INIT_ASSEMBLY_FAILED 2
#define CMN_LOADER_STATE_DISABLE_LOGGING 3
#define CMN_LOADER_STATE_MAX 4
#define CMN_LOADER_STATE_INVALID -1

#define CONCAT2(A, B) A##B
#define CONCAT(A, B) CONCAT2(A, B)

#ifndef DECLARE_API_FUNC
    #define DECLARE_API_FUNC(RETURN_TYPE, NAME, ...)
#endif

#ifndef CMN_EMIT
    #define CMN_EMIT

    // Emit delegates
    #define CMN_EMIT_DELEGATES
    #define DECLARE_API_FUNC(RETURN_TYPE, NAME, ...) [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate RETURN_TYPE CONCAT(NAME, Delegate)(__VA_ARGS__);
    #include "CommonLoaderAPI.h"
    #undef CMN_EMIT_DELEGATES

#define DECLARE_API_FUNC(RETURN_TYPE, NAME, ...) public CONCAT(NAME, Delegate) NAME;
#include "CommonLoaderAPI.h"

#define DECLARE_API_FUNC(RETURN_TYPE, NAME, ...) NAME = GetFunction<CONCAT(NAME, Delegate)>(ApiFunctions.NAME);
public CommonLoaderAPI(IntPtr base)
{
    Base = base;
    #include "CommonLoaderAPI.h"
}

public static Type[] ApiDelegates =
{
    #define DECLARE_API_FUNC(RETURN_TYPE, NAME, ...) typeof(CONCAT(NAME, Delegate)),
    #include "CommonLoaderAPI.h"
};

public enum ApiFunctions 
{
    #define DECLARE_API_FUNC(RETURN_TYPE, NAME, ...) NAME,
    #include "CommonLoaderAPI.h"
}
    #define DECLARE_API_FUNC(RETURN_TYPE, NAME, ...)
#endif

    DECLARE_API_FUNC(int, GetVersion)
    DECLARE_API_FUNC(void*, ScanSignature, [MarshalAs(UnmanagedType.LPStr)] string pattern, [MarshalAs(UnmanagedType.LPStr)] string mask)
    DECLARE_API_FUNC(void*, ScanSignatureEx, [MarshalAs(UnmanagedType.LPStr)] string pattern, [MarshalAs(UnmanagedType.LPStr)] string mask, void* begin, nint size)
    DECLARE_API_FUNC(int, GetApplicationOption, [MarshalAs(UnmanagedType.LPStr)] string key, byte* outOption, nint out_size)
    DECLARE_API_FUNC(int, GetApplicationOptionEx, [MarshalAs(UnmanagedType.LPStr)] string section, [MarshalAs(UnmanagedType.LPStr)] string key, byte* outOption, nint out_size)
    DECLARE_API_FUNC(void, SaveApplicationOptions)
    DECLARE_API_FUNC(AssemblerResult*, CompileAssembly, [MarshalAs(UnmanagedType.LPStr)] string code)
    DECLARE_API_FUNC(void, FreeAssemblerObject, AssemblerResult*)
    DECLARE_API_FUNC(void, SetAssemblerSymbol, [MarshalAs(UnmanagedType.LPStr)] string name, uint64_t value)
    DECLARE_API_FUNC(bool, GetAssemblerSymbol, [MarshalAs(UnmanagedType.LPStr)] string name, uint64_t* out)
    DECLARE_API_FUNC(bool, RemoveAssemblerSymbol, [MarshalAs(UnmanagedType.LPStr)] string name)
    DECLARE_API_FUNC(void, SetState, nint state, nint value)
    DECLARE_API_FUNC(void, SetStateFlag, nint state, nint flag, bool set)
    DECLARE_API_FUNC(nint, GetState, nint state)
    DECLARE_API_FUNC(bool, FindCode, [MarshalAs(UnmanagedType.LPStr)] string id, Code_t* code)
    DECLARE_API_FUNC(bool, DisableCode, Code_t* code)
    DECLARE_API_FUNC(bool, WriteAsmHook, [MarshalAs(UnmanagedType.LPStr)] string instructions, void* address, int behavior, int parameter)