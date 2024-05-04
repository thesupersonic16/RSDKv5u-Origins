#ifndef S3K_H
#define S3K_H

namespace RSDK
{
    enum CharacterIDs : uint32 {
        ID_NONE     = 0b0000,
        ID_SONIC    = 0b0001,
        ID_TAILS    = 0b0010,
        ID_KNUCKLES = 0b0100,
        ID_AMY      = 0b1000
    };

    enum MedalMods : uint32 {
        MEDAL_DEBUGMODE   = 0b0000001,
        MEDAL_ANDKNUCKLES = 0b0000010,
        MEDAL_PEELOUT     = 0b0000100,
        MEDAL_INSTASHIELD = 0b0001000,
        MEDAL_NODROPDASH  = 0b0010000,
        MEDAL_NOTIMEOVER  = 0b0100000,
        MEDAL_COINS       = 0b1000000,
    };

    enum GameTypes {
        GAME_S1,
        GAME_CD,
        GAME_S2,
        GAME_SM,
        GAME_S3K,
        GAME_S3,
        GAME_SK,
    };

    enum NotifyCallbackIDs {
        NOTIFY_DEATH_EVENT = 128,
        NOTIFY_TOUCH_SIGNPOST,
        NOTIFY_HUD_ENABLE,
        NOTIFY_ADD_COIN,
        NOTIFY_KILL_ENEMY,
        NOTIFY_SAVESLOT_SELECT,
        NOTIFY_FUTURE_PAST,
        NOTIFY_GOTO_FUTURE_PAST,
        NOTIFY_BOSS_END,
        NOTIFY_SPECIAL_END,
        NOTIFY_DEBUGPRINT,
        NOTIFY_KILL_BOSS,
        NOTIFY_TOUCH_EMERALD,
        NOTIFY_STATS_ENEMY,
        NOTIFY_STATS_CHARA_ACTION,
        NOTIFY_STATS_RING,
        NOTIFY_STATS_MOVIE,
        NOTIFY_STATS_PARAM_1,
        NOTIFY_STATS_PARAM_2,
        NOTIFY_CHARACTER_SELECT,
        NOTIFY_SPECIAL_RETRY,
        NOTIFY_TOUCH_CHECKPOINT,
        NOTIFY_ACT_FINISH,
        NOTIFY_1P_VS_SELECT,
        NOTIFY_CONTROLLER_SUPPORT,
        NOTIFY_STAGE_RETRY,
        NOTIFY_SOUND_TRACK,
        NOTIFY_GOOD_ENDING,
        NOTIFY_BACK_TO_MAINMENU,
        NOTIFY_LEVEL_SELECT_MENU,
        NOTIFY_PLAYER_SET,
        NOTIFY_EXTRAS_MODE,
        NOTIFY_SPIN_DASH_TYPE,
        NOTIFY_TIME_OVER,
        NOTIFY_TIMEATTACK_MODE,
        NOTIFY_STATS_BREAK_OBJECT,
        NOTIFY_STATS_SAVE_FUTURE,
        NOTIFY_STATS_CHARA_ACTION2,
    };

    enum AchievementIDs {
        ID_00_All_COMPLETE,
        ID_01_SONIC1_WATCH_OPENING,
        ID_02_SONICCD_WATCH_OPENING,
        ID_03_SONIC2_WATCH_OPENING,
        ID_04_SONIC3K_WATCH_OPENING,
        ID_05_SONIC1_DEFEAT_MOTORA,
        ID_06_SONIC1_BREATHING_COUNT,
        ID_07_SONICCD_TIME_WARP,
        ID_08_SONICCD_WIN_METAL_SONIC,
        ID_09_SONIC2_DEFEAT_STINGER,
        ID_10_SONIC2_WIN_THE_JACKPOT,
        ID_11_SONIC3K_DEFEAT_RHINOBOT,
        ID_12_SONIC3K_GET_ALL_BARRIERS,
        ID_13_RING_COLLECTOR,
        ID_14_DEFEAT_ENEMY_BY_SPIN_DASH,
        ID_15_NOVICE_HERO,
        ID_16_WATCH_AT_MUSEUM,
        ID_17_CLEAR_FIRST_MISSION,
        ID_18_KNUCKLES_GLIDING,
        ID_19_TAILS_FLYING,
        ID_20_CHALLENGE_BOSSRUSH,
        ID_21_SONIC1_CLEAR_S_RANK_MISSION,
        ID_22_SONICCD_CLEAR_S_RANK_MISSION,
        ID_23_SONIC2_CLEAR_S_RANK_MISSION,
        ID_24_SONIC3K_CLEAR_S_RANK_MISSION,
        ID_25_PLAY_MIRRORING,
        ID_26_MOVIE_MANIA,
        ID_27_SOUND_MANIA,
        ID_28_ART_MANIA,
        ID_29_HERO_FOR_ALL,
        ID_30_TRANSFORM_SUPER_SONIC,
        ID_31_SONIC1_CLEAR_ALL_STAGE,
        ID_32_SONICCD_CLEAR_ALL_STAGE,
        ID_33_SONIC2_CLEAR_ALL_STAGE,
        ID_34_SONIC3K_CLEAR_ALL_STAGE,
        ID_35_CLEAR_ALL_TITLE,
    };

