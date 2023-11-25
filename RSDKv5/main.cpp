#include "RSDK/Core/RetroEngine.hpp"
#include "main.hpp"
#include "HiteModLoader.h"
#include "Helpers.h"

#if RETRO_STANDALONE
#define LinkGameLogic RSDK::LinkGameLogic
#else
#define EngineInfo RSDK::EngineInfo
#define LinkGameLogic LinkGameLogicDLL
#endif

HOOK(bool, __fastcall, D3D11CreateDevice, PROC_ADDRESS("d3d11.dll", "D3D11CreateDevice"))
{
    //MessageBoxA(NULL, "", "", NULL);
    RSDK::linkGameLogic = (RSDK::LogicLinkHandle)0x1400AD750;

    // Force the console for now
    auto argc = 1;
    auto argv = new const char*[1]{ "console=true" };

    RSDK::InitCoreAPI();

    int32 exitCode = RSDK::RunRetroEngine(argc, (char**)argv);

    RSDK::ReleaseCoreAPI();

    return false;
}

extern "C" __declspec(dllexport) void Init(ModInfo * modInfo)
{ INSTALL_HOOK(D3D11CreateDevice) }
