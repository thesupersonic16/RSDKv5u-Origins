#if RETRO_REV02
void SteamAchievements::TryUnlockAchievement(AchievementID *id)
{
    if (GetAchievementsEnabled()) {
        SteamUserStats()->SetAchievement(id->identifier);
        SteamUserStats()->StoreStats();
    }
}
#endif
