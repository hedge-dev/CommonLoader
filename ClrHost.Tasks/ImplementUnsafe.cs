namespace ClrHost.Tasks;
using dnlib.DotNet;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using System.IO;
using System.Linq;
using dnlib.DotNet.Writer;
public class ImplementUnsafe : Task
{
    [Required]
    public string Input { get; set; }

    public string Output { get; set; }
    
    public override bool Execute()
    {
        if (string.IsNullOrEmpty(Output))
        {
            Output = Input;
        }

        if (!File.Exists(Input))
        {
            Log.LogError($"Input '{Input}' does not exist");
            return false;
        }

        var assemblyDef = AssemblyDef.Load(File.ReadAllBytes(Input), new ModuleCreationOptions());

        foreach (var module in assemblyDef.Modules)
        {
            foreach (var type in module.Types)
            {
                if (type.FullName == "System.Runtime.CompilerServices.Unsafe")
                {
                    foreach(var method in type.Methods)
                    {
                        UnsafeIntrinsicsProcessor.ProcessMethod(method);
                    }
                }
            }
        }

        assemblyDef.Write(Output, new ModuleWriterOptions(assemblyDef.Modules.First())
        {
            WritePdb = true,
        });

        return true;
    }
}