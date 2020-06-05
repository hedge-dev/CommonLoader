#pragma once
#include "AssemblyLoader.h"
#include "AssemblerService.h"

namespace CommonLoader {
	public ref class ManagedCommonLoader
	{
	protected:
		static AssemblyLoader^ assemblyLoader;

	public:
		static void InitializeLoader(const char* path) {
			assemblyLoader = gcnew AssemblyLoader(path);
			AssemblerService::Init();
		}

		static void RaiseInitializers() {
			assemblyLoader->raiseInitializers();
		}

		static void RaiseUpdates() {
			assemblyLoader->raiseUpdates();
		}
	};
}