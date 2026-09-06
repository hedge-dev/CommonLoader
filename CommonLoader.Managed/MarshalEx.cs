namespace CommonLoader.Managed;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class MarshalEx
{
    public static Dictionary<string, nint> StringSymbols { get; } = new(128);
    public static unsafe nint StringToHGlobalUTF8(string str)
    {
        if (string.IsNullOrEmpty(str))
        {
            return 0;
        }

        fixed(char* pStr = str)
        {
            var mbLength = WideCharToMultiByte(CP_UTF8, 0, pStr, str.Length, 
                0, 0, 0, 0);

            var mbPtr = Marshal.AllocHGlobal(mbLength + 1);
            ((byte*)mbPtr)[mbLength] = 0;

            var converted = WideCharToMultiByte(CP_UTF8, 0, pStr, str.Length,
                mbPtr, mbLength, 0, 0);
            
            return mbPtr;
        }
    }

    public static nint StringToHGlobalSymbolUTF8(string str)
    {
        if (string.IsNullOrEmpty(str)) return 0;

        if (StringSymbols.TryGetValue(str, out var value)) return value;

        value = StringToHGlobalUTF8(str);
        StringSymbols.Add(str, value);

        return value;
    }

    public const uint CP_UTF8 = 65001;

    [DllImport("kernel32.dll")]
    public unsafe extern static nint WideCharToMultiByte(uint cp, uint flags, void* lpWideChar, nint cchWideChar, nint lpMultibyte, nint cbMultiByte, nint lpDefaultChar, nint outDefaultCharUsed);
}