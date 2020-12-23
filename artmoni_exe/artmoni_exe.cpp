#include "../common/artmoni.h"
using namespace std;
extern "C" __declspec(dllexport) BOOL getRWregions(const HANDLE pHandle, vector<rwMem>*rwmemVector);
extern "C" __declspec(dllexport) BOOL scanRWmemForValue(int value, const HANDLE pHandle, vector<rwMem>*rwmemVector, vector<PVOID>*result);
extern 

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
	scanRWmemForValue(475, hProcess, &rwmem, &valuePointers);
	CloseHandle(hProcess);
	return 0;
}

