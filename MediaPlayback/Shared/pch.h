﻿//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

// uncomment next line if you want to build the library without FFMPEG support and dependencies 
#define NO_FFMPEG 1


#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// std c++
#include <memory>
#include <vector>
#include <map>

// windows api's
#include <windows.h>
#include <assert.h>
#include <strsafe.h>

#include <d3d11_4.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")

#include <mfapi.h> // dxgimanager
#include <mfidl.h>
#include <mferror.h>
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")

// wrl
#include <wrl.h>
#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <windows.storage.h>
#include <windows.system.threading.h>

#pragma comment(lib, "runtimeobject")

#include <shcore.h>
#pragma comment(lib, "ShCore")


// unity
#include "Unity\IUnityGraphics.h"
#include "Unity\IUnityGraphicsD3D11.h"

#if !_DEBUG
#define DebugMessage(x)
#else
#define DebugMessage(x)// OutputDebugString(x)
#endif

typedef enum Log_Level
{
    Log_Level_Any,
    Log_Level_Error,
    Log_Level_Warning,
    Log_Level_Info,
} Log_Level;

#ifndef LOG_LEVEL
#if _DEBUG
#define LOG_LEVEL Log_Level_Warning
#else
#define LOG_LEVEL Log_Level_Error
#endif
#endif

inline void __stdcall Log(
    _In_ Log_Level level,
    _In_ _Printf_format_string_ STRSAFE_LPCWSTR pszFormat,
    ...)
{
    if (LOG_LEVEL < level)
    {
        return;
    }

    wchar_t szTextBuf[2048];

    va_list args;
    va_start(args, pszFormat);

    StringCchVPrintf(szTextBuf, _countof(szTextBuf), pszFormat, args);

    va_end(args);

    OutputDebugStringW(szTextBuf);
}

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100

inline const LPWSTR ErrorMessage(HRESULT hr)
{
    LPWSTR pszMsg;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&pszMsg,
        0,
        NULL);

    if (pszMsg != NULL) 
    {
#ifdef UNICODE
        size_t const nLen = wcslen(pszMsg);
#else
        size_t const nLen = strlen(m_pszMsg);
#endif
        if (nLen > 1 && pszMsg[nLen - 1] == '\n') 
        {
            pszMsg[nLen - 1] = 0;
            
            if (pszMsg[nLen - 2] == '\r') 
            {
                pszMsg[nLen - 2] = 0;
            }
        }
    }

    return pszMsg;
}

inline void __stdcall LogResult(
    _In_ LPWSTR pszFile, //__FILEW__
    _In_ LPWSTR pszFunc, //__FUNCTIONW__
    _In_ long nLine, //__LINE__
    _In_ HRESULT hr,
    _In_ LPCWSTR message = L"")
{
    if (SUCCEEDED(hr))
    {
        return;
    }

    // if filename contains a path, move the ptr to the first char of name
    LPWSTR psz = pszFile + wcsnlen_s(pszFile, 255);
    for (; psz > pszFile; psz--)
    {
        if (*psz == L'\\')
        {
            pszFile = psz + 1;
            break;
        }
    }

    // Gets the plain text error message
    Log(
        Log_Level_Error,
        L"%sHR: 0x%x - %s\r\n\t%s(%d): %s\n"
        , message, hr, ErrorMessage(hr), pszFile, nLine, pszFunc);
}

#ifndef LOG_RESULT
#define LOG_RESULT_MSG(hrToCheck, message) LogResult(__FILEW__, __FUNCTIONW__, __LINE__, hrToCheck, message);
#define LOG_RESULT(hrToCheck) LOG_RESULT_MSG(hrToCheck, L"");
#endif

#ifndef IFR
#define IFR_MSG(hrToCheck, message) if (FAILED(hrToCheck)) { LOG_RESULT_MSG(hrToCheck, message); return hrToCheck; }
#define IFR(hrToCheck) IFR_MSG(hrToCheck, L"RETURN_")
#endif

