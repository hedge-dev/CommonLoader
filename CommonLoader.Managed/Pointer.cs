namespace CommonLoader.Managed;
using System.Runtime.CompilerServices;

public unsafe struct Pointer<T> where T: unmanaged
{
    public nint Value;

    public Pointer() { }

    public Pointer(nint value)
    {
        Set(value);
    }

    public Pointer(ref T value)
    {
        Set(ref value);
    }

    public void Set(nint value)
    {
        Value = value;
    }

    public void Set(ref T value)
    {
        Value = (nint)Unsafe.AsPointer(ref value);
    }

    public readonly ref T Get()
    {
        return ref Unsafe.AsRef<T>((void*)Value);
    }
}