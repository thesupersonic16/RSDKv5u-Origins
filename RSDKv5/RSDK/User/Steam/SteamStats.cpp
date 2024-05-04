#if RETRO_REV02
void SteamStats::TryTrackStat(StatInfo *stat)
{
    if (GetAchievementsEnabled() && SteamUserStats()->RequestCurrentStats()) {
        SteamUserStats()->SetStat(stat->name, *(int*)stat->data);
        SteamUserStats()->StoreStats();
    }
}
#endif