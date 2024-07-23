#include "Symbols.h"
#include "vector"
#include "windows.h"
#include "SigScan.h"

#if RETRO_USE_MOD_LOADER
namespace Symbols {

    void* scanFileBuffer = nullptr;
    std::vector<SignatureScan> scans;
    std::vector<SignatureScanResult> scanResults;

    void loadAndProcess(const char* filePath)
    {
        // Reset
        if (scanFileBuffer)
            free(scanFileBuffer);
        scanFileBuffer = nullptr;
        scanResults.clear();

        // Load file into memory
        FILE* handle;
        fopen_s(&handle, filePath, "rb");
        if (handle) {
            fseek(handle, 0, SEEK_END);
            int fileSize = ftell(handle);
            fseek(handle, 0, SEEK_SET);
            scanFileBuffer = malloc(fileSize);
            if (scanFileBuffer)
                fread(scanFileBuffer, 1, fileSize, handle);
            fclose(handle);
        }

        // Parse file
        if (!scanFileBuffer)
            return;
        int8* pointer = (int8*)scanFileBuffer;
        
        int8 scanCount = *pointer++;

        for (int i = 0; i < scanCount; ++i) {
            SignatureScan scan;
            pointer++; // Skip version
            int8 nameLength = *pointer++;
            scan.name = (char*)pointer;
            pointer += nameLength;
            scan.patternLength = *pointer++;
            scan.pattern = (char*)pointer;
            pointer += scan.patternLength;
            scan.mask = (char*)pointer;
            pointer += scan.patternLength;
            scan.offset = *(int32*)pointer;
            pointer += sizeof(int32);
            pointer++; // Skip type
            scan.hintCount = *pointer++;
            scan.hints = (void**)pointer;
            pointer += sizeof(void*) * scan.hintCount;
            scans.push_back(scan);

            // Workaround to convert strings to null-terminated
            scan.name[nameLength] = '\0';
            scan.mask[scan.patternLength] = '\0';
        }
        RSDK::PrintLog(RSDK::PRINT_NORMAL, "[Symbols] Loaded %i scans", scans.size());

        // Scan
        for (auto& scan : scans) {
            SignatureScanResult result;
            result.name = scan.name;
            result.address = sigScan(scan.pattern, scan.mask, *scan.hints, scan.hintCount);
            if (!result.address)
                RSDK::PrintLog(RSDK::PRINT_ERROR, "Failed to find address for \"%s\"!", result.name);
            
            scanResults.push_back(result);
        }
    }
}
#endif