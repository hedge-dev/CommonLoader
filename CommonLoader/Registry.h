#pragma once

namespace CommonLoader::Registry
{
    template <typename T>
    static bool Read(const std::filesystem::path& in_rPath, const std::wstring& in_rValueName, T& out_rData);

    template <typename T>
    static bool Read(const std::filesystem::path& in_rPath, T& out_rData);

    template <typename T>
    static bool Write(const std::filesystem::path& in_rPath, const std::filesystem::path& in_rValueName, const T& in_rData);

    template <typename T>
    static bool Write(const std::filesystem::path& in_rPath, const T& in_rData);
}

#include "Registry.inl"
