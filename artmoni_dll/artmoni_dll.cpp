#include "../common/artmoni.h"
using namespace std;

BOOL isPresent(PVOID address, vector<rwMemBlock> rwmem) {
    for (int i = 0; i < rwmem.size(); i++) {
        PVOID startAddr = rwmem[i].base;
        PVOID endAddr = (PVOID)((PBYTE)startAddr + rwmem[i].size);
        if (startAddr < address && address < endAddr) {
            wprintf(TEXT("%p is present\n"), address);
            return TRUE;
        }
    }
    return FALSE;
}
extern "C" __declspec(dllexport) BOOL writeRWPointerUintValue(const HANDLE pHandle, int newValue, vector<PVOID>*valuePointers) {
    int* ptrNewValue = (int*)malloc(sizeof(int));
    if (ptrNewValue == NULL) {
        wprintf(TEXT("Malloc failed\n"));
        return FALSE;
    }
    //*ptrNewValue = newValue;
    for (int i = 0; i < valuePointers->size(); i++) {
        SIZE_T bytesWritten = 0;
        PVOID targetAddress = (*valuePointers)[i];
        WriteProcessMemory(pHandle, targetAddress, &newValue, sizeof(int), &bytesWritten);
        if (bytesWritten != sizeof(int)) {
            wprintf(TEXT("Can't write integer at address %p\n"), targetAddress);
            return FALSE;
        }
    }
    //free(ptrNewValue);
    return TRUE;
}


extern "C" __declspec(dllexport) BOOL filterRWpointersByUint(const HANDLE pHandle, int newValue, vector<PVOID>*valuePointers) {
    vector<PVOID>::iterator it = valuePointers->begin();
    for (; it != valuePointers->end(); ) {
        PVOID curAddr = (PVOID) * it;
        int* buf = (int*) malloc(sizeof(int));
        if (buf == NULL) {
            wprintf(TEXT("Malloc failed %p (size: %zu bytes)\n"), curAddr, sizeof(int));
            return FALSE;
        }
        SIZE_T readBytes = 0;
        ReadProcessMemory(pHandle, curAddr, buf, sizeof(int), &readBytes);
        if (readBytes != sizeof(int)) {
            wprintf(TEXT("Can't read integer at address %p\n"), curAddr);
            return FALSE;
        }
        if (*buf != newValue) {
            it = valuePointers->erase(it);
        }
        else {
            ++it;
        }
        free(buf);
    }
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL getRWblocksOfProcess(const HANDLE pHandle, vector<rwMemBlock>* rwmemVector) {
    PVOID pvAddress = NULL;
    MEMORY_BASIC_INFORMATION mbi;
    while (VirtualQueryEx(pHandle, pvAddress, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.AllocationProtect & PAGE_READWRITE
            && mbi.State & MEM_COMMIT
            //&& mbi.Type & MEM_PRIVATE
            ) {
            rwMemBlock newBlock;
            newBlock.base = mbi.BaseAddress;
            newBlock.size = mbi.RegionSize;
            rwmemVector->push_back(newBlock);  // from xtcrackme - push_back makes a copy of element, so we don't care that m is local var
        }
        pvAddress = (PVOID)((PBYTE)pvAddress + mbi.RegionSize);
    }
    wprintf(TEXT("Found %zu memory RW blocks\n"), rwmemVector->size());
    if (rwmemVector->size() != 0) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

extern "C" __declspec(dllexport) BOOL scanRWblocksForUintValue(int value, const HANDLE pHandle, vector<rwMemBlock>*rwmemVector, vector<PVOID>* result ) {
    vector<rwMemBlock> newVector;

    for (int i = 0; i < rwmemVector->size(); i++) {
        PVOID pAddr = (*rwmemVector)[i].base;
        SIZE_T size = (*rwmemVector)[i].size;
        PVOID buf = malloc(size);
        if (buf == NULL) {
            wprintf(TEXT("Malloc failed %p (size: %zu bytes)\n"), pAddr, size);
            return FALSE;
        }
        SIZE_T readBytes = NULL;
        ReadProcessMemory(pHandle, pAddr, buf, size, &readBytes);
        if (readBytes != size) {
            continue; // Can't read protected page
        }
        UINT offset = 0;
        PVOID rwBlockStart = buf;
        PVOID rwBlockEnd = (PVOID) ((PBYTE)buf + size);
        PVOID curAddr = (PVOID)((PBYTE)rwBlockStart + offset);
        while (curAddr < rwBlockEnd) {
            int curValue = *(int*)curAddr;
            if (curValue == value) {
                PVOID addrInProcessLayout = (PVOID)((PBYTE)pAddr + offset);
                result->push_back(addrInProcessLayout);
            }
            offset += sizeof(int);
            curAddr = (PVOID)((PBYTE)rwBlockStart + offset);
        }
        free(buf);
    }
    return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

