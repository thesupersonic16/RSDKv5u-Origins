#include "RSDK/Core/RetroEngine.hpp"
#include "S3K.hpp"
#include "Helpers.h"
#include "SigScan.h"

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
    uint8 usedShields = 0;
    char streamFileName[0x40];
    bool isStreamFast = false;

    void OnEngineInit()
    {
        usePathTracer = (bool *)SigusePathTracer();

        RegisterLoopPoints();
        RegisterAchievements();
        // Should really be in the callback but this function at the monent is not async
        //SKU::TryDeleteUserFile("OriginsData.bin", NULL);
        if (!SKU::TryLoadUserFile("OriginsData.bin", &originsData, sizeof(OriginsData), NULL))
            LoadDefaultOriginsData(&originsData);
        *usePathTracer = originsData.usePathTracer;
        OnGlobalsLoaded(globalVarsPtr);
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

        if (globalVars) {
            globalVars->disableLives = originsData.disableLives;
            globalVars->useCoins     = originsData.useCoins;
            globalVars->coinCount    = originsData.coinCount;
            globalVars->playMode     = originsData.playMode;
        }
    }

    void OnFrameInit()
    {
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
            UnlockAchievement(ID_04_SONIC3K_WATCH_OPENING);
        }
        if (globalVars) {
            AddViewableVariable("Play Mode", &globalVars->playMode, VIEWVAR_UINT8, 0, 8);
            AddViewableVariable("Disable Lives", &globalVars->disableLives, VIEWVAR_BOOL, false, true);
            AddViewableVariable("Use Coins", &globalVars->useCoins, VIEWVAR_BOOL, false, true);
            AddViewableVariable("Coin Count", &globalVars->coinCount, VIEWVAR_INT16, 0, 999);
            AddViewableVariable("Music Type", &globalVars->ostStyle, VIEWVAR_INT8, 0, 6);
        }
        AddViewableVariable("Use Path Tracer", usePathTracer, VIEWVAR_BOOL, false, true);
        AddViewableVariable("Has Seen Intro", &originsData.hasSeenIntro, VIEWVAR_BOOL, false, true);
    }
    
    // Runs on game finish state which is after the movie is played
    void OnGameFinish()
    {
        UnlockAchievement(ID_34_SONIC3K_CLEAR_ALL_STAGE);

        // Restart game
        sceneInfo.activeCategory = 0;
        sceneInfo.listPos        = 0;
        sceneInfo.state          = ENGINESTATE_LOAD;
    }

    void OnSfxPlay(ChannelInfo *info)
    { 
        for (int i = 0; i < sizeof(loopChanges) / sizeof(LoopPointChangeInfo); ++i) {
            LoopPointChangeInfo *changeInfo = &loopChanges[i];
            if (HASH_MATCH_MD5(sfxList[info->soundID].hash, changeInfo->hash) && info->loop == changeInfo->oldLoopPoint)
            {
                info->loop = changeInfo->newLoopPoint;
                return;
            }
        }
    }

    bool32 OnStreamPlay(char **filename, uint32* slot, uint32* startPos, uint32* loopPoint, int32* speed)
    {
        PrintLog(PRINT_NORMAL, "Playing Steam \"%s\" with a loop point of %d and begins on sample %d", *filename, *loopPoint, *startPos);
        
        // Replace loop point
        RETRO_HASH_MD5(hash);
        GEN_HASH_MD5(*filename, hash);
        for (int i = 0; i < sizeof(loopChanges) / sizeof(LoopPointChangeInfo); ++i) {
            LoopPointChangeInfo *changeInfo = &loopChanges[i];
            if (HASH_MATCH_MD5(hash, changeInfo->hash) && *loopPoint == changeInfo->oldLoopPoint) {
                *loopPoint = changeInfo->newLoopPoint;
                break;
            }
        }

        // Handle speed
        // The code built in S3K for handling the speed is faulty
        std::string path = std::string("Data/Music/") + *filename;
        std::string name = *filename;
        bool isFast      = name.find("/F/") != std::string::npos;

        if (isFast)
        {
            path.replace(path.find("/F/"), 3, "/");
            name.replace(name.find("/F/"), 3, "/");
        }

        ChannelInfo *channel = &channels[*slot];
        if (!strcmp(streamFilePath, path.c_str()))
        {
            strcpy_s(streamFileName, name.c_str());
            *filename = streamFileName;
            channel->speed    = (int32)((isFast ? 1.2 : 1.0) * TO_FIXED(1));
            PrintLog(PRINT_NORMAL, "  Speed is now %f", isFast ? 1.2 : 1.0);
            bool speedChanged = isStreamFast != isFast;
            isStreamFast      = isFast;
            return !(channel->state == CHANNEL_STREAM && speedChanged);
        }
        isFast = false;
        return true;
    }

    void OnCallbackNotify(int32 callback, int32 param1, int32 param2, int32 param3)
    {
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
            case NOTIFY_BOSS_END:            PrintLog(PRINT_POPUP, "NOTIFY: BossEnd() -> %d", param1); break;
            case NOTIFY_SPECIAL_END:         PrintLog(PRINT_POPUP, "NOTIFY: SpecialEnd() -> %d", param1); break;
            case NOTIFY_DEBUGPRINT:          PrintLog(PRINT_POPUP, "NOTIFY: DebugPrint() -> %d", param1); break;
            case NOTIFY_KILL_BOSS:           PrintLog(PRINT_POPUP, "NOTIFY: KillBoss() -> %d", param1); break;
            case NOTIFY_TOUCH_EMERALD:       PrintLog(PRINT_POPUP, "NOTIFY: TouchEmerald() -> %d", param1); break;
            case NOTIFY_STATS_ENEMY:
                originsData.totalEnemies += param1;
                if (originsData.totalEnemies >= 50)
                    UnlockAchievement(ID_15_NOVICE_HERO);
                if (originsData.totalEnemies >= 200)
                    UnlockAchievement(ID_29_HERO_FOR_ALL);
                if (param3 >= 30)
                    UnlockAchievement(ID_14_DEFEAT_ENEMY_BY_SPIN_DASH);
                if (param3 >= 10)
                    UnlockAchievement(ID_11_SONIC3K_DEFEAT_RHINOBOT);
                break;
            case NOTIFY_STATS_CHARA_ACTION:
                if (param1)
                    UnlockAchievement(ID_30_TRANSFORM_SUPER_SONIC);
                if (param2)
                    UnlockAchievement(ID_19_TAILS_FLYING);
                if (param3)
                    PrintLog(PRINT_POPUP, "NOTIFY: StatsCharaAction() -> %d, %d, %d", param1, param2, param3);
                break;
            case NOTIFY_STATS_RING:
                originsData.totalRings += param1;

                if (originsData.totalRings >= 1000)
                    UnlockAchievement(ID_13_RING_COLLECTOR);
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

                if (usedShields == 0b111)
                    UnlockAchievement(ID_12_SONIC3K_GET_ALL_BARRIERS);
                break;
            case NOTIFY_STATS_PARAM_2:       PrintLog(PRINT_POPUP, "NOTIFY: StatsParam2() -> %d", param1); break;
            case NOTIFY_CHARACTER_SELECT:    PrintLog(PRINT_POPUP, "NOTIFY: CharacterSelect() -> %d", param1); break;
            case NOTIFY_SPECIAL_RETRY:
                if (globalVars->coinCount)
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
            case NOTIFY_CONTROLLER_SUPPORT:  PrintLog(PRINT_POPUP, "NOTIFY: ControllerSupport() -> %d", param1); break;
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
                    UnlockAchievement(ID_18_KNUCKLES_GLIDING);
                if (param2)
                    PrintLog(PRINT_POPUP, "NOTIFY: StatsCharaAction2() -> %d, %d, %d", param1, param2, param3);
                if (param3)
                    PrintLog(PRINT_POPUP, "NOTIFY: StatsCharaAction2() -> %d, %d, %d", param1, param2, param3);
                break;
                break;
            default: PrintLog(PRINT_NORMAL, "NOTIFY: %d -> %d, %d, %d", callback, param1, param2, param3); break;
                // 1003 might be score related?
        }
    }

    void AddLoopReplacement(const char *filename, uint32 oldLoopPoint, uint32 newLoopPoint)
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
    }

    void RegisterLoopPoints()
    {
        // SoundFX
        AddLoopReplacement("Stage/Airship.wav", 179497, 0);
        AddLoopReplacement("Stage/Airflow.wav", 82292, 0);
        AddLoopReplacement("Stage/Drill.wav", 13611, 0);
        AddLoopReplacement("Stage/Hover.wav", 67735, 0);
        AddLoopReplacement("3K_SSZ/DeathEggRise.wav", 116772, 0);

        // Music
        AddLoopReplacement("3K/AngelIsland1.ogg"  , 1, 161209);
        AddLoopReplacement("3K/AngelIsland2.ogg"  , 1, 95776);
        AddLoopReplacement("3K/AzureLake.ogg"     , 96970, 150175);
        AddLoopReplacement("3K/BalloonPark.ogg"   , 39500, 127295);
        AddLoopReplacement("3K/Boss.ogg"          , 141019, 132660);
        AddLoopReplacement("3K/CarnivalNight1.ogg", 1, 82540);
        AddLoopReplacement("3K/CarnivalNight2.ogg", 1, 82580);
        AddLoopReplacement("3K/ChromeGadget.ogg"  , 1, 113331);
        AddLoopReplacement("3K/Competition.ogg"   , 1, 103687);
        AddLoopReplacement("3K/DeathEgg1.ogg"     , 1, 181948);
        AddLoopReplacement("3K/DeathEgg2.ogg"     , 1, 158988);
        AddLoopReplacement("3K/DesertPalace.ogg"  , 1, 81737);
        AddLoopReplacement("3K/Doomsday.ogg"      , 626645, 661556);
        AddLoopReplacement("3K/EndlessMine.ogg"   , 76852, 158392);
        AddLoopReplacement("3K/FlyingBattery1.ogg", 17688, 265102);
        AddLoopReplacement("3K/FlyingBattery2.ogg", 70605, 353223);
        AddLoopReplacement("3K/GachaBonus.ogg"    , 160668, 239783);
        AddLoopReplacement("3K/Hydrocity1.ogg"    , 1, 164780);
        AddLoopReplacement("3K/Hydrocity2.ogg"    , 1, 92643);
        AddLoopReplacement("3K/IceCap1.ogg"       , 155860, 176411);
        AddLoopReplacement("3K/IceCap2.ogg"       , 163241, 194634);
        AddLoopReplacement("3K/Invincibility3.ogg", 39528, 113882);
        AddLoopReplacement("3K/InvincibilityK.ogg", 30539, 0);
        AddLoopReplacement("3K/KnucklesK.ogg", 74967, 0); // Origins does not loop this track
        AddLoopReplacement("3K/LaunchBase1.ogg"   , 301429, 75356);
        AddLoopReplacement("3K/LaunchBase1.ogg"   , 345426, 75356);
        AddLoopReplacement("3K/LaunchBase2.ogg"   , 1, 82574);
        AddLoopReplacement("3K/LavaReef1.ogg"     , 82394, 165594);
        AddLoopReplacement("3K/LavaReef2.ogg"     , 440503, 264351);
        AddLoopReplacement("3K/MarbleGarden1.ogg" , 89756, 153288);
        AddLoopReplacement("3K/MarbleGarden2.ogg" , 22793, 371016);
        AddLoopReplacement("3K/MushroomHill1.ogg" , 499847, 177404);
        AddLoopReplacement("3K/MushroomHill2.ogg" , 336319, 162685);
        AddLoopReplacement("3K/Options.ogg"       , 41104, 135411);
        AddLoopReplacement("3K/Sandopolis1.ogg"   , 303715, 380198);
        AddLoopReplacement("3K/Sandopolis2.ogg"   , 321489, 403521);
        AddLoopReplacement("3K/SkySanctuary.ogg"  , 1, 286305);
        AddLoopReplacement("3K/SkySanctuary.ogg"  , 160668, 286305);
        AddLoopReplacement("3K/SlotBonus.ogg"     , 160668, 403618);
        AddLoopReplacement("3K/SpecialStage.ogg"  , 247006, 413050);
        AddLoopReplacement("3K/SpecialStageS0.ogg", 247006, 413050);
        AddLoopReplacement("3K/SpecialStageS1.ogg", 247006, 413050);
        AddLoopReplacement("3K/SpecialStageS2.ogg", 247006, 413050);
        AddLoopReplacement("3K/SpecialStageS3.ogg", 247006, 413050);
        AddLoopReplacement("3K/SphereBonus.ogg"   , 154449, 228188);
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
        savedata->usePathTracer   = false; // Due to an engine bug path tracer will be disabled by default
        savedata->useCoins        = true;
        savedata->coinCount       = 100;
        savedata->playMode        = 1;  // 1: Anniversary
        savedata->lastSaveSlot    = -1; // -1: No save
        savedata->lastCharacterID = ID_SONIC | ID_TAILS;
        savedata->totalRings      = 0;
        savedata->hasSeenIntro    = false;
    }

    void UploadCollisionData()
    {
        // Sonic Origins 2.0.2 addresses
        *((Entity **)(0x143363150)) = collisionEntity;
        *((uint32 *)0x14336316C)    = collisionTolerance;
        memcpy((void *)0x143D97160, tileLayers, sizeof(tileLayers));
        memcpy((void *)0x1432D3150, collisionMasks, sizeof(collisionMasks));
        memcpy((void *)0x143353150, tileInfo, sizeof(tileInfo));
    }
    void DownloadCollisionData()
    {
        // Sonic Origins 2.0.2 addresses
        collisionEntity = *((Entity **)(0x143363150));
        memcpy(tileLayers, (void *)0x143D97160, sizeof(tileLayers));
        memcpy(collisionMasks, (void *) 0x1432D3150, sizeof(collisionMasks));
        memcpy(tileInfo, (void *) 0x143353150, sizeof(tileInfo));
    }

    void RedirectSensorToOrigins(void* address, CollisionSensor* sensor)
    {
        UploadCollisionData();
        auto sensorFunc = (void(__fastcall *)(CollisionSensor *sensor))(address);
        sensorFunc(sensor);
        DownloadCollisionData();
    }

} // namespace RSDK
