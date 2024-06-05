# include <windows.h>
# include <stdio.h>
# include <psapi.h>

int checkProcess(DWORD pid) {
	unsigned int ret_value = 0;
	HANDLE hprocess;

	printf("test");

	//run task and get process id

	hprocess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);

	if (!hprocess) {
		return (-1);
	}

	//ReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead);

	//K32GetProcessMemoryInfo(HANDLE Process, PPROCESS_MEMORY_COUNTERS ppsmemCounters, DWORD cb);

	//GetProcessorSystemCycleTime(USHORT Group, PSYSTEM_PROCESSOR_CYCLE_TIME_INFORMATION Buffer, PDWORD ReturnedLength);


	return (ret_value);
}