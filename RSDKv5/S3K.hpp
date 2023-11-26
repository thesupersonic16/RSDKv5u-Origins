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

    struct GlobalS3KVariables {
        uint8 gameMode;
        CharacterIDs playerID;
        uint8 gap8[0xC2CAC];
        MedalMods medalMods;
        uint8 gapC2CB8[0x400748];
        GameTypes playerSpriteStyle;
        GameTypes gameSpriteStyle;
        GameTypes ostStyle;
        uint8 gap4C340C[0xC8];
        int disableLives;
        uint8 gap4C34DC[0x34];
        bool32 hasPlusDLC;
        bool32 isAnniversary;
    };

    void OnEngineInit();
    void OnFrameInit();
    void OnStageLoad();

} // namespace RSDK
#endif // !ifdef S3K_H