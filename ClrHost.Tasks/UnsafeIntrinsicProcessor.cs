namespace ClrHost.Tasks;
using dnlib.DotNet;
using dnlib.DotNet.Emit;
using System.Linq;

public static class UnsafeIntrinsicsProcessor
{
    public static void ProcessMethod(MethodDef method)
    {
        switch (method.Name)
        {
            case "AsPointer":
                method.SetOpCodes(OpCodes.Ldarg_0, OpCodes.Conv_U, OpCodes.Ret);
                break;

            case "SkipInit":
                method.SetOpCodes(OpCodes.Ret);
                break;

            case "AreSame":
                method.SetOpCodes(OpCodes.Ldarg_0, OpCodes.Ldarg_1, OpCodes.Ceq, OpCodes.Ret);
                break;

            case "InitBlock":
                method.EmitInitBlock(false);
                break;

            case "InitBlockUnaligned":
                method.EmitInitBlock(true);
                break;

            case "SizeOf":
                method.EmitSizeOf();
                break;

            case "As":
            case "AsRef":
                method.SetOpCodes(OpCodes.Ldarg_0, OpCodes.Ret);
                break;

            case "Add":
                method.EmitAdd();
                break;

            case "Read":
                method.EmitRead(false);
                break;

            case "ReadUnaligned":
                method.EmitRead(true);
                break;

            case "Copy":
                method.EmitCopy();
                break;

            case "CopyBlock":
                method.EmitCopyBlock(false);
                break;

            case "CopyBlockUnaligned":
                method.EmitCopyBlock(true);
                break;

        }
    }

    private static void EmitAdd(this MethodDef method)
    {
        var body = new CilBody();
        method.Body = body;

        var type = method.GenericParameters.First();

        method.Emit(OpCodes.Ldarg_0);
        method.Emit(OpCodes.Ldarg_1);
        method.Emit(OpCodes.Sizeof.ToInstruction(new GenericMVar(type.Number).ToTypeDefOrRef()));
        method.Emit(OpCodes.Mul);
        method.Emit(OpCodes.Add);
        method.Emit(OpCodes.Ret);
    }

    private static void EmitSizeOf(this MethodDef method)
    {
        var body = new CilBody();
        method.Body = body;

        var type = method.GenericParameters.First();

        method.Emit(OpCodes.Sizeof.ToInstruction(new GenericMVar(type.Number).ToTypeDefOrRef()));
        method.Emit(OpCodes.Ret);
    }

    private static void EmitRead(this MethodDef method, bool unaligned)
    {
        var body = new CilBody();
        method.Body = body;

        method.Emit(OpCodes.Ldarg_0);

        if (unaligned)
        {
            method.Emit(OpCodes.Unaligned.ToInstruction((byte)1));
        }

        var type = method.GenericParameters.First();
        method.Emit(OpCodes.Ldobj.ToInstruction(new GenericMVar(type.Number).ToTypeDefOrRef()));
        method.Emit(OpCodes.Ret);
    }

    private static void EmitCopy(this MethodDef method)
    {
        var body = new CilBody();
        method.Body = body;

        method.Emit(OpCodes.Ldarg_0);
        method.Emit(OpCodes.Ldarg_1);

        var type = method.GenericParameters.First();
        method.Emit(OpCodes.Ldobj.ToInstruction(new GenericMVar(type.Number).ToTypeDefOrRef()));
        method.Emit(OpCodes.Stobj.ToInstruction(new GenericMVar(type.Number).ToTypeDefOrRef()));
        method.Emit(OpCodes.Ret);
    }

    private static void EmitInitBlock(this MethodDef method, bool unaligned)
    {
        var body = new CilBody();
        method.Body = body;

        method.Emit(OpCodes.Ldarg_0);
        method.Emit(OpCodes.Ldarg_1);
        method.Emit(OpCodes.Ldarg_2);
        if (unaligned)
        {
            method.Emit(OpCodes.Unaligned.ToInstruction((byte)1));
        }
        method.Emit(OpCodes.Initblk);
        method.Emit(OpCodes.Ret);
    }

    private static void EmitCopyBlock(this MethodDef method, bool unaligned)
    {
        var body = new CilBody();
        method.Body = body;

        method.Emit(OpCodes.Ldarg_0);
        method.Emit(OpCodes.Ldarg_1);
        method.Emit(OpCodes.Ldarg_2);
        if (unaligned)
        {
            method.Emit(OpCodes.Unaligned.ToInstruction((byte)1));
        }
        method.Emit(OpCodes.Cpblk);
        method.Emit(OpCodes.Ret);
    }
}
