#include "RSDK/Core/RetroEngine.hpp"
#include "main.hpp"
#include "S3K.hpp"
#include "Helpers.h"
#include "SigScan.h"
#include "Symbols.h"
#include "S3KFunctions.hpp"

#if RETRO_USE_MOD_LOADER
#define ADD_PUBLIC_FUNC(func) AddPublicFunction(#func, (void *)(func))
#endif
// Links to Audio.cpp
extern char streamFilePath[0x40];

namespace RSDK
{
    // Variables
    LoopPointChangeInfo loopChanges[0x100] {};
    OriginsData originsData;
    GlobalS3KVariables *globalVars = NULL;
    SKU::AchievementID achievementIDs[0x40]{};
    int32 achievementIDCount = 0;
    bool *usePathTracer            = NULL;
    bool usingLevelSelect          = false;
    bool32 isMirrorMode            = false;
    uint8 usedShields = 0;
    float streamSpeed = 1.0f;
    uint16 flipBuffer[SCREEN_XMAX * SCREEN_YSIZE];
    bool flipFramebuffer = false;

    void CBOriginsDataLoad(int32 status)
    {
        if (status == SKU::STATUS_NOTFOUND)
            LoadDefaultOriginsData(&originsData);
        *usePathTracer = originsData.usePathTracer;
        OnGlobalsLoaded(globalVarsPtr);
    }

    void OnEngineInit()
    {
        usePathTracer = (bool *)SigusePathTracer();

        // Load data pack
        char packPath[MAX_PATH];
        sprintf_s(packPath, "%s\\Update.rsdk", modPath);
        LoadDataPack(packPath, 0, false);
        sprintf_s(packPath, "%s\\Base.rsdk", modPath);
        LoadDataPack(packPath, 0, false);

        RegisterLoopPoints();
        RegisterAchievements();
        //SKU::TryDeleteUserFile("OriginsData.bin", NULL);
        SKU::TryLoadUserFile("OriginsData.bin", &originsData, sizeof(OriginsData), CBOriginsDataLoad);
    }

    void OnEngineShutdown()
    {
        if (globalVars) {
            originsData.disableLives  = globalVars->disableLives;
            originsData.usePathTracer = (bool32)*usePathTracer;
            originsData.useCoins      = globalVars->useCoins;
            originsData.coinCount     = globalVars->coinCount;
            originsData.playMode      = globalVars->playMode;
        }
        SKU::TrySaveUserFile("OriginsData.bin", &originsData, sizeof(OriginsData), NULL, false);
    }

    void OnGlobalsLoaded(int32* globals)
    { 
        globalVars = (GlobalS3KVariables *)globalVarsPtr;
        //PrintLog(PRINT_NORMAL, "%llx\n", globalVars);
        //printf("%llx\n", globalVars);

        if (globalVars) {
            globalVars->disableLives = originsData.disableLives;
            globalVars->useCoins     = originsData.useCoins;
            globalVars->coinCount    = originsData.coinCount;
            globalVars->playMode     = originsData.playMode;
        }
    }

    void OnFrameInit()
    {
        // Call Origins ProcessEngine for ML callbacks
        ((void(__fastcall *)())((char *)SigProcessEngine_0A() - 0x0A))();
        // Call Origins RunCore for ML callbacks
        ((void(__fastcall *)())((char *)SigRunCore_0A() - 0x0A))();

        // Check for Plus DLC
        if (globalVars)
            globalVars->hasPlusDLC = SKU::userCore->CheckDLC(0);
    }

