#pragma once
#include "coreclrhost.h"
#include "host_runtime_contract.h"
#include "resource.h"

#define DOTNET_DEPENDS_PREFIX ".deps.json"
#define DOTNET_RUNTIMECONFIG_PREFIX ".runtimeconfig.json"
#define LOG(MSG, ...)
#define LOG_WARN(MSG, ...)
#define LOG_ERROR(MSG, ...)

namespace dotnet
{
    struct Version
    {
        int major{};
        int minor{};
        int patch{};
        std::string preview{};

        inline static std::optional<Version> parse(std::string_view input)
        {
            Version version{};
            auto majorLen = input.find_first_of('.');
            auto minorEnd = majorLen == std::string::npos ? std::string::npos : input.find_first_of('.', majorLen + 1);
            auto patchEnd = minorEnd == std::string::npos ? std::string::npos : input.find_first_of('-', minorEnd + 1);
            patchEnd = patchEnd == std::string::npos ? input.size() : patchEnd;

            if (majorLen == std::string::npos) return std::nullopt;

            std::from_chars(input.data(), input.data() + majorLen, version.major);

            if (minorEnd != std::string::npos)
            {
                std::from_chars(input.data() + majorLen + 1, input.data() + minorEnd, version.minor);
            }

            if (patchEnd != std::string::npos)
            {
                std::from_chars(input.data() + minorEnd + 1, input.data() + patchEnd, version.patch);
            }

            if (patchEnd != input.size())
            {
                version.preview = input.substr(patchEnd + 1);
            }

            return version;
        }

        inline static int compare(const Version& lhs, const Version& rhs)
        {
            if (lhs.major != rhs.major) return lhs.major - rhs.major;
            if (lhs.minor != rhs.minor) return lhs.minor - rhs.minor;
            if (lhs.patch != rhs.patch) return lhs.patch - rhs.patch;

            if (lhs.preview.empty() && !rhs.preview.empty()) return 1;
            if (!lhs.preview.empty() && rhs.preview.empty()) return -1;
            return lhs.preview.compare(rhs.preview);
        }

        inline bool operator==(const Version& rhs) const
        {
            return compare(*this, rhs) == 0;
        }

        inline bool operator!=(const Version& rhs) const
        {
            return compare(*this, rhs) != 0;
        }

        inline bool operator<(const Version& rhs) const
        {
            return compare(*this, rhs) < 0;
        }

        inline bool operator<=(const Version& rhs) const
        {
            return compare(*this, rhs) <= 0;
        }

        inline bool operator>(const Version& rhs) const
        {
            return compare(*this, rhs) > 0;
        }

        inline bool operator>=(const Version& rhs) const
        {
            return compare(*this, rhs) >= 0;
        }
    };

    struct RuntimeInfo
    {
        std::string name;
        Version version;
        std::filesystem::path path;
        std::optional<nlohmann::json> config{};

        const nlohmann::json& LoadConfig()
        {
            if (config.has_value()) return config.value();

            auto configPath = path / (name + DOTNET_RUNTIMECONFIG_PREFIX);

            std::ifstream configFile(configPath);

            if (!configFile)
            {
                LOG_ERROR("Failed to open dotnet config file");

                config = nlohmann::json{};
                return config.value();
            }

            config = nlohmann::json::parse(configFile);
            return config.value();
        }

        std::vector<RuntimeInfo> GetFrameworks()
        {
            std::vector<RuntimeInfo> result{};

            auto parseFramework = [&](const nlohmann::json& fx) 
            {
                auto versionStr = fx["version"].get<std::string>();
                auto version = Version::parse(versionStr);

                if (version != std::nullopt && fx.contains("name"))
                {
                    result.push_back({ fx["name"].get<std::string>(), version.value() });
                }
                else
                {
                    LOG_WARN("Invalid runtime version {} - {}", name, versionStr);
                }
            };

            auto& config = LoadConfig();


            if (config.contains("runtimeOptions"))
            {
                auto& runtimeOptions = config["runtimeOptions"];
                if (runtimeOptions.contains("framework"))
                {
                    auto framework = runtimeOptions["framework"];
                    parseFramework(framework);
                }
                if (runtimeOptions.contains("frameworks"))
                {
                    auto frameworks = runtimeOptions["frameworks"];
                    for (const auto& fx : frameworks)
                    {
                        parseFramework(fx);
                    }
                }
            }

            return result;
        }
    };

    inline std::optional<std::filesystem::path> FindRoot()
    {
        auto envRoot = CommonLoader::GetEnvironment<std::filesystem::path>(L"DOTNET_ROOT");
        if (envRoot.has_value()) return envRoot.value();

        std::filesystem::path regRoot = (sizeof(size_t) == 8) ? 
            L"HKEY_LOCAL_MACHINE\\SOFTWARE\\dotnet\\Setup\\InstalledVersions\\x64" : 
            L"HKEY_LOCAL_MACHINE\\SOFTWARE\\dotnet\\Setup\\InstalledVersions\\x86";

        std::filesystem::path rootPath{};
        if (CommonLoader::Registry::Read(regRoot, L"InstallLocation", rootPath)) return rootPath;
        else if (CommonLoader::Registry::Read(regRoot / "sharedhost", L"Path", rootPath)) return rootPath;

        PWSTR programFilesRaw;
        if (SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, &programFilesRaw) != S_OK)
        {
            return std::nullopt; // Not really possible;
        }

        std::filesystem::path dotnetPath = programFilesRaw;
        dotnetPath /= "dotnet";

        CoTaskMemFree(programFilesRaw);

        if (std::filesystem::is_directory(dotnetPath) && std::filesystem::exists(dotnetPath / "dotnet.exe")) return dotnetPath;

        return std::nullopt;
    }

    inline std::vector<RuntimeInfo> FindRuntimes(const std::filesystem::path& root)
    {
        std::vector<RuntimeInfo> result{};
        result.reserve(16); // surely that's a good average

        auto runtimeRoot = root / "shared";
        if (!std::filesystem::is_directory(runtimeRoot)) return result;
        
        for (const auto& runtime : std::filesystem::directory_iterator{ runtimeRoot })
        {
            if (!runtime.is_directory()) continue;

            auto runtimeName = runtime.path().filename().string();
            auto configName = runtimeName + DOTNET_RUNTIMECONFIG_PREFIX;

            for (const auto& runtimeVersion : std::filesystem::directory_iterator{ runtime.path() })
            {
                if (!runtimeVersion.is_directory()) continue;

                if (std::filesystem::exists(runtimeVersion.path() / configName))
                {
                    auto parsedVersion = Version::parse(runtimeVersion.path().filename().string());
                    if (parsedVersion.has_value())
                    {
                        result.push_back({ runtimeName, parsedVersion.value(), runtimeVersion });
                    }
                }
            }
        }

        return result;
    }

    struct coreclr
    {
        HMODULE module{};
        coreclr_initialize_ptr initialize{};
        coreclr_shutdown_ptr shutdown{};
        coreclr_create_delegate_ptr create_delegate{};
        coreclr_set_error_writer_ptr set_error_writer{};

        coreclr() = default;
        coreclr(HMODULE module) : module(module)
        {
            initialize = (coreclr_initialize_ptr)GetProcAddress(module, "coreclr_initialize");
            shutdown = (coreclr_shutdown_ptr)GetProcAddress(module, "coreclr_shutdown");
            create_delegate = (coreclr_create_delegate_ptr)GetProcAddress(module, "coreclr_create_delegate");
            set_error_writer = (coreclr_set_error_writer_ptr)GetProcAddress(module, "coreclr_set_error_writer");
        }
    };
}