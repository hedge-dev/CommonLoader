namespace CommonLoader.Managed;
using System.Runtime.CompilerServices;

public class Logger
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void Log(LogLevel level, string message) => CodeLoader.Log(level, message);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void Info(string message) => Log(LogLevel.Info, message);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void Debug(string message) => Log(LogLevel.Debug, message);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void Warning(string message) => Log(LogLevel.Warning, message);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void Error(string message) => Log(LogLevel.Error, message);
}