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

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <windows.h>
#include <xinput.h>

namespace Controller {

	static const DWORD STATE_NONE			= 0x00000000;
	static const DWORD STATE_READY			= 0x00000001;
	static const DWORD STATE_CONNECTED		= 0x00000002;
	static const DWORD STATE_FAIL			= 0x80000000;

	static const DWORD OPTIONS_NONE			= 0x00000000;
	static const DWORD OPTIONS_SWAPMOTOR	= 0x00000001;
	static const DWORD OPTIONS_DEADZONE		= 0x00000002;
	static const DWORD OPTIONS_TDEADZONE	= 0x00000004;
	static const DWORD OPTIONS_ADEADZONE	= 0x00000008;
	static const DWORD OPTIONS_AXIS2DPAD	= 0x00000010;
	static const DWORD OPTIONS_PASSTROUGH	= 0x80000000;

	class Controller
	{
	protected:
		DWORD state;
		DWORD options;
		DWORD userIndex;

	public:
		Controller():
			state(0),
			options(0),
			userIndex(-1)
		{};
		virtual ~Controller();

		virtual HRESULT UpdateDeviceState();
		virtual HRESULT GetDeviceProps(XINPUT_CAPABILITIES &caps); 
		virtual HRESULT GetDeviceState(XINPUT_STATE &state); 
		//virtual BOOL SetDeviceState(XINPUT_VIBRATION* pVib); 

		virtual HRESULT SetDeviceConfig(DWORD userIndex, DWORD options, VOID* pContext){this->userIndex = userIndex; this->options = options;}
		virtual HRESULT GetOptions(DWORD o){o = options;}
		virtual HRESULT SetOptions(DWORD o){options |= o;}
		virtual HRESULT GetState(DWORD s){s = state;}
		virtual HRESULT SetState(DWORD s){state |= s;}

	};
}

#endif //_CONTROLLER_H_