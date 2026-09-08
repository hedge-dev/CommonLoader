namespace ClrHost.Managed;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;

public class TypeMarshal
{
    public static AssemblyBuilder DynamicAssemblyBuilder => MemoryMarshal.DynamicAssemblyBuilder;
    public static ModuleBuilder DynamicModuleBuilder => MemoryMarshal.DynamicModuleBuilder;
    public static MethodInfo ToStringMethod { get; } = typeof(object).GetMethod("ToString");
    public static Dictionary<string, Type> ProxyCache { get; } = new Dictionary<string, Type>(16);

    public static TInterface CreateProxy<TInterface>(object instance)
    {
        if (instance is TInterface iface) return iface;

        return (TInterface)CreateProxy(instance, typeof(TInterface));
    }

    public static object CreateProxy(object instance, Type proxyType)
    {
        var instType = instance.GetType();
        var proxyTypeName = $"Proxy+{instType.FullName}+{proxyType.FullName}";
        if (ProxyCache.TryGetValue(proxyTypeName, out var proxyImpl))
        {
            return Activator.CreateInstance(proxyImpl, [ instance ]);
        }

        var proxyBuilder = DynamicModuleBuilder.DefineType(proxyTypeName, TypeAttributes.Public, typeof(object), [ proxyType ]);
        var proxyInstField = proxyBuilder.DefineField("Instance", instType, FieldAttributes.Public | FieldAttributes.InitOnly);
        
        // Build constructor
        {
            var ctBuilder = proxyBuilder.DefineConstructor(MethodAttributes.Public, CallingConventions.Standard, [ instType ]);
            var generator = ctBuilder.GetILGenerator();
            
            generator.Emit(OpCodes.Ldarg_0);
            generator.Emit(OpCodes.Ldarg_1);
            generator.Emit(OpCodes.Stfld, proxyInstField);
            generator.Emit(OpCodes.Ret);
        }

        // Build ToString()
        {
            var toStringBuilder = proxyBuilder.DefineMethod("ToString", MethodAttributes.Public | MethodAttributes.Virtual, typeof(string), []);
            proxyBuilder.DefineMethodOverride(toStringBuilder, ToStringMethod);
            var generator = toStringBuilder.GetILGenerator();

            generator.Emit(OpCodes.Ldarg_0);
            generator.Emit(OpCodes.Ldfld, proxyInstField);
            generator.Emit(OpCodes.Call, ToStringMethod);
            generator.Emit(OpCodes.Ret);
        }

        ImplementType(proxyType);
        foreach (var t in proxyType.GetInterfaces()) 
        {
            ImplementType(t);
        }

        proxyImpl = proxyBuilder.CreateType();
        ProxyCache.Add(proxyTypeName, proxyImpl);

        return Activator.CreateInstance(proxyImpl, [ instance ]);

        void ImplementType(Type type)
        {
            foreach (var method in type.GetMethods())
            {
                var methodParameters = method.GetParameters().Select(x => x.ParameterType).ToArray();
                
                var methodBuilder = proxyBuilder.DefineMethod(method.Name, MethodAttributes.Public | MethodAttributes.Virtual,
                    method.CallingConvention, method.ReturnType, methodParameters);

                var instMethod = instType.GetRuntimeMethod(method.Name, methodParameters);
                
                if (method.IsGenericMethod)
                {
                    instMethod = instType.GetMethods()
                        .FirstOrDefault(x => x.IsGenericMethod && x.Name == method.Name
                            && x.GetGenericArguments().Length == method.GetGenericArguments().Length
                            && x.GetParameters().Length ==  method.GetParameters().Length);

                    var genericParams = method.GetGenericArguments();
                    var genericNames = Enumerable.Range(0, genericParams.Length).Select(x => $"T_{x}").ToArray();

                    methodBuilder.DefineGenericParameters(genericNames);
                    methodBuilder.MakeGenericMethod(method.GetGenericArguments());
                }

                proxyBuilder.DefineMethodOverride(methodBuilder, method);

                var generator = methodBuilder.GetILGenerator();

                generator.Emit(OpCodes.Ldarg_0);
                generator.Emit(OpCodes.Ldfld, proxyInstField);

                for (int i = 0; i < methodParameters.Length; i++)
                {
                    generator.Emit(OpCodes.Ldarg, (short)(i + 1));
                }

                generator.Emit(OpCodes.Call, instMethod);
                generator.Emit(OpCodes.Ret);
            }
        }
    }
}