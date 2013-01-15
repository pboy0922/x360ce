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
#include "Utilities\Log.h"
#include <Shlwapi.h>

HANDLE hConsole = INVALID_HANDLE_VALUE;
BOOL writelog = FALSE;
BOOL enableconsole = FALSE;
//LPWSTR lpLogFileName = NULL;

wstring logfilename;

static LPCWSTR LogTypeNames[] =
{
    L"[Core]    ",
    L"[XInput]  ",
    L"[DInput]  ",
    L"[IHook]   ",
    L"[HookDI]  ",
    L"[HookWMI] ",
    L"[HookWT]  ",
};

void WriteStamp()
{
    if(enableconsole)
    {
        wprintf(L"%s",L"TIME           THREAD  TYPE      DATA");
        wprintf(L"\n");
		fflush(stdout);
    }

    if (writelog)
    {
		if(logfilename.empty()) return;
        FILE * fp;
        _wfopen_s(&fp, logfilename.c_str(), L"a");

        //fp is null, file is not open.
        if (fp==NULL)
            return;

        fwprintf(fp, L"%s",L"TIME           THREAD  TYPE      DATA");
        fwprintf(fp, L"\n");
        fclose(fp);
    }
}

void ConsoleEnable(BOOL console)
{
    enableconsole = console;
}

void Console()
{
    if(enableconsole)
    {

    }
}

void LogEnable(BOOL log)
{
    writelog = log;
}

void LogCleanup()
{
    if(enableconsole)FreeConsole();
}

BOOL CreateLog()
{
    BOOL bRet = FALSE;

    if (writelog)
    {
		wstring name = L"x360ce";

        SYSTEMTIME systime;
        GetLocalTime(&systime);

		wostringstream s;
		s << setfill(L'0') << name << L"\\" << name << L'_' 
			<< setw(2) << systime.wYear 
			<< setw(2) << systime.wMonth
			<< setw(2) << systime.wDay << L'-' 
			<< setw(2) << systime.wHour 
			<< setw(2) << systime.wMinute 
			<< setw(2) << systime.wSecond << L".log";

		logfilename = s.str();

        if(!PathIsDirectory(name.c_str())) CreateDirectory(name.c_str(), NULL);
        bRet = TRUE;
    }

    return bRet;
}

BOOL WriteLog(LogType logType, LPWSTR str,...)
{
    SYSTEMTIME systime;
    GetLocalTime(&systime);

    BOOL ret = FALSE;

    if(enableconsole)
    {
		if(hConsole == INVALID_HANDLE_VALUE)
		{
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			AllocConsole();
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTitle(L"x360ce");
			GetConsoleScreenBufferInfo(hConsole,&csbi);

			//SetConsoleTextAttribute(hConsole,csbi.wAttributes| FOREGROUND_INTENSITY);
		}

		WCHAR buf[MAX_PATH];
		DWORD dwChars = NULL;

        swprintf_s(buf,L"%02u:%02u:%02u.%03u %08u %s",\
                systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds,GetCurrentThreadId(),LogTypeNames[logType]);

		WriteConsole(hConsole, buf, wcslen(buf),&dwChars,NULL);


        va_list arglist;
        va_start(arglist,str);
        vswprintf_s(buf,str,arglist);

		WriteConsole(hConsole, buf, wcslen(buf),&dwChars,NULL);

        va_end(arglist);
        //wprintf(L" \n");
		WriteConsole(hConsole, L" \n", wcslen(L" \n"),&dwChars,NULL);

        ret = TRUE;
    }

    if (writelog)
    {
		if(logfilename.empty()) return FALSE;
        FILE * fp;
        _wfopen_s(&fp, logfilename.c_str(), L"a");

        //fp is null, file is not open.
        if (fp==NULL)
            return 0;

        fwprintf(fp, L"%02u:%02u:%02u.%03u %08u %s",\
                 systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds,GetCurrentThreadId(),LogTypeNames[logType]);
        va_list arglist;
        va_start(arglist,str);
        vfwprintf(fp,str,arglist);
        va_end(arglist);
        fwprintf(fp, L" \n");
        fclose(fp);
        ret = TRUE;
    }

    return ret;
}
