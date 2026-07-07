#pragma once

#include "ApplicationStore.h"
#include "CommonLoaderAPI.h"
#include <string>

#define FOREGROUND_WHITE  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define FOREGROUND_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)

namespace Logger
{
    enum class LogType
    {
        Info,
        Debug,
        Warning,
        Error
    };

    inline static HANDLE g_hStdOut{};

    template<class ... Args>
    static void Log(const std::string_view text, LogType type, Args... format)
    {
        if (!GetConsoleWindow() || CommonLoader::ApplicationStore::GetState(CMN_LOADER_STATE_DISABLE_LOGGING))
            return;

        if (!g_hStdOut)
            g_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

        std::string prefix = "[INFO] ";

        switch (type)
        {
            case LogType::Debug:
                prefix = "[DEBUG]";
                SetConsoleTextAttribute(g_hStdOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                break;

            case LogType::Warning:
                prefix = "[WARN] ";
                SetConsoleTextAttribute(g_hStdOut, FOREGROUND_YELLOW | FOREGROUND_INTENSITY);
                break;

            case LogType::Error:
                prefix = "[ERROR]";
                SetConsoleTextAttribute(g_hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
                break;

            default:
                SetConsoleTextAttribute(g_hStdOut, FOREGROUND_WHITE);
                break;
        }

        fmt::println("[CL] {} {}", prefix, fmt::format(text, format...));

        SetConsoleTextAttribute(g_hStdOut, FOREGROUND_WHITE);
    }

    template<class ... Args>
    static void Info(const std::string_view text, Args... format)
    {
        Log(text, LogType::Info, format...);
    }

    template<class ... Args>
    static void Debug(const std::string_view text, Args... format)
    {
        Log(text, LogType::Debug, format...);
    }

    template<class ... Args>
    static void Warning(const std::string_view text, Args... format)
    {
        Log(text, LogType::Warning, format...);
    }

    template<class ... Args>
    static void Error(const std::string_view text, Args... format)
    {
        Log(text, LogType::Error, format...);
    }
}
