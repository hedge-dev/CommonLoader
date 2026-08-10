#pragma once

#include "AssemblyLoader.h"
#include "AssemblerService.h"
#include "ApplicationStore.h"

namespace CommonLoader
{
	public ref class ManagedCommonLoader
	{
	public:
		static AssemblyLoader^ AssemblyLoader;

		static bool LoadAssembly(const char* path)
		{
			if (AssemblyLoader == nullptr)
			{
				AssemblyLoader = gcnew CommonLoader::AssemblyLoader();
			}

			return AssemblyLoader->Load(path);
		}

		static void RaiseInitializers()
		{
			AssemblyLoader->raiseInitializers();
		}

		static void RaiseUpdates()
		{
			AssemblyLoader->raiseUpdates();
		}
	};
}