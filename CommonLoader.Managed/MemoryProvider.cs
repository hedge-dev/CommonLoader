namespace CommonLoader.Managed;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

public unsafe class MemoryProvider : IMemoryProvider
{
    public static MemoryProvider Instance { get; private set; } = new();
    public static CommonLoaderAPI Loader { get; private set; }
    public static nint ModuleBase { get; } = GetModuleHandle(null);

    // Big marshal cost, but it's fine as a one-off
    public static void Initialize(IntPtr api)
    {
        Loader = new(api);
    }

    #region Backwards Compatability

    public byte[] ReadMemory(IntPtr address, IntPtr length)
    {
        var buffer = new byte[(int)length];
        if (((nint)address % Unsafe.SizeOf<IntPtr>()) == 0)
        {
            Unsafe.CopyBlock(ref buffer[0], ref Unsafe.AsRef<byte>((void*)address), (uint)length);
        }
        else
        {
            Unsafe.CopyBlockUnaligned(ref buffer[0], ref Unsafe.AsRef<byte>((void*)address), (uint)length);
        }

        return buffer;
    }

    public T ReadMemory<T>(IntPtr address)
    {
        return Unsafe.AsRef<T>((void*)address);
    }

    public void WriteMemory(IntPtr address, IntPtr dataPtr, IntPtr length)
    {
        if (!IsMemoryWritable(address)) return;

        if (((nint)address % Unsafe.SizeOf<IntPtr>()) == 0
            && ((nint)dataPtr % Unsafe.SizeOf<IntPtr>()) == 0)
        {
            Unsafe.CopyBlock((void*)address, (void*)dataPtr, (uint)length);
        }
        else
        {
            Unsafe.CopyBlockUnaligned((void*)address, (void*)dataPtr, (uint)length);
        }
    }

    public void WriteMemory<T>(IntPtr address, T data)
    {
        WriteMemory(address, (IntPtr)Unsafe.AsPointer(ref data), (IntPtr)Unsafe.SizeOf<T>());
    }

    public void WriteMemory<T>(IntPtr address, T[] data)
    {
        WriteMemory(address, (IntPtr)Unsafe.AsPointer(ref data[0]), (IntPtr)(data.Length * Unsafe.SizeOf<T>()));
    }

    #endregion

    public IntPtr ScanSignature(byte[] pattern, string mask)
    {
        var maskBytes = Encoding.ASCII.GetBytes(mask);
        return Loader.ScanSignature(ref pattern[0], ref maskBytes[0]);
    }

    public void WriteASMHook(string instructions, IntPtr address, int behavior, int parameter)
    {
        if (!IsMemoryWritable(address))
            return;

        Loader.WriteAsmHook(instructions, address, behavior, parameter);
    }

    public void WriteASMHook(string instructions, IntPtr address, int behavior)
    {
        WriteASMHook(instructions, address, behavior, (int)HookParameter.Jump);
    }

    public bool GetAssemblerSymbol(string name, out ulong value)
    {
        return Loader.GetAssemblerSymbol(name, out value);
    }

    public void SetAssemblerSymbol(string name, ulong value)
    {
        Loader.SetAssemblerSymbol(name, value);
    }

    public bool RemoveAssemblerSymbol(string name)
    {
        return Loader.RemoveAssemblerSymbol(name);
    }

    public byte[] AssembleInstructions(string instructions)
    {
        ref var result = ref Loader.CompileAssembly(instructions);

        var bytes = new byte[result.Length];
        Unsafe.CopyBlock(ref bytes[0], ref Unsafe.AsRef<byte>((void*)result.Data), (uint)bytes.Length);

        return bytes;
    }

    public bool IsMemoryWritable(nint address)
        => address > ModuleBase;

    [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
    public static extern nint GetModuleHandle(string lpModuleName);
}