#include "RSDK/Core/RetroEngine.hpp"
#include "S3K.hpp"
#include "Helpers.h"
#include "SigScan.h"

namespace RSDK
{
    // Variables
    GlobalS3KVariables *globalVars = NULL;
    bool *usePathTracer            = NULL;

    void OnEngineInit()
    {
        usePathTracer = (bool *)SigusePathTracer();
        
        // Due to an engine bug we will disable path tracer
        *usePathTracer = false;
    }

    void OnFrameInit()
    {
        // Check for PLus DLC
        if (globalVars)
            globalVars->hasPlusDLC = SKU::userCore->CheckDLC(0);
    }

    void OnStageLoad()
    {
        globalVars = (GlobalS3KVariables *)globalVarsPtr;
        AddViewableVariable("Play Mode", &globalVars->playMode, VIEWVAR_UINT8, 0, 4);
        AddViewableVariable("Disable Lives", &globalVars->disableLives, VIEWVAR_BOOL, false, true);
        AddViewableVariable("Use Coins", &globalVars->useCoins, VIEWVAR_BOOL, false, true);
        AddViewableVariable("Coin Count", &globalVars->coinCount, VIEWVAR_INT16, 0, 999);
        AddViewableVariable("Use Path Tracer", usePathTracer, VIEWVAR_BOOL, false, true);
    }

    void OnCallbackNotify(int32 callback, int32 param1, int32 param2, int32 param3)
    {
        switch (callback) {
            case NOTIFY_DEATH_EVENT:         PrintLog(PRINT_NORMAL, "NOTIFY: DeathEvent() -> %d", param1); break;
            case NOTIFY_TOUCH_SIGNPOST:      PrintLog(PRINT_NORMAL, "NOTIFY: TouchSignPost() -> %d", param1); break;
            case NOTIFY_HUD_ENABLE:          PrintLog(PRINT_NORMAL, "NOTIFY: HUDEnable() -> %d", param1); break;
            case NOTIFY_ADD_COIN:
                globalVars->coinCount += param1;
                break;
            case NOTIFY_KILL_ENEMY:          PrintLog(PRINT_NORMAL, "NOTIFY: KillEnemy() -> %d", param1); break;
            case NOTIFY_SAVESLOT_SELECT:     PrintLog(PRINT_NORMAL, "NOTIFY: SaveSlotSelect() -> %d", param1); break;
            case NOTIFY_FUTURE_PAST:         PrintLog(PRINT_NORMAL, "NOTIFY: FuturePast() -> %d", param1); break;
            case NOTIFY_GOTO_FUTURE_PAST:    PrintLog(PRINT_NORMAL, "NOTIFY: GotoFuturePast() -> %d", param1); break;
            case NOTIFY_BOSS_END:            PrintLog(PRINT_NORMAL, "NOTIFY: BossEnd() -> %d", param1); break;
            case NOTIFY_SPECIAL_END:         PrintLog(PRINT_NORMAL, "NOTIFY: SpecialEnd() -> %d", param1); break;
            case NOTIFY_DEBUGPRINT:          PrintLog(PRINT_NORMAL, "NOTIFY: DebugPrint() -> %d", param1); break;
            case NOTIFY_KILL_BOSS:           PrintLog(PRINT_NORMAL, "NOTIFY: KillBoss() -> %d", param1); break;
            case NOTIFY_TOUCH_EMERALD:       PrintLog(PRINT_NORMAL, "NOTIFY: TouchEmerald() -> %d", param1); break;
            case NOTIFY_STATS_ENEMY:         PrintLog(PRINT_NORMAL, "NOTIFY: StatsEnemy() -> %d, %d, %d", param1, param2, param3); break;
            case NOTIFY_STATS_CHARA_ACTION:  PrintLog(PRINT_NORMAL, "NOTIFY: StatsCharaAction() -> %d, %d, %d", param1, param2, param3); break;
            case NOTIFY_STATS_RING:          PrintLog(PRINT_NORMAL, "NOTIFY: StatsRing() -> %d", param1); break;
            case NOTIFY_STATS_MOVIE:
                PlayStream("Outro.ogg", 0, 0, 0, false);
                LoadVideo("Outro.ogv", 0, VideoSkipCB);
                sceneInfo.activeCategory = 0;
                sceneInfo.listPos        = 0;
                sceneInfo.state          = ENGINESTATE_LOAD;
                break;
            case NOTIFY_STATS_PARAM_1:       PrintLog(PRINT_NORMAL, "NOTIFY: StatsParam1() -> %d, %d, %d", param1, param2, param3); break;
            case NOTIFY_STATS_PARAM_2:       PrintLog(PRINT_NORMAL, "NOTIFY: StatsParam2() -> %d", param1); break;
            case NOTIFY_CHARACTER_SELECT:    PrintLog(PRINT_NORMAL, "NOTIFY: CharacterSelect() -> %d", param1); break;
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
            case NOTIFY_1P_VS_SELECT:        PrintLog(PRINT_NORMAL, "NOTIFY: 1PVSSelect() -> %d", param1); break;
            case NOTIFY_CONTROLLER_SUPPORT:  PrintLog(PRINT_NORMAL, "NOTIFY: ControllerSupport() -> %d", param1); break;
            case NOTIFY_STAGE_RETRY:         PrintLog(PRINT_NORMAL, "NOTIFY: StageRetry() -> %d", param1); break;
            case NOTIFY_SOUND_TRACK:         PrintLog(PRINT_NORMAL, "NOTIFY: SoundTrack() -> %d", param1); break;
            case NOTIFY_GOOD_ENDING:         PrintLog(PRINT_NORMAL, "NOTIFY: GoodEnding() -> %d", param1); break;
            case NOTIFY_BACK_TO_MAINMENU:    PrintLog(PRINT_NORMAL, "NOTIFY: BackToMainMenu() -> %d", param1); break;
            case NOTIFY_LEVEL_SELECT_MENU:   PrintLog(PRINT_NORMAL, "NOTIFY: LevelSelectMenu() -> %d", param1); break;
            case NOTIFY_PLAYER_SET:          PrintLog(PRINT_NORMAL, "NOTIFY: PlayerSet() -> %d", param1); break;
            case NOTIFY_EXTRAS_MODE:         PrintLog(PRINT_NORMAL, "NOTIFY: ExtrasMode() -> %d", param1); break;
            case NOTIFY_SPIN_DASH_TYPE:      PrintLog(PRINT_NORMAL, "NOTIFY: SpindashType() -> %d", param1); break;
            case NOTIFY_TIME_OVER:           PrintLog(PRINT_NORMAL, "NOTIFY: TimeOver() -> %d", param1); break;
            case NOTIFY_TIMEATTACK_MODE:     PrintLog(PRINT_NORMAL, "NOTIFY: TimeAttackMode() -> %d", param1); break;
            case NOTIFY_STATS_BREAK_OBJECT:  PrintLog(PRINT_NORMAL, "NOTIFY: StatsBreakObject() -> %d, %d", param1, param2); break;
            case NOTIFY_STATS_SAVE_FUTURE:   PrintLog(PRINT_NORMAL, "NOTIFY: StatsSaveFuture() -> %d", param1); break;
            case NOTIFY_STATS_CHARA_ACTION2: PrintLog(PRINT_NORMAL, "NOTIFY: StatsCharaAction2() -> %d, %d, %d", param1, param2, param3); break;
            default: PrintLog(PRINT_NORMAL, "NOTIFY: %d -> %d, %d, %d", callback, param1, param2, param3); break;
        }
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

} // namespace RSDK
