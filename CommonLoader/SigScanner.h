#pragma once

namespace CommonLoader
{
	void InitSigScanner();
	void* Scan(const char* p_pattern, const char* p_mask);
	void* Scan(const char* p_pattern, const char* p_mask, size_t pattern_length, void* p_begin, size_t size);
	void* ScanUncached(const char* p_pattern, const char* p_mask, size_t pattern_length, void* p_begin, size_t size);
}