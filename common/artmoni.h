#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>

struct rwMem {
    PVOID base;
    SIZE_T size;
};