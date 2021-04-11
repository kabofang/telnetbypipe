#include"injectdll.h"
BOOL APIENTRY DllMain(HMODULE hModule,
						DWORD reason,
						PVOID reserved)
{
	switch (reason) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}