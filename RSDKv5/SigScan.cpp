#include "windows.h"
#include "SigScan.h"
#include <Psapi.h>

#ifndef _countof
#define _countof(a) (sizeof(a)/sizeof(*(a)))
#endif

bool SigValid = true;
const char* InvalidSig = nullptr;

MODULEINFO moduleInfo;

const MODULEINFO& getModuleInfo()
{
    if (moduleInfo.SizeOfImage)
        return moduleInfo;

    ZeroMemory(&moduleInfo, sizeof(MODULEINFO));
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &moduleInfo, sizeof(MODULEINFO));

    return moduleInfo;
}

void* sigScan(const char* signature, const char* mask)
{
    const MODULEINFO& moduleInfo = getModuleInfo();
    const size_t length = strlen(mask);

    for (size_t i = 0; i < moduleInfo.SizeOfImage; i++)
    {
        char* memory = (char*)moduleInfo.lpBaseOfDll + i;

        size_t j;
        for (j = 0; j < length; j++)
            if (mask[j] != '?' && signature[j] != memory[j])
                break;

        if (j == length)
            return memory;
    }

    return nullptr;
}

#define SIG_SCAN(x, ...) \
    void* x##Addr; \
    void* x() \
    { \
        static const char* x##Data[] = { __VA_ARGS__ }; \
        if (!x##Addr) \
        { \
            for (int i = 0; i < _countof(x##Data); i += 2) \
            { \
                x##Addr = sigScan(x##Data[i], x##Data[i + 1]); \
                if (x##Addr) \
                    return x##Addr; \
            } \
            SigValid = false; \
            InvalidSig = #x; \
        } \
        return x##Addr; \
    }

// Scans
SIG_SCAN(SigusePathTracer,     "\x00\x00\x00\x00\x00\x02\x01\x01\x02\x01\x03\x03\x02\x01\x02\x00\x01\x03\x02\x02\x02\x01\x01\x03\x01\x01\x02\x02\x02\x03\x03\x03", "?xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
SIG_SCAN(SigLinkGameLogic,     "\x48\x83\xEC\x78\x4C\x8B\x09\x4C\x8B\x05\x00\x00\x00\x00\x8B\x15\x00\x00\x00\x00\x4C\x89\x0D\x00\x00\x00\x00\x48\x8B\x41\x68\x48\x89\x05\x00\x00\x00\x00\x48\x8B", "xxxxxxxxxx????xx????xxx????xxxxxxx????xx");
SIG_SCAN(SigNukeSystemReq,     "\x74\x19\x45\x33\xC9\x4C\x8D\x05\x00\x00\x00\x00\x48\x8D\x15\x00\x00\x00\x00\x33\xC9\xFF\x15\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8D\x8D\x00\x00\x00\x00\xE8", "xxxxxxxx????xxx????xxxx????x????xxx????x");
SIG_SCAN(SigEngine_LoadFile,   "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x48\x83\x79\x00\x00\x41\x8B\xF8\x48\x8B\xD9\x0F\x85\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x4C\x24", "?????????????xxx??xxxxxxxx????xx????xxxx");
SIG_SCAN(SigFindFloorPosition, "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x4C\x8B\x3D\x00\x00\x00\x00\xB8\x00\x00\x00\x00\x44\x0F\xBF\x61\x00\x48\x8B\xF9", "xxxx?xxxx?xxxxxxxxxxxxx????x????xxxx?xxx");
SIG_SCAN(SigStateMachineRun,   "\x8B\x41\x08\x4C\x8B\xC1\x85\xC0\x74\x05\xFF\xC8\x89\x41\x08\x48\x83\x39\x00\x74\x11\x48\x8B\xCA\x48\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\x49\xFF\x20\xC3"    , "xxxxxxxxxxxxxxxxxxxxxxxxxxx????????xxxx" );