#pragma once

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <tuple>

typedef __int32 lookupType;
typedef std::vector<std::tuple<PVOID, SIZE_T>> memBlocks;