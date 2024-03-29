#if RETRO_REV02

struct EOSCore : UserCore {
    void *platformHandle;     // EOS_HPlatform
    void *authHandle;         // EOS_HAuth
    void *ecomHandle;         // EOS_HEcom
    void *achievementsHandle; // EOS_HAchievements
    void *connectHandle;      // EOS_HConnect
    void *authToken;          // EOS_Auth_Token
    void *productUserId;      // EOS_ProductUserId
    char *accountId;
    bool32 init = false;

    bool32 hasPlusDLC = false;

    void Shutdown();
    bool32 CheckAPIInitialized()
    {
        // check if EGS is running
        return true;
    }
    void FrameInit();
    int32 GetUserLanguage()
    {
        // gets the language from EGS
        return LANGUAGE_EN;
    }
    int32 GetUserRegion() { return REGION_US; }
    int32 GetUserPlatform() { return PLATFORM_PC; }
    bool32 GetConfirmButtonFlip() { return false; }
    void LaunchManual() {}
    void ExitGame() { RenderDevice::isRunning = false; }
    bool32 IsOverlayEnabled(uint32 overlay) { return false; }

    bool32 SetupExtensionOverlay()
    {
        // show a popup or something?
        return true;
    }
    virtual bool32 CanShowExtensionOverlay(int32 overlay) { return false; }
    virtual bool32 ShowExtensionOverlay(int32 overlay)
    {
        // do some EGS api stuff
        return true;
    }
    virtual bool32 CanShowAltExtensionOverlay(int32 overlay) { return true; }
    virtual bool32 ShowAltExtensionOverlay(int32 overlay)
    {
        // show the user: https://store.epicgames.com/en-US/p/sonic-mania--encore-dlc
        return true;
    }
    int32 GetConnectingStringID()
    {
        return 68; // STR_CONNECTEGS
    }
    virtual bool32 ShowLimitedVideoOptions(int32 id) { return false; }
    void InitInputDevices() { RSDK::InitInputDevices(); }
    bool32 CheckDLC(uint8 id) { return id == 0 ? hasPlusDLC : false; }
};

EOSCore *InitEOSCore();
#endif
