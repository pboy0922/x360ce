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

#ifndef _DIRECTINPUT_H_
#define _DIRECTINPUT_H_

#include <Controller.h>
#include <dinput.h>

namespace Controller {

	LPDIRECTINPUT8 di=NULL;

	class DiController : public Controller
	{
	protected:
		GUID productid;
		GUID instanceid;

		LPDIRECTINPUTDEVICE8 device;
		DIJOYSTATE distate;
		DIJOYSTATE2 distate2;

	public:
		DiController():
			productid(GUID_NULL),
			instanceid(GUID_NULL)
		{};
		virtual ~DiController();

		bool DiController::AcquireDevice(LPDIRECTINPUTDEVICE8 lpDirectInputDevice);
		BOOL CALLBACK DiController::EnumGamepadsCallback( const DIDEVICEINSTANCE* pInst, VOID* pContext );


		/*
		DWORD dwPadIndex;
		UINT dwAxisCount;
		UINT tdeadzone;
		SHORT adeadzone[4];
		SHORT antidz[4];
		INT axistodpaddeadzone;
		INT axistodpadoffset;
		SHORT axislinear[4];
		BYTE gamepadtype;
		*/
	};

#if 0
	HRESULT InitDirectInput( HWND hDlg, DWORD idx );
	BOOL ButtonPressed(DWORD, INT);
	HRESULT UpdateState( DWORD );
	WORD EnumPadCount();
	HRESULT Enumerate(DWORD idx);
	void ReleaseDirectInput();
	void Deactivate();
	BOOL CALLBACK EnumEffectsCallback(LPCDIEFFECTINFO di, LPVOID pvRef);

	HRESULT SetDeviceForces(DWORD idx, WORD force, WORD effidx);
	HRESULT PrepareForce(DWORD idx, WORD effidx);

	HRESULT SetDeviceForcesFailsafe(DWORD idx, WORD force, WORD effidx);
	HRESULT PrepareForceFailsafe(DWORD idx, WORD effidx);

	HRESULT SetDeviceForcesEjocys(DWORD idx, WORD force, WORD effidx);
	HRESULT PrepareForceEjocys(DWORD idx, WORD effidx);

	HRESULT SetDeviceForcesNew(DWORD idx, WORD force, WORD effidx);
	HRESULT PrepareForceNew(DWORD idx, WORD effidx);
#endif

}

#endif
