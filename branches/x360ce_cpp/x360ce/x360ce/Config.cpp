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
#include "Utilities\Ini.h"
#include "Utilities\Log.h"
#include "Utilities\Misc.h"
#include "Config.h"
#include "DirectInput.h"

BOOL bInitBeep=true;
WORD wNativeMode=0;

extern iHook g_iHook;

//IHOOK_CONIFG x360ce_InputHookConfig;
//IHOOK_GAMEPAD_CONIFG x360ce_InputHookGamepadConfig[4];

BOOL g_Disable;

static LPCSTR buttonNames[] =
{
	"A",
	"B",
	"X",
	"Y",
	"Left Shoulder",
	"Right Shoulder",
	"Back",
	"Start",
	"Left Thumb",
	"Right Thumb",
};

static LPCSTR povNames[] =
{
	"D-pad Up",
	"D-pad Down",
	"D-pad Left",
	"D-pad Right"
};

static LPCSTR axisNames[] =
{
	"Left Analog X",
	"Left Analog Y",
	"Right Analog X",
	"Right Analog Y"
};

static LPCSTR triggerNames[] =
{
	"Right Trigger",
	"Left Trigger"
};

static LPCSTR axisDZNames[] =
{
	"Left Analog X DeadZone",
	"Left Analog Y DeadZone",
	"Right Analog X DeadZone",
	"Right Analog Y DeadZone",
};

static LPCSTR axisADZNames[] =
{
	"Left Analog X AntiDeadZone",
	"Left Analog Y AntiDeadZone",
	"Right Analog X AntiDeadZone",
	"Right Analog Y AntiDeadZone",
};

static LPCSTR axisLNames[] =
{
	"Left Analog X Linear",
	"Left Analog Y Linear",
	"Right Analog X Linear",
	"Right Analog Y Linear"
};

static LPCSTR axisBNames[] =
{
	"Left Analog X+ Button",
	"Left Analog X- Button",
	"Left Analog Y+ Button",
	"Left Analog Y- Button",
	"Right Analog X+ Button",
	"Right Analog X- Button",
	"Right Analog Y+ Button",
	"Right Analog Y- Button"
};

static LPCSTR padNames[] =
{
	"PAD1",
	"PAD2",
	"PAD3",
	"PAD4",
};

GamepadMap GamepadMapping[4];

void ReadConfig(char* ininame)
{
	// Read global options
	InI ini;
	ini.open(ininame);

	ini.useSection("Options");
	g_Disable = ini.getValueAsBoolean("Disable");
	bInitBeep = ini.getValueAsBoolean("UseInitBeep"); 
	LogEnable(ini.getValueAsBoolean("Log"));
	enableconsole = ini.getValueAsBoolean("Console");
	if(g_Disable) return;

	//InputHook
	ini.useSection("InputHook");
	g_iHook.SetHookMode(ini.getValueAsLong("HookMode"));
	g_iHook.EnableANSIMode(ini.getValueAsBoolean("HookUseANSI"));
	g_iHook.EnableTrustHook(ini.getValueAsBoolean("HookWinTrust"));

	if(g_iHook.GetHookMode())
	{
		g_iHook.Enable();
		DWORD vid =  ini.getValueAsLong("HookVID");
		DWORD pid = ini.getValueAsLong("HookPID");
		if(vid != 0x045E || pid != 0x28E) g_iHook.SetHookVIDPID(MAKELONG(pid,vid));
	}
	// Read pad mappings
	for (DWORD i = 0; i < 4; ++i)
		ReadPadConfig(i, ini);
}


