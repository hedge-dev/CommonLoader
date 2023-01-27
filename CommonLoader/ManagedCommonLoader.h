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
		static bool LoadAssembly(const char* path)
		{
			if (assemblyLoader == nullptr)
			{
				assemblyLoader = gcnew AssemblyLoader();
			}

			return assemblyLoader->Load(path);
		}

		static void RaiseInitializers()
		{
			assemblyLoader->raiseInitializers();
		}

		static void RaiseUpdates()
		{
			assemblyLoader->raiseUpdates();
		}
	};
}