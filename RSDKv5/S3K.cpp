#include "RSDK/Core/RetroEngine.hpp"
#include "S3K.hpp"
#include "Helpers.h"
namespace RSDK
{
    GlobalS3KVariables *globalVars = NULL;

    void OnProcessEngine() {}

    void OnStageLoad()
    {
        globalVars = (GlobalS3KVariables *)globalVarsPtr;
        AddViewableVariable("Anniversary", &globalVars->isAnniversary, VIEWVAR_BOOL, false, true);
        AddViewableVariable("Disable Lives", &globalVars->disableLives, VIEWVAR_BOOL, false, true);
    }
} // namespace RSDK
