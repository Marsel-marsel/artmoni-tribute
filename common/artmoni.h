#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>

struct rwMemBlock {
    PVOID base;
    SIZE_T size;
};