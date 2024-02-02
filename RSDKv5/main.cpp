#include "RSDK/Core/RetroEngine.hpp"
#include "main.hpp"
#include "HiteModLoader.h"
#include "Helpers.h"
#include "SigScan.h"
#ifdef STEAM_API_NODLL
#include <steam_api.h>
#endif
#if (RETRO_USERCORE_EOS && !EOS_USE_DLLEXPORT)
#include "eos/eos_init.h"
#include "eos/eos_sdk.h"
#include "eos/eos_auth.h"
#include "eos/eos_ecom.h"
#endif

#if RETRO_STANDALONE
#define LinkGameLogic RSDK::LinkGameLogic
#else
#define EngineInfo RSDK::EngineInfo
#define LinkGameLogic LinkGameLogicDLL
#endif

const char *epicToken = "";
ModLoader *ModLoaderData;
char modPath[MAX_PATH];
bool entryCalled = false;

// Steam links
#ifdef STEAM_API_NODLL
HMODULE steamAPIHandle = nullptr;

S_API bool S_CALLTYPE SteamAPI_Init()
{
    if (steamAPIHandle) {
        void *address = GetProcAddress(steamAPIHandle, "SteamAPI_Init");
        return ((bool(S_CALLTYPE *)())(address))();
    }
    return false;
}

S_API void S_CALLTYPE SteamAPI_Shutdown()
{
    if (steamAPIHandle) {
        void *address = GetProcAddress(steamAPIHandle, "SteamAPI_Shutdown");
        return ((void(S_CALLTYPE *)())(address))();
    }
}

S_API void S_CALLTYPE SteamAPI_RunCallbacks()
{
    if (steamAPIHandle) {
        void *address = GetProcAddress(steamAPIHandle, "SteamAPI_RunCallbacks");
        return ((void(S_CALLTYPE *)())(address))();
    }
}

S_API bool S_CALLTYPE SteamAPI_IsSteamRunning()
{
    if (steamAPIHandle) {
        void *address = GetProcAddress(steamAPIHandle, "SteamAPI_IsSteamRunning");
        return ((bool(S_CALLTYPE *)())(address))();
    }
    return false;
}

S_API HSteamPipe SteamAPI_GetHSteamPipe()
{
    if (steamAPIHandle) {
        void *address = GetProcAddress(steamAPIHandle, "SteamAPI_GetHSteamPipe");
        return ((HSteamPipe(S_CALLTYPE *)())(address))();
    }
    return 0;
}

S_API HSteamUser SteamAPI_GetHSteamUser()
{
    if (steamAPIHandle) {
        void *address = GetProcAddress(steamAPIHandle, "SteamAPI_GetHSteamUser");
        return ((HSteamUser(S_CALLTYPE *)())(address))();
    }
    return 0;
}

S_API void * S_CALLTYPE SteamInternal_ContextInit(void *pContextInitData)
{
    if (steamAPIHandle) {
        void *address = GetProcAddress(steamAPIHandle, "SteamInternal_ContextInit");
        return ((void *(S_CALLTYPE *)(void *pContextInitData))(address))(pContextInitData);
    }
    return nullptr;
}

S_API void * S_CALLTYPE SteamInternal_CreateInterface(const char *ver)
{
    if (steamAPIHandle) {
        void *address = GetProcAddress(steamAPIHandle, "SteamInternal_CreateInterface");
        return ((void *(S_CALLTYPE *)(const char *ver))(address))(ver);
    }
    return nullptr;
}
#endif

#if (RETRO_USERCORE_EOS && !EOS_USE_DLLEXPORT)
HMODULE EOSHandle = nullptr;

