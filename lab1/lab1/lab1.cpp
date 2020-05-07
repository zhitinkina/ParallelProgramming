#include "pch.h"
#include <windows.h>
#include <string>
#include <iostream>

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	int val1 = *(int*)lpParam;
	std::cout << val1 << std::endl;

	ExitThread(0);
}


int main(int argc)
{
	HANDLE* handles = new HANDLE[argc];

	int* saveThread = new int(argc);
	for (int i = 0; i < argc; i++) {
		saveThread[i] = i;
	}
	// создание двух потоков
	for (int i = 0; i < argc; i++) {
		handles[i] = CreateThread(NULL, i, &ThreadProc, &saveThread[i], CREATE_SUSPENDED, NULL);
	}

	for (int i = 0; i < argc; i++) {
		ResumeThread(handles[i]);
	}

	// ожидание окончания работы двух потоков
	WaitForMultipleObjects(argc, handles, true, INFINITE);
	return 0;
}
