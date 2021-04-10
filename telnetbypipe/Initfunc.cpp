#include"Initfunc.h"
BOOL CreateTelnet(const char* path, HANDLE PipeInForTelnet, HANDLE PipeOutForTelnet, \
	PROCESS_INFORMATION* Proc_info) {
	STARTUPINFOA Init_info;
	memset(&Init_info, 0, sizeof(Init_info));
	Init_info.cb = sizeof(Init_info);
	Init_info.dwFlags = STARTF_USESTDHANDLES;
	Init_info.hStdInput = PipeInForTelnet;
	Init_info.hStdOutput = PipeOutForTelnet;
	Init_info.hStdError = PipeOutForTelnet;
	if (!CreateProcessA(NULL, (LPSTR)path, NULL, NULL, true, CREATE_SUSPENDED|CREATE_NEW_CONSOLE, \
		NULL, NULL, &Init_info, Proc_info))
		return false;
	return true;
}
BOOL CreateTwoPipe(HANDLE* PipeForTelnetIn, HANDLE* PipeForUserIn, HANDLE* PipeForUserOut, HANDLE* PipeForTelnetOut) {
	SECURITY_ATTRIBUTES Pipe_sa;
	Pipe_sa.nLength = sizeof(Pipe_sa);
	Pipe_sa.lpSecurityDescriptor = NULL;
	Pipe_sa.bInheritHandle = true;
	if (!CreatePipe(PipeForTelnetIn, PipeForUserIn, &Pipe_sa, 1024))
		return false;
	if (!CreatePipe(PipeForUserOut, PipeForTelnetOut, &Pipe_sa, 1024))
		return false;
	return true;
}