    void OnStageLoad()
    {
        if (!originsData.hasSeenIntro)
        {
            // This may cause issues if you the command line to jump to a stage on a new save
            originsData.hasSeenIntro = true;
            PlayStream("Intro.ogg", 0, 0, 0, false);
            LoadVideo("Intro.ogv", 0, VideoSkipCB);
            UpdateStatsInt32("WATCH_OPENING2", 1);
        }

        flipFramebuffer = globalVars && globalVars->mirrorMode && sceneInfo.activeCategory != 0;

        if (globalVars) {
            AddViewableVariable("GameMode Type", &globalVars->gameMode, VIEWVAR_UINT8, 0, 6);
            AddViewableVariable("playerID Type", &globalVars->playerID, VIEWVAR_UINT8, 0, 32);
            AddViewableVariable("Play Mode", &globalVars->playMode, VIEWVAR_UINT8, 0, 6);
            AddViewableVariable("Disable Lives", &globalVars->disableLives, VIEWVAR_BOOL, false, true);
            AddViewableVariable("Use Coins", &globalVars->useCoins, VIEWVAR_BOOL, false, true);
            AddViewableVariable("Coin Count", &globalVars->coinCount, VIEWVAR_INT16, 0, 999);
            AddViewableVariable("Player Type", &globalVars->playerSpriteStyle, VIEWVAR_INT8, 0, 6);
            AddViewableVariable("Game Type", &globalVars->gameSpriteStyle, VIEWVAR_INT8, 0, 6);
            AddViewableVariable("Music Type", &globalVars->ostStyle, VIEWVAR_INT8, 0, 6);
            AddViewableVariable("Starpost Type", &globalVars->starpostStyle, VIEWVAR_INT8, 0, 6);
            AddViewableVariable("Mirror Mode", &globalVars->mirrorMode, VIEWVAR_BOOL, false, true);
            AddViewableVariable("Mania Behavior", &globalVars->useManiaBehavior, VIEWVAR_BOOL, false, true);
            AddViewableVariable("Sound Test", &globalVars->soundTestEnabled, VIEWVAR_BOOL, false, true);
            AddViewableVariable("Secrets", &globalVars->secrets, VIEWVAR_INT16, 0, 999);
            AddViewableVariable("Medal Mods", &globalVars->medalMods, VIEWVAR_INT16, 0, 999);
        }
        AddViewableVariable("Use Path Tracer", usePathTracer, VIEWVAR_BOOL, false, true);
        AddViewableVariable("Has Seen Intro", &originsData.hasSeenIntro, VIEWVAR_BOOL, false, true);
    }
    
    // Runs on game finish state which is after the movie is played
    void OnGameFinish()
    {
        UpdateStatsInt32("CLEAR_ALL_STAGE2", 1);

        // Restart game
        sceneInfo.activeCategory = 0;
        sceneInfo.listPos        = 0;
        sceneInfo.state          = ENGINESTATE_LOAD;
    }

