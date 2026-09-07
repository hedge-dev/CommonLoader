using System;
namespace CommonLoader.Managed;

public interface IMemoryProvider
{
    byte[] ReadMemory(IntPtr address, IntPtr length);
    T ReadMemory<T>(IntPtr address);
    void WriteMemory(IntPtr address, IntPtr dataPtr, IntPtr length);
    void WriteMemory<T>(IntPtr address, T data);
    void WriteMemory<T>(IntPtr address, T[] data);

    IntPtr ScanSignature(byte[] pattern, string mask);
    void WriteASMHook(string instructions, IntPtr address, int behavior, int parameter);
    bool GetAssemblerSymbol(string name, out ulong value);
    void SetAssemblerSymbol(string name, ulong value);
    bool RemoveAssemblerSymbol(string name);
    byte[] AssembleInstructions(string instructions);
    bool IsMemoryWritable(nint address);
}