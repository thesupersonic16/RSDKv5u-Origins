#pragma once
#include <string>
#include <vector>
#include <memory>

namespace Symbols {
    struct SignatureScan {
        std::string* name;
        int patternLength;
        char* pattern;
        char* mask;
        void* hint;
        void* address;
    };

    extern std::vector<SignatureScan> symbols;

    void loadScanFile(const char* filePath);
    void parseScanFile();
    void scanAll();
}