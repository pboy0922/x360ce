/*  x360ce - XBOX360 Controler Emulator
*  Copyright (C) 2002-2010 Racer_S
*  Copyright (C) 2010-2011 Robert Krawczyk
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
#include "x360ce.h"
#include "Utilities\Log.h"
#include "Utilities\Misc.h"
#include "Config.h"
#include "InputHook\InputHook.h"
#include "DirectInput.h"

namespace Controller {
	
	DiController::DiController()
	{
		HRESULT hr = DirectInput8Create(g_hX360ceInstance,DIRECTINPUT_VERSION,IID_IDirectInput8,(VOID**)&di,NULL);
		if(FAILED(hr))
			di = NULL;
	}


	bool DiController::AcquireDevice(LPDIRECTINPUTDEVICE8 lpDirectInputDevice)
	{
		if (FAILED(lpDirectInputDevice->Acquire()))
		{
			HRESULT result = lpDirectInputDevice->Acquire();
			if (result == DIERR_OTHERAPPHASPRIO)
				return FALSE;
			if (FAILED (result))
			{
				//Deactivate();
				return FALSE;
			}

		}
		return TRUE;
	}
	
	BOOL CALLBACK EnumGamepadsCallback( const DIDEVICEINSTANCE* pInst, VOID* pContext )
	{
		DiController* controller = (DiController*) pContext;
		if(IsEqualGUID(controller->productid, pInst->guidProduct) && IsEqualGUID(instanceid, pInst->guidInstance) )
		{
			di->CreateDevice(pInst->guidInstance, &device, NULL );

			if(device)
			{
				state |= STATE_CONNECTED;
				WriteLog(LOG_DINPUT,L"[PAD%d] Device \"%s\" created",userIndex,pInst->tszProductName);
			}

			return DIENUM_STOP;
		}
		else
		{
			return DIENUM_CONTINUE;
		}
	}

	BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext )
	{

		Controller * gp = (Controller*) pContext;

		// For axes that are returned, set the DIPROP_RANGE property for the
		// enumerated axis in order to scale min/max values.
		if( pdidoi->dwType & DIDFT_AXIS )
		{
			DIPROPRANGE diprg;
			diprg.diph.dwSize       = sizeof(DIPROPRANGE);
			diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			diprg.diph.dwHow        = DIPH_BYID;
			diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
			diprg.lMin              = -32767;
			diprg.lMax              = +32767;

			// Set the range for the axis
			if( FAILED( gp->pGamepad->lpVtbl->SetProperty(gp->pGamepad, DIPROP_RANGE, &diprg.diph ) ) )
				return DIENUM_STOP;
		}

		gp->dwAxisCount++;
		return DIENUM_CONTINUE;
	}

	//-----------------------------------------------------------------------------
	// Name: EnumFFAxesCallback()
	// Desc: Callback function for enumerating the axes on a joystick and counting
	//       each force feedback enabled axis
	//-----------------------------------------------------------------------------
	BOOL CALLBACK EnumFFAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext )
	{
		DWORD* pdwNumForceFeedbackAxis = (DWORD*) pContext;

		if( ((pdidoi->dwFlags && DIDOI_FFACTUATOR) != 0) )
			(*pdwNumForceFeedbackAxis)++;

		return DIENUM_CONTINUE;
	}

	HRESULT UpdateState(DWORD idx )
	{
		HRESULT hr=E_FAIL;

		if( (!g_Gamepad[idx].pGamepad))
			return E_FAIL;

		// Poll the device to read the current state
		// not all devices must be polled so checking result code is useless
		g_Gamepad[idx].pGamepad->lpVtbl->Poll(g_Gamepad[idx].pGamepad);

		//But GetDeviceState must be succesed
		hr = g_Gamepad[idx].pGamepad->lpVtbl->GetDeviceState(g_Gamepad[idx].pGamepad, sizeof( DIJOYSTATE2 ), &g_Gamepad[idx].state );

		if(FAILED(hr))
		{
			WriteLog(LOG_DINPUT,L"[PAD%d] Device Reacquired",idx+1);
			hr = g_Gamepad[idx].pGamepad->lpVtbl->Acquire(g_Gamepad[idx].pGamepad);
		}

		return hr;
	}

	HRESULT Enumerate(DWORD idx)
	{
		HRESULT hr;

		Deactivate(idx);
		LPDIRECTINPUT8 lpDI8 = GetDirectInput();

		WriteLog(LOG_DINPUT,L"[PAD%d] Enumerating UserIndex %d",idx+1,idx);
		hr = di->EnumDevices(DI8DEVCLASS_GAMECTRL, DiController::EnumGamepadsCallback, &g_Gamepad[idx], DIEDFL_ATTACHEDONLY );

		if FAILED(hr)
		{
			WriteLog(LOG_DINPUT,L"[PAD%d] Enumeration FAILED !!!",idx+1);
			return hr;
		}

		if(!g_Gamepad[idx].pGamepad) WriteLog(LOG_DINPUT,L"[PAD%d] Enumeration FAILED !!!",idx+1);

		return ERROR_SUCCESS;
	}

	HRESULT InitDirectInput( HWND hDlg, DWORD idx )
	{

		if(!g_Gamepad[idx].pGamepad) return ERROR_DEVICE_NOT_CONNECTED;

		DIPROPDWORD dipdw;
		HRESULT hr=S_OK;
		HRESULT coophr=S_OK;

		// Set the data format to "simple joystick" - a predefined data format. A
		// data format specifies which controls on a device we are interested in,
		// and how they should be reported.
		//
		// This tells DirectInput that we will be passing a DIJOYSTATE structure to
		// IDirectInputDevice8::GetDeviceState(). Even though we won't actually do
		// it in this sample. But setting the data format is important so that the
		// DIJOFS_* values work properly.
		if( FAILED( hr = g_Gamepad[idx].pGamepad->lpVtbl->SetDataFormat(g_Gamepad[idx].pGamepad, &c_dfDIJoystick2 ) ) )
		{
			WriteLog(LOG_DINPUT,L"[PAD%d] SetDataFormat failed with code HR = %s", idx+1, DXErrStr(hr));
			return hr;
		}

		// Set the cooperative level to let DInput know how this device should
		// interact with the system and with other DInput applications.
		// Exclusive access is required in order to perform force feedback.
		if( FAILED( coophr = g_Gamepad[idx].pGamepad->lpVtbl->SetCooperativeLevel( g_Gamepad[idx].pGamepad, hDlg,
			DISCL_EXCLUSIVE |
			DISCL_BACKGROUND ) ) )
		{
			WriteLog(LOG_DINPUT,L"[PAD%d] SetCooperativeLevel (1) failed with code HR = %s", idx+1, DXErrStr(coophr));
			//return coophr;
		}

		if(coophr!=S_OK)
		{
			WriteLog(LOG_DINPUT,L"[Device not exclusive acquired, disabling ForceFeedback");
			g_Gamepad[idx].ff.useforce = 0;

			if( FAILED( coophr = g_Gamepad[idx].pGamepad->lpVtbl->SetCooperativeLevel( g_Gamepad[idx].pGamepad,hDlg,
				DISCL_NONEXCLUSIVE |
				DISCL_BACKGROUND ) ) )
			{
				WriteLog(LOG_DINPUT,L"[PAD%d] SetCooperativeLevel (2) failed with code HR = %s", idx+1, DXErrStr(coophr));
				//return coophr;
			}
		}

		// Since we will be playing force feedback effects, we should disable the
		// auto-centering spring.
		dipdw.diph.dwSize = sizeof( DIPROPDWORD );
		dipdw.diph.dwHeaderSize = sizeof( DIPROPHEADER );
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = FALSE;
		// not all gamepad drivers need this (like PS3), so do not check result code
		g_Gamepad[idx].pGamepad->lpVtbl->SetProperty(g_Gamepad[idx].pGamepad, DIPROP_AUTOCENTER, &dipdw.diph );

		if( FAILED( hr = g_Gamepad[idx].pGamepad->lpVtbl->EnumObjects(g_Gamepad[idx].pGamepad, EnumObjectsCallback,
			( VOID* )&g_Gamepad[idx], DIDFT_AXIS ) ) )
		{
			WriteLog(LOG_DINPUT,L"[PAD%d] EnumObjects failed with code HR = %s", idx+1, DXErrStr(hr));
			//return hr;
		}
		else
		{
			WriteLog(LOG_DINPUT,L"[PAD%d] Detected axis count: %d",idx+1,g_Gamepad[idx].dwAxisCount);
		}

		if( FAILED( hr = g_Gamepad[idx].pGamepad->lpVtbl->EnumObjects(g_Gamepad[idx].pGamepad, EnumFFAxesCallback,
			( VOID* )&g_Gamepad[idx].ff.dwNumForceFeedbackAxis, DIDFT_AXIS ) ) )
		{
			WriteLog(LOG_DINPUT,L"[PAD%d] EnumFFAxesCallback failed with code HR = %s", idx+1, DXErrStr(hr));
			//return hr;
		}

		if( g_Gamepad[idx].ff.dwNumForceFeedbackAxis > 2 )
			g_Gamepad[idx].ff.dwNumForceFeedbackAxis = 2;

		if( g_Gamepad[idx].ff.dwNumForceFeedbackAxis <= 0 )
			g_Gamepad[idx].ff.useforce = 0;

		return g_Gamepad[idx].pGamepad->lpVtbl->Acquire(g_Gamepad[idx].pGamepad);
	}

	BOOL ButtonPressed(DWORD buttonidx, INT idx)
	{
		return (g_Gamepad[idx].state.rgbButtons[buttonidx] & 0x80) != 0;
	}

}