#include "stdafx.h"
#include "SimpleIni.h"
#include "Logger.h"
#include "InputHook\InputHook.h"

#include "Config.h"

static const char * legal_notice = {
	"\nx360ce - XBOX 360 Controller emulator\n"
	"https://code.google.com/p/x360ce/\n\n"
	"Copyright (C) 2010-2014 Robert Krawczyk\n\n"
	"This program is free software you can redistribute it and/or modify it under\n"
	"the terms of the GNU Lesser General Public License as published by the Free\n"
	"Software Foundation, either version 3 of the License, or any later version.\n" };

extern iHook g_Hooks;
extern std::string exename;
//extern std::vector<DInputDevice> g_Devices;

ConfigINI::ConfigINI():
m_config_filename("x360ce.ini")
{
}


ConfigINI::~ConfigINI()
{
}

bool ConfigINI::SaveConfig(CSimpleIni* ini)
{
	//TODO: make this more usefull
	ini->SetLongValue("Version", "Configuration", CONFIG_VERSION);
	ini->SaveFile(m_config_filename);
}
bool ConfigINI::LoadConfig()
{
	CSimpleIni ini;
	ini.LoadFile("x360ce.ini");

	// check if config is valid
	int32_t version = ini.GetLongValue("Version", "Configuration");
	if (version != CONFIG_VERSION || ini.IsEmpty())
	{
		PrintLog("WARNING: Configuration file is empty or wrong version");
		SaveConfig(&ini);
		ini.LoadFile("x360ce.ini");
	}

	// Read global options
	m_global_disable = ini.GetBoolValue("Options", "Disable");
	if (m_global_disable) return true;

	m_init_beep = ini.GetBoolValue("Options", "UseInitBeep", true);

	bool file = ini.GetBoolValue("Options", "Log");
	bool con = ini.GetBoolValue("Options", "Console");
	if (con) LogConsole("x360ce", legal_notice);
	if (file)
	{
		//TODO: print the date insted of GetTickCount()
		char logfilename[MAX_PATH];
		sprintf_s(logfilename, "x360ce_%s_%u.log", exename.c_str(), GetTickCount());
		LogFile(logfilename);
	}

	bool override = ini.GetBoolValue("InputHook", "Override"); 

	CSimpleIni gdb;
	gdb.LoadFile("x360ce.gdb");
	DWORD hookMask = gdb.GetLongValue(exename.c_str(), "HookMask");

	if (hookMask && override == false)
	{
		g_Hooks.SetMask(hookMask);
		g_Hooks.Enable();
	}
	else
	{
		hookMask = ini.GetLongValue("InputHook", "HookMask");
		if (hookMask)
		{
			g_Hooks.SetMask(hookMask);
			g_Hooks.Enable();
		}
		else
		{
			hookMask = 0;
			if (ini.GetBoolValue("InputHook", "HookLL"))
				hookMask |= iHook::HOOK_LL;

			if (ini.GetBoolValue("InputHook", "HookCOM"))
				hookMask |= iHook::HOOK_COM;

			if (ini.GetBoolValue("InputHook", "HookDI"))
				hookMask |= iHook::HOOK_DI;

			if (ini.GetBoolValue("InputHook", "HookPIDVID"))
				hookMask |= iHook::HOOK_PIDVID;

			if (ini.GetBoolValue("InputHook", "HookSA"))
				hookMask |= iHook::HOOK_SA;

			if (ini.GetBoolValue("InputHook", "HookNAME"))
				hookMask |= iHook::HOOK_NAME;

			if (ini.GetBoolValue("InputHook", "HookSTOP"))
				hookMask |= iHook::HOOK_STOP;

			if (ini.GetBoolValue("InputHook", "HookWT"))
				hookMask |= iHook::HOOK_WT;

			if (ini.GetBoolValue("InputHook", "HookNoTimeout"))
				hookMask |= iHook::HOOK_NOTIMEOUT;

			g_Hooks.SetMask(hookMask);
			if (g_Hooks.GetMask()) g_Hooks.Enable();
		}
	}
	if (g_Hooks.GetState(iHook::HOOK_PIDVID))
	{
		DWORD vid = ini.GetLongValue("InputHook", "FakeVID", 0x045E);
		DWORD pid = ini.GetLongValue("InputHook", "FakePID", 0x028E);
		g_Hooks.SetFakePIDVID(MAKELONG(vid, pid));
	}

	DWORD timeout = ini.GetLongValue("InputHook", "Timeout", 30);
	g_Hooks.SetTimeout(timeout);

	// Read pad mappings
	for (DWORD i = 0; i < 4; ++i)
		ReadPadConfig(i, &ini);
}

bool ReadPadConfig(DWORD dwUserIndex, CSimpleIni* ini)
{
	return true;
}