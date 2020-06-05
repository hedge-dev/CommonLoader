#pragma once

namespace CommonLoader 
{
	class CommonLoader
		{
		public:
			static void InitializeAssemblyLoader(const char* path);
			static void RaiseInitializers();
			static void RaiseUpdates();
		};
}