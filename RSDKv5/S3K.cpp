#include "RSDK/Core/RetroEngine.hpp"
#include "S3K.hpp"
#include "Helpers.h"
#include "SigScan.h"

namespace RSDK
{
    // Variables
    GlobalS3KVariables *globalVars = NULL;
    bool *usePathTracer            = NULL;

    void OnEngineInit()
    {
        usePathTracer = (bool *)SigusePathTracer();
        
        // Due to an engine bug we will disable path tracer
        *usePathTracer = false;
    }

    void OnFrameInit()
    {
        // Check for PLus DLC
        if (globalVars)
            globalVars->hasPlusDLC = SKU::userCore->CheckDLC(0);
    }

    void OnStageLoad()
    {
        globalVars = (GlobalS3KVariables *)globalVarsPtr;
        AddViewableVariable("Anniversary", &globalVars->isAnniversary, VIEWVAR_BOOL, false, true);
        AddViewableVariable("Disable Lives", &globalVars->disableLives, VIEWVAR_BOOL, false, true);
        AddViewableVariable("Use Path Tracer", usePathTracer, VIEWVAR_BOOL, false, true);
    }
    } // namespace RSDK
