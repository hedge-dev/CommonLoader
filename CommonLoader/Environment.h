#pragma once
#include <string>
#include <optional>
#include <Windows.h>

namespace CommonLoader
{
    template<typename T = std::wstring>
    std::optional<T> GetEnvironment(const wchar_t* name, std::optional<T> defaultValue)
    {
        auto bufferSize = GetEnvironmentVariableW(name, nullptr, 0);

        if (!bufferSize)
        {
            return std::nullopt;
        }

        std::wstring value{};
        value.resize(bufferSize + 1);

        GetEnvironmentVariableW(name, value.data(), bufferSize);
        if constexpr (std::is_same_v<T, std::wstring>) return value;
        if constexpr (std::is_same_v<T, std::filesystem::path>) return std::filesystem::path(value);
        else if constexpr (std::is_same_v<T, int32_t>) return std::stoi(value);
        else if constexpr (std::is_same_v<T, int64_t>) return std::stoll(value);
        else if constexpr (std::is_same_v<T, uint32_t>) return std::stoul(value);
        else if constexpr (std::is_same_v<T, uint64_t>) return std::stoull(value);
        else if constexpr (std::is_same_v<T, float>) return std::stof(value);

        // idk override it yourself
        return (T)value;
    }

    template<typename T = std::string>
    std::optional<T> GetEnvironment(const wchar_t* name)
    {
        return GetEnvironment<T>(name, std::nullopt);
    }
}