    struct GlobalS3KVariables {
        uint8 gameMode;
        CharacterIDs playerID;
        int32 specialCleared;
        int32 specialRingID;
        int32 blueSpheresID;
        int32 blueSpheresInit;
        int32 atlEnabled;
        int32 atlEntityCount;
        int32 atlEntitySlot[288];
        void* atlEntityData[288][276];
        int32 saveLoaded;
        int32 saveRAM[7168];
        int32 saveSlotID;
        int32 noSaveSlot[256];
        int32 menuParam[16384];
        int32 itemMode;
        int32 suppressTitlecard;
        int32 suppressAutoMusic;
        int32 competitionSession[16384];
        MedalMods medalMods;
        int32 parallaxOffset[0x100];
        int32 enableIntro;
        int32 optionsLoaded;
        int32 optionsRAM[0x80];
        int32 presenceID;
        int32 medallionDebug;
        int32 noSave;
        int32 notifiedAutosave;
        int32 recallEntities;
        int32 restartRings;
        int32 restart1UP;
        int32 restartPowerups;
        Vector2 restartPos[4];
        int32 restartSlot[4];
        int32 restartDir[4];
        int32 restartMinutes;
        int32 restartSeconds;
        int32 restartMilliseconds;
        int32 restartScore;
        int32 restartScore1UP;
        int32 restartLives[4];
        int32 restartMusicID;
        bool32 restartFlags;
        int32 field_47B4C;
        int32 showExtendedTimeHUD;
        int32 overrideRestart;
        int32 overrideUnknown;
        Vector2 overrideRestartPos[4];
        int32 overrideRestartSlot[4];
        int32 overrideRestartDir[4];
        int32 overrideRestartMinutes;
        int32 overrideRestartSeconds;
        int32 overrideRestartMilliseconds;
        int32 tempMinutes;
        int32 tempSeconds;
        int32 tempMilliseconds;
        int32 persistentTimer;
        int32 tempFlags;
        int32 continues;
        bool32 initCoolBonus;
        int32 coolBonus[4];
        int32 replayWriteBuffer[0x40000];
        int32 replayReadBuffer[0x40000];
        int32 replayTempWBuffer[0x40000];
        int32 replayTempRBuffer[0x40000];
        int32 replayTable;
        int32 replayTableLoaded;
        int32 taTable;
        int32 taTableLoaded;
        int32 stock;
        int32 characterFlags;
        bool32 vapeMode;
        int32 secrets;
        int32 titleIntroThanks;
        bool32 soundTestEnabled;
        bool32 superMusicEnabled;
        GameTypes playerSpriteStyle;
        GameTypes gameSpriteStyle;
        GameTypes ostStyle;
        GameTypes starpostStyle;
        bool32 stageFinished;
        int32 field_447C14;
        int32 storedOverrideUnknown;
        int32 atlCameraBoundsL[4];
        int32 atlCameraBoundsR[4];
        int32 atlCameraBoundsT[4];
        int32 atlCameraBoundsB[4];
        Vector2 atlCameraPos[4];
        Vector2 atlOffset;
        int32 unknownValues[4];
        int32 unknownValues2[4];
        int32 field_447CA4;
        bool32 tileCollisionMode;
        uint8 gravityDir;
        uint8 blueSpheresSeed[4];
        bool32 blueSpheresHasPerfect;
        int32 field_447CB8;
        int32 field_447CBC;
        int32 field_447CC0;
        int32 field_447CC4;
        int32 field_447CC8;
        uint8 field_447CCC;
        int32 field_447CD0;
        bool32 disableLives;
        bool32 mirrorMode;
        bool32 useManiaBehavior;
        int32 coinCount;
        uint8 gap4C34DC[0x28];
        bool32 hasPlusDLC;
        int32 playMode;
        int32 callbackParam0;
        int32 callbackParam1;
        int32 callbackParam2;
        int32 callbackParam3;
        bool32 hudEnable;
        bool32 useCoins;
        uint8 gap4C352C[0x60];
        bool32 waitSSRetry;
    };

    struct OriginsData {
        uint32 version;
        bool32 disableLives;
        bool32 usePathTracer;
        bool32 useCoins;
        int32  coinCount;
        bool32 playMode;
        int32  lastSaveSlot;
        uint32 lastCharacterID;
        int32 totalRings;
        int32 totalEnemies;
        bool32 hasSeenIntro;
    };

    struct LoopPointChangeInfo {
        RETRO_HASH_MD5(hash);
        uint32 oldLoopPoint;
        uint32 newLoopPoint;
    };

    void OnEngineInit();
    void OnFrameInit();
    void OnStageLoad();
    void OnGameFinish();
    void OnEngineShutdown();
    void OnGlobalsLoaded(int32 *globals);
    void OnDrawGroupDraw();
    void OnInputProcess();
    void OnSfxPlay(ChannelInfo* info);
    bool32 OnStreamPlay(char **filename, uint32 *slot, uint32 *startPos, uint32 *loopPoint, int32 *speed);
    void OnCallbackNotify(int32 callback, int32 param1, int32 param2, int32 param3);
    void AddLoopReplacement(const char *filename, uint32 oldLoopPoint, uint32 newLoopPoint, bool32 use12FastLoop);
    void RegisterLoopPoints();
    void UnlockAchievement(AchievementIDs id);
    void RegisterAchievementID(const char *name);
    void RegisterAchievements();
    bool32 VideoSkipCB();
    void DrawSpecialStageRetryMessage();
    void LoadDefaultOriginsData(OriginsData *savedata);

    // Collision workaround functions
    void UploadCollisionData();
    void DownloadCollisionData();
    void RedirectSensorToOrigins(void* address, CollisionSensor *sensor);

#if RETRO_USE_MOD_LOADER
    OriginsData *GetOriginsData();
    void AddPublicFunctions();
#endif
    } // namespace RSDK

#endif // !ifdef S3K_H