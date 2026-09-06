namespace ClrHost.Tasks;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using System;
using System.IO;
using System.Text.RegularExpressions;

public class HexDump : Task
{
    [Required]
    public string Input { get; set; }

    [Required]
    public string Output { get; set; }

    public string FieldName { get; set; }

    public int ColumnSize { get; set; } = 12;

    public override bool Execute()
    {
        if (!File.Exists(Input))
        {
            Log.LogError($"Input '{Input}' does not exist");
            return false;
        }

        if (string.IsNullOrEmpty(FieldName))
            FieldName = Regex.Replace(Path.GetFileName(Input), "[^\\w\\-_]", (m) => "_");

        using var inputStream = File.OpenRead(Input);
        using var outStream = File.Create(Output);

        using var writer = new StreamWriter(outStream);

        writer.WriteLine($"unsigned char const {FieldName}[] = {{");

        var bytesRead = 0;
        var dataBuffer = new byte[4096];
        bytesRead = inputStream.Read(dataBuffer, 0, dataBuffer.Length);
        var bytesWritten = 0;

        writer.Write(" ");
        while(bytesRead != 0)
        {
            for(int i = 0; i < bytesRead; i++)
            {
                // Using decimals gives smaller file sizes
                writer.Write($" {dataBuffer[i]},");

                if (++bytesWritten % ColumnSize == 0)
                {
                    writer.WriteLine();
                    writer.Write(" ");
                }
            }

            bytesRead = inputStream.Read(dataBuffer, 0, dataBuffer.Length);
        }

        writer.WriteLine();
        writer.WriteLine("};");
        writer.WriteLine($"unsigned int {FieldName}_len = {inputStream.Length};");

        writer.WriteLine(
$"""

#define CLRHOST_RESOURCE_IMPL
#include "../resource.h"
CLRHOST_DECLARE_RESOURCE({FieldName}, {FieldName}_len, "{Path.GetFileName(Input)}");
""");

        return true;
    }
}