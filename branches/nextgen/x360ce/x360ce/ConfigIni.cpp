#include "stdafx.h"
#include "ConfigIni.h"
#include "Logger.h"
#include "InputHook\InputHook.h"

extern iHook g_Hooks;
extern std::string exename;
//extern std::vector<DInputDevice> g_Devices;

ConfigINI::ConfigINI() :
m_config_filename("x360ce.ini")
{
}

ConfigINI::ConfigINI(string filename) :
m_config_filename(filename)
{
}


ConfigINI::~ConfigINI()
{
	delete m_ini;
}

void ConfigINI::Initialize()
{
	// Open as UTF8
	m_ini = new CSimpleIni(true, false, false);
	m_ini->LoadFile(m_config_filename.c_str());
}

bool ConfigINI::SaveConfig()
{
	//TODO: make this more usefull
	m_ini->SetLongValue("Version", "Configuration", CONFIG_VERSION);
	m_ini->SaveFile(m_config_filename.c_str());
}
bool ConfigINI::LoadConfig()
{
	// check if config is valid
	int32_t version = m_ini->GetLongValue("Version", "Configuration");
	if (version != CONFIG_VERSION || m_ini->IsEmpty())
	{
		PrintLog("WARNING: Configuration file is empty or wrong version");
		SaveConfig();
		m_ini->LoadFile(m_config_filename.c_str());
	}

	// Read global options
	m_global_disable = m_ini->GetBoolValue("Options", "Disable");
	if (m_global_disable) return true;

	m_init_beep = m_ini->GetBoolValue("Options", "UseInitBeep", true);

	bool file = m_ini->GetBoolValue("Options", "Log");
	bool con = m_ini->GetBoolValue("Options", "Console");
	if (con) LogConsole("x360ce", legal_notice);
	if (file)
	{
		//TODO: print the date insted of GetTickCount()
		char logfilename[MAX_PATH];
		sprintf_s(logfilename, "x360ce_%s_%u.log", exename.c_str(), GetTickCount());
		LogFile(logfilename);
	}

	bool override = m_ini->GetBoolValue("InputHook", "Override");

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
		hookMask = m_ini->GetLongValue("InputHook", "HookMask");
		if (hookMask)
		{
			g_Hooks.SetMask(hookMask);
			g_Hooks.Enable();
		}
		else
		{
			hookMask = 0;
			if (m_ini->GetBoolValue("InputHook", "HookLL"))
				hookMask |= iHook::HOOK_LL;

			if (m_ini->GetBoolValue("InputHook", "HookCOM"))
				hookMask |= iHook::HOOK_COM;

			if (m_ini->GetBoolValue("InputHook", "HookDI"))
				hookMask |= iHook::HOOK_DI;

			if (m_ini->GetBoolValue("InputHook", "HookPIDVID"))
				hookMask |= iHook::HOOK_PIDVID;

			if (m_ini->GetBoolValue("InputHook", "HookSA"))
				hookMask |= iHook::HOOK_SA;

			if (m_ini->GetBoolValue("InputHook", "HookNAME"))
				hookMask |= iHook::HOOK_NAME;

			if (m_ini->GetBoolValue("InputHook", "HookSTOP"))
				hookMask |= iHook::HOOK_STOP;

			if (m_ini->GetBoolValue("InputHook", "HookWT"))
				hookMask |= iHook::HOOK_WT;

			if (m_ini->GetBoolValue("InputHook", "HookNoTimeout"))
				hookMask |= iHook::HOOK_NOTIMEOUT;

			g_Hooks.SetMask(hookMask);
			g_Hooks.Enable();
		}
	}
	if (g_Hooks.GetState(iHook::HOOK_PIDVID))
	{
		DWORD vid = m_ini->GetLongValue("InputHook", "FakeVID", 0x045E);
		DWORD pid = m_ini->GetLongValue("InputHook", "FakePID", 0x028E);
		g_Hooks.SetFakePIDVID(MAKELONG(vid, pid));
	}

	DWORD timeout = m_ini->GetLongValue("InputHook", "Timeout", 30);
	g_Hooks.SetTimeout(timeout);

	// Read pad mappings
	for (u8 i = 0; i < 4; ++i)
		ReadPadConfig(i);
}

bool ConfigINI::ReadPadConfig(u8 index)
{
	return true;
}