#pragma once

#include "memory.h"
#include "AssemblerService.h"
#include "HookService.h"

using namespace System::Runtime::InteropServices;

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
			const char* source = (char*)Marshal::StringToHGlobalAnsi(instructions).ToPointer();
			AssemblerResult* result = AssemblerService::CompileAssembly(source);
			
			array<Byte>^ bytes = gcnew array<Byte>(result->length);
			pin_ptr<Byte> ptr = &bytes[0];
			memcpy(ptr, result->data, result->length);

			delete result;
			Marshal::FreeHGlobal((IntPtr)(void*)source);
			return bytes;
		}

		void WriteASMHook(String^ instructions, IntPtr address, int behavior) 
		{
			const char* source = (char*)Marshal::StringToHGlobalAnsi(instructions).ToPointer();
			HookService::WriteASMHook(source, (size_t)address.ToPointer(), behavior);
			Marshal::FreeHGlobal((IntPtr)(void*)source);
		}
	};
}