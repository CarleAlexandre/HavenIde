# include <windows.h>
# include <stdio.h>
# include <psapi.h>
# include <queue>
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

#ifdef _WIN32
unsigned long execCmd(char *cmd, std::queue<char> *out, int max_size) {
	STARTUPINFO  sInfo;
	PROCESS_INFORMATION pInfo;
	LPDWORD lpExitCode;
	char buff;

	ZeroMemory(&pInfo,  sizeof(PROCESS_INFORMATION));
	ZeroMemory(&sInfo,  sizeof(STARTUPINFO));

	HANDLE rdPipe, wrPipe;
	if (!CreatePipe(&rdPipe, &wrPipe, NULL, 4096)) {
		abort();
	}

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
	NORMAL_PRIORITY_CLASS | CREATE_PROTECTED_PROCESS | CREATE_NO_WINDOW, 
	NULL, 
	NULL, 
	&sInfo, 
	&pInfo);

	assert(create_proc);
	CloseHandle(&sInfo);
	CloseHandle(&pInfo);
	CloseHandle(wrPipe);

	bool bRead;
	DWORD dwRead;

	int x = 0;
	for (;;) {
		bRead = ReadFile(rdPipe, &buff, 1, &dwRead, NULL);
		if (!bRead || !dwRead) break;
		if (x > max_size) {
			out->pop();
		} else {
			x++;
		}
		out->push(buff);
	}

	GetExitCodeProcess(&pInfo, lpExitCode);
	return (*lpExitCode);
}
#elif 

unsigned long execCmd(char *cmd, std::queue<char> *out, int max_size) {
	return (-1);
}

#endif
