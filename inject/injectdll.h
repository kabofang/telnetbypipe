#pragma once
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include"../detours/detours.h"
#ifdef INJECT_EXPORTS
#define DLL_API _declspec(dllexport)
#else
#define DLL_API _declspec(dllimport)
#endif

extern "C" BOOL DLL_API Inject(HANDLE, LPCSTR);