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
    PVOID t1 = (PVOID)0x02B7F9CE3D34;
    isPresent(t1, *rwmemVector);
    PVOID t2 = (PVOID)0x02B7FB2F8BAC;
    isPresent(t2, *rwmemVector);
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

