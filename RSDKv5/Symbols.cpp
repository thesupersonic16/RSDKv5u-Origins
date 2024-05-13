#include "Symbols.h"
#include "RSDK/Core/RetroEngine.hpp"
#include <iostream>
#include <sstream>
#include "vector"
#include "string"
#include "windows.h"
#include "SigScan.h"
#include <Psapi.h>

namespace Symbols {

    void* scanFile = nullptr;
    std::vector<SignatureScan> symbols;

    // I did not write this
    std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(std::move(token));
        }
        return tokens;
    }

    void* hexToPtr(std::string& hex)
    {
        void* ptr = nullptr;
        std::stringstream ss(hex);
        ss >> std::hex >> ptr;

        if (ss.fail() || ss.peek() != EOF)
            RSDK::PrintLog(RSDK::PRINT_ERROR, "Invalid hex format: %s", hex.c_str());

        return ptr;
    }

    char hexToByte(std::string& hex, int offset)
    {
        if (offset < 0 || offset >= hex.length() - 1) {
            RSDK::PrintLog(RSDK::PRINT_ERROR, "Invalid hex offset: %s - %d", hex.c_str(), offset);
            return '\x00';
        }
    
        return (char)(std::stoi(hex.substr(offset, 2), nullptr, 16));
    }

    int countPattern(std::string& pattern)
    {
        int count = 0;
        for (int i = 0; i < pattern.length(); ++i)
            count += (pattern.c_str()[i] == '?') ? 2 : 1;
        return count / 2;
    }

    void loadScanFile(const char* filePath)
    {
        // Assume all strings are from the same file
        if (scanFile)
            free(scanFile);
        symbols.clear();

        FILE* handle;
        fopen_s(&handle, filePath, "r");
        if (handle)
        {
            fseek(handle, 0, SEEK_END);
            int fileSize = ftell(handle);
            fseek(handle, 0, SEEK_SET);
            scanFile = malloc(fileSize);
            if (scanFile)
                fread(scanFile, 1, fileSize, handle);
            fclose(handle);
        }
    }

    void parseScanFile()
    {
        symbols.clear();

        if (!scanFile)
            return;

        // Splitting the file contents into lines
        std::istringstream dataStream((char*)scanFile);
        std::string line;
        while (std::getline(dataStream, line))
        {
            if (line.length() == 0 || line[0] == '#')
                continue;

            std::vector<std::string> splits = split(line, '|');
            if (splits.size() < 3) {
                RSDK::PrintLog(RSDK::PRINT_ERROR, "Invalid signature scan line: %s", line.c_str());
                continue;
            }

            SignatureScan symbol;
            symbol.name    = new std::string(splits[0]);
            symbol.patternLength = countPattern(splits[1]);
            symbol.pattern = (char*)malloc(symbol.patternLength);
            symbol.mask    = (char*)malloc(symbol.patternLength + 1);
            symbol.hint    = hexToPtr(splits[2]);
            
            // Null byte
            symbol.mask[symbol.patternLength] = 0;
            
            // Convert pattern
            for (int pi = 0, mi = 0; pi < splits[1].length() && mi < symbol.patternLength; ++pi) {
                if (splits[1].at(pi) == '?') {
                    symbol.pattern[mi] = '\x00';
                    symbol.mask[mi] = '?';
                }
                else
                {
                    symbol.pattern[mi] = hexToByte(splits[1], pi);
                    symbol.mask[mi] = 'x';
                    ++pi; // skip byte
                }
                ++mi;
            }

            symbols.push_back(symbol);
        }
    }

    void scanAll()
    {
        for (auto& scan : symbols)
        {
            scan.address = sigScan(scan.pattern, scan.mask, scan.hint);
            if (!scan.address)
                RSDK::PrintLog(RSDK::PRINT_ERROR, "Failed to find signature \"%s\"!", scan.name->c_str());
        }
    }
}