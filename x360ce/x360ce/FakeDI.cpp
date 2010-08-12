/*  x360ce - XBOX360 Controler Emulator
*  Copyright (C) 2002-2010 ToCA Edit
*
*  x360ce is free software: you can redistribute it and/or modify it under the terms
*  of the GNU Lesser General Public License as published by the Free Software Found-
*  ation, either version 3 of the License, or (at your option) any later version.
*
*  x360ce is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
*  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE.  See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with x360ce.
*  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "globals.h"
#include "FakeAPI.h"
#include "Utils.h"

#include <detours.h>
#define CINTERFACE	//needed for detours
#include "DirectInput.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT (WINAPI *OriginalDirectInput8Create)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter) = DirectInput8Create;
HRESULT (STDMETHODCALLTYPE *OriginalCreateDevice) (LPDIRECTINPUT8 This, REFGUID rguid, LPDIRECTINPUTDEVICE8 *lplpDirectInputDevice, LPUNKNOWN pUnkOuter) = NULL;
HRESULT (STDMETHODCALLTYPE *OriginalGetProperty) (LPDIRECTINPUTDEVICE8 This, REFGUID rguidProp, LPDIPROPHEADER pdiph) = NULL;
HRESULT (STDMETHODCALLTYPE *OriginalGetDeviceInfo) (LPDIRECTINPUTDEVICE8 This, LPDIDEVICEINSTANCE pdidi) = NULL;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE FakeGetDeviceInfo (LPDIRECTINPUTDEVICE8 This, LPDIDEVICEINSTANCE pdidi)
{
	HRESULT hr;
	hr = OriginalGetDeviceInfo ( This, pdidi );

	if(pdidi)
	{
		WriteLog(_T("[FAKEDI]  FakeGetDeviceInfo"));

		if(wFakeMode == 2)
		{
			//ANSI or UNICODE ?
			if(sizeof(DIDEVICEINSTANCEA) ==  pdidi->dwSize)					//ANSI
			{
				WriteLog(_T("[FAKEDI]  FakeGetDeviceInfo:: Using ANSI"));

				for(int i = 0; i < 4; i++)
				{
					if(Gamepad[i].product.Data1 != NULL && Gamepad[i].product.Data1 == pdidi->guidProduct.Data1)
					{
						DIDEVICEINSTANCEA OrigInst, FakeInst;
						memcpy(&OrigInst,pdidi,pdidi->dwSize);
						memcpy(&FakeInst,pdidi,pdidi->dwSize);

						DWORD dwFakePIDVID = MAKELONG(wFakeVID,wFakePID);

						FakeInst.guidProduct.Data1=dwFakePIDVID;
						FakeInst.guidProduct.Data2=0x0000;
						FakeInst.guidProduct.Data3=0x0000;
						BYTE pdata4[8] = {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44};
						memcpy(&FakeInst.guidProduct.Data4, pdata4, sizeof(pdata4));

						TCHAR strOriginalguidProduct[50];
						TCHAR strFakeguidProduct[50];
						GUIDtoString(strOriginalguidProduct,&OrigInst.guidProduct);
						GUIDtoString(strFakeguidProduct,&FakeInst.guidProduct);
						WriteLog(_T("[FAKEDI]  GUID change from %s to %s"),strOriginalguidProduct,strFakeguidProduct);

						FakeInst.dwDevType = 66069;
						FakeInst.wUsage = 5;
						FakeInst.wUsagePage = 1;

						sprintf_s(FakeInst.tszProductName, "%s", "XBOX 360 For Windows (Controller)");
						sprintf_s(FakeInst.tszInstanceName, "%s", "XBOX 360 For Windows (Controller)");

						WriteLog(L"[FAKEDI]  Product Name change from %S to %S",OrigInst.tszProductName,FakeInst.tszProductName);
						WriteLog(L"[FAKEDI]  Instance Name change from %S to %S",OrigInst.tszInstanceName,FakeInst.tszInstanceName);

						memcpy(pdidi,&FakeInst,FakeInst.dwSize);
						hr=DI_OK;
					}
				}
			}
			else															//UNICODE
			{
				WriteLog(_T("[FAKEDI]  FakeGetDeviceInfo:: Using UNICODE"));
				for(int i = 0; i < 4; i++)
				{
					if(Gamepad[i].product.Data1 != NULL && Gamepad[i].product.Data1 == pdidi->guidProduct.Data1)
					{
						DIDEVICEINSTANCEW OrigInst, FakeInst;						
						memcpy(&OrigInst,pdidi,pdidi->dwSize);
						memcpy(&FakeInst,pdidi,pdidi->dwSize);

						DWORD dwFakePIDVID = MAKELONG(wFakeVID,wFakePID);
						FakeInst.guidProduct.Data1=dwFakePIDVID;
						FakeInst.guidProduct.Data2=0x0000;
						FakeInst.guidProduct.Data3=0x0000;
						BYTE pdata4[8] = {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44};
						memcpy(&FakeInst.guidProduct.Data4, pdata4, sizeof(pdata4));

						TCHAR strFakeguidProduct[50];
						TCHAR strOriginalguidProduct[50];
						GUIDtoString(strOriginalguidProduct,&OrigInst.guidProduct);
						GUIDtoString(strFakeguidProduct,&FakeInst.guidProduct);
						WriteLog(_T("[FAKEDI]  GUID change from %s to %s"),strOriginalguidProduct,strFakeguidProduct);

						FakeInst.dwDevType = 66069;
						FakeInst.wUsage = 5;
						FakeInst.wUsagePage = 1;
						_stprintf_s(FakeInst.tszProductName, _T("%s"), _T("XBOX 360 For Windows (Controller)"));
						_stprintf_s(FakeInst.tszInstanceName, _T("%s"), _T("XBOX 360 For Windows (Controller)"));
						WriteLog(_T("[FAKEDI]  Product Name change from %s to %s"), OrigInst.tszProductName, FakeInst.tszProductName );
						WriteLog(_T("[FAKEDI]  Instance Name change from %s to %s"), OrigInst.tszInstanceName, FakeInst.tszInstanceName );

						memcpy(pdidi,&FakeInst,FakeInst.dwSize);
						hr=DI_OK;
					}
				}
			}
		}
	}
	return hr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE FakeGetProperty (LPDIRECTINPUTDEVICE8 This, REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
	HRESULT hr;
	hr = OriginalGetProperty (This, rguidProp, pdiph);
	WriteLog(_T("[FAKEDI]  FakeGetProperty"));

	if ( (&rguidProp==&DIPROP_VIDPID) && (wFakeMode == 2) )
	{
		DWORD dwFakePIDVID = MAKELONG(wFakeVID,wFakePID);

		WriteLog(_T("[FAKEDI]  Original VIDPID = %08X"),((LPDIPROPDWORD)pdiph)->dwData);
		((LPDIPROPDWORD)pdiph)->dwData = dwFakePIDVID;
		WriteLog(_T("[FAKEDI]  Fake VIDPID = %08X"),((LPDIPROPDWORD)pdiph)->dwData);
	}
	if ( (&rguidProp==&DIPROP_PRODUCTNAME) && (wFakeMode == 2) )
	{
		WriteLog(_T("[FAKEDI]  Original PRODUCTNAME = %s"),((LPDIPROPSTRING)pdiph)->wsz);
		_stprintf_s( ((LPDIPROPSTRING)pdiph)->wsz, _T("%s"), _T("XBOX 360 For Windows (Controller)") );
		WriteLog(_T("[FAKEDI]  Fake PRODUCTNAME = %s"),((LPDIPROPSTRING)pdiph)->wsz);

	}
	return hr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE FakeCreateDevice (LPDIRECTINPUT8 This, REFGUID rguid, LPDIRECTINPUTDEVICE8 * lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	HRESULT hr;
	LPDIRECTINPUTDEVICE8 pDID;

	hr = OriginalCreateDevice (This, rguid, lplpDirectInputDevice, pUnkOuter);
	if(lplpDirectInputDevice)
	{
		WriteLog(_T("[FAKEDI]  FakeCreateDevice"));
		pDID = (LPDIRECTINPUTDEVICE8) *lplpDirectInputDevice;
		if(pDID != NULL)
		{
			if(!OriginalGetDeviceInfo)
			{
				OriginalGetDeviceInfo = pDID->lpVtbl->GetDeviceInfo;
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourAttach(&(PVOID&)OriginalGetDeviceInfo, FakeGetDeviceInfo);
				DetourTransactionCommit();
			}
			if(!OriginalGetProperty)
			{
				OriginalGetProperty = pDID->lpVtbl->GetProperty;
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourAttach(&(PVOID&)OriginalGetProperty, FakeGetProperty);
				DetourTransactionCommit();
			}
		}
	}
	return hr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT WINAPI FakeDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{

	HRESULT hr;
	LPDIRECTINPUT8 pDI;

	hr = OriginalDirectInput8Create(hinst,dwVersion,riidltf,ppvOut,punkOuter);

	if(ppvOut) 
	{
		WriteLog(_T("[FAKEDI]  FakeDirectInput8Create"));

		pDI = (LPDIRECTINPUT8) *ppvOut;
		if(pDI) 
		{
			if(!OriginalCreateDevice)
			{
				OriginalCreateDevice = pDI->lpVtbl->CreateDevice;

				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourAttach(&(PVOID&)OriginalCreateDevice, FakeCreateDevice);
				DetourTransactionCommit();
			}
		}
	}

	return hr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FakeDI()
{
	WriteLog(_T("[FAKEAPI] FakeDI:: Attaching"));
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)OriginalDirectInput8Create, FakeDirectInput8Create);
	DetourTransactionCommit();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void FakeDI_Detach()
{
	WriteLog(_T("[FAKEAPI] FakeDI:: Detaching"));

	if(OriginalGetDeviceInfo)
	{
		WriteLog(_T("[FAKEDI]  FakeGetDeviceInfo:: Detaching"));
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OriginalGetDeviceInfo, FakeGetDeviceInfo);
		DetourTransactionCommit();
	}

	if(OriginalGetProperty)
	{
		WriteLog(_T("[FAKEDI]  FakeGetProperty:: Detaching"));
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OriginalGetProperty, FakeGetProperty);
		DetourTransactionCommit();
	}

	if(OriginalCreateDevice)
	{
		WriteLog(_T("[FAKEDI]  FakeCreateDevice:: Detaching"));
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OriginalCreateDevice, FakeCreateDevice);
		DetourTransactionCommit();
	}

	if(OriginalDirectInput8Create)
	{
		WriteLog(_T("[FAKEDI]  FakeDirectInput8Create:: Detaching"));
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OriginalDirectInput8Create, FakeDirectInput8Create);
		DetourTransactionCommit();
	}
}