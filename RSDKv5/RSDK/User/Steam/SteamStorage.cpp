#if RETRO_REV02

SteamUserStorage::SteamUserStorage()
{
    // Generate base path
    char userProfile[0x40];
    CSteamID steamID = SteamUser()->GetSteamID();
    ExpandEnvironmentStringsA("%USERPROFILE%", userProfile, sizeof(userProfile));
    sprintf_s(basePath, "%s\\AppData\\Roaming\\SEGA\\SonicOrigins\\steam\\%llu\\game\\", userProfile, steamID.ConvertToUint64());
}

int32 SteamUserStorage::TryAuth()
{
    authStatus = SteamUser()->BLoggedOn() ? STATUS_OK : STATUS_ERROR;
    return authStatus;
}
int32 SteamUserStorage::TryInitStorage()
{
    storageStatus = STATUS_OK;
    return storageStatus;
}

bool32 SteamUserStorage::GetUsername(RSDK::String *name)
{
    const char *personaName = SteamFriends()->GetPersonaName();
    if (strlen(personaName) > 0)
        InitString(name, personaName, 0);
    else
        InitString(name, "Invalid Steam User", 0);
    return true;
}

bool32 SteamUserStorage::TryLoadUserFile(const char *filename, void *buffer, uint32 size, void (*callback)(int32 status))
{
    char filePath[MAX_PATH];
    memset(buffer, 0, size);
    sprintf_s(filePath, "%s%s", basePath, filename);

    FILE *handle      = NULL;
    errno_t errorCode = fopen_s(&handle, filePath, "rb");
    if (!errorCode)
    {
        // Opened
        fread_s(buffer, size, 1, size, handle);
        fclose(handle);
        if (callback)
            callback(STATUS_OK);
    }
    else if (errorCode == ENOENT)
    {
        if (callback)
            callback(STATUS_NOTFOUND);
    }
    else
    {
        if (callback)
            callback(STATUS_ERROR);        
    }

    return true;
}

bool32 SteamUserStorage::TrySaveUserFile(const char *filename, void *buffer, uint32 size, void (*callback)(int32 status), bool32 compressed)
{
    char filePath[MAX_PATH];
    sprintf_s(filePath, "%s%s", basePath, filename);

    FILE *handle;
    errno_t errorCode = fopen_s(&handle, filePath, "wb");
    if (!errorCode) {
        // Opened
        fwrite(buffer, 1, size, handle);
        fclose(handle);
        if (callback)
            callback(STATUS_OK);
    }
    else {
        if (callback)
            callback(STATUS_ERROR);
    }

    return true;
}

bool32 SteamUserStorage::TryDeleteUserFile(const char *filename, void (*callback)(int32 status))
{
    char filePath[MAX_PATH];
    sprintf_s(filePath, "%s%s", basePath, filename);

    FILE *handle      = NULL;
    errno_t errorCode = remove(filePath);
    if (!errorCode) {
        if (callback)
            callback(STATUS_OK);
    }
    else {
        if (callback)
            callback(STATUS_ERROR);
    }

    return true;
}
#endif
