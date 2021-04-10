#pragma once
#include<windows.h>
#include<stdio.h>
BOOL CreateTelnet(const char*, HANDLE, HANDLE, PROCESS_INFORMATION*);
BOOL CreateTwoPipe(HANDLE*, HANDLE*, HANDLE*, HANDLE*);