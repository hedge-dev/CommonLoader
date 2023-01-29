#pragma once

#include "AssemblerService.h"
#include "HookService.h"
#include "SigScanner.h"

using namespace Runtime::InteropServices;

namespace CommonLoader
{
	public struct ManagedStringANSI
	{
		char* ptr{};

		ManagedStringANSI(String^ str)
		{
			ptr = static_cast<char*>(Marshal::StringToHGlobalAnsi(str).ToPointer());
		}

		~ManagedStringANSI()
		{
			Marshal::FreeHGlobal(safe_cast<IntPtr>(ptr));
		}

		operator const char*() const
		{
			return ptr;
		}
	};

	public ref class MemoryProvider
	{
	public:
		void WriteMemory(IntPtr address, IntPtr dataPtr, IntPtr length) 
		{
			memcpy(address.ToPointer(), dataPtr.ToPointer(), (size_t)length.ToPointer());
		}

		generic<typename T>
		void WriteMemory(IntPtr address, array<T>^ data)
		{
			if (!address.ToPointer())
				return;

			pin_ptr<T> source = &data[0];
			memcpy(address.ToPointer(), source, sizeof(T) * data->Length);
		}

		generic<typename T>
		void WriteMemory(IntPtr address, T data)
		{
			if (!address.ToPointer())
				return;

			pin_ptr<T> source = &data;
			memcpy(address.ToPointer(), source, sizeof(T));
		}

		array<char>^ ReadMemory(IntPtr address, IntPtr length) 
		{
			array<char>^ arr = gcnew array<char>(length.ToInt32());
			pin_ptr<char> ptr = &arr[0];
			memcpy(ptr, address.ToPointer(), (size_t)length.ToPointer());

			return arr;
		}

		generic<typename T>
		T ReadMemory(IntPtr address) 
		{
			T def;
			pin_ptr<T> ptr = &def;
			memcpy(ptr, address.ToPointer(), sizeof(T));

			return def;
		}

		void SetAssemblerSymbol(String^ name, unsigned long value)
		{
			const ManagedStringANSI str{ name };
			AssemblerService::SetSymbol(str, value);
		}

		bool GetAssemblerSymbol(String^ name, [Runtime::InteropServices::Out] unsigned long% value)
		{
			const ManagedStringANSI str{ name };
			uint64_t v{};
			const bool result = AssemblerService::GetSymbol(str, v);
			
			value = v;
			return result;
		}

		bool RemoveAssemblerSymbol(String^ name)
		{
			const ManagedStringANSI str{ name };
			return AssemblerService::RemoveSymbol(str);
		}

		array<Byte>^ AssembleInstructions(String^ instructions)
		{
			const ManagedStringANSI source{ instructions };
			const AssemblerResult* result = AssemblerService::CompileAssembly(source);
			
			array<Byte>^ bytes = gcnew array<Byte>(result->length);
			pin_ptr<Byte> ptr = &bytes[0];
			memcpy(ptr, result->data, result->length);

			delete result;
			return bytes;
		}

		void WriteASMHook(String^ instructions, IntPtr address, int behavior, int parameter)
		{
			const ManagedStringANSI source{ instructions };
			HookService::WriteASMHook(source, reinterpret_cast<size_t>(address.ToPointer()), behavior, parameter);
		}
		
		void WriteASMHook(String^ instructions, IntPtr address, int behavior)
		{
			WriteASMHook(instructions, address, behavior, eHookParameter_Jump);
		}

		unsigned int NopInstructions(IntPtr address, uint32_t count)
		{
			if (count == 0)
				return 0;
			
			return HookService::NopInstructions(reinterpret_cast<size_t>(address.ToPointer()), count);
		}

		IntPtr ScanSignature(array<Byte>^ pattern, String^ mask)
		{
			if (pattern->Length != mask->Length)
				return IntPtr::Zero;

			array<Byte>^ maskBytes = Text::Encoding::ASCII->GetBytes(mask);
			const pin_ptr<Byte> patPtr = &pattern[0];
			const pin_ptr<Byte> maskPtr = &maskBytes[0];
			
			return IntPtr(Scan(reinterpret_cast<char*>(patPtr), reinterpret_cast<char*>(maskPtr)));
		}
	};
}