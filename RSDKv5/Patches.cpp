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

        // Fix coin positioning
        WRITE_MEMORY(((char *)SigHUD_Draw_E55() + 0x00), (char)0x81, (char)0x45, (char)0x77, (char)0x00, (char)0x00, (char)0xFE, (char)0xFF);
        WRITE_MEMORY(((char *)SigHUD_Draw_E55() + 0x14), (char)0x81, (char)0x45, (char)0x77, (char)0x00, (char)0x00, (char)0x2C, (char)0x00);
    }

} // namespace RSDK
