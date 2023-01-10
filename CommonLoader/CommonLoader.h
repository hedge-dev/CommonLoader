#pragma once

namespace CommonLoader 
{
	class CommonLoader
		{
		public:
			static bool InitializeAssemblyLoader(const char* path);
			static void RaiseInitializers();
			static void RaiseUpdates();
		};
}