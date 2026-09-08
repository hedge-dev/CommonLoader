#nullable enable
namespace CommonLoader.Managed;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

public static class CodeLoader
{
    public static List<Assembly> LoadedAssemblies { get; private set; } = new(16);
    public static List<CodeObject> Codes { get; private set; } = new(64);
    public static nint[] NativeCodeInfos { get; set; } = [];
    public static GCHandle NativeCodeInfosHandle { get; set; }
    public static bool InitializersRaised { get; set; }

    public static event Action? UpdateEvents;
    public static Pointer<NativeContext> NativeLoader;

    // .NET Framework can't marshal a delegate* for some reason.
    // Why does it even try to marshal an unmanaged reference parameter?
    public static void Initialize(nint loader)
    {
        NativeLoader = new(loader);
        MemoryProvider.Initialize(NativeLoader.Ref().ApiTable);
    }

    public static unsafe void Log(LogLevel level, string message)
    {
        fixed(char* msg = message)
        {
            NativeLoader.Ref().Log((nint)level, (nint)msg);
        }
    }

    public static CodeObject? RegisterCode(Type type)
    {
        var executableMethod = type.GetMethod("IsLoaderExecutable");
        if (executableMethod?.Invoke(null, null) is true)
        {
            var code = new CodeObject(type);
            if (code.FrameAction != null)
            {
                UpdateEvents += code.FrameAction;
            }

            Codes.Add(code);
            return code;
        }

        return null;
    }

    public static nint DisableCode(nint info)
    {
        var codeIdx = Codes.FindIndex(c => c.NativeInfo == info);
        if (codeIdx < 0) return 0;

        if (InitializersRaised)
        {
            Logger.Warning($"An attempt was made to disable a code after initializing. Code: ({Codes[codeIdx].Name})");
            return 0;
        }

        UpdateEvents -= Codes[codeIdx].FrameAction;
        Codes[codeIdx].Dispose();
        Codes.RemoveAt(codeIdx);

        for (int i = codeIdx; i < NativeCodeInfos.Length - 1; i++)
        {
            NativeCodeInfos[i] = NativeCodeInfos[i + 1];
        }

        NativeLoader.Ref().NumCodes--;

        return 1;
    }

    public static void RefreshNativeInfo()
    {
        if (Codes.Count != NativeCodeInfos.Length)
        {
            NativeCodeInfos = new nint[Codes.Count];
            for (int i = 0; i < NativeCodeInfos.Length; i++)
            {
                NativeCodeInfos[i] = Codes[i].NativeInfo;
            }

            if (NativeCodeInfosHandle.IsAllocated)
            {
                NativeCodeInfosHandle.Free();
            }

            NativeCodeInfosHandle = GCHandle.Alloc(NativeCodeInfos, GCHandleType.Pinned);
            
            NativeLoader.Ref().NumCodes = Codes.Count;
            NativeLoader.Ref().Codes = NativeCodeInfosHandle.AddrOfPinnedObject();
        }
    }

    public static void RegistryAssembly(Assembly assembly)
    {
        foreach (var type in assembly.GetExportedTypes()) 
        {
            if (RegisterCode(type) != null) continue;

            if (type.Name == "MemoryService")
            {
                var registerMethod = type.GetMethod("RegisterProvider");
                registerMethod?.Invoke(null, [ MemoryProvider.Instance ]);
            }
        }

        RefreshNativeInfo();
    }

    public static nint LoadFile([MarshalAs(UnmanagedType.LPWStr)] string path)
    {
        path = Path.GetFullPath(path);

        if (!File.Exists(path)) return 0;

        try
        {
            RegistryAssembly(Assembly.Load(File.ReadAllBytes(path)));
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine($"Failed to load codes, {ex.Message}");
        }

        return 1;
    }

    public static nint GetCodesPtr()
    {
        if (!NativeCodeInfosHandle.IsAllocated) return 0;

        return NativeCodeInfosHandle.AddrOfPinnedObject();
    }

    public static nint GetCodes(nint outCodes, nint numOutCodes)
    {
        if (Codes.Count > numOutCodes) return numOutCodes;
        
        numOutCodes = Math.Min(Codes.Count, (int)numOutCodes);
        for (int i = 0; i < numOutCodes; i++)
        {
            Unsafe.Add(ref Unsafe.AsRef<nint>(outCodes), i) = Codes[i].NativeInfo;
        }

        return numOutCodes;
    }

    public static nint GetNumCodes()
    {
        return Codes.Count;
    }

    public static nint GetCode(nint idx)
    {
        if (idx > Codes.Count) return 0;

        return Codes[(int)idx].NativeInfo;
    }

    public static void RaiseInitializers()
    {
        InitializersRaised = true;
        foreach(var code in Codes)
        {
            Logger.Info($"Loading Code: {code.FullName}");
            code.InitAction?.Invoke();
        }
    }

    public static void RaiseUpdates()
    {
        UpdateEvents?.Invoke();
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct NativeContext
    {
        public nint ApiTable;

        // size_t level, const wchar_t* message
        public delegate* unmanaged[Stdcall]<nint, nint, void> LogCallback;
        public nint NumCodes;
        public nint Codes;

        public readonly void Log(nint level, nint msg)
            => LogCallback(level, msg);
    }
}