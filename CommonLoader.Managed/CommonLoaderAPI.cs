namespace CommonLoader.Managed;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

public struct CommonLoaderAPI
{
    public IntPtr Base;
    public GetVersionDelegate GetVersion;
    public ScanSignatureDelegate ScanSignature;
    public ScanSignatureExDelegate ScanSignatureEx;
    public GetApplicationOptionDelegate GetApplicationOption;
    public GetApplicationOptionExDelegate GetApplicationOptionEx;
    public SaveApplicationOptionsDelegate SaveApplicationOptions;
    public CompileAssemblyDelegate CompileAssembly;
    public FreeAssemblerObjectDelegate FreeAssemblerObject;
    public SetAssemblerSymbolDelegate SetAssemblerSymbol;
    public GetAssemblerSymbolDelegate GetAssemblerSymbol;
    public RemoveAssemblerSymbolDelegate RemoveAssemblerSymbol;
    public SetStateDelegate SetState;
    public SetStateFlagDelegate SetStateFlag;
    public GetStateDelegate GetState;
    public FindCodeDelegate FindCode;
    public DisableCodeDelegate DisableCode;
    public WriteAsmHookDelegate WriteAsmHook;

    public CommonLoaderAPI(IntPtr basePtr)
    {
        Base = basePtr;
        GetVersion = GetFunction<GetVersionDelegate>(ApiFunctions.GetVersion);
        ScanSignature = GetFunction<ScanSignatureDelegate>(ApiFunctions.ScanSignature);
        ScanSignatureEx = GetFunction<ScanSignatureExDelegate>(ApiFunctions.ScanSignatureEx);
        GetApplicationOption = GetFunction<GetApplicationOptionDelegate>(ApiFunctions.GetApplicationOption);
        GetApplicationOptionEx = GetFunction<GetApplicationOptionExDelegate>(ApiFunctions.GetApplicationOptionEx);
        SaveApplicationOptions = GetFunction<SaveApplicationOptionsDelegate>(ApiFunctions.SaveApplicationOptions);
        CompileAssembly = GetFunction<CompileAssemblyDelegate>(ApiFunctions.CompileAssembly);
        FreeAssemblerObject = GetFunction<FreeAssemblerObjectDelegate>(ApiFunctions.FreeAssemblerObject);
        SetAssemblerSymbol = GetFunction<SetAssemblerSymbolDelegate>(ApiFunctions.SetAssemblerSymbol);
        GetAssemblerSymbol = GetFunction<GetAssemblerSymbolDelegate>(ApiFunctions.GetAssemblerSymbol);
        RemoveAssemblerSymbol = GetFunction<RemoveAssemblerSymbolDelegate>(ApiFunctions.RemoveAssemblerSymbol);
        SetState = GetFunction<SetStateDelegate>(ApiFunctions.SetState);
        SetStateFlag = GetFunction<SetStateFlagDelegate>(ApiFunctions.SetStateFlag);
        GetState = GetFunction<GetStateDelegate>(ApiFunctions.GetState);
        FindCode = GetFunction<FindCodeDelegate>(ApiFunctions.FindCode);
        DisableCode = GetFunction<DisableCodeDelegate>(ApiFunctions.DisableCode);
        WriteAsmHook = GetFunction<WriteAsmHookDelegate>(ApiFunctions.WriteAsmHook);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public readonly TDelegate GetFunction<TDelegate>(ApiFunctions function) where TDelegate : Delegate
    {
        ref var apiStart = ref Unsafe.AsRef<IntPtr>(Base);
        var apiPtr = Unsafe.Add(ref apiStart, (int)function);

        return (TDelegate)Marshal.GetDelegateForFunctionPointer(apiPtr, ApiDelegates[(int)function]);
    }

    public static Type[] ApiDelegates =
    {
        typeof(GetVersionDelegate),
        typeof(ScanSignatureDelegate),
        typeof(ScanSignatureExDelegate),
        typeof(GetApplicationOptionDelegate),
        typeof(GetApplicationOptionExDelegate),
        typeof(SaveApplicationOptionsDelegate),
        typeof(CompileAssemblyDelegate),
        typeof(FreeAssemblerObjectDelegate),
        typeof(SetAssemblerSymbolDelegate),
        typeof(GetAssemblerSymbolDelegate),
        typeof(RemoveAssemblerSymbolDelegate),
        typeof(SetStateDelegate),
        typeof(SetStateFlagDelegate),
        typeof(GetStateDelegate),
        typeof(FindCodeDelegate),
        typeof(WriteAsmHookDelegate)
    };

    public enum ApiFunctions
    {
        GetVersion,
        ScanSignature,
        ScanSignatureEx,
        GetApplicationOption,
        GetApplicationOptionEx,
        SaveApplicationOptions,
        CompileAssembly,
        FreeAssemblerObject,
        SetAssemblerSymbol,
        GetAssemblerSymbol,
        RemoveAssemblerSymbol,
        SetState,
        SetStateFlag,
        GetState,
        FindCode,
        DisableCode,
        WriteAsmHook
    }
}

public enum HookBehavior
{
    Before, After, Replace
}

public enum HookParameter
{
    Jump, Call
}

public unsafe struct AssemblerResult
{
    public nint NumInstructions;
    public nint Length;
    public IntPtr Data;
    public AssemblerError* Errors;
    public nint NumErrors;
}

[StructLayout(LayoutKind.Sequential)]
public unsafe struct AssemblerError
{
    public nint LineNum;
    public nint LineChar;
    public uint ErrNum;
    public fixed byte Line[256];
}


// Generated using clang -E CommonLoaderAPI.h
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate int GetVersionDelegate();
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate IntPtr ScanSignatureDelegate(ref byte pattern, ref byte mask);
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate void* ScanSignatureExDelegate([MarshalAs(UnmanagedType.LPStr)] string pattern, [MarshalAs(UnmanagedType.LPStr)] string mask, void* begin, nint size);
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate int GetApplicationOptionDelegate([MarshalAs(UnmanagedType.LPStr)] string key, byte* outOption, nint out_size);
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate int GetApplicationOptionExDelegate([MarshalAs(UnmanagedType.LPStr)] string section, [MarshalAs(UnmanagedType.LPStr)] string key, byte* outOption, nint out_size);
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate void SaveApplicationOptionsDelegate();
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate ref AssemblerResult CompileAssemblyDelegate([MarshalAs(UnmanagedType.LPStr)] string code);
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate void FreeAssemblerObjectDelegate(ref AssemblerResult result);
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate void SetAssemblerSymbolDelegate([MarshalAs(UnmanagedType.LPStr)] string name, ulong value);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
[return:  MarshalAs(UnmanagedType.Bool)]
public unsafe delegate bool GetAssemblerSymbolDelegate([MarshalAs(UnmanagedType.LPStr)] string name, out ulong value);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
[return: MarshalAs(UnmanagedType.Bool)]
public unsafe delegate bool RemoveAssemblerSymbolDelegate([MarshalAs(UnmanagedType.LPStr)] string name);
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate void SetStateDelegate(nint state, nint value);
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate void SetStateFlagDelegate(nint state, nint flag, bool set);
[UnmanagedFunctionPointer(CallingConvention.Cdecl)] public unsafe delegate nint GetStateDelegate(nint state);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
[return: MarshalAs(UnmanagedType.Bool)] 
public unsafe delegate bool DisableCodeDelegate(CodeObject.NativeCodeInfo* code);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
[return: MarshalAs(UnmanagedType.Bool)] 
public unsafe delegate bool FindCodeDelegate([MarshalAs(UnmanagedType.LPStr)] string id, out CodeObject.NativeCodeInfo* code);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
[return: MarshalAs(UnmanagedType.Bool)]
public unsafe delegate bool WriteAsmHookDelegate([MarshalAs(UnmanagedType.LPStr)] string instructions, IntPtr address, int behavior, int parameter);