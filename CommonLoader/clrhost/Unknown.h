#pragma once
#include <windows.h>

namespace clrhost
{
    template<typename... TBase>
    class BaseUnknown : public IUnknown, public TBase...
    {
        ULONG ref{ 1 };

    public:
        template<typename T>
        void CheckInterface(const IID& riid, void*& rpv)
        {
            if (__uuidof(T) == riid)
            {
                rpv = static_cast<T*>(this);
            }
        }

        void* FindInterface(const IID& riid)
        {
            void* pv{};
            (CheckInterface<TBase>(riid, pv), ...);

            return pv;
        }

        virtual HRESULT __stdcall QueryInterface(const IID& riid, void** ppv) override
        {
            if (riid == IID_IUnknown)
            {
                if (!ppv) return E_POINTER;

                *ppv = reinterpret_cast<IUnknown*>(this);
                AddRef();
                return S_OK;
            }

            void* pv = FindInterface(riid);
            if (pv == nullptr) return E_NOINTERFACE;
            if (ppv == nullptr) return E_POINTER;

            *ppv = pv;
            AddRef();

            return S_OK;
        }

        virtual ULONG STDMETHODCALLTYPE AddRef(void)
        {
            return InterlockedIncrement(&ref);
        }

        virtual ULONG STDMETHODCALLTYPE Release(void)
        {
            if (InterlockedDecrement(&ref) == 0)
            {
                delete this;
                return 0;
            }

            return ref;
        }

        virtual ~BaseUnknown() = default;
    };

}

//class TestInterface : public BaseUnknown<IHostControl>
//{
//    virtual HRESULT GetHostManager(REFIID riid, void** ppv)
//    {
//        return E_NOTIMPL;
//    }
//
//    virtual HRESULT SetAppDomainManager(DWORD id, IUnknown* manager)
//    {
//        return S_OK;
//    }
//};