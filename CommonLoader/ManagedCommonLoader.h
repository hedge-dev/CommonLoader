#pragma once
#include "AssemblyLoader.h"
#include "AssemblerService.h"
#include "ApplicationStore.h"

namespace CommonLoader {
	public ref class ManagedCommonLoader
	{
	protected:
		static AssemblyLoader^ assemblyLoader;

	public:
		static bool InitializeLoader(const char* path) {
			ApplicationStore::Init();
			assemblyLoader = gcnew AssemblyLoader();
			bool result = assemblyLoader->Init(path);
			AssemblerService::Init();
			return result;
		}

		static void RaiseInitializers() {
			assemblyLoader->raiseInitializers();
		}

		static void RaiseUpdates() {
			assemblyLoader->raiseUpdates();
		}
	};
}