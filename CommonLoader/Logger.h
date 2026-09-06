#pragma once

#include "ApplicationStore.h"
#include "CommonLoader.h"
#include "CommonLoaderAPI.h"

#include <format>
#include <iostream>
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

    template <typename T>
    static T GetLogTypePrefix(LogType in_type)
    {
        T result{};

        if constexpr (std::is_same_v<T, std::string_view>)
        {
            switch (in_type)
            {
                case LogType::Info:    result = "[INFO] "; break;
                case LogType::Debug:   result = "[DEBUG]"; break;
                case LogType::Warning: result = "[WARN] "; break;
                case LogType::Error:   result = "[ERROR]"; break;
            }
        }
        else if constexpr (std::is_same_v<T, std::wstring_view>)
        {
            switch (in_type)
            {
                case LogType::Info:    result = L"[INFO] "; break;
                case LogType::Debug:   result = L"[DEBUG]"; break;
                case LogType::Warning: result = L"[WARN] "; break;
                case LogType::Error:   result = L"[ERROR]"; break;
            }
        }
        else
        {
            static_assert(false, "Unsupported string type.");
        }

        return result;
    }

    static void SetLogTypeColour(LogType in_type)
    {
        if (!g_hStdOut)
            return;

        switch (in_type)
        {
        case LogType::Debug:
            SetConsoleTextAttribute(g_hStdOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            break;

        case LogType::Warning:
            SetConsoleTextAttribute(g_hStdOut, FOREGROUND_YELLOW | FOREGROUND_INTENSITY);
            break;

        case LogType::Error:
            SetConsoleTextAttribute(g_hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
            break;

        default:
            SetConsoleTextAttribute(g_hStdOut, FOREGROUND_WHITE);
            break;
        }
    }

    template <typename T, typename... TArgs>
    static void Log(const T in_text, LogType in_type, TArgs... in_format)
    {
        static_assert(std::is_same_v<T, std::string_view> || std::is_same_v<T, std::wstring_view>,
            "T must be either std::string_view or std::wstring_view.");

        if (!GetConsoleWindow() || CommonLoader::ApplicationStore::GetState(CMN_LOADER_STATE_DISABLE_LOGGING))
            return;

        if (!g_hStdOut)
            g_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

        SetLogTypeColour(in_type);

        std::cout << "[CL] ";

        if constexpr (std::is_same_v<T, std::string_view>)
        {
            std::cout << std::format("{} ", GetLogTypePrefix<T>(in_type));
            std::cout << std::vformat(in_text.data(), std::make_format_args(in_format...));
            std::cout << std::endl;
        }
        else if constexpr (std::is_same_v<T, std::wstring_view>)
        {
            std::wcout << std::format(L"{} ", GetLogTypePrefix<T>(in_type));
            std::wcout << std::vformat(in_text.data(), std::make_wformat_args(in_format...));
            std::wcout << std::endl;
        }

        SetConsoleTextAttribute(g_hStdOut, FOREGROUND_WHITE);
    }

    template <typename... TArgs>
    static void Info(const std::string_view in_text, TArgs... in_format)
    {
        Log(in_text, LogType::Info, in_format...);
    }

    template <typename... TArgs>
    static void Info(const std::wstring_view in_text, TArgs... in_format)
    {
        Log(in_text, LogType::Info, in_format...);
    }

    template <typename... TArgs>
    static void Debug(const std::string_view in_text, TArgs... in_format)
    {
        Log(in_text, LogType::Debug, in_format...);
    }

    template <typename... TArgs>
    static void Debug(const std::wstring_view in_text, TArgs... in_format)
    {
        Log(in_text, LogType::Debug, in_format...);
    }

    template <typename... TArgs>
    static void Warning(const std::string_view in_text, TArgs... in_format)
    {
        Log(in_text, LogType::Warning, in_format...);
    }

    template <typename... TArgs>
    static void Warning(const std::wstring_view in_text, TArgs... in_format)
    {
        Log(in_text, LogType::Warning, in_format...);
    }

    template <typename... TArgs>
    static void Error(const std::string_view in_text, TArgs... in_format)
    {
        Log(in_text, LogType::Error, in_format...);
    }

    template <typename... TArgs>
    static void Error(const std::wstring_view in_text, TArgs... in_format)
    {
        Log(in_text, LogType::Error, in_format...);
    }
}