    void OnDrawGroupDraw()
    {
        // Flip frame buffer for mirror mode
        if (flipFramebuffer && sceneInfo.currentDrawGroup == 14) {
            memcpy(flipBuffer, currentScreen->frameBuffer, sizeof(flipBuffer));

            for (int32 x = 0; x < currentScreen->size.x; x++) {
                for (int32 y = 0; y < currentScreen->size.y; y++) {
                for (int32 x = 0; x < currentScreen->pitch; x++) {
                    currentScreen->frameBuffer[y * currentScreen->pitch + x] = flipBuffer[y * currentScreen->pitch + (currentScreen->pitch - x)];
                }
            }
        }
    }

    void OnInputProcess()
    {
        if (flipFramebuffer && sceneInfo.state == ENGINESTATE_REGULAR) {
            for (auto &input : controller)
            {
                InputState buffer = input.keyLeft;
                input.keyLeft.down   = input.keyRight.down;
                input.keyLeft.press  = input.keyRight.press;
                input.keyRight.down  = buffer.down;
                input.keyRight.press = buffer.press;
            }
        }
    }

    void OnSfxPlay(ChannelInfo *info)
    { 
        for (int i = 0; i < sizeof(loopChanges) / sizeof(LoopPointChangeInfo); ++i) {
            LoopPointChangeInfo *changeInfo = &loopChanges[i];
            if (HASH_MATCH_MD5(sfxList[info->soundID].hash, changeInfo->hash)
                && (changeInfo->oldLoopPoint == -1 || info->loop == changeInfo->oldLoopPoint))
            {
                info->loop = changeInfo->newLoopPoint;
                return;
            }
        }
    }

    bool32 OnStreamPlay(char **filename, uint32* slot, uint32* startPos, uint32* loopPoint, int32* speed)
    {
        PrintLog(PRINT_NORMAL, "Playing Stream \"%s\" with a loop point of %d and begins on sample %d", *filename, *loopPoint, *startPos);
        
        // Replace loop point
        RETRO_HASH_MD5(hash);
        GEN_HASH_MD5(*filename, hash);
        for (int i = 0; i < sizeof(loopChanges) / sizeof(LoopPointChangeInfo); ++i) {
            LoopPointChangeInfo *changeInfo = &loopChanges[i];
            if (HASH_MATCH_MD5(hash, changeInfo->hash) && 
                (changeInfo->oldLoopPoint == -1 || *loopPoint == changeInfo->oldLoopPoint)) {
                *loopPoint = changeInfo->newLoopPoint;
                break;
            }
        }

        // Handle speed
        // The code built in S3K for handling the speed is faulty
        bool isTransition   = false;
        std::string path    = std::string("Data/Music/") + *filename;
        bool isFast         = path.find("/F/") != std::string::npos;
        bool isSpecialStage = path.find("3K/SpecialStage") != std::string::npos;
        float fastSpeed     = 1.2f;

        // Return if not 3K music
        if (path.find("/3K/") == std::string::npos)
            return true;

        if (isFast)
        {
            path.replace(path.find("/F/"), 3, "/");
            isTransition = !strcmp(streamFilePath, path.c_str());
        }
        else
        {
            path.replace(path.find("/3K/"), 4, "/3K/F/");
            isTransition = !strcmp(streamFilePath, path.c_str());
        }

        // Handle blue spheres
        if (isSpecialStage)
        {
            // 1 = S0, 2 = S1, ...
            int32 speedStage = 0;
            size_t pos       = path.find("StageS");
            if (isTransition = isFast = (pos != std::string::npos))
            {
                int32 speedStep = path.c_str()[pos + 6] - '0' + 1;
                fastSpeed       = speedStep * 0.05f + 1.0f;
            }
        }

        if (!isFast && !isTransition)
        {
            streamSpeed = 1.0f;
        }

        ChannelInfo *channel = &channels[*slot];
        if (isTransition)
        {
            float newSpeed    = (isFast ? fastSpeed : 1.0f);
            bool speedChanged = streamSpeed != newSpeed;
            if (speedChanged)
            {
                PrintLog(PRINT_NORMAL, "  Speed change %f -> %f", streamSpeed, newSpeed);
                float ratio = streamSpeed / newSpeed;
                *startPos   = GetChannelPos(*slot) * ratio;
                *loopPoint  = *loopPoint * (1.0f / newSpeed);
                streamSpeed = newSpeed;
            }
        }
        isFast = false;
        return true;
    }

    void OnChannelAttributesChanged(uint32* channel, float* volume, float* panning, float* speed)
    {
        if (globalVars->mirrorMode)
            *panning = -*panning;
    }

    void OnCallbackNotify(int32 callback, int32 param1, int32 param2, int32 param3)
    {
        int32 kinds = 0;
        switch (callback) {
            case NOTIFY_DEATH_EVENT:         PrintLog(PRINT_NORMAL, "NOTIFY: DeathEvent() -> %d", param1); break;
            case NOTIFY_TOUCH_SIGNPOST:      PrintLog(PRINT_POPUP, "NOTIFY: TouchSignPost() -> %d", param1); break;
            case NOTIFY_HUD_ENABLE:          PrintLog(PRINT_POPUP, "NOTIFY: HUDEnable() -> %d", param1); break;
            case NOTIFY_ADD_COIN: 
                globalVars->coinCount = CLAMP(globalVars->coinCount + param1, 0, 999);
                break;
            case NOTIFY_KILL_ENEMY:          PrintLog(PRINT_POPUP, "NOTIFY: KillEnemy() -> %d", param1); break;
            case NOTIFY_SAVESLOT_SELECT:
                if (!usingLevelSelect)
                    originsData.lastSaveSlot = param1;
                break;
            case NOTIFY_FUTURE_PAST:         PrintLog(PRINT_POPUP, "NOTIFY: FuturePast() -> %d", param1); break;
            case NOTIFY_GOTO_FUTURE_PAST:    PrintLog(PRINT_POPUP, "NOTIFY: GotoFuturePast() -> %d", param1); break;
            case NOTIFY_BOSS_END:            PrintLog(PRINT_NORMAL, "NOTIFY: BossEnd() -> %d", param1); break;
            case NOTIFY_SPECIAL_END:         PrintLog(PRINT_POPUP, "NOTIFY: SpecialEnd() -> %d", param1); break;
            case NOTIFY_DEBUGPRINT:          PrintLog(PRINT_POPUP, "NOTIFY: DebugPrint() -> %d", param1); break;
            case NOTIFY_KILL_BOSS:           PrintLog(PRINT_NORMAL, "NOTIFY: KillBoss() -> %d", param1); break;
            case NOTIFY_TOUCH_EMERALD:       PrintLog(PRINT_NORMAL, "NOTIFY: TouchEmerald() -> %d", param1); break;
            case NOTIFY_STATS_ENEMY:
                originsData.totalEnemies += param1;
                UpdateStatsInt32("DEFEAT_ENEMY", originsData.totalEnemies);
                // TODO: Check if these are totals
                UpdateStatsInt32("DEFEAT_ENEMY_BY_SPIN_DASH", param2);
                UpdateStatsInt32("DEFEAT_RHINOBOT", param3);
                break;
            case NOTIFY_STATS_CHARA_ACTION:
                if (param1)
                    UpdateStatsInt32("TRANSFORM_SUPER_SONIC_COUNT", 1);
                if (param2)
                    UpdateStatsInt32("FLYING_COUNT", 1);
                if (param3)
                    PrintLog(PRINT_POPUP, "NOTIFY: StatsCharaAction() -> %d, %d, %d", param1, param2, param3);
                break;
            case NOTIFY_STATS_RING:
                originsData.totalRings += param1;
                UpdateStatsInt32("RING_COUNT", originsData.totalRings);
                break;
            case NOTIFY_STATS_MOVIE:
                if (param1)
                {
                    PlayStream("Outro.ogg", 0, 0, 0, false);
                    LoadVideo("Outro.ogv", 0, VideoSkipCB);
                } else
                    PrintLog(PRINT_POPUP, "NOTIFY: StatsMovie() -> %d, %d, %d", param1, param2, param3);
                break;
            case NOTIFY_STATS_PARAM_1:
                if (param1)
                    usedShields |= (1 << 0);
                if (param2)
                    usedShields |= (1 << 1);
                if (param3)
                    usedShields |= (1 << 2);

                kinds += (usedShields & 0b001) ? 1 : 0;
                kinds += (usedShields & 0b010) ? 1 : 0;
                kinds += (usedShields & 0b100) ? 1 : 0;
                UpdateStatsInt32("GET_BARRIER_KIND", kinds);
                break;
            case NOTIFY_STATS_PARAM_2:       PrintLog(PRINT_NORMAL, "NOTIFY: StatsParam2() -> %d", param1); break;
            case NOTIFY_CHARACTER_SELECT:    PrintLog(PRINT_POPUP, "NOTIFY: CharacterSelect() -> %d", param1); break;
            case NOTIFY_SPECIAL_RETRY:
                if (globalVars->useCoins && !param1 && globalVars->coinCount)
                {
                    // Using devmenu as we have no other way to display this message
                    OpenDevMenu();
                    devMenu.state = DrawSpecialStageRetryMessage;
                }
                globalVars->waitSSRetry = 0;
                break;
            case NOTIFY_TOUCH_CHECKPOINT:    PrintLog(PRINT_NORMAL, "NOTIFY: TouchCheckpoint() -> %d", param1); break;
            case NOTIFY_ACT_FINISH:          PrintLog(PRINT_NORMAL, "NOTIFY: ActFinish() -> %d", param1); break;
            case NOTIFY_1P_VS_SELECT:        PrintLog(PRINT_POPUP, "NOTIFY: 1PVSSelect() -> %d", param1); break;
            case NOTIFY_CONTROLLER_SUPPORT:  PrintLog(PRINT_NORMAL, "NOTIFY: ControllerSupport() -> %d", param1); break;
            case NOTIFY_STAGE_RETRY:         PrintLog(PRINT_POPUP, "NOTIFY: StageRetry() -> %d", param1); break;
            case NOTIFY_SOUND_TRACK:         PrintLog(PRINT_POPUP, "NOTIFY: SoundTrack() -> %d", param1); break;
            case NOTIFY_GOOD_ENDING:         PrintLog(PRINT_POPUP, "NOTIFY: GoodEnding() -> %d", param1); break;
            case NOTIFY_BACK_TO_MAINMENU:    PrintLog(PRINT_NORMAL, "NOTIFY: BackToMainMenu() -> %d", param1); break;
            case NOTIFY_LEVEL_SELECT_MENU:   usingLevelSelect = param1; break;
            case NOTIFY_PLAYER_SET: 
                if (!usingLevelSelect)
                    originsData.lastCharacterID = param1;
                break;
            case NOTIFY_EXTRAS_MODE:         PrintLog(PRINT_POPUP, "NOTIFY: ExtrasMode() -> %d", param1); break;
            case NOTIFY_SPIN_DASH_TYPE:      PrintLog(PRINT_POPUP, "NOTIFY: SpindashType() -> %d", param1); break;
            case NOTIFY_TIME_OVER:           PrintLog(PRINT_POPUP, "NOTIFY: TimeOver() -> %d", param1); break;
            case NOTIFY_TIMEATTACK_MODE:     PrintLog(PRINT_POPUP, "NOTIFY: TimeAttackMode() -> %d", param1); break;
            case NOTIFY_STATS_BREAK_OBJECT:  PrintLog(PRINT_POPUP, "NOTIFY: StatsBreakObject() -> %d, %d", param1, param2); break;
            case NOTIFY_STATS_SAVE_FUTURE:   PrintLog(PRINT_POPUP, "NOTIFY: StatsSaveFuture() -> %d", param1); break;
            case NOTIFY_STATS_CHARA_ACTION2: 
                if (param1)
                    UpdateStatsInt32("GLIDING_COUNT", 1);
                if (param2)
                    PrintLog(PRINT_NORMAL, "NOTIFY: StatsCharaAction2() -> %d, %d, %d", param1, param2, param3); // Amy Hammer
                if (param3)
                    PrintLog(PRINT_POPUP, "NOTIFY: StatsCharaAction2() -> %d, %d, %d", param1, param2, param3);
                break;
                break;
            default: PrintLog(PRINT_NORMAL, "NOTIFY: %d -> %d, %d, %d", callback, param1, param2, param3); break;
                // 1003 might be score related?
        }
    }

    void AddLoopReplacement(const char *filename, uint32 oldLoopPoint, uint32 newLoopPoint, bool32 use12FastLoop)
    {
        // Find empty slot
        int slot = 0;
        for (int i = 0; i < sizeof(loopChanges); ++i) {
            if (!loopChanges[i].hash[0]) {
                slot = i;
                break;
            }
        }

        LoopPointChangeInfo *info = &loopChanges[slot];

        GEN_HASH_MD5(filename, info->hash);
        info->oldLoopPoint = oldLoopPoint;
        info->newLoopPoint = newLoopPoint;

        // Create fast loop for 1.2x
        if (use12FastLoop)
        {
            std::string newFileName = std::string(filename);
            newFileName             = newFileName.replace(newFileName.find("3K/"), 3, "3K/F/");
            float loopPoint = (44100.0f / 48000.0f) * newLoopPoint;
            AddLoopReplacement(newFileName.c_str(), -1, (int32)loopPoint, false);
        }
    }

    void RegisterLoopPoints()
    {
        // SoundFX
        AddLoopReplacement("Stage/Airship.wav"      , 179497, 0, false);
        AddLoopReplacement("Stage/Airflow.wav"      , 82292 , 0, false);
        AddLoopReplacement("Stage/Drill.wav"        , 13611 , 0, false);
        AddLoopReplacement("Stage/Drill.wav"        , 43643 , 0, false); // MGZ2 Boss drill loop and LBZ1 falling building... wtf
        AddLoopReplacement("Stage/Hover.wav"        , 67735 , 0, false);
        AddLoopReplacement("Stage/TakeOff.wav"      , 33159 , 0, false); // LBZ2 Death Egg takeoff
        AddLoopReplacement("3K_SSZ/DeathEggRise.wav", 116772, 0, false);
        //AddLoopReplacement("Stage/DrillShort.wav"   , -1    , 0, false);
        
        AddLoopReplacement("Stage/DrillWarble.wav"  , -1    , 0, false);	
        AddLoopReplacement("Stage/Lava.wav"         , 81870 , -1, false);
        AddLoopReplacement("3K_DEZ/Engine.wav"      , 72263 , 12903, false);

        // Music
        AddLoopReplacement("3K/AngelIsland1.ogg"  , 1     , 161209, true);
        AddLoopReplacement("3K/AngelIsland2.ogg"  , 1     , 95776 , true);
        AddLoopReplacement("3K/AzureLake.ogg"     , 96970 , 150175, true);
        AddLoopReplacement("3K/BalloonPark.ogg"   , 39500 , 127295, true);
        AddLoopReplacement("3K/Boss.ogg"          , 141019, 53971 , true);
        AddLoopReplacement("3K/CarnivalNight1.ogg", 1     , 82540 , true);
        AddLoopReplacement("3K/CarnivalNight2.ogg", 1     , 82580 , true);
        AddLoopReplacement("3K/ChromeGadget.ogg"  , 1     , 113331, true);
        AddLoopReplacement("3K/Competition.ogg"   , 1     , 103687, true);
        AddLoopReplacement("3K/DeathEgg1.ogg"     , 1     , 181948, true);
        AddLoopReplacement("3K/DeathEgg2.ogg"     , 1     , 158988, true);
        AddLoopReplacement("3K/DesertPalace.ogg"  , 1     , 81737 , true);
        AddLoopReplacement("3K/Doomsday.ogg"      , 626645, 661556, true);
        AddLoopReplacement("3K/EndlessMine.ogg"   , 76852 , 158392, true);
        AddLoopReplacement("3K/FinalBoss.ogg"     , 721127, 722165, true);
        AddLoopReplacement("3K/FlyingBattery1.ogg", 17688 , 265102, true);
        AddLoopReplacement("3K/FlyingBattery2.ogg", 70605 , 353223, true);
        AddLoopReplacement("3K/GachaBonus.ogg"    , 160668, 239783, false);
        AddLoopReplacement("3K/Hydrocity1.ogg"    , 1     , 164780, true);
        AddLoopReplacement("3K/Hydrocity2.ogg"    , 1     , 92643 , true);
        AddLoopReplacement("3K/IceCap1.ogg"       , 155860, 176411, true);
        AddLoopReplacement("3K/IceCap2.ogg"       , 163241, 194634, true);
        AddLoopReplacement("3K/Invincibility3.ogg", 39528 , 113882, true);
        AddLoopReplacement("3K/InvincibilityK.ogg", 30539 , 0     , true);
        AddLoopReplacement("3K/KnucklesK.ogg"     , 74967 , 0     , false); // Origins does not loop this track
        AddLoopReplacement("3K/LaunchBase1.ogg"   , 301429, 75356 , true);
        AddLoopReplacement("3K/LaunchBase1.ogg"   , 345426, 75356 , true);
        AddLoopReplacement("3K/LaunchBase2.ogg"   , 1     , 82574 , true);
        AddLoopReplacement("3K/LavaReef1.ogg"     , 82394 , 165594, true);
        AddLoopReplacement("3K/LavaReef2.ogg"     , 440503, 264351, true);
        AddLoopReplacement("3K/MarbleGarden1.ogg" , 89756 , 153288, true);
        AddLoopReplacement("3K/MarbleGarden2.ogg" , 22793 , 371016, true);
        AddLoopReplacement("3K/Miniboss.ogg"      , 70625 , 132660, true);//
        AddLoopReplacement("3K/MiniBossK.ogg"     , 70625 , 132660, true);
        AddLoopReplacement("3K/MushroomHill1.ogg" , 499847, 177404, true);
        AddLoopReplacement("3K/MushroomHill2.ogg" , 336319, 162685, true);
        AddLoopReplacement("3K/Options.ogg"       , 41104 , 135411, false);
        AddLoopReplacement("3K/Sandopolis1.ogg"   , 303715, 380198, true);
        AddLoopReplacement("3K/Sandopolis2.ogg"   , 321489, 403521, true);
        AddLoopReplacement("3K/SkySanctuary.ogg"  , 1     , 286305, true);
        AddLoopReplacement("3K/SkySanctuary.ogg"  , 160668, 286305, true);
        AddLoopReplacement("3K/SlotBonus.ogg"     , 160668, 403618, false);
        AddLoopReplacement("3K/SpecialStage.ogg"  , -1    , 413050, false);
        AddLoopReplacement("3K/SpecialStageS0.ogg", -1    , 413050, false);
        AddLoopReplacement("3K/SpecialStageS1.ogg", -1    , 413050, false);
        AddLoopReplacement("3K/SpecialStageS2.ogg", -1    , 413050, false);
        AddLoopReplacement("3K/SpecialStageS3.ogg", -1    , 413050, false);
        AddLoopReplacement("3K/SphereBonus.ogg"   , 154449, 228188, false);
        AddLoopReplacement("3K/StageBoss.ogg"     , 141019, 53971 , true);
        AddLoopReplacement("3K/Super.ogg"         , 141502, 1     , true);
        AddLoopReplacement("3K/TheDoomsday.ogg"   , 605488, 722165, true);
    }
    
    void UpdateStatsInt32(const char* name, int progress)
    {
        SKU::StatInfo info;
        info.name = name;
        *(int*)info.data = progress;

        SKU::TryTrackStat(&info);
    }

    void RegisterAchievementID(const char *name)
    {
        achievementIDs[achievementIDCount].identifier = name;
        achievementIDs[achievementIDCount].idUnknown = achievementIDCount++;
    }

    void UnlockAchievement(AchievementIDs id) { SKU::TryUnlockAchievement(&achievementIDs[id]); }

    void RegisterAchievements()
    {
        RegisterAchievementID("ID_00_All_COMPLETE");
        RegisterAchievementID("ID_01_SONIC1_WATCH_OPENING");
        RegisterAchievementID("ID_02_SONICCD_WATCH_OPENING");
        RegisterAchievementID("ID_03_SONIC2_WATCH_OPENING");
        RegisterAchievementID("ID_04_SONIC3K_WATCH_OPENING");
        RegisterAchievementID("ID_05_SONIC1_DEFEAT_MOTORA");
        RegisterAchievementID("ID_06_SONIC1_BREATHING_COUNT");
        RegisterAchievementID("ID_07_SONICCD_TIME_WARP");
        RegisterAchievementID("ID_08_SONICCD_WIN_METAL_SONIC");
        RegisterAchievementID("ID_09_SONIC2_DEFEAT_STINGER");
        RegisterAchievementID("ID_10_SONIC2_WIN_THE_JACKPOT");
        RegisterAchievementID("ID_11_SONIC3K_DEFEAT_RHINOBOT");
        RegisterAchievementID("ID_12_SONIC3K_GET_ALL_BARRIERS");
        RegisterAchievementID("ID_13_RING_COLLECTOR");
        RegisterAchievementID("ID_14_DEFEAT_ENEMY_BY_SPIN_DASH");
        RegisterAchievementID("ID_15_NOVICE_HERO");
        RegisterAchievementID("ID_16_WATCH_AT_MUSEUM");
        RegisterAchievementID("ID_17_CLEAR_FIRST_MISSION");
        RegisterAchievementID("ID_18_KNUCKLES_GLIDING");
        RegisterAchievementID("ID_19_TAILS_FLYING");
        RegisterAchievementID("ID_20_CHALLENGE_BOSSRUSH");
        RegisterAchievementID("ID_21_SONIC1_CLEAR_S_RANK_MISSION ");
        RegisterAchievementID("ID_22_SONICCD_CLEAR_S_RANK_MISSION");
        RegisterAchievementID("ID_23_SONIC2_CLEAR_S_RANK_MISSION ");
        RegisterAchievementID("ID_24_SONIC3K_CLEAR_S_RANK_MISSION");
        RegisterAchievementID("ID_25_PLAY_MIRRORING");
        RegisterAchievementID("ID_26_MOVIE_MANIA");
        RegisterAchievementID("ID_27_SOUND_MANIA");
        RegisterAchievementID("ID_28_ART_MANIA");
        RegisterAchievementID("ID_29_HERO_FOR_ALL");
        RegisterAchievementID("ID_30_TRANSFORM_SUPER_SONIC");
        RegisterAchievementID("ID_31_SONIC1_CLEAR_ALL_STAGE");
        RegisterAchievementID("ID_32_SONICCD_CLEAR_ALL_STAGE");
        RegisterAchievementID("ID_33_SONIC2_CLEAR_ALL_STAGE");
        RegisterAchievementID("ID_34_SONIC3K_CLEAR_ALL_STAGE");
        RegisterAchievementID("ID_35_CLEAR_ALL_TITLE");
    }

    bool32 VideoSkipCB()
    {
        if (controller->keyA.press || controller->keyB.press || controller->keyStart.press) {
            StopChannel(0);
            return true;
        }

        return false;
    }

    void DrawSpecialStageRetryMessage()
    {
        int32 dy = currentScreen->center.y - 32;
        DrawRectangle(currentScreen->center.x - 128, dy, 0x100, 0x40, 0x80, 0xFF, INK_NONE, true);
        DrawDevString("Retry Special Stage?", currentScreen->center.x, currentScreen->center.y - 4, ALIGN_CENTER, 0xFFFFFF);
        
        DrawDevString(("Coins: " + std::to_string(globalVars->coinCount)).c_str(), currentScreen->center.x - 124,
            currentScreen->center.y + 21, ALIGN_LEFT, 0xFFFFFF);

        if (controller->keyA.down || controller->keyStart.down) {
            sceneInfo.state = ENGINESTATE_LOAD;
            --globalVars->coinCount;
        }
        else if (controller->keyB.down) {
            CloseDevMenu();
        }
    }

    void LoadDefaultOriginsData(OriginsData* savedata)
    {
        savedata->version         = 0;
        savedata->disableLives    = true;
        savedata->usePathTracer   = true;
        savedata->useCoins        = true;
        savedata->coinCount       = 100;
        savedata->playMode        = 1;  // 1: Anniversary
        savedata->lastSaveSlot    = -1; // -1: No save
        savedata->lastCharacterID = ID_SONIC | ID_TAILS;
        savedata->totalRings      = 0;
        savedata->hasSeenIntro    = false;
    }

#if RETRO_USE_MOD_LOADER
    OriginsData *GetOriginsData() { return &originsData; }

    void AddPublicFunctions()
    {
        usePathTracer = (bool *)SigusePathTracer();

        for (auto& scan : Symbols::scanResults)
        {
            if (!scan.address)
                PrintLog(PRINT_ERROR, "Scan for \"%s\" is null!");
            //else
            //    PrintLog(PRINT_NORMAL, "Added \"%s\": 0x%llx", scan.name, scan.address);
            AddPublicFunction(scan.name, scan.address);
        }
        // Might aswell make everything a public function
        ADD_PUBLIC_FUNC(OnEngineInit);
        ADD_PUBLIC_FUNC(OnFrameInit);
        ADD_PUBLIC_FUNC(OnStageLoad);
        ADD_PUBLIC_FUNC(OnGameFinish);
        ADD_PUBLIC_FUNC(OnEngineShutdown);
        ADD_PUBLIC_FUNC(OnGlobalsLoaded);
        ADD_PUBLIC_FUNC(OnDrawGroupDraw);
        ADD_PUBLIC_FUNC(OnInputProcess);
        ADD_PUBLIC_FUNC(OnSfxPlay);
        ADD_PUBLIC_FUNC(OnStreamPlay);
        ADD_PUBLIC_FUNC(OnCallbackNotify);
        ADD_PUBLIC_FUNC(AddLoopReplacement);
        ADD_PUBLIC_FUNC(RegisterLoopPoints);
        ADD_PUBLIC_FUNC(UpdateStatsInt32);
        ADD_PUBLIC_FUNC(UnlockAchievement);
        ADD_PUBLIC_FUNC(RegisterAchievementID);
        ADD_PUBLIC_FUNC(RegisterAchievements);
        ADD_PUBLIC_FUNC(VideoSkipCB);
        ADD_PUBLIC_FUNC(DrawSpecialStageRetryMessage);
        ADD_PUBLIC_FUNC(LoadDefaultOriginsData);

        // Register S3K functions
        RegisterPublicFunctions();
        ADD_PUBLIC_FUNC(usePathTracer);
    }
#endif
} // namespace RSDK
