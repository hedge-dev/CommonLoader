#pragma once

#include "CodeObject.h"
#include "Logger.h"
#include "MemoryProvider.h"

using namespace System;
using namespace System::IO;
using namespace System::Security;
using namespace System::Reflection;
using namespace System::Collections::Generic;
using namespace System::Security::Permissions;

namespace CommonLoader
{
	public ref class AssemblyLoader 
	{
	protected:
		Assembly^ loadedAssembly;

	public:
		List<CodeObject^>^ Codes;

		bool Load(const char* path)
		{
			String^ fullPath = Path::GetFullPath(gcnew String(path));

			if (Codes == nullptr)
			{
				Codes = gcnew List<CodeObject^>();
			}

			if (!File::Exists(fullPath))
				return false;

			loadedAssembly = Assembly::Load(File::ReadAllBytes(fullPath));

			for each (Type ^ type in loadedAssembly->GetExportedTypes())
			{
				MethodInfo^ method = type->GetMethod("IsLoaderExecutable");
				if (method != nullptr && method->Invoke(nullptr, nullptr))
				{
					CodeObject^ obj = gcnew CodeObject(type);
					Codes->Add(obj);
					continue;
				}

				if (type->Name == "MemoryService")
				{
					MethodInfo^ registerMethod = type->GetMethod("RegisterProvider");
					if (registerMethod != nullptr)
					{
						array<Object^>^ args = gcnew array<Object^>(1);

						args[0] = gcnew MemoryProvider();

						registerMethod->Invoke(nullptr, args);
					}
				}
			}

			return true;
		}

		void raiseInitializers() 
		{
			for each (CodeObject ^ code in Codes)
			{
				const std::string* id = code->GetIdentifier(true);

				if (id && !id->empty())
				{
					Logger::Info("Loading Code: {}", *id);
				}

				code->Init();
			}
		}

		void raiseUpdates() 
		{
			for each (CodeObject ^ code in Codes)
			{
				code->Update();
			}
		}
	};
}
