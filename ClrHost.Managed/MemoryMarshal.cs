#nullable enable
namespace ClrHost.Managed;

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;

public class MemoryMarshal
{
    // Enforce consistency across platforms
    public const CallingConvention DefaultCallingConvention = CallingConvention.StdCall;

    [UnmanagedFunctionPointer(DefaultCallingConvention)]
    public delegate IntPtr DelegateCreateDelegate(string assemblyName, string typeName, string methodName);

    [UnmanagedFunctionPointer(DefaultCallingConvention)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public delegate bool AssemblyResolveCallback([MarshalAs(UnmanagedType.LPStr)] string name, out IntPtr data, out IntPtr size);

    public static AssemblyBuilder DynamicAssemblyBuilder { get; }
    public static ModuleBuilder DynamicModuleBuilder { get; }
    public static DelegateCreateDelegate CreateDelegatePtr { get; } = CreateDelegate;
    public static Dictionary<string, Type> DelegateTypeCache { get; } = new(128);
    public static Dictionary<string, IntPtr> DelegateCache { get; } = new(128);
    public static AssemblyResolveCallback? ResolveCallback { get; private set; }

    static MemoryMarshal()
    {
        DynamicAssemblyBuilder = AssemblyBuilder.DefineDynamicAssembly(new("ClrHost.Managed.DynamicAssembly"), AssemblyBuilderAccess.Run);
        DynamicModuleBuilder = DynamicAssemblyBuilder.DefineDynamicModule("DynamicAssembly");

        AppDomain.CurrentDomain.AssemblyResolve += AssemblyResolve;
    }

    private static Assembly? AssemblyResolve(object sender, ResolveEventArgs args)
    {
        if (ResolveCallback == null) return null;

        if (!ResolveCallback($"{args.Name}.dll", out var dataPtr, out var size)) return null;

        var data = new byte[size.ToInt32()];
        Marshal.Copy(dataPtr, data, 0, data.Length);

        return Assembly.Load(data);
    }

    public static void SetAssemblyResolve(AssemblyResolveCallback callback)
    {
        ResolveCallback = callback;
    }

    public static IntPtr GetCreateDelegatePtr()
    {
        return Marshal.GetFunctionPointerForDelegate(CreateDelegatePtr);
    }

    // Workaround for .NET Framework hosts
    public static IntPtr CreateDelegate([MarshalAs(UnmanagedType.LPStr)] string assemblyName,
        [MarshalAs(UnmanagedType.LPStr)] string typeName, [MarshalAs(UnmanagedType.LPStr)] string methodName)
    {
        var cacheName = $"{assemblyName}+{typeName}+{methodName}";
        if (DelegateCache.TryGetValue(cacheName, out var cacheImpl)) return cacheImpl;

        Assembly assembly = AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(x => x.GetName().Name == assemblyName);
        try
        {
            if (assembly == null)
            {
                assembly = AppDomain.CurrentDomain.Load(new AssemblyName(assemblyName));
            }
        }
        catch (Exception ex)
        {
            Debug.WriteLine(ex);
        }

        if (assembly == null)
            return IntPtr.Zero;

        var type = assembly.GetType(typeName, false);
        if (type == null) return IntPtr.Zero;

        var method = type.GetMethod(methodName);
        if (method == null) return IntPtr.Zero;

        var delegateType = GenerateDelegateType(method);
        var del = method.CreateDelegate(delegateType);
        var funcPtr = Marshal.GetFunctionPointerForDelegate(del);

        DelegateCache.Add(cacheName, funcPtr);
        return funcPtr;
    }

    public static Type GenerateDelegateType(MethodInfo method)
    {
        var name = $"{method.ReturnType.Name}^{string.Join("+", [.. method.GetParameters().Select(p => p.ParameterType.FullName)])}";
        if (DelegateTypeCache.TryGetValue(name, out var type)) return type;

        var typeBuilder = DynamicModuleBuilder.DefineType(
            name, TypeAttributes.Sealed | TypeAttributes.Public, typeof(MulticastDelegate));

        var attributeConstructor = typeof(UnmanagedFunctionPointerAttribute).GetConstructor([typeof(CallingConvention)]);
        typeBuilder.SetCustomAttribute(new CustomAttributeBuilder(attributeConstructor, [DefaultCallingConvention]));

        var constructor = typeBuilder.DefineConstructor(
            MethodAttributes.RTSpecialName | MethodAttributes.HideBySig | MethodAttributes.Public,
            CallingConventions.Standard, [typeof(object), typeof(IntPtr)]);

        constructor.SetImplementationFlags(MethodImplAttributes.CodeTypeMask);

        var parameters = method.GetParameters();

        var invokeMethod = typeBuilder.DefineMethod(
            "Invoke", MethodAttributes.HideBySig | MethodAttributes.Virtual | MethodAttributes.Public,
            method.ReturnType, [.. parameters.Select(p => p.ParameterType)]);

        invokeMethod.SetImplementationFlags(MethodImplAttributes.CodeTypeMask);

        for (int i = 0; i < parameters.Length; i++)
        {
            var parameter = parameters[i];
            invokeMethod.DefineParameter(i + 1, parameter.Attributes, parameter.Name);
        }

        type = typeBuilder.CreateType();
        DelegateTypeCache.Add(name, type);

        return type;
    }
}