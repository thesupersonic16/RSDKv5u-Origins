#if RETRO_REV02

struct SteamAchievements : UserAchievements {
    void TryUnlockAchievement(AchievementID *id);
};

#endif
