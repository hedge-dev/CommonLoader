#pragma once

#include "AssemblerService.h"
#include "HookService.h"
#include "SigScanner.h"

using namespace Runtime::InteropServices;

namespace CommonLoader
{
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

		array<Byte>^ AssembleInstructions(String^ instructions)
		{
			char* source = static_cast<char*>(Marshal::StringToHGlobalAnsi(instructions).ToPointer());
			AssemblerResult* result = AssemblerService::CompileAssembly(source);
			
			array<Byte>^ bytes = gcnew array<Byte>(result->length);
			pin_ptr<Byte> ptr = &bytes[0];
			memcpy(ptr, result->data, result->length);

			delete result;
			Marshal::FreeHGlobal(static_cast<IntPtr>(static_cast<void*>(source)));
			return bytes;
		}

		void WriteASMHook(String^ instructions, IntPtr address, int behavior, int parameter)
		{
			char* source = static_cast<char*>(Marshal::StringToHGlobalAnsi(instructions).ToPointer());
			HookService::WriteASMHook(source, reinterpret_cast<size_t>(address.ToPointer()), behavior, parameter);
			Marshal::FreeHGlobal(static_cast<IntPtr>(source));
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