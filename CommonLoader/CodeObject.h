#pragma once

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

namespace CommonLoader 
{
	public ref class CodeObject
	{
	protected:
		Type^ baseType;
		Object^ baseObject;
		Action^ initMethod;
		Action^ frameMethod;

	public:
		const char* Name;

		CodeObject(Type^ base) 
		{
			baseType = base;
			baseObject = Activator::CreateInstance(base);

			// TODO: add managed fields for name, category and ID.
			Name = (const char*)Marshal::StringToHGlobalAnsi(base->Name).ToPointer();

			// Omit hash suffix from reflected name.
			char* c = (char*)Name;
			while (c && *c != '\0')
			{
				if (*c == '_')
				{
					*c = '\0';
					break;
				}

				c++;
			}

			MethodInfo^ init = baseType->GetMethod("Init");
			MethodInfo^ onFrame = baseType->GetMethod("OnFrame");

			if (init)
				initMethod = safe_cast<Action^>(Delegate::CreateDelegate(Action::typeid, baseObject, init));
			
			if (onFrame)
				frameMethod = safe_cast<Action^>(Delegate::CreateDelegate(Action::typeid, baseObject, onFrame));
		}

		~CodeObject()
		{
			if (Name)
				Marshal::FreeHGlobal((IntPtr)(void*)Name);
		}

		void Init() 
		{
			if (initMethod)
				initMethod();
		}

		void Update() 
		{
			if (frameMethod)
				frameMethod();
		}
	};

}
