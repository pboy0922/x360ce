#pragma once

#include "Config.h"
#include "SimpleIni.h"

class ConfigINI : public ConfigBase
{
public:

	ConfigINI();
	ConfigINI(string filename);
	virtual ~ConfigINI();

	bool SaveConfig();
	bool LoadConfig();

private:
	void Initialize();
	bool ReadPadConfig(u8 index);

	CSimpleIni* m_ini;
	string m_config_filename;
};