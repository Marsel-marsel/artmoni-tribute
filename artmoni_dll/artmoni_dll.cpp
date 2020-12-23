#include "../common/artmoni.h"
using namespace std;
//vector<rwMem> rwmem;

BOOL isPresent(PVOID address, vector<rwMem> rwmem) {
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


extern "C" __declspec(dllexport) BOOL getRWregions(const HANDLE pHandle, vector<rwMem>* rwmemVector) {
    PVOID pvAddress = NULL;
    MEMORY_BASIC_INFORMATION mbi;
    while (VirtualQueryEx(pHandle, pvAddress, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.AllocationProtect & PAGE_READWRITE
            && mbi.State & MEM_COMMIT
            //&& mbi.Type & MEM_PRIVATE
            ) {
            rwMem m;
            m.base = mbi.BaseAddress;
            m.size = mbi.RegionSize;
            rwmemVector->push_back(m);  // from xtcrackme - push_back makes a copy of element
            //wprintf(TEXT("%p, size: %u kB\n"), pvAddress, mbi.RegionSize / 1024);
        }
        pvAddress = (PVOID)((PBYTE)pvAddress + mbi.RegionSize);
    }
    wprintf(TEXT("Found %u memory RW regions\n"), rwmemVector->size());
    //PVOID t1 = (PVOID)0x02B7F9CE3D34;
    //isPresent(t1, *rwmemVector);
    //PVOID t2 = (PVOID)0x02B7FB2F8BAC;
    //isPresent(t2, *rwmemVector);
    if (rwmemVector->size() != 0) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void scanForIntValue(int value, PVOID memRegion, SIZE_T memRegionSize, vector<PVOID>* result) {
    SIZE_T valueSize = sizeof(int);
    PVOID ptr = memRegion;
    PVOID endAddr = (PVOID)((PBYTE)memRegion + memRegionSize);
    while (ptr < endAddr) {
        if (*(int*)ptr == value) {
            wprintf(TEXT("Address %p contains value %d\n"), ptr, value);
            result->push_back(ptr);
        }
        ptr = (PVOID)((PBYTE)ptr + valueSize);
    }
    return;
}

extern "C" __declspec(dllexport) BOOL scanRWmemForValue(int value, const HANDLE pHandle, vector<rwMem>*rwmemVector, vector<PVOID>* result ) {
    vector<rwMem> newVector;

    for (int i = 0; i < rwmemVector->size(); i++) {
        PVOID pAddr = (*rwmemVector)[i].base;
        SIZE_T size = (*rwmemVector)[i].size;
        PVOID buf = malloc(size);
        if (buf == NULL) {
            wprintf(TEXT("Malloc failed %p (size: %zu bytes)"), pAddr, size);
            return FALSE;
        }
        SIZE_T readBytes = NULL;
        ReadProcessMemory(pHandle, pAddr, buf, size, &readBytes);
        if (readBytes != size) {
            wprintf(TEXT("ReadProcessMemory error. Can't read memory block %p (size: %zu bytes)"), pAddr, size);
            continue;
        }
        UINT offset = 0;
        PVOID startAddr = buf;
        PVOID endAddr = (PVOID) ((PBYTE)buf + size);
        PVOID curAddr = (PVOID)((PBYTE)startAddr + offset);
        while (curAddr < endAddr) {
            int curValue = *(int*)curAddr;
            if (curValue == value) {
                PVOID addrInProcessLayout = (PVOID)((PBYTE)pAddr + offset);
                result->push_back(addrInProcessLayout);
            }
            offset += sizeof(int);
            curAddr = (PVOID)((PBYTE)startAddr + offset);
        }
        //scanForIntValue(value, buf, size, result);
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

