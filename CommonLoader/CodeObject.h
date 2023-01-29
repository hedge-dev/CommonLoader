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
		Action^ initMethod;
		Action^ frameMethod;

	public:
		CodeObject(Type^ base) 
		{
			baseType = base;
			baseObject = Activator::CreateInstance(base);
			MethodInfo^ init = baseType->GetMethod("Init");
			MethodInfo^ onFrame = baseType->GetMethod("OnFrame");

			if (init)
				initMethod = safe_cast<Action^>(Delegate::CreateDelegate(Action::typeid, baseObject, init));
			
			if (onFrame)
				frameMethod = safe_cast<Action^>(Delegate::CreateDelegate(Action::typeid, baseObject, onFrame));
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