#ifndef IFG
#define IFG_MSG(hrToCheck, message) if (FAILED(hrToCheck)) { LOG_RESULT_MSG(hrToCheck, message); goto done; }
#define IFG(hrToCheck) IFR_MSG(hrToCheck, L"GOTO DONE_")
#endif

#ifndef NULL_CHK
#define NULL_CHK_HR_MSG(pointer, hrToCheck, message) if(nullptr == pointer) { IFR_MSG(hrToCheck, message); return hrToCheck; }
#define NULL_CHK_HR(pointer, hrToCheck) NULL_CHK_HR_MSG(pointer, hrToCheck, L"NULL_CHK_")
#define NULL_CHK(pointer) NULL_CHK_HR(pointer, E_INVALIDARG)
#endif

class SafeString
{
public:
    SafeString() throw();
    ~SafeString() throw();
    operator const HSTRING&() const;
    HSTRING* GetAddressOf();
    const wchar_t* c_str() const;

private:
    HSTRING m_hstring;
};

class SharedVirtualBase 
    : public std::enable_shared_from_this<SharedVirtualBase>
{
protected:
    SharedVirtualBase() {}

    virtual ~SharedVirtualBase() {}

public:
    template <typename T>
    std::shared_ptr<T> GetSharedPtr()
    {
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }

    template <typename T>
    std::weak_ptr<T> GetWeakPtr()
    {
        return std::weak_ptr<T>(GetSharedPtr<T>());
    }
};

class SharedFromThis 
    : public virtual SharedVirtualBase
{
public:
    SharedFromThis() : SharedVirtualBase() {}

    virtual ~SharedFromThis() {}
};

#define shared_from_this please_use_GetSharedPtr_or_GetWeakPtr_instead_of_shared_from_this
#define enable_shared_from_this please_use_SharedFromThis_instead_of_enable_shared_from_this


static HRESULT AddStringProperty(ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable*>* pIMap, PCWSTR key, PCWSTR value)
{
	Microsoft::WRL::ComPtr<IActivationFactory> spGenericActivationFactoryInterface;
	Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IPropertyValueStatics> pFactory;

	IFR(Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get(), &spGenericActivationFactoryInterface));
	IFR(spGenericActivationFactoryInterface.As(&pFactory));

	Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IPropertyValue> spValue;
	IFR(pFactory->CreateString(Microsoft::WRL::Wrappers::HStringReference(value).Get(), &spValue));

	boolean replaced;
	return pIMap->Insert(Microsoft::WRL::Wrappers::HStringReference(key).Get(),
		spValue.Get(), &replaced);
}

static HRESULT AddBooleanProperty(ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable*>* pIMap, PCWSTR key, bool value)
{
	Microsoft::WRL::ComPtr<IActivationFactory> spGenericActivationFactoryInterface;
	Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IPropertyValueStatics> pFactory;

	IFR(Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get(), &spGenericActivationFactoryInterface));
	IFR(spGenericActivationFactoryInterface.As(&pFactory));

	Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IPropertyValue> spValue;
	IFR(pFactory->CreateBoolean(value, &spValue));

	boolean replaced;
	return pIMap->Insert(Microsoft::WRL::Wrappers::HStringReference(key).Get(),
		spValue.Get(), &replaced);
}

static HRESULT AddInt64Property(ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable*>* pIMap, PCWSTR key, __int64 value)
{
	Microsoft::WRL::ComPtr<IActivationFactory> spGenericActivationFactoryInterface;
	Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IPropertyValueStatics> pFactory;

	IFR(Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get(), &spGenericActivationFactoryInterface));
	IFR(spGenericActivationFactoryInterface.As(&pFactory));

	Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IPropertyValue> spValue;
	IFR(pFactory->CreateInt64(value, &spValue));

	boolean replaced;
	return pIMap->Insert(Microsoft::WRL::Wrappers::HStringReference(key).Get(),
		spValue.Get(), &replaced);
}