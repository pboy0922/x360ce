#pragma once

#ifndef CURRENT_MODULE
extern "C" IMAGE_DOS_HEADER __ImageBase;
#define CURRENT_MODULE reinterpret_cast<HMODULE>(&__ImageBase)
#endif

extern HWND hMsgWnd;

class XInputEnabled
{
public:
	bool bEnabled;
	bool bUseEnabled;
	XInputEnabled() :
		bEnabled(false),
		bUseEnabled(false)
	{}
	virtual ~XInputEnabled() {};
};

struct xinput_dll
{
	HMODULE dll;

	// XInput 1.3 and older functions
	DWORD(WINAPI* XInputGetState)(DWORD dwUserIndex, XINPUT_STATE* pState);
	DWORD(WINAPI* XInputSetState)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
	DWORD(WINAPI* XInputGetCapabilities)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);
	VOID(WINAPI* XInputEnable)(BOOL enable);
	DWORD(WINAPI* XInputGetDSoundAudioDeviceGuids)(DWORD dwUserIndex, GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid);
	DWORD(WINAPI* XInputGetBatteryInformation)(DWORD  dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION* pBatteryInformation);
	DWORD(WINAPI* XInputGetKeystroke)(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeystroke);

	// XInput 1.3 undocumented functions
	DWORD(WINAPI* XInputGetStateEx)(DWORD dwUserIndex, XINPUT_STATE *pState); // 100
	DWORD(WINAPI* XInputWaitForGuideButton)(DWORD dwUserIndex, DWORD dwFlag, LPVOID pVoid); // 101
	DWORD(WINAPI* XInputCancelGuideButtonWait)(DWORD dwUserIndex); // 102
	DWORD(WINAPI* XInputPowerOffController)(DWORD dwUserIndex); // 103

	// XInput 1.4 functions
	DWORD(WINAPI* XInputGetAudioDeviceIds)(DWORD dwUserIndex, LPWSTR pRenderDeviceId, UINT* pRenderCount, LPWSTR pCaptureDeviceId, UINT* pCaptureCount);

	// XInput 1.4 undocumented functionss
	DWORD(WINAPI* XInputGetBaseBusInformation)(DWORD dwUserIndex, struct XINPUT_BUSINFO* pBusinfo); // 104
	DWORD(WINAPI* XInputGetCapabilitiesEx)(DWORD unk1, DWORD dwUserIndex, DWORD dwFlags, struct XINPUT_CAPABILITIESEX* pCapabilitiesEx); // 108

	xinput_dll(){ ZeroMemory(this, sizeof(xinput_dll)); }
};

extern xinput_dll xinput;

template<typename N>
inline void LoadFunctionTemplate(HMODULE module, const char* funcname, N* ppfunc)
{
	if (ppfunc) *ppfunc = reinterpret_cast<N>(GetProcAddress(module, funcname));
}

#define LoadFunction(handle_struct, func) LoadFunctionTemplate(handle_struct ## .dll, #func, &handle_struct ## .func)
#define LoadFunctionOrdinal(handle_struct, ordinal, func) LoadFunctionTemplate(handle_struct ## .dll, (const char*)ordinal, &handle_struct ## .func)
#define LoadFunctionOrdinal2(handle_struct, ordinal) LoadFunctionTemplate(handle_struct ## .dll, (const char*)ordinal, &handle_struct ## .___XXX___ ## ordinal)
