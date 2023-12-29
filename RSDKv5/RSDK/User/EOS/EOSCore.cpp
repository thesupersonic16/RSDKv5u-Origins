#include "RSDK/Core/RetroEngine.hpp"
#include "eos/eos_init.h"
#include "eos/eos_sdk.h"
#include "eos/eos_auth.h"
#include "eos/eos_ecom.h"
#include "eos/eos_achievements.h"
#if RETRO_REV02

// TODO: Create user if EOS_InvalidUser
void EOSConnectCallback(const EOS_Connect_LoginCallbackInfo* info)
{
    EOSCore *core = (EOSCore *)info->ClientData;
    if (info->ResultCode != EOS_EResult::EOS_Success) {
        printf("EOS connect failed: ResultCode=%d\n", (int)info->ResultCode);
        core->init = true;
        return;
    }

    printf("EOS profile login completed!\n");

    core->productUserId = info->LocalUserId;
    core->init          = true;
}

void EOSDLCCallback(const EOS_Ecom_QueryOwnershipCallbackInfo *info)
{
    EOSCore *core = (EOSCore *)info->ClientData;
    if (info->ResultCode != EOS_EResult::EOS_Success) {
        printf("EOS ownership check failed: ResultCode=%d\n", (int)info->ResultCode);
        core->init = true;
        return;
    }

    if (info->ItemOwnership[0].OwnershipStatus == EOS_EOwnershipStatus::EOS_OS_Owned)
    {
        printf("You own the Plus DLC.\n");
        core->hasPlusDLC = true;
    }
    else
    {
        printf("You do not own Plus :(\n");
        core->hasPlusDLC = false;
    }
}

void EOSLoginCallback(const EOS_Auth_LoginCallbackInfo *info)
{
    EOSCore *core = (EOSCore *)info->ClientData;
    if (info->ResultCode != EOS_EResult::EOS_Success) {
        printf("EOS login failed: ResultCode=%d\n", (int)info->ResultCode);
        core->accountId = nullptr;
        core->init      = true;
        return;
    }

    EOS_Auth_CopyUserAuthTokenOptions options { };
    options.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;

    EOS_Auth_CopyUserAuthToken((EOS_HAuth)core->authHandle, &options, info->LocalUserId, (EOS_Auth_Token **)&core->authToken);

    RSDK::AllocateStorage((void**)&core->accountId, 128, RSDK::DATASET_STR, true);
    int32_t bufferLength = 128;

    EOS_EpicAccountId_ToString(info->LocalUserId, core->accountId, &bufferLength);
    printf("EOS login completed!\n");

    // Check for plus
    core->ecomHandle = EOS_Platform_GetEcomInterface((EOS_HPlatform)core->platformHandle);
    EOS_Ecom_CatalogItemId plusDLC = "7d69ac8dfeaf42d7ba461ad37c953a0a"; 
    EOS_Ecom_QueryOwnershipOptions ownershipOptions { };
    ownershipOptions.ApiVersion = EOS_ECOM_QUERYOWNERSHIP_API_LATEST;
    ownershipOptions.CatalogItemIds = &plusDLC;
    ownershipOptions.CatalogItemIdCount = 1;
    ownershipOptions.CatalogNamespace = "78705aae6f39495e920966615c7a22ae";
    ownershipOptions.LocalUserId        = info->LocalUserId;
    EOS_Ecom_QueryOwnership((EOS_HEcom)core->ecomHandle, &ownershipOptions, core, EOSDLCCallback);

    // Login to user profile
    core->connectHandle = EOS_Platform_GetConnectInterface((EOS_HPlatform)core->platformHandle);

    EOS_Auth_CopyIdTokenOptions tokenOptions { };
    tokenOptions.ApiVersion = EOS_AUTH_COPYIDTOKEN_API_LATEST;
    tokenOptions.AccountId  = info->LocalUserId;
    EOS_Auth_IdToken *idToken;
    EOS_Auth_CopyIdToken((EOS_HAuth)core->authHandle, &tokenOptions, &idToken);

    EOS_Connect_Credentials connectLoginCredentials { };
    connectLoginCredentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
    connectLoginCredentials.Token = idToken->JsonWebToken;
    connectLoginCredentials.Type  = EOS_EExternalCredentialType::EOS_ECT_EPIC_ID_TOKEN;

    EOS_Connect_LoginOptions connectLoginOptions { };
    connectLoginOptions.ApiVersion    = EOS_CONNECT_LOGIN_API_LATEST;
    connectLoginOptions.Credentials   = &connectLoginCredentials;
    connectLoginOptions.UserLoginInfo = nullptr;
    EOS_Connect_Login((EOS_HConnect)core->connectHandle, &connectLoginOptions, core, EOSConnectCallback);
    EOS_Auth_IdToken_Release(idToken);

    // Prepare Achievements
    core->achievementsHandle = EOS_Platform_GetAchievementsInterface((EOS_HPlatform)core->platformHandle);
}

