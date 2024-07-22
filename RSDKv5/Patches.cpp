#include "Patches.hpp"
#include "Helpers.h"
#include "SigScan.h"

namespace RSDK
{

    void ApplyPatches()
    {
        // Remove Origins events while allowing HML events
        WRITE_MEMORY(SigProcessEngine_0A(), (char)0x48, (char)0x83, (char)0xC4, (char)0x28, (char)0xC3);
        WRITE_MEMORY(SigRunCore_0A(), (char)0xC3);

        // Replace AVX instructions with SSE
        WRITE_MEMORY(((char *)SigAVXPatch() + 0x00), (char)0xF3, (char)0x0F);
        WRITE_MEMORY(((char *)SigAVXPatch() + 0x08), (char)0xF3, (char)0x0F);
        WRITE_MEMORY(((char *)SigAVXPatch() + 0x20), (char)0xF3, (char)0x0F);
        WRITE_MEMORY(((char *)SigAVXPatch() + 0x28), (char)0xF3, (char)0x0F);
        WRITE_MEMORY(((char *)SigAVXPatch() + 0x40), (char)0xF3, (char)0x0F);
        WRITE_MEMORY(((char *)SigAVXPatch() + 0x48), (char)0xF3, (char)0x0F);
        WRITE_MEMORY(((char *)SigAVXPatch() + 0x60), (char)0xF3, (char)0x0F);
        WRITE_MEMORY(((char *)SigAVXPatch() + 0x68), (char)0xF3, (char)0x0F);
        WRITE_MEMORY(((char *)SigAVXPatch() + 0x85), (char)0x66, (char)0x0F, (char)0x7E, (char)0xC0, (char)0xF3, (char)0x0F);

        // Remove blue sphere log spam
        WRITE_MEMORY(((char *)SigS3K_SS_Player_Input_Following() + 0x73), (char)0x90, (char)0x90, (char)0x90, (char)0x90, (char)0x90, (char)0x90);
        WRITE_MEMORY(((char *)SigS3K_SS_Player_Input_Following() + 0x89), (char)0x90, (char)0x90, (char)0x90, (char)0x90, (char)0x90, (char)0x90);
    }

} // namespace RSDK
