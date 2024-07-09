#ifndef S3KFUNCTIONS_H
#define S3KFUNCTIONS_H
#include "RSDK/Core/RetroEngine.hpp"

#define ADD_PADDING(size) int8 padding##__LINE__[size]

struct HUD : RSDK::Entity {

    ADD_PADDING(0x158);
    RSDK::Animator numbersAnimator;

    static int32 CharacterIndexFromID(int32 characterID);
    static void DrawNumbersBase16(HUD* entity, RSDK::Vector2* drawPos, int32 value);
};


void RegisterPublicFunctions();

#endif // !ifdef S3KFUNCTIONS_H