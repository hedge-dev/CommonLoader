#pragma once
#include "Unknown.h"
#include "xclrdata.h"

namespace clrhost
{
    class MyDataTarget : public clrhost::BaseUnknown<ICLRDataTarget>
    {
        virtual HRESULT STDMETHODCALLTYPE GetMachineType(ULONG32* machineType) override
        {
#ifdef _WIN64
            * machineType = IMAGE_FILE_MACHINE_AMD64;
#else
            * machineType = IMAGE_FILE_MACHINE_I386;
#endif
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE GetPointerSize(ULONG32* pointerSize) override
        {
            *pointerSize = sizeof(void*);
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE GetImageBase(
            /* [string][in] */ LPCWSTR imagePath,
            /* [out] */ CLRDATA_ADDRESS* baseAddress) override
        {
            *baseAddress = (CLRDATA_ADDRESS)GetModuleHandleW(imagePath);
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE ReadVirtual(
            /* [in] */ CLRDATA_ADDRESS address,
            /* [length_is][size_is][out] */ BYTE* buffer,
            /* [in] */ ULONG32 bytesRequested,
            /* [out] */ ULONG32* bytesRead) override
        {
            *bytesRead = bytesRequested;
            memcpy(buffer, (void*)address, bytesRequested);

            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE WriteVirtual(
            /* [in] */ CLRDATA_ADDRESS address,
            /* [size_is][in] */ BYTE* buffer,
            /* [in] */ ULONG32 bytesRequested,
            /* [out] */ ULONG32* bytesWritten) override
        {
            *bytesWritten = bytesRequested;
            memcpy((void*)address, buffer, bytesRequested);

            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE GetTLSValue(
            /* [in] */ ULONG32 threadID,
            /* [in] */ ULONG32 index,
            /* [out] */ CLRDATA_ADDRESS* value)
        {
            *value = (CLRDATA_ADDRESS)TlsGetValue(index);
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE SetTLSValue(
            /* [in] */ ULONG32 threadID,
            /* [in] */ ULONG32 index,
            /* [in] */ CLRDATA_ADDRESS value) override
        {
            TlsSetValue(index, (void*)value);
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE GetCurrentThreadID(
            /* [out] */ ULONG32* threadID)
        {
            *threadID = ::GetCurrentThreadId();
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE GetThreadContext(
            /* [in] */ ULONG32 threadID,
            /* [in] */ ULONG32 contextFlags,
            /* [in] */ ULONG32 contextSize,
            /* [size_is][out] */ BYTE* context)
        {
            auto thread = ::OpenThread(THREAD_ALL_ACCESS, false, threadID);
            if (!thread)
            {
                return E_ACCESSDENIED;
            }

            ::GetThreadContext(thread, (LPCONTEXT)context);
            CloseHandle(thread);
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE SetThreadContext(
            /* [in] */ ULONG32 threadID,
            /* [in] */ ULONG32 contextSize,
            /* [size_is][in] */ BYTE* context) override
        {
            auto thread = ::OpenThread(THREAD_ALL_ACCESS, false, threadID);
            if (!thread)
            {
                return E_ACCESSDENIED;
            }

            ::SetThreadContext(thread, (LPCONTEXT)context);
            CloseHandle(thread);
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE Request(
            /* [in] */ ULONG32 reqCode,
            /* [in] */ ULONG32 inBufferSize,
            /* [size_is][in] */ BYTE* inBuffer,
            /* [in] */ ULONG32 outBufferSize,
            /* [size_is][out] */ BYTE* outBuffer) override
        {
            return E_NOTIMPL;
        }
    };
}