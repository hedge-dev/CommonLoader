#nullable enable
namespace CommonLoader.Managed;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

public class CodeObject : IDisposable
{
    public static IReadOnlyDictionary<string, string> EmptyMetadata = new Dictionary<string, string>();

    public object Instance { get; private set; }
    public Type BaseType { get; private set; }
    public Action? InitAction { get; private set; }
    public Action? FrameAction { get; private set; }

    public string ID { get; }
    public string Name { get; }
    public string? Author { get; }
    public string? Category { get; }
    public string FullName => string.IsNullOrEmpty(Category) ? Name : $"{Category}/{Name}";

    public IReadOnlyDictionary<string, string> Metadata { get; private set; }
    public IntPtr NativeInfo { get; private set; }

    public CodeObject(Type type)
    {
        BaseType = type;
        Name = type.Name;
        var underscoreIdx = Name.IndexOf('_');
        if (underscoreIdx > 0)
        {
            Name = Name.Substring(0, underscoreIdx);
        }

        Instance = Activator.CreateInstance(type);

        var initMethod = type.GetMethod("Init");
        var frameMethod = type.GetMethod("OnFrame");

        if (initMethod != null) InitAction = (Action)Delegate.CreateDelegate(typeof(Action), Instance, initMethod);
        if (frameMethod != null) FrameAction = (Action)Delegate.CreateDelegate(typeof(Action), Instance, frameMethod);

        var metadataField = type.GetField("__META__", BindingFlags.Public | BindingFlags.Static);
        Metadata = (metadataField?.GetValue(null) as IReadOnlyDictionary<string, string>) ?? EmptyMetadata;

        Name = GetMetadata("Name") ?? Name;
        Author = GetMetadata("Author");
        Category = GetMetadata("Category");

        // FullName won't work without ID
        ID = GetMetadata("ID") ?? FullName;

        NativeInfo = NativeCodeInfo.Build(this);
    }

    public string? GetMetadata(string name)
    {
        if (Metadata.TryGetValue(name, out var value)) return value;

        return null;
    }

    protected void Dispose(bool disposing)
    {
        if (NativeInfo != IntPtr.Zero)
        {
            NativeCodeInfo.Destroy(NativeInfo);
            NativeInfo = IntPtr.Zero;
        }
        
        if (disposing)
        {
            GC.SuppressFinalize(this);
        }
    }

    public void Dispose()
    {
        Dispose(true);
    }

    ~CodeObject()
    {
        Dispose(false);
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct NativeCodeInfo : IDisposable
    {
        public int Size;
        public nint ID;
        public nint Name;
        public nint FullName;
        public nint Author;
        public nint Category;
        public nint ModID;
        public nint Type;
        public nint MetadataLength;
        public MetadataPair* Metadata;

        public NativeCodeInfo(CodeObject code)
        {
            Size = Marshal.SizeOf<NativeCodeInfo>();
            ID = MarshalEx.StringToHGlobalUTF8(code.ID);
            Name = MarshalEx.StringToHGlobalUTF8(code.Name);
            Author = 0;
            Category = 0;
            ModID = 0;
            Type = 0;
            FullName = Name;

            if (code.Name != code.FullName)
            {
                FullName = MarshalEx.StringToHGlobalSymbolUTF8(code.FullName);
            }

            if (code.Author != null)
                Author = MarshalEx.StringToHGlobalUTF8(code.Author);

            if (code.Category != null)
                Category = MarshalEx.StringToHGlobalUTF8(code.Category);

            if (code.Metadata.TryGetValue("Type", out var codeType))
                Type = MarshalEx.StringToHGlobalSymbolUTF8(codeType);

            if (code.Metadata.TryGetValue("Mod.ID", out var modID))
                ModID = MarshalEx.StringToHGlobalUTF8(modID);

            var metadata = code.Metadata.Where(x => !IsBasicField(x.Key)).ToList();
            if (metadata.Count > 0)
            {
                MetadataLength = metadata.Count;
                Metadata = (MetadataPair*)Marshal.AllocHGlobal(Unsafe.SizeOf<MetadataPair>() * MetadataLength);

                var i = 0;
                foreach (var data in metadata)
                {
                    Metadata[i++] = new MetadataPair(data.Key, data.Value);
                }
            }
        }

        public void Dispose()
        {
            if (Name == FullName)
                FullName = 0;

            Marshal.FreeHGlobal(ID);
            Marshal.FreeHGlobal(Name);

            if (FullName != 0)
                Marshal.FreeHGlobal(FullName);

            if (Author != 0)
                Marshal.FreeHGlobal(Author);
            
            if (Category != 0)
                Marshal.FreeHGlobal(Category);

            if (ModID != 0)
                Marshal.FreeHGlobal(ModID);

            if (Metadata != null)
            {
                for(int i = 0; i < MetadataLength; i++)
                {
                    Metadata[i].Dispose();
                }

                Marshal.FreeHGlobal((IntPtr)Metadata);
            }

            ID = 0;
            Name = 0;
            Author = 0;
            Category = 0;
            ModID = 0;
            Type = 0; // No need to free this one, it's pooled
            Metadata = null;
        }

        public static nint Build(CodeObject code)
        {
            var nativeData = Marshal.AllocHGlobal(Marshal.SizeOf<NativeCodeInfo>());
            *(NativeCodeInfo*)nativeData = new(code);

            return nativeData;
        }

        public static void Destroy(nint info)
        {
            unsafe
            {
                if (info == 0) return;
                ((NativeCodeInfo*)info)->Dispose();

                Marshal.FreeHGlobal(info);
            }
        }

        public static bool IsBasicField(string key)
        {
            switch(key)
            {
                case nameof(ID):
                case nameof(Name):
                case nameof(Author):
                case nameof(Category):
                case nameof(Type):
                case "Mod.ID":
                    return true;

                default:
                    return false;
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct MetadataPair : IDisposable
    {
        public nint Key;
        public nint Value;

        public MetadataPair() { }
        public MetadataPair(string key, string value)
        {
            Key = MarshalEx.StringToHGlobalUTF8(key);
            Value = MarshalEx.StringToHGlobalUTF8(value);
        }

        public void Dispose()
        {
            if (Key != 0) Marshal.FreeHGlobal(Key);
            if (Value != 0) Marshal.FreeHGlobal(Value);

            Key = 0;
            Value = 0;
        }
    }
}