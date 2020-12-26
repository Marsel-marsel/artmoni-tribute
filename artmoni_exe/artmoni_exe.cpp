#include "../common/artmoni.h"
#include <iostream>
using namespace std;
extern "C" __declspec(dllexport) BOOL getRWblocksOfProcess(const HANDLE pHandle, vector<rwMemBlock>*rwmemVector);
extern "C" __declspec(dllexport) BOOL scanRWblocksForUintValue(int value, const HANDLE pHandle, vector<rwMemBlock>*rwmemVector, vector<PVOID>*result);
extern "C" __declspec(dllexport) BOOL filterRWpointersByUint(const HANDLE pHandle, int newValue, vector<PVOID>*valuePointers);
extern "C" __declspec(dllexport) BOOL writeRWPointerUintValue(const HANDLE pHandle, int newValue, vector<PVOID>*valuePointers);


int _tmain(int argc, TCHAR* argv[]) {
	if (argc != 2) {
		wprintf(TEXT("Usage: artmoney-tribute <PID>\n"));
		return 1;
	}
	DWORD dwProcessId;
	try {
		dwProcessId = std::stoi(argv[1]);
		wprintf(TEXT("Process ID %ld\n"), dwProcessId);
	}
	catch (std::exception) {
		wprintf(TEXT("Exit. Is <PID> numeric?\n"));
		return 1;
	}
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
	if (hProcess == NULL) {
		wprintf(TEXT("Can't open process %ld\n"), dwProcessId);
		return 1;
	}
	
	vector<rwMemBlock> rwmem;
	if (!getRWblocksOfProcess(hProcess, &rwmem)) {
		CloseHandle(hProcess);
		return 1;
	}

	vector<PVOID> valuePointers;
	wprintf(TEXT("Insert target value: "));
	int targetValue;
	cin >> targetValue;
	
	scanRWblocksForUintValue(targetValue, hProcess, &rwmem, &valuePointers);
	if (valuePointers.size() == 0) {
		wprintf(TEXT("Can't find the value of %d in memory layout of process %d\n"), targetValue, dwProcessId);
		CloseHandle(hProcess);
		return 1;
	}
	wprintf(TEXT("Found %zu pointers to the value %d\n"), valuePointers.size(), targetValue);

	int newValue;
	cin.width(21);
	while(TRUE){
		wprintf(TEXT("Make changes in the process and insert new value (n to next step): "));
		char userInput[21] = { 0 };
		cin >> userInput;
		try {
			newValue = std::stoi(userInput);
			filterRWpointersByUint(hProcess, newValue, &valuePointers);
			wprintf(TEXT("Found %zu pointers to the value %d\n"), valuePointers.size(), newValue);
		}
		catch (std::exception) {
			break;
		}
	}

	wprintf(TEXT("Rewrite uint pointers "));
	for (int i = 0; i < valuePointers.size(); i++) {
		wprintf(TEXT("0x%p "), valuePointers[i]);
	}
	wprintf(TEXT("with value: "));

	cin >> newValue;
	if (!writeRWPointerUintValue(hProcess, newValue, &valuePointers)) {
		wprintf(TEXT("Can't rewrite memory\n"));
		CloseHandle(hProcess);
		return 1;
	}
	wprintf(TEXT("Done. Exit"));
	CloseHandle(hProcess);
	return 0;
}

