#pragma unmanaged

#include "SigScanner.h"
#include "ApplicationStore.h"
#include "CommonLoaderAPI.h"
#include <unordered_map>

constexpr const char* ScannerSection = "SigScanner";
struct SignatureKey
{
    uint32_t pattern;
    uint32_t mask;

    void MakeString(char* buffer, size_t buffer_size) const
    {
        sprintf_s(buffer, 32, "%x.%x", pattern, mask);
    }

    bool ParseString(const char* str)
    {
		return sscanf_s(str, "%x.%x", &pattern, &mask) == 2;
    }

	bool operator==(const SignatureKey& other) const
	{
		return pattern == other.pattern && mask == other.mask;
	}
};

namespace std
{
    template<>
    struct hash<SignatureKey>
    {
        size_t operator()(const SignatureKey& key) const noexcept
		{
            return XXH32(&key, sizeof(SignatureKey), 0);
		}
	};
};

std::unordered_map<SignatureKey, void*> sig_lookup_cache{};

void MakePatternHash(const char* p_pattern, const char* p_mask, size_t pattern_length, SignatureKey& out);
bool SearchSignatureCache(const SignatureKey& key, void* p_begin, size_t size, void*& out);
void CommitSignatureCache(const SignatureKey& key, void* p_memory);

void CommonLoader::InitSigScanner()
{
    std::vector<std::pair<const char*, const char*>> values{};
	ApplicationStore::GetOptions(ScannerSection, values);

    for (const auto& v : values)
    {
        SignatureKey key{};
		if (key.ParseString(v.first))
		{
            uint64_t address;
            if (sscanf_s(v.second, "%llx", &address) == 1)
            {
				if (address != 0)
				{
                    address += reinterpret_cast<intptr_t>(ApplicationStore::GetModule().base);
				}

				sig_lookup_cache[key] = reinterpret_cast<void*>(address);
            }
		}
    }
}

void* CommonLoader::Scan(const char* p_pattern, const char* p_mask)
{
    const auto& moduleInfo = ApplicationStore::GetModule();
	return Scan(p_pattern, p_mask, strlen(p_mask), moduleInfo.base, moduleInfo.size);
}

void* CommonLoader::Scan(const char* p_pattern, const char* p_mask, size_t pattern_length, void* p_begin, size_t size)
{
    SignatureKey key{};
    MakePatternHash(p_pattern, p_mask, pattern_length, key);

	void* cachedResult{};
    if (SearchSignatureCache(key, p_begin, size, cachedResult))
    {
        return ApplicationStore::GetState(CMN_LOADER_STATE_SKIP_SIG_VALIDATION) ? cachedResult :
    		ScanUncached(p_pattern, p_mask, pattern_length, cachedResult, pattern_length);
    }
    
    void* address = ScanUncached(p_pattern, p_mask, pattern_length, p_begin, size);
	CommitSignatureCache(key, address);
    return address;
}

void* CommonLoader::ScanUncached(const char* p_pattern, const char* p_mask, size_t pattern_length, void* p_begin, size_t size)
{
    if (p_begin == nullptr)
    {
        return nullptr;
    }

    for (size_t i = 0; i < size; i++)
    {
        char* memory = (char*)p_begin + i;
        size_t j;
        for (j = 0; j < pattern_length; j++)
        {
            if (p_mask[j] != '?' && p_pattern[j] != memory[j])
                break;
        }

        if (j == pattern_length)
            return memory;
    }

    return nullptr;
}

bool SearchSignatureCache(const SignatureKey& key, void* p_begin, size_t size, void*& out)
{
    const auto& result = sig_lookup_cache.find(key);
    if (result == sig_lookup_cache.end())
    {
        return nullptr;
    }

    char* address = static_cast<char*>(result->second);
    if ((address >= p_begin && address < static_cast<char*>(p_begin) + size) || address == nullptr)
    {
		out = address;
        return true;
    }

    return false;
}


void CommitSignatureCache(const SignatureKey& key, void* p_memory)
{
    void* addressUnBased = static_cast<char*>(p_memory) - reinterpret_cast<size_t>(CommonLoader::ApplicationStore::GetModule().base);
    if (p_memory == nullptr)
    {
        addressUnBased = nullptr;
    }

	char buf[32];
    key.MakeString(buf, sizeof(buf));

	const std::string hashKey = buf;
	const uint64_t address = reinterpret_cast<size_t>(addressUnBased);
	sprintf_s(buf, sizeof(buf), "%llx", address);

    sig_lookup_cache.insert_or_assign(key, p_memory);
	CommonLoader::ApplicationStore::SetOption(ScannerSection, hashKey, buf);
    CommonLoader::ApplicationStore::Save();
}

void MakePatternHash(const char* p_pattern, const char* p_mask, size_t pattern_length, SignatureKey& out)
{
	out.pattern = XXH32(p_pattern, pattern_length, 0);
	out.mask = XXH32(p_mask, pattern_length, 0);
}