#if RETRO_REV02

struct SteamCore : UserCore {
    void Shutdown();
    bool32 CheckAPIInitialized();
    bool32 CheckFocusLost();
    void FrameInit();
    int32 GetUserLanguage();
    int32 GetUserRegion();
    int32 GetUserPlatform();
    bool32 GetConfirmButtonFlip();
    void LaunchManual();
    void ExitGame();
    bool32 IsOverlayEnabled(uint32 overlay);
    bool32 ShowExtensionOverlay(uint8 overlay);
    bool32 CheckDLC(uint8 id);

    bool32 initialized = false;
};

SteamCore *InitSteamCore();
#endif
