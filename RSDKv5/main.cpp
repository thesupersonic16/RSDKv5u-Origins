#include "RSDK/Core/RetroEngine.hpp"
#include "main.hpp"
#include "HiteModLoader.h"
#include "Helpers.h"
#include "SigScan.h"

#if RETRO_STANDALONE
#define LinkGameLogic RSDK::LinkGameLogic
#else
#define EngineInfo RSDK::EngineInfo
#define LinkGameLogic LinkGameLogicDLL
#endif

ModLoader *ModLoaderData;
char modPath[MAX_PATH];
bool entryCalled = false;

void GetRedirectedPath(const char *path, char *out) { ModLoaderData->GetRedirectedPath(path, out); }

HOOK(HRESULT, __fastcall, D3D11CreateDevice, PROC_ADDRESS("d3d11.dll", "D3D11CreateDevice"),
    void *pAdapter, UINT DriverType, void* Software, UINT Flags, void *pFeatureLevels, UINT FeatureLevels,
    UINT SDKVersion, void **ppDevice, void *pFeatureLevel, void **ppImmediateContext)
{
    if (entryCalled)
        return originalD3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels,
            SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
    entryCalled = true;

    // Workaround for selecting datapack for the modloader
    auto MLEngine_LoadFile = (void(__fastcall *)(FileInfo *info, const char *filePath, int openMode))(SigEngine_LoadFile());
    FileInfo info;
    MLEngine_LoadFile(&info, "retro/Sonic3ku.rsdk", 0xFF);
    
    RSDK::linkGameLogic = (RSDK::LogicLinkHandle)SigLinkGameLogic();
	
#if RETRO_RENDERDEVICE_DIRECTX9 || RETRO_RENDERDEVICE_DIRECTX11
    RSDK::RenderDevice::hInstance = NULL;
    RSDK::RenderDevice::hPrevInstance = NULL;
    RSDK::RenderDevice::nShowCmd      = SW_SHOW;
#endif

    // Force the console for now
    auto argc = 1;
    auto argv = new const char*[1]{ "console=true" };
    
    RSDK::InitCoreAPI();

    int32 exitCode = RSDK::RunRetroEngine(argc, (char**)argv);

    RSDK::ReleaseCoreAPI();

    return E_FAIL;
}

#ifdef RETRO_USE_MOD_LOADER
HOOK(void, __fastcall, StateMachineRun, 0x1400AD8F0, void (**state)(void *), void *data) {
    if (*state)
        RSDK::StateMachineRun(*state, data);
}
#endif

extern "C" __declspec(dllexport) void Init(ModInfo *modInfo)
{
	SigLinkGameLogic();
    INSTALL_HOOK(D3D11CreateDevice);
#ifdef RETRO_USE_MOD_LOADER
    INSTALL_HOOK(StateMachineRun);
#endif
    // Nuke message box
    WRITE_MEMORY(SigNukeSystemReq(), (char)0xEB);

    GetCurrentDirectoryA(MAX_PATH, modPath);

    ModLoaderData = modInfo->ModLoader;
}
