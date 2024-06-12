# include <windows.h>
# include <stdio.h>
# include <psapi.h>
# include <queue>
# include <iostream>
# include <cassert>

//int checkProcess(DWORD pid) {
//	unsigned int ret_value = 0;
//	HANDLE hprocess;

//	printf("test");

//	//run task and get process id

//	hprocess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);

//	if (!hprocess) {
//		return (-1);
//	}

//	//ReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead);

//	//K32GetProcessMemoryInfo(HANDLE Process, PPROCESS_MEMORY_COUNTERS ppsmemCounters, DWORD cb);

//	//GetProcessorSystemCycleTime(USHORT Group, PSYSTEM_PROCESSOR_CYCLE_TIME_INFORMATION Buffer, PDWORD ReturnedLength);


//	return (ret_value);
//}

int createChildP(char *cmd, HANDLE &wrPipe, PROCESS_INFORMATION &pInfo) {
	STARTUPINFO  sInfo;

	ZeroMemory(&pInfo,  sizeof(PROCESS_INFORMATION));
	ZeroMemory(&sInfo,  sizeof(STARTUPINFO));
	sInfo.cb = sizeof(STARTUPINFO);
	sInfo.hStdOutput = wrPipe;
	sInfo.hStdError = wrPipe;
	sInfo.dwFlags |= STARTF_USESTDHANDLES;

	bool create_proc = CreateProcessA(
	NULL, 
	cmd, 
	NULL, 
	NULL, 
	true, 
	0, 
	NULL, 
	NULL, 
	&sInfo, 
	&pInfo);

	if (!create_proc) {
        std::cerr << "CreateProcess failed" << std::endl;
        return (-1);
    }
	CloseHandle(wrPipe);
	return (1);
}

#ifdef _WIN32
unsigned long execCmd(char *cmd, std::queue<char> &out, int max_size) {
	HANDLE rdPipe = 0x00, wrPipe = 0x00;
	SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

	assert(CreatePipe(&rdPipe, &wrPipe, &saAttr, 4096));
	assert(SetHandleInformation(rdPipe, HANDLE_FLAG_INHERIT, 0));

	PROCESS_INFORMATION pInfo;
	if (!createChildP(cmd, wrPipe, pInfo)){
		CloseHandle(rdPipe);
		CloseHandle(wrPipe);
		return (-1);
	}

	bool bRead = false;
	DWORD dwRead;
	CHAR buff[4096];

	for (;;) {
		bRead = ReadFile(rdPipe, buff, sizeof(buff) - 1, &dwRead, NULL);
		if (!bRead || dwRead == 0) break;

		buff[dwRead] = '\0';
		for (int i = 0; buff[i];i++) {
			out.push(buff[i]);
		}
	}

	WaitForSingleObject(pInfo.hProcess, INFINITE);

	CloseHandle(&pInfo.hProcess);
	CloseHandle(&pInfo.hThread);
	CloseHandle(rdPipe);

	LPDWORD lpExitCode;
	GetExitCodeProcess(&pInfo, lpExitCode);
	return (*lpExitCode);
}
#elif 

unsigned long execCmd(char *cmd, std::queue<char> *out, int max_size) {
	return (-1);
}

#endif
