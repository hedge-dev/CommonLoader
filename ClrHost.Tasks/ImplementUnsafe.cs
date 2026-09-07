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
    public string TargetType { get; set; }

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

        if (string.IsNullOrEmpty(TargetType))
        {
            TargetType = "System.Runtime.CompilerServices.Unsafe";
        }

        var assemblyDef = AssemblyDef.Load(File.ReadAllBytes(Input), new ModuleCreationOptions());
        var targetFound = false;

        foreach (var module in assemblyDef.Modules)
        {
            foreach (var type in module.Types)
            {
                if (type.FullName == TargetType)
                {
                    targetFound = true;
                    foreach (var method in type.Methods)
                    {
                        UnsafeIntrinsicsProcessor.ProcessMethod(method);
                    }
                }
            }
        }

        if (!targetFound)
        {
            Log.LogError($"Could not find type ({TargetType}) in assembly ({assemblyDef.Name})");
            return false;
        }

        assemblyDef.Write(Output, new ModuleWriterOptions(assemblyDef.Modules.First())
        {
            WritePdb = true,
        });

        return true;
    }
}