#include "SigScanner.h"
#include <Psapi.h>
#include "ApplicationStore.h"

constexpr const char* ScannerSection = "SigScanner";
void MakePatterHash(const char* p_pattern, const char* p_mask, size_t pattern_length, uint32_t& out_patHash, uint32_t& out_maskHash);
void* SearchSignatureCache(uint32_t patternHash, uint32_t maskHash, void* p_begin, size_t size);
void CommitSignatureCache(uint32_t patternHash, uint32_t maskHash, void* p_memory);

void* CommonLoader::Scan(const char* p_pattern, const char* p_mask)
{
    const MODULEINFO& moduleInfo = ApplicationStore::GetModule();
	return Scan(p_pattern, p_mask, strlen(p_mask), moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage);
}

void* CommonLoader::Scan(const char* p_pattern, const char* p_mask, size_t pattern_length, void* p_begin, size_t size)
{
    uint32_t patternHash;
    uint32_t maskHash;
	MakePatterHash(p_pattern, p_mask, pattern_length, patternHash, maskHash);

	void* cachedResult = SearchSignatureCache(patternHash, maskHash, p_begin, size);
    if (cachedResult && ScanUncached(p_pattern, p_mask, pattern_length, cachedResult, pattern_length) == cachedResult)
    {
        return cachedResult;
    }

    void* address = ScanUncached(p_pattern, p_mask, pattern_length, p_begin, size);
	CommitSignatureCache(patternHash, maskHash, address);
    return address;
}

void* CommonLoader::ScanUncached(const char* p_pattern, const char* p_mask, size_t pattern_length, void* p_begin, size_t size)
{
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


void* SearchSignatureCache(uint32_t patternHash, uint32_t maskHash, void* p_begin, size_t size)
{
    char hashKey[32];
    sprintf_s(hashKey, 32, "%x.%x", patternHash, maskHash);
    std::string value = "";
    if (CommonLoader::ApplicationStore::GetOption(ScannerSection, hashKey, value))
    {
        try
        {
            const auto parsed = std::stoull(value, nullptr, 16);
            char* address = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(CommonLoader::ApplicationStore::GetModule().lpBaseOfDll) + static_cast<intptr_t>(parsed));
			if (address >= p_begin && address < static_cast<char*>(p_begin) + size)
            {
				return address;
            }
        }
        catch (std::exception& e)
        {
            return nullptr;
        }
    }

    return nullptr;
}


void CommitSignatureCache(uint32_t patternHash, uint32_t maskHash, void* p_memory)
{
    void* addressUnBased = static_cast<char*>(p_memory) - reinterpret_cast<size_t>(CommonLoader::ApplicationStore::GetModule().lpBaseOfDll);

	char buf[32];
    sprintf_s(buf, 32, "%x.%x", patternHash, maskHash);

	const std::string hashKey = buf;
	const uint64_t address = reinterpret_cast<size_t>(addressUnBased);
	sprintf_s(buf, 32, "%llx", address);

	CommonLoader::ApplicationStore::SetOption(ScannerSection, hashKey, buf);
    CommonLoader::ApplicationStore::Save();
}

void MakePatterHash(const char* p_pattern, const char* p_mask, size_t pattern_length, uint32_t& out_patHash, uint32_t& out_maskHash)
{
	out_patHash = XXH32(p_pattern, pattern_length, 0);
	out_maskHash = XXH32(p_mask, pattern_length, 0);
}