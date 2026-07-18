#pragma once

#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;

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
		std::string* Name;

		CodeObject(Type^ base) 
		{
			baseType = base;
			baseObject = Activator::CreateInstance(base);

			// TODO: add managed fields for name, category and ID.
			Name = new std::string();
			*Name = marshal_as<std::string>(base->Name);
			*Name = Name->substr(0, Name->find('_'));

			MethodInfo^ init = baseType->GetMethod("Init");
			MethodInfo^ onFrame = baseType->GetMethod("OnFrame");

			if (init)
				initMethod = safe_cast<Action^>(Delegate::CreateDelegate(Action::typeid, baseObject, init));
			
			if (onFrame)
				frameMethod = safe_cast<Action^>(Delegate::CreateDelegate(Action::typeid, baseObject, onFrame));
		}

		~CodeObject()
		{
			this->!CodeObject();
		}

		!CodeObject()
		{
			if (Name)
			{
				delete Name;
				Name = nullptr;
			}
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
