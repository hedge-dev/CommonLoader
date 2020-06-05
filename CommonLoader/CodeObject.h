#pragma once

using namespace System;
using namespace System::Reflection;

namespace CommonLoader 
{
	public ref class CodeObject
	{
	protected:
		Type^ baseType;
		Object^ baseObject;
		MethodInfo^ initMethod;
		MethodInfo^ frameMethod;

	public:
		CodeObject(Type^ base) 
		{
			baseType = base;
			baseObject = Activator::CreateInstance(base);
			initMethod = baseType->GetMethod("Init");
			frameMethod = baseType->GetMethod("OnFrame");
		}

		void Init() 
		{
			if (initMethod)
				initMethod->Invoke(baseObject, nullptr);
		}

		void Update() 
		{
			if (frameMethod)
				frameMethod->Invoke(baseObject, nullptr);
		}
	};

}