extern "C" EOS_EResult EOS_Initialize(const EOS_InitializeOptions *Options)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Initialize");
        return ((EOS_EResult(__fastcall *)(const EOS_InitializeOptions *))(address))(Options);
    }
    return EOS_EResult::EOS_UnexpectedError;
}
EOS_DECLARE_FUNC(EOS_EResult) EOS_Shutdown()
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Shutdown");
        return ((EOS_EResult(EOS_CALL *)())(address))();
    }
    return EOS_EResult::EOS_UnexpectedError;
}
EOS_DECLARE_FUNC(EOS_HPlatform) EOS_Platform_Create(const EOS_Platform_Options *Options)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Platform_Create");
        return ((EOS_HPlatform(EOS_CALL *)(const EOS_Platform_Options *))(address))(Options);
    }
    return nullptr;
}
EOS_DECLARE_FUNC(void) EOS_Platform_Release(EOS_HPlatform Handle)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Platform_Release");
        ((void(EOS_CALL *)(EOS_HPlatform))(address))(Handle);
    }
}

EOS_DECLARE_FUNC(EOS_HEcom) EOS_Platform_GetEcomInterface(EOS_HPlatform Handle)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Platform_GetEcomInterface");
        return ((EOS_HEcom(EOS_CALL *)(EOS_HPlatform))(address))(Handle);
    }
    return nullptr;
}

EOS_DECLARE_FUNC(void)
EOS_Ecom_QueryOwnership(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipOptions *Options, void *ClientData,
                        const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Ecom_QueryOwnership");
        ((void(EOS_CALL *)(EOS_HEcom, const EOS_Ecom_QueryOwnershipOptions *, void *, const EOS_Ecom_OnQueryOwnershipCallback))(address))(Handle, Options, ClientData, CompletionDelegate);
    }
}

EOS_DECLARE_FUNC(void)
EOS_Auth_Login(EOS_HAuth Handle, const EOS_Auth_LoginOptions *Options, void *ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Auth_Login");
        ((void(EOS_CALL *)(EOS_HAuth, const EOS_Auth_LoginOptions *, void *, const EOS_Auth_OnLoginCallback))(address))(
            Handle, Options, ClientData, CompletionDelegate);
    }
}

EOS_DECLARE_FUNC(EOS_EResult)
EOS_Auth_CopyUserAuthToken(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions *Options, EOS_EpicAccountId LocalUserId,
                           EOS_Auth_Token **OutUserAuthToken)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Auth_CopyUserAuthToken");
        return ((EOS_EResult(EOS_CALL *)(EOS_HAuth, const EOS_Auth_CopyUserAuthTokenOptions *, EOS_EpicAccountId, EOS_Auth_Token **))(address))(
            Handle, Options, LocalUserId, OutUserAuthToken);
    }
    return EOS_EResult::EOS_UnexpectedError;
}

EOS_DECLARE_FUNC(EOS_EResult)
EOS_Auth_CopyIdToken(EOS_HAuth Handle, const EOS_Auth_CopyIdTokenOptions *Options, EOS_Auth_IdToken **OutIdToken)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Auth_CopyIdToken");
        return ((EOS_EResult(EOS_CALL *)(EOS_HAuth, const EOS_Auth_CopyIdTokenOptions *, EOS_Auth_IdToken **))(address))(
            Handle, Options, OutIdToken);
    }
    return EOS_EResult::EOS_UnexpectedError;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_EpicAccountId_ToString(EOS_EpicAccountId AccountId, char *OutBuffer, int32_t *InOutBufferLength)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_EpicAccountId_ToString");
        return ((EOS_EResult(EOS_CALL *)(EOS_EpicAccountId, char *, int32_t *))(address))(AccountId, OutBuffer, InOutBufferLength);
    }
    return EOS_EResult::EOS_UnexpectedError;
}

EOS_DECLARE_FUNC(EOS_HAuth) EOS_Platform_GetAuthInterface(EOS_HPlatform Handle)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Platform_GetAuthInterface");
        return ((EOS_HAuth(EOS_CALL *)(EOS_HPlatform))(address))(Handle);
    }
    return nullptr;
}

EOS_DECLARE_FUNC(void) EOS_Platform_Tick(EOS_HPlatform Handle)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Platform_Tick");
        ((void(EOS_CALL *)(EOS_HPlatform))(address))(Handle);
    }
}

EOS_DECLARE_FUNC(EOS_HConnect) EOS_Platform_GetConnectInterface(EOS_HPlatform Handle)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Platform_GetConnectInterface");
        return ((EOS_HConnect(EOS_CALL *)(EOS_HPlatform))(address))(Handle);
    }
    return nullptr;
}

