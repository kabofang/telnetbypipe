#pragma once
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include"../detours/detours.h"
void Hook();
BOOL HookInit();