namespace System.Runtime.CompilerServices;

public static unsafe class Unsafe
{
    public static TTo BitCast<TFrom, TTo>(TFrom source)
        where TFrom : struct
        where TTo : struct
    {
        return ReadUnaligned<TTo>(ref As<TFrom, byte>(ref source));
    }

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void* AsPointer<T>(ref T value);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern int SizeOf<T>();

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern T As<T>(object? o) where T : class?;

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern ref TTo As<TFrom, TTo>(ref TFrom source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern ref T AsRef<T>(IntPtr source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern ref T AsRef<T>(void* source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern ref T AsRef<T>(in T source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void Copy<T>(void* destination, in T source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void Copy<T>(ref T destination, void* source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void CopyBlock(IntPtr destination, IntPtr source, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void CopyBlock(void* destination, void* source, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void CopyBlock(ref byte destination, ref byte source, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void CopyBlockUnaligned(IntPtr destination, IntPtr source, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void CopyBlockUnaligned(void* destination, void* source, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void CopyBlockUnaligned(ref byte destination, ref byte source, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern ref T Add<T>(ref T source, int n);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern ref T Add<T>(ref T source, IntPtr n);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void* Add<T>(void* source, int n);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void* Add<T>(void* source, nint n);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void SkipInit<T>(out T value);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern bool AreSame<T>(ref T left, ref T right);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void InitBlock(void* startAddress, byte value, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void InitBlock(ref byte startAddress, byte value, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void InitBlockUnaligned(void* startAddress, byte value, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern void InitBlockUnaligned(ref byte startAddress, byte value, uint byteCount);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern T Read<T>(void* source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern T Read<T>(ref byte source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern T ReadUnaligned<T>(void* source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static extern T ReadUnaligned<T>(ref byte source);

    [CompilerGenerated]
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ref T NullRef<T>() => ref AsRef<T>(IntPtr.Zero);
}