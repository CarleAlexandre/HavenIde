#include <queue>
#include <cassert>

#ifdef _WIN32
# include <windows.h>
# include <psapi.h>
# include <stdio.h>
# include <iostream>

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
# elif defined (__linux__)

# include <sys/stat.h>
# include <unistd.h>
# include <sys/wait.h>
# include <raylib.h>
# include <string>

int splitn(char *data, char *const arg[], int count, int sep) {
	int k = 0;
	std::string str;

	for (int i = 0;data[i] && k < count; i++) {
		if (data[i] == sep) {
			str.copy(arg[k], str.size());
			str.clear();
			k++;
		}
		str += data[i];
	}
	return (0);
}

bool execCmd(char *cmd, std::queue<char> &out, int max_size) {
	int pipes[2] = {};

	assert(pipe(pipes));
	int ret = -1;
	int pid = fork();

	if (pid == 0) {
		char *const arg[10] = {};
		splitn(cmd, arg, 10, ' ');
		assert(arg[0]);
		execv(arg[0], arg);
	}
	if (pid > 0) {
		waitpid(pid, &ret, 0);
	}
	return (ret);
}

#endif