void ReadPadConfig(DWORD idx, InI &ini)
{
	//char section[MAX_PATH] = "Mappings";
	//char key[MAX_PATH];
	
	uint32_t val = 0;

	string section = "Mappings";
	string key = "";
	stringstream converter;
	string tmp = "";

	converter << "PAD" << idx+1;
	key = converter.str();

	//sprintf_s(key,"PAD%u",idx+1);

	section = ini.getValue(section,key);
	ini.useSection(section);

	//WCHAR NullGUIDStr[50];
	//GUIDtoString(GUID_NULL,NullGUIDStr);

	GamepadMap &PadMap = GamepadMapping[idx];

	tmp = ini.getValue("ProductGuid");
	StringToGUID(tmp.c_str(),&g_Gamepad[idx].productGUID);

	tmp = ini.getValue("InstanceGuid");
	StringToGUID(tmp.c_str(),&g_Gamepad[idx].instanceGUID);

	for (int i=0; i<4; ++i)
		g_Gamepad[idx].antidz[i] = clamp(ini.getValueAsLong(axisADZNames[i]),0,32767);

	//FIXME
	g_Gamepad[idx].passthrough = ini.getValueAsBoolean("PassThrough");

	if(g_Gamepad[idx].passthrough)
	{
		wNativeMode = 1;
		g_Gamepad[idx].configured = true;
		PadMap.enabled = false;
		return;
	}

	if (!(IsEqualGUID(g_Gamepad[idx].productGUID,GUID_NULL)) && !(IsEqualGUID(g_Gamepad[idx].instanceGUID,GUID_NULL)))
	{
		g_Gamepad[idx].configured = true;
		PadMap.enabled = true;
	}
	else
	{
		return;
	}

	g_Gamepad[idx].dwPadIndex = idx;

	g_Gamepad[idx].ff.type = (BYTE) ini.getValueAsLong("FFBType");
	g_Gamepad[idx].swapmotor = ini.getValueAsLong("SwapMotor");
	g_Gamepad[idx].tdeadzone = ini.getValueAsLong("TriggerDeadzone");
	g_Gamepad[idx].ff.useforce = ini.getValueAsLong("UseForceFeedback"); 
	g_Gamepad[idx].gamepadtype = ini.getValueAsLong("ControllerType");
	g_Gamepad[idx].axistodpad = ini.getValueAsBoolean("AxisToDPad");
	g_Gamepad[idx].axistodpaddeadzone = ini.getValueAsLong("AxisToDPadDeadZone"); 
	g_Gamepad[idx].axistodpadoffset = ini.getValueAsLong("AxisToDPadOffset");
	g_Gamepad[idx].ff.forcepercent = ini.getValueAsLong("ForcePercent") * 0.01; 
	g_Gamepad[idx].ff.leftPeriod = ini.getValueAsLong("LeftMotorPeriod");
	g_Gamepad[idx].ff.rightPeriod = ini.getValueAsLong("RightMotorPeriod");

	memset(PadMap.Button,-1,sizeof(PadMap.Button));

	for (int i = 0; i < 2; ++i) PadMap.Trigger[i].type = NONE;

	///////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < 2; ++i) PadMap.Trigger[i].but = -1;

	///////////////////////////////////////////////////////////////////////////////////////

	PadMap.DpadPOV = (WORD) -1;

	for (int i=0; i<10; ++i)
	{	
		if (ini.getValueAsLong(buttonNames[i]) > 0)
			PadMap.Button[i] = ini.getValueAsLong(buttonNames[i]) - 1;
	}

	for (int i=0; i<4; ++i)
	{
		if (ini.getValue(povNames[i]).length() > 0)
		{
			val = ini.getValueAsLong(povNames[i]);

			if(val < 100 && val != 0)
			{
				PadMap.pov[i] = val - 1;
				PadMap.PovIsButton = true;
			}
			else 
			{
				PadMap.pov[i] = val;
				PadMap.PovIsButton = false;
			}
		}

		if (ini.getValue(axisNames[i]).length() > 0)
		{
			tmp = ini.getValue(axisNames[i]);
			char a = tmp[0];

			if (towlower(a) == L's')   // Slider
			{
				PadMap.Axis[i].analogType = SLIDER;
				tmp.erase(0,1);
				PadMap.Axis[i].id = atoi(tmp.c_str());
			}
			else
			{
				// Axis
				PadMap.Axis[i].analogType = AXIS;
				PadMap.Axis[i].id = atoi(tmp.c_str());
			}
		}

		g_Gamepad[idx].adeadzone[i] =  ini.getValueAsLong(axisDZNames[i]);
		g_Gamepad[idx].axislinear[i] = ini.getValueAsLong(axisLNames[i]); 

		if (INT ret = ini.getValueAsLong(axisBNames[i*2]) > 0)
		{
			PadMap.Axis[i].hasDigital = true;
			PadMap.Axis[i].positiveButtonID = ret - 1;
		}

		if (INT ret = ini.getValueAsLong( axisBNames[i*2+1]) > 0)
		{
			PadMap.Axis[i].hasDigital = true;
			PadMap.Axis[i].negativeButtonID = ret - 1;
		}
	}

	for (INT i=0; i<2; ++i)
	{
		if (ini.getValue( triggerNames[i],"0").length() > 0)
		{
			tmp = ini.getValue(triggerNames[i]);

			char a = tmp[0];

			if ((PadMap.Trigger[0].type = getTriggerType(a)) == DIGITAL)
			{
				PadMap.Trigger[0].id = atoi(tmp.c_str()) - 1;
			}
			else
			{
				tmp.erase(0,1);
				PadMap.Trigger[0].id = atoi(tmp.c_str());
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	if (ini.getValueAsLong("Left Trigger But") > 0)
	{
		PadMap.Trigger[0].but = ini.getValueAsLong("Left Trigger But") - 1;
	}

	if (ini.getValueAsLong("Right Trigger But") > 0)
	{
		PadMap.Trigger[1].but = ini.getValueAsLong("Left Trigger But") - 1;
	}

	///////////////////////////////////////////////////////////////////////////////////////

	if (ini.getValueAsLong("D-pad POV") > 0)
	{
		PadMap.DpadPOV = ini.getValueAsLong("D-pad POV") - 1;
	}
}

// NOTE: Letters corresponding to mapping types changed. Include in update notes.
MappingType getTriggerType(char c)
{
	if (towlower(c) == L'a') return AXIS;	// Axis
	if (towlower(c) == L's') return SLIDER;	// Slider
	if (towlower(c) == L'x') return HAXIS;	// Half range axis
	if (towlower(c) == L'h') return HSLIDER;	// Half range slider
	if (towlower(c) == L'z') return CBUT;

	return DIGITAL;							// Digital
}
