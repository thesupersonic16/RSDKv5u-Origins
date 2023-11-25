#if RETRO_REV02

void SteamCore::Shutdown() { SteamAPI_Shutdown(); }
bool32 SteamCore::CheckAPIInitialized() { return SteamAPI_IsSteamRunning(); }
bool32 SteamCore::CheckFocusLost() { return false; }
void SteamCore::FrameInit() { SteamAPI_RunCallbacks(); }
int32 SteamCore::GetUserLanguage()
{
    // TODO
    //SteamApps()->GetCurrentGameLanguage()
    return LANGUAGE_EN;
}
int32 SteamCore::GetUserRegion() { return REGION_US; }
int32 SteamCore::GetUserPlatform() { return PLATFORM_PC; }
bool32 SteamCore::GetConfirmButtonFlip() { return false; }
void SteamCore::LaunchManual() { SteamFriends()->ActivateGameOverlayToWebPage("https://manuals.sega.com/sonic/origins/"); }
void SteamCore::ExitGame() { RenderDevice::isRunning = false; }
bool32 SteamCore::IsOverlayEnabled(uint32 overlay)
{
    for (int32 i = 0; i < inputDeviceCount; ++i) {
        if (inputDeviceList[i] && inputDeviceList[i]->id == overlay) {
            if (((inputDeviceList[i]->gamepadType >> 16) & 0xFF) != DEVICE_API_STEAM)
                return false;

            return SteamUtils()->IsOverlayEnabled();
        }
    }

    return false;
}
bool32 SteamCore::ShowExtensionOverlay(uint8 overlay)
{
    switch (overlay)
    {
    case 0: SteamFriends()->ActivateGameOverlay("Friends"); break;
    case 1: SteamFriends()->ActivateGameOverlay("Community"); break;
    case 2: SteamFriends()->ActivateGameOverlay("Players"); break;
    case 3: SteamFriends()->ActivateGameOverlay("Settings"); break;
    case 4: SteamFriends()->ActivateGameOverlay("OfficialGameGroup"); break;
    case 5: SteamFriends()->ActivateGameOverlay("Stats"); break;
    case 6: SteamFriends()->ActivateGameOverlay("Achievements"); break;
    default: return false;
    }

    return true;
}


SKU::SteamCore *InitSteamCore()
{
    // Initalize API subsystems
    SteamCore *core = new SteamCore;

    // Should already be initialised
    SteamAPI_Init();

    if (achievements)
        delete achievements;
    achievements = new SteamAchievements;

    if (leaderboards)
        delete leaderboards;
    leaderboards = new SteamLeaderboards;

    if (richPresence)
        delete richPresence;
    richPresence = new SteamRichPresence;

    if (stats)
        delete stats;
    stats = new SteamStats;

    if (userStorage)
        delete userStorage;
    userStorage = new SteamUserStorage;

    core->initialized = true;

    return core;
}
#endif
