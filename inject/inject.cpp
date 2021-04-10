#include"injectdll.h"
extern "C" BOOL DLL_API Inject(HANDLE Handle, LPCSTR DllName) {
	return DetourContinueProcessWithDllA(Handle, DllName);
}