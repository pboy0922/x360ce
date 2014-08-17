#include "stdafx.h"
#include "Logger.h"
#include <Softpub.h>

#include "InputHook.h"

static iHook *iHookThis = NULL;

typedef LONG (WINAPI* WinVerifyTrust_t)(HWND hwnd, GUID *pgActionID,LPVOID pWVTData);

WinVerifyTrust_t oWinVerifyTrust = NULL;

LONG WINAPI HookWinVerifyTrust(HWND hwnd, GUID *pgActionID,LPVOID pWVTData)
{
    if(!iHookThis->GetState(iHook::HOOK_WT)) return oWinVerifyTrust(hwnd,pgActionID,pWVTData);
    PrintLog("*WinVerifyTrust*");

    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(pgActionID);
    UNREFERENCED_PARAMETER(pWVTData);
    return 0;
}

void iHook::HookWT()
{
    iHookThis = this;
    if(MH_CreateHook(WinVerifyTrust,HookWinVerifyTrust,reinterpret_cast<void**>(&oWinVerifyTrust)) == MH_OK)
        PrintLog("Hooking WinVerifyTrust");
}