#include "S3KFunctions.hpp"
#if RETRO_USE_MOD_LOADER

// Collection of recreated functions
#define ADD_PUBLIC_FUNC(func) RSDK::AddPublicFunction(#func, (void *)(func))

int32 HUD::CharacterIndexFromID(int32 characterID)
{
    int32 id = -1;
    for (int32 i = characterID; i > 0; ++id, i >>= 1) ;
    return id;
}
void HUD::DrawNumbersBase16(HUD* entity, RSDK::Vector2* drawPos, int32 value)
{
    int32 mult = 1;
    for (int32 i = 4; i; --i) {
        entity->numbersAnimator.frameID = value / mult & 0xF;
        RSDK::DrawSprite(&entity->numbersAnimator, drawPos, true);
        drawPos->x -= 0x80000;
        mult *= 16;
    }
}

void RegisterPublicFunctions()
{
    ADD_PUBLIC_FUNC(HUD::DrawNumbersBase16);
    ADD_PUBLIC_FUNC(HUD::CharacterIndexFromID);
}
#endif
