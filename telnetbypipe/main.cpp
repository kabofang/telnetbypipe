#include"Initfunc.h"
#include "../detours/detours.h"

#define CMD_BYTE_MAX 128
#define RET_BYTE_MAX 512
#define TIMEMS_FOR_PROC 200

int main(int argc, char* argv[]) {
	//********************       Prepare     **************************	
	const char* InjectDllName = "inject.dll";
	const char* InjectFuncName = "Inject";
	const char* HookDllName = "hook.dll";
	PROCESS_INFORMATION TelnetInfo;
	HMODULE DllInject;
	HANDLE PipeForTelnetIn, PipeForTelnetOut, PipeForUserIn, PipeForUserOut;
	typedef BOOL(*DllInjectFunc)(HANDLE, LPCSTR);

	DllInject = LoadLibraryA("inject.dll");
	if (!DllInject) {
		fprintf(stderr, "Load %s fail!\n", InjectDllName);
		return -1;
	}
	DllInjectFunc Inject = (DllInjectFunc)GetProcAddress(DllInject, InjectFuncName);
	if (!Inject) {
		fprintf(stderr, "Can't get export in %s!\n", InjectDllName);
		return -1;
	}

	//*********************        Process Initialize       **************************
	if (!CreateTwoPipe(&PipeForTelnetIn, &PipeForUserIn, &PipeForUserOut, &PipeForTelnetOut)) {
		fprintf(stderr, "Create pipe fail!\n");
		return -1;
	}
	if (!CreateTelnet(".\\telnet", PipeForTelnetIn, PipeForTelnetOut, &TelnetInfo)) {
		fprintf(stderr, "Create telnet process fail!\n");
		return -1;
	}
	if (!Inject(TelnetInfo.hProcess, HookDllName)) {
		fprintf(stderr, "Detours fail!\n");
		return -1;
	}
	ResumeThread(TelnetInfo.hThread);

	//***************     One Example     ************************
	DWORD Num;
	BOOL Login=false;
	char Cmd[CMD_BYTE_MAX];
	char Ret[RET_BYTE_MAX];

	memset(Cmd, 0, CMD_BYTE_MAX);
	memset(Ret, 0, RET_BYTE_MAX);
	Sleep(TIMEMS_FOR_PROC);
	ReadFile(PipeForUserOut, Ret, RET_BYTE_MAX, &Num, NULL);
	fputs(Ret, stdout);
	while (true) {
		memset(Cmd, 0, CMD_BYTE_MAX);
		memset(Ret, 0, RET_BYTE_MAX);
		fgets(Cmd, CMD_BYTE_MAX, stdin);
		if (!strcmp(Cmd, "exit\n"))
			break;
		Cmd[strlen(Cmd) - 1] = '\r';
		WriteFile(PipeForUserIn, Cmd, CMD_BYTE_MAX, &Num, NULL);
		Sleep(TIMEMS_FOR_PROC);
		if (Login)
			Sleep(TIMEMS_FOR_PROC * 5 * 20);
		Login = false;
		ReadFile(PipeForUserOut, Ret, RET_BYTE_MAX, &Num, NULL);
		fputs(Ret, stdout);
		if (Num > 7)
			if (!strcmp(&Ret[Num - 8], "n name: "))
				Login = true;
	}
	//**************************************************************
	TerminateProcess(TelnetInfo.hProcess, 0);

	return 0;
}