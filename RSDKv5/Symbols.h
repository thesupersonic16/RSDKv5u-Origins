#pragma once
#include "RSDK/Core/RetroEngine.hpp"
#include <vector>

namespace Symbols {
    struct SignatureScan {
        char* name;
        int8 patternLength;
        char* pattern;
        char* mask;
        int32 offset;
        int8 hintCount;
        void** hints;
    };

    struct SignatureScanResult {
        char* name;
        void* address;
    };

    extern std::vector<SignatureScanResult> scanResults;

    void loadAndProcess(const char* filePath);
}