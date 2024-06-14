#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <queue>
#include <iostream>
#include <cassert>

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
void cleanupP(HANDLE *rdPipe, HANDLE *wrPipe, PROCESS_INFORMATION *pInfo) {
    if (rdPipe != NULL) {
        CloseHandle(rdPipe);
        rdPipe = NULL;
    }

    if (wrPipe != NULL) {
        CloseHandle(wrPipe);
        wrPipe = NULL;
    }

    if (pInfo->hProcess != NULL) {
        CloseHandle(pInfo->hProcess);
        pInfo->hProcess = NULL;
    }

    if (pInfo->hThread != NULL) {
        CloseHandle(pInfo->hThread);
        pInfo->hThread = NULL;
    }
}

bool createChildP(char *cmd, HANDLE &wrPipe, PROCESS_INFORMATION &pInfo) {
	STARTUPINFO  sInfo;
	bool create_proc = false;

	ZeroMemory(&pInfo,  sizeof(PROCESS_INFORMATION));
	ZeroMemory(&sInfo,  sizeof(STARTUPINFO));
	sInfo.cb = sizeof(STARTUPINFO);
	sInfo.hStdOutput = wrPipe;
	sInfo.hStdError = wrPipe;
	sInfo.dwFlags |= STARTF_USESTDHANDLES;

	create_proc = CreateProcessA(
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
        return (false);
    }
	return (true);
}

bool execCmd(char *cmd, std::queue<char> &out, int max_size) {
	HANDLE rdPipe = 0x00, wrPipe = 0x00;
	SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

	assert(CreatePipe(&rdPipe, &wrPipe, &saAttr, 4096));
	assert(SetHandleInformation(rdPipe, HANDLE_FLAG_INHERIT, 0));

	PROCESS_INFORMATION pInfo;
	bool create_proc = false;
	create_proc = createChildP(cmd, wrPipe, pInfo);
	if (!create_proc) {
		cleanupP(&rdPipe, &wrPipe, &pInfo);
		return false;
	}

	CloseHandle(wrPipe);
	wrPipe = NULL;

	bool bRead = false;
	DWORD dwRead;
	CHAR buff[4096];

	for (;;) {
		bRead = ReadFile(rdPipe, buff, sizeof(buff) - 1, &dwRead, NULL);
		if (!bRead || dwRead == 0) break;

		buff[dwRead] = '\0';
		std::cout << buff << std::endl;
		for (int i = 0; buff[i];i++) {
			out.push(buff[i]);
		}
	}

	WaitForSingleObject(pInfo.hProcess, INFINITE);

	cleanupP(&rdPipe, &wrPipe, &pInfo);
	return true;
}
#elif 

unsigned long execCmd(char *cmd, std::queue<char> *out, int max_size) {
	return (-1);
}

#endif
