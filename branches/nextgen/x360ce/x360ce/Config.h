#pragma once

namespace Mappings
{
	static const char* const buttonNames[] =
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

	static const char* const povNames[] =
	{
		"D-pad Up",
		"D-pad Down",
		"D-pad Left",
		"D-pad Right"
	};

	static const char* const axisNames[] =
	{
		"Left Analog X",
		"Left Analog Y",
		"Right Analog X",
		"Right Analog Y"
	};

	static const char* const axisDZNames[] =
	{
		"Left Analog X DeadZone",
		"Left Analog Y DeadZone",
		"Right Analog X DeadZone",
		"Right Analog Y DeadZone",
	};

	static const char* const axisADZNames[] =
	{
		"Left Analog X AntiDeadZone",
		"Left Analog Y AntiDeadZone",
		"Right Analog X AntiDeadZone",
		"Right Analog Y AntiDeadZone",
	};

	static const char* const axisLNames[] =
	{
		"Left Analog X Linear",
		"Left Analog Y Linear",
		"Right Analog X Linear",
		"Right Analog Y Linear"
	};

	static const char* const axisBNames[] =
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

	enum MappingType { NONE, AXIS, SLIDER, HAXIS, HSLIDER };

	enum PovIDs
	{
		GAMEPAD_DPAD_UP = 0,
		GAMEPAD_DPAD_RIGHT = 9000,
		GAMEPAD_DPAD_DOWN = 18000,
		GAMEPAD_DPAD_LEFT = 27000,
		GAMEPAD_DPAD_NONE = -1,
	};

	struct DpadPov
	{
		int32_t up;
		int32_t right;
		int32_t down;
		int32_t left;

		DpadPov() :
			up(GAMEPAD_DPAD_UP),
			right(GAMEPAD_DPAD_RIGHT),
			down(GAMEPAD_DPAD_DOWN),
			left(GAMEPAD_DPAD_LEFT)
		{}
	};

	struct AxisMap
	{
		MappingType type;
		int32_t id;
		int32_t positiveButtonID, negativeButtonID;
		bool is_button;
		bool is_reversed;

		AxisMap() :
			type(NONE),
			id(-1),
			is_button(false)
		{}
	};

	struct TriggerMap
	{
		MappingType type;
		int32_t id;
		int32_t button;

		bool is_button;
		bool is_reversed;

		TriggerMap() :
			type(NONE),
			id(-1),
			button(0)
		{}
	};

	struct Mapping
	{
		TriggerMap trigger[2];
		AxisMap axis[4];
		DpadPov pov;
		int8_t button[10];
		int8_t guide_button;
		int8_t pov_index;

		bool PovIsButton;
		Mapping() :
			guide_button(0),
			pov_index(-1)
		{}
	};
}

#define CONFIG_VERSION 100

class ConfigBase
{
public:
	ConfigBase():
		m_global_disable(false),
		m_init_beep(true)
	{}
	virtual ~ConfigBase(){};

	virtual bool LoadConfig() = 0;

	bool is_disabled()
	{
		return m_global_disable;
	}

	bool init_beep()
	{
		return m_init_beep;
	}

protected:
	bool m_global_disable;
	bool m_init_beep;
};

class ConfigINI : public ConfigBase
{
public:

	ConfigINI();
	virtual ~ConfigINI();

	bool LoadConfig();
	bool SaveConfig(CSimpleIni* ini);

private:
	const char* m_config_filename;
};