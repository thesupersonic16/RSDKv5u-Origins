#include "RSDK/Core/RetroEngine.hpp"
#include "eos/eos_init.h"
#include "eos/eos_sdk.h"
#include "eos/eos_auth.h"
#include "eos/eos_ecom.h"
#include "eos/eos_achievements.h"
#if RETRO_REV02

void EOSUnlockAchievementsCallback(const EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo *info)
{
    if (info->ResultCode != EOS_EResult::EOS_Success) {
        printf("Achievement unlock failed: ResultCode=%d\n", (int)info->ResultCode);
        return;
    }

    printf("Achievement unlock completed!\n");
}

void EOSAchievements::TryUnlockAchievement(AchievementID *id)
{
    EOSCore *core = (EOSCore *)RSDK::SKU::userCore;

    if (GetAchievementsEnabled() && core->productUserId) {
        EOS_Achievements_UnlockAchievementsOptions options { };
        options.ApiVersion        = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;
        options.AchievementIds    = &id->identifier;
        options.AchievementsCount = 1;
        options.UserId            = (EOS_ProductUserId)core->productUserId;
        EOS_Achievements_UnlockAchievements((EOS_HAchievements)core->achievementsHandle, &options, nullptr, EOSUnlockAchievementsCallback);
    }
}
#endif
