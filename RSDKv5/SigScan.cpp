#include "windows.h"
#include "SigScan.h"
#include <Psapi.h>
#define _countof(a) (sizeof(a)/sizeof(*(a)))

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
SIG_SCAN(SigusePathTracer,   "\x00\x00\x00\x00\x00\x02\x01\x01\x02\x01\x03\x03\x02\x01\x02\x00\x01\x03\x02\x02\x02\x01\x01\x03\x01\x01\x02\x02\x02\x03\x03\x03", "?xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
SIG_SCAN(SigLinkGameLogic,   "\x48\x83\xEC\x78\x4C\x8B\x09\x4C\x8B\x05\x00\x00\x00\x00\x8B\x15\x00\x00\x00\x00\x4C\x89\x0D\x00\x00\x00\x00\x48\x8B\x41\x68\x48\x89\x05\x00\x00\x00\x00\x48\x8B", "xxxxxxxxxx????xx????xxx????xxxxxxx????xx");
SIG_SCAN(SigNukeSystemReq,   "\x74\x19\x45\x33\xC9\x4C\x8D\x05\x00\x00\x00\x00\x48\x8D\x15\x00\x00\x00\x00\x33\xC9\xFF\x15\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x8D\x8D\x00\x00\x00\x00\xE8", "xxxxxxxx????xxx????xxxx????x????xxx????x");
SIG_SCAN(SigEngine_LoadFile, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x48\x83\x79\x00\x00\x41\x8B\xF8\x48\x8B\xD9\x0F\x85\x00\x00\x00\x00\x41\xB8\x00\x00\x00\x00\x48\x8D\x4C\x24", "?????????????xxx??xxxxxxxx????xx????xxxx");