EOS_DECLARE_FUNC(EOS_HAchievements) EOS_Platform_GetAchievementsInterface(EOS_HPlatform Handle)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Platform_GetAchievementsInterface");
        return ((EOS_HAchievements(EOS_CALL *)(EOS_HPlatform))(address))(Handle);
    }
    return nullptr;
}

EOS_DECLARE_FUNC(void)
EOS_Achievements_UnlockAchievements(EOS_HAchievements Handle, const EOS_Achievements_UnlockAchievementsOptions *Options, void *ClientData,
                                    const EOS_Achievements_OnUnlockAchievementsCompleteCallback CompletionDelegate)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Achievements_UnlockAchievements");
        ((void(EOS_CALL *)(EOS_HAchievements, const EOS_Achievements_UnlockAchievementsOptions *, void *,
                           const EOS_Achievements_OnUnlockAchievementsCompleteCallback))(address))(Handle, Options, ClientData, CompletionDelegate);
    }
}

EOS_DECLARE_FUNC(void)
EOS_Connect_Login(EOS_HConnect Handle, const EOS_Connect_LoginOptions *Options, void *ClientData,
                                    const EOS_Connect_OnLoginCallback CompletionDelegate)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Connect_Login");
        ((void(EOS_CALL *)(EOS_HConnect, const EOS_Connect_LoginOptions *, void *, const EOS_Connect_OnLoginCallback))(address))(
            Handle, Options, ClientData, CompletionDelegate);
    }
}

EOS_DECLARE_FUNC(void) EOS_Auth_IdToken_Release(EOS_Auth_IdToken* IdToken)
{
    if (EOSHandle) {
        void *address = GetProcAddress(EOSHandle, "EOS_Auth_IdToken_Release");
        ((void(EOS_CALL *)(EOS_Auth_IdToken*))(address))(IdToken);
    }
}
#endif


void GetRedirectedPath(const char *path, char *out) { ModLoaderData->GetRedirectedPath(path, out); }

HOOK(HRESULT, __fastcall, D3D11CreateDevice, PROC_ADDRESS("d3d11.dll", "D3D11CreateDevice"),
    void *pAdapter, UINT DriverType, void* Software, UINT Flags, void *pFeatureLevels, UINT FeatureLevels,
    UINT SDKVersion, void **ppDevice, void *pFeatureLevel, void **ppImmediateContext)
{
    if (entryCalled)
        return originalD3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels,
            SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
    entryCalled = true;

#ifdef STEAM_API_NODLL
    if (!(steamAPIHandle = LoadLibraryA("steam_api64.dll")))
        steamAPIHandle = nullptr;
#endif
#if (RETRO_USERCORE_EOS && !EOS_USE_DLLEXPORT)
    if (!(EOSHandle = LoadLibraryA("EOSSDK-Win64-Shipping.dll")))
        EOSHandle = nullptr;
#endif
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
    auto argv = new const char*[1]{ "console=false" };
    
    RSDK::InitCoreAPI();

    int32 exitCode = RSDK::RunRetroEngine(argc, (char**)argv);

    RSDK::ReleaseCoreAPI();

    return E_FAIL;
}

#ifdef RETRO_USE_MOD_LOADER
HOOK(void, __fastcall, StateMachineRun, SigStateMachineRun(), void (**state)(void *), void *data)
{
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

    // Find epic token
    std::string args = GetCommandLineA();
    std::string token;
    size_t pos;
    if ((pos = args.find("-AUTH_PASSWORD=")) != std::string::npos) {
        token = args.substr(pos + 15);
        size_t endPos;
        if ((endPos = token.find_first_of(" ")) != std::string::npos)
            token = token.substr(0, endPos);
        if (epicToken = (const char*)malloc(token.size() + 1)) {
            memset((void*)epicToken, 0, token.size() + 1);
            memcpy((void *)epicToken, token.c_str(), token.size());
        }
    }

    ModLoaderData = modInfo->ModLoader;
}

#if RETRO_RENDERDEVICE_DIRECTX9 || RETRO_RENDERDEVICE_DIRECTX11
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DllHandle = hModule;
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
#endif