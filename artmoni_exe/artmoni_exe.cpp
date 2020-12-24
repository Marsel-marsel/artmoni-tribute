#include "../common/artmoni.h"
#include <iostream>
using namespace std;
extern "C" __declspec(dllexport) BOOL getRWregions(const HANDLE pHandle, vector<rwMem>*rwmemVector);
extern "C" __declspec(dllexport) BOOL scanRWmemForValue(int value, const HANDLE pHandle, vector<rwMem>*rwmemVector, vector<PVOID>*result);
extern "C" __declspec(dllexport) BOOL filterRWPointers(const HANDLE pHandle, int newValue, vector<PVOID>*valuePointers);
extern "C" __declspec(dllexport) BOOL writeRWPointers(const HANDLE pHandle, int newValue, vector<PVOID>*valuePointers);


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
	
	vector<rwMem> rwmem;
	if (!getRWregions(hProcess, &rwmem)) {
		CloseHandle(hProcess);
		return 1;
	}

	vector<PVOID> valuePointers;
	wprintf(TEXT("Insert target value:\n"));
	int targetValue;
	cin >> targetValue;
	
	scanRWmemForValue(targetValue, hProcess, &rwmem, &valuePointers);
	if (valuePointers.size() == 0) {
		wprintf(TEXT("Can't find the value of %d in memory layout of process %d\n"), targetValue, dwProcessId);
		CloseHandle(hProcess);
		return 1;
	}
	wprintf(TEXT("Found %zu pointers to the value %d\n"), valuePointers.size(), targetValue);

	int newValue;
	cin.width(21);
	while(TRUE){
		wprintf(TEXT("Insert new value (enter to escape): \n"));
		char userInput[21] = { 0 }; //cin >> newValue;
		cin >> userInput;
		try {
			newValue = std::stoi(userInput);
			filterRWPointers(hProcess, newValue, &valuePointers);
			wprintf(TEXT("Found %zu pointers to the value %d\n"), valuePointers.size(), newValue);
		}
		catch (std::exception) {
			break;
		}
	}
	wprintf(TEXT("Insert rewrite value : \n"));
	cin >> newValue;
	if (!writeRWPointers(hProcess, newValue, &valuePointers)) {
		wprintf(TEXT("Can't rewrite memory\n"));
		CloseHandle(hProcess);
		return 1;
	}
	CloseHandle(hProcess);
	return 0;
}

