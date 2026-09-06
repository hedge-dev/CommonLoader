#pragma once

namespace StringUtils
{
    inline std::string Convert(const std::wstring_view& wide)
    {
        int bytesNeeded = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wide.size(), nullptr, 0, 0, nullptr);

        std::string result{};
        result.resize(bytesNeeded, 0);
        
        WideCharToMultiByte(CP_UTF8, 0, wide.data(), wide.size(), result.data(), result.length(), 0, nullptr);

        return result;
    }

    inline std::wstring Convert(const std::string_view& mbStr)
    {
        int bytesNeeded = MultiByteToWideChar(CP_UTF8, 0, mbStr.data(), mbStr.size(), nullptr, 0);

        std::wstring result{};
        result.resize(bytesNeeded, 0);

        MultiByteToWideChar(CP_UTF8, 0, mbStr.data(), mbStr.size(), result.data(), result.length());

        return result;
    }
}