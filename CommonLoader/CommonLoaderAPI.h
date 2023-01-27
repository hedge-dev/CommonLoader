#pragma once

#define CMN_LOADER_API __cdecl
#define CMN_LOADER_API_VERSION 1

#define DECLARE_API_FUNC(RETURN_TYPE, NAME, ...) RETURN_TYPE (CMN_LOADER_API *NAME)(__VA_ARGS__) = nullptr;

struct AssemblerResult
{
	size_t instruction_count{};
	size_t length{};
	unsigned char* data{};
	const char* error_string{};

	// Private data
#ifdef CMN_LOADER_IMPL
	~AssemblerResult();
#endif
};

struct CommonLoaderAPI
{
	DECLARE_API_FUNC(int, GetVersion);
	DECLARE_API_FUNC(void*, ScanSignature, const char* pattern, const char* mask);
	DECLARE_API_FUNC(void*, ScanSignatureEx, const char* pattern, const char* mask, void* begin, size_t size);
	DECLARE_API_FUNC(int, GetApplicationOption, const char* key, char* out, size_t out_size);
	DECLARE_API_FUNC(int, GetApplicationOptionEx, const char* section, const char* key, char* out, size_t out_size);
	DECLARE_API_FUNC(void, SaveApplicationOptions);
	DECLARE_API_FUNC(AssemblerResult*, CompileAssembly, const char* code);
	DECLARE_API_FUNC(void, FreeAssemblerObject, AssemblerResult*);
	DECLARE_API_FUNC(void, SetAssemblerSymbol, const char* name, uint64_t value);
	DECLARE_API_FUNC(bool, GetAssemblerSymbol, const char* name, uint64_t* out);
	DECLARE_API_FUNC(bool, RemoveAssemblerSymbol, const char* name);
};

extern const CommonLoaderAPI api_table;
inline const CommonLoaderAPI* GetCommonLoaderAPI()
{
	return &api_table;
}

#undef DECLARE_API_FUNC