#pragma once

#include <msclr/marshal_cppstd.h>
#include <sstream>

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
		std::string* reflectedName;

	private:
		const std::string getFullName()
		{
			std::stringstream result{};

			if (Name->empty())
				return result.str();

			if (Category->empty())
			{
				result << *Name;
			}
			else
			{
				result << *Category << '/' << *Name;
			}

			return result.str();
		}

	public:
		std::string* ID;
		std::string* Name;
		std::string* Author;
		std::string* Category;

		std::string* FullName;

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

			reflectedName = new std::string(marshal_as<std::string>(base->Name));
			*reflectedName = reflectedName->substr(0, reflectedName->find('_'));

			FieldInfo^ meta = baseType->GetField("__META__");

			if (meta)
			{
				std::string metaValue = marshal_as<std::string>(safe_cast<String^>(meta->GetValue(nullptr)));
				nlohmann::json metaJson = nlohmann::json::parse(metaValue);

				ID = new std::string(metaJson["ID"]);
				Name = new std::string(metaJson["Name"]);
				Author = new std::string(metaJson["Author"]);
				Category = new std::string(metaJson["Category"]);
			}
			else
			{
				ID = new std::string();
				Name = reflectedName;
				Author = new std::string();
				Category = new std::string();
			}

			FullName = new std::string(getFullName());
		}

		~CodeObject()
		{
			this->!CodeObject();
		}

		!CodeObject()
		{
			delete reflectedName;
			delete ID;
			delete Name;
			delete Author;
			delete Category;
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

		const std::string* GetIdentifier(bool useFullName)
		{
			if (ID->empty())
				return useFullName ? FullName : Name;

			return ID;
		}

		const std::string* GetIdentifier()
		{
			return GetIdentifier(false);
		}
	};
}