void EOSLoginUsingSession(EOSCore *core)
{
    core->authHandle = EOS_Platform_GetAuthInterface((EOS_HPlatform)core->platformHandle);
    EOS_Auth_Credentials authCredentials{};
    authCredentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
    authCredentials.Type       = EOS_ELoginCredentialType::EOS_LCT_ExchangeCode;
    authCredentials.Token      = epicToken;

    EOS_Auth_LoginOptions loginOptions{};
    loginOptions.ApiVersion  = EOS_AUTH_LOGIN_API_LATEST;
    loginOptions.Credentials = &authCredentials;

    EOS_Auth_Token *eosToken;

    EOS_Auth_Login((EOS_HAuth)core->authHandle, &loginOptions, core, EOSLoginCallback);
}

EOSCore *InitEOSCore()
{
    // Initalize API subsystems
    EOSCore *core = new EOSCore;

    // Init API
    EOS_InitializeOptions initOptions { };
    initOptions.ApiVersion            = EOS_INITIALIZE_API_LATEST;
	initOptions.ProductName           = "SonicOrigins";
	initOptions.ProductVersion        = "1.00";
    EOS_Initialize(&initOptions);

    // Init Platform
    EOS_Platform_Options platformOptions { };
    platformOptions.ApiVersion                     = EOS_PLATFORM_OPTIONS_API_LATEST;
    platformOptions.ProductId                      = "e56eb08284a7440a81c91d447285c8c1";
    platformOptions.SandboxId                      = "78705aae6f39495e920966615c7a22ae";
    platformOptions.ClientCredentials.ClientId     = "xyza7891whGBaKUSXSUzaJpZYhHsQ04V";
    platformOptions.ClientCredentials.ClientSecret = "yQ3jEKlOUkdViaYMhJ4CpaUmnX+HsnR7XCNGekBqk/I";
    platformOptions.EncryptionKey                  = "761c91978509cdcca583b819ed6793e900b0fe1ba9dfef6d066de5887ad55c43";
    platformOptions.DeploymentId                   = "f4767199a65e423593fa5be59de78b95";
    core->platformHandle                           = EOS_Platform_Create(&platformOptions);

    // Login
    EOSLoginUsingSession(core);

    // I don't think I should be doing this
    while (!core->init) EOS_Platform_Tick((EOS_HPlatform)core->platformHandle);

    if (achievements)
        delete achievements;
    achievements = new EOSAchievements;

    if (leaderboards)
        delete leaderboards;
    leaderboards = new EOSLeaderboards;

    if (richPresence)
        delete richPresence;
    richPresence = new EOSRichPresence;

    if (stats)
        delete stats;
    stats = new EOSStats;

    if (userStorage)
        delete userStorage;
    userStorage = new EOSUserStorage(core);

    return core;
}

void EOSCore::Shutdown()
{
    EOS_Platform_Release((EOS_HPlatform)platformHandle);
    EOS_Shutdown();
}

void EOSCore::FrameInit() { EOS_Platform_Tick((EOS_HPlatform)platformHandle); }

#endif
