namespace ClrHost.Tasks;
using dnlib.DotNet;
using dnlib.DotNet.Emit;

public static class AssemblyHelpers
{
    public static void SetOpCodes(this MethodDef method, params OpCode[] opcodes)
    {
        var body = new CilBody();
        method.Body = body;

        foreach (var op in opcodes)
        {
            body.Instructions.Add(op.ToInstruction());
        }
    }

    public static void Emit(this MethodDef method, OpCode op)
    {
        method.Body.Instructions.Add(op.ToInstruction());
    }

    public static void Emit(this MethodDef method, Instruction inst)
    {
        method.Body.Instructions.Add(inst);
    }
}