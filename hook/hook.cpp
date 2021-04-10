#include<malloc.h>
#include<memory.h>
#include<stdio.h>
#include"hookdll.h"
#define READ_BUF_NUM 1024
#define WAIT_PROC_TIMEMS 1001


BOOL hooked = false;
HANDLE OldStdInputfile;
HANDLE OldStdOutputfile;
HANDLE NewStdInputfile;
HANDLE NewStdOutputfile;

DETOUR_TRAMPOLINE(BOOL WINAPI Trampoline_WriteConsoleA(HANDLE hConsoleOutput,
	CONST VOID* lpBuffer,
	DWORD nNumberOfCharsToWrite,
	LPDWORD lpNumberOfCharsWritten,
	LPVOID lpReserved),
	WriteConsoleA);
static BOOL WINAPI Detour_WriteConsoleA(HANDLE hConsoleOutput,
	CONST VOID* lpBuffer,
	DWORD nNumberOfCharsToWrite,
	LPDWORD lpNumberOfCharsWritten,
	LPVOID lpReserved)
{
	DWORD num;
	WriteFile(OldStdOutputfile, lpBuffer, nNumberOfCharsToWrite, &num, NULL);
	//Trampoline_WriteConsoleA(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved);
	return true;
}

DETOUR_TRAMPOLINE(BOOL WINAPI Trampoline_WriteConsoleW(HANDLE hConsoleOutput,
	CONST VOID* lpBuffer,	
	DWORD nNumberOfCharsToWrite,	
	LPDWORD lpNumberOfCharsWritten,	
	LPVOID lpReserved),
	WriteConsoleW);
static BOOL WINAPI Detour_WriteConsoleW(HANDLE hConsoleOutput,
	CONST VOID* lpBuffer,	
	DWORD nNumberOfCharsToWrite,
	LPDWORD lpNumberOfCharsWritten,
	LPVOID lpReserved)
{
	DWORD LenBuf=WideCharToMultiByte(CP_ACP,
		0,
		(LPCWCH)lpBuffer,
		nNumberOfCharsToWrite,
		NULL,
		0,
		NULL,NULL
		);
	char* Buf = (char*)malloc(LenBuf+1);
	WideCharToMultiByte(CP_ACP,
		0,
		(LPCWCH)lpBuffer,
		nNumberOfCharsToWrite,
		Buf,
		LenBuf,
		NULL, NULL
	);
	Buf[LenBuf] = '\0';
	DWORD writeno;
	if (!WriteFile(OldStdOutputfile, Buf, LenBuf, &writeno, NULL))
		return false;
	*lpNumberOfCharsWritten = writeno;
	free(Buf);
	//Trampoline_WriteConsoleW(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved);
	return true;
}

DWORD WINAPI ThreadForWriteInputConsole(PVOID lpArg) {
	DWORD rnum,wnum;
	char buf[READ_BUF_NUM];
	INPUT_RECORD KeyMsg;
	memset(&KeyMsg, 0, sizeof(KeyMsg));
	KeyMsg.EventType = KEY_EVENT;
	KeyMsg.Event.KeyEvent.dwControlKeyState = 0;
	KeyMsg.Event.KeyEvent.wRepeatCount = 1;
	while (true)
	{
		if (!ReadFile(OldStdInputfile, buf, READ_BUF_NUM, &rnum, NULL))
			continue;
		for (int i = 0; i < rnum; i++)
		{
			KeyMsg.Event.KeyEvent.bKeyDown = TRUE;
			KeyMsg.Event.KeyEvent.uChar.AsciiChar = buf[i];

			if (!WriteConsoleInputA(NewStdInputfile, &KeyMsg, 1, &wnum))
				return -1;
			KeyMsg.Event.KeyEvent.bKeyDown = FALSE;

			if (!WriteConsoleInputA(NewStdInputfile, &KeyMsg, 1, &wnum))
				return -1;
		}
	}
	return 0;
}

BOOL HookInit() {
	OldStdInputfile = GetStdHandle(STD_INPUT_HANDLE);
	OldStdOutputfile = GetStdHandle(STD_OUTPUT_HANDLE);
	NewStdInputfile = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, \
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	NewStdOutputfile = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetStdHandle(STD_INPUT_HANDLE,NewStdInputfile);
	SetStdHandle(STD_OUTPUT_HANDLE, NewStdOutputfile);
	if (!CreateThread(NULL, 0, ThreadForWriteInputConsole, NULL, 0, NULL))
		return false;
	return true;
}


void Hook() {
	if (hooked)
		return;
	hooked = true;
	HookInit();
	DetourFunctionWithTrampoline((PBYTE)Trampoline_WriteConsoleA, (PBYTE)Detour_WriteConsoleA);
	DetourFunctionWithTrampoline((PBYTE)Trampoline_WriteConsoleW, (PBYTE)Detour_WriteConsoleW);
	return;
}