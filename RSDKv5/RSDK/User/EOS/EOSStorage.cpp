#if RETRO_REV02

EOSUserStorage::EOSUserStorage(EOSCore* eosCore)
{
    core = eosCore;
    // Generate base path
    char userProfile[0x40];
    ExpandEnvironmentStringsA("%USERPROFILE%", userProfile, sizeof(userProfile));
    sprintf_s(basePath, "%s\\AppData\\Roaming\\SEGA\\SonicOrigins\\epic\\%s\\game\\", userProfile, core->accountId);
}

int32 EOSUserStorage::TryAuth()
{
    authStatus = core->productUserId ? STATUS_OK : STATUS_ERROR;
    return authStatus;
}
int32 EOSUserStorage::TryInitStorage()
{
    storageStatus = STATUS_OK;
    return storageStatus;
}

bool32 EOSUserStorage::GetUsername(RSDK::String *name)
{
    InitString(name, "IntegerGeorge802", 0);
    return true;
}

bool32 EOSUserStorage::TryLoadUserFile(const char *filename, void *buffer, uint32 size, void (*callback)(int32 status))
{
    char filePath[MAX_PATH];
    memset(buffer, 0, size);
    sprintf_s(filePath, "%s%s", basePath, filename);

    FILE *handle      = NULL;
    errno_t errorCode = fopen_s(&handle, filePath, "rb");
    if (!errorCode) {
        // Opened
        fread_s(buffer, size, 1, size, handle);
        fclose(handle);
        if (callback)
            callback(STATUS_OK);
    }
    else if (errorCode == ENOENT) {
        if (callback)
            callback(STATUS_NOTFOUND);
    }
    else {
        if (callback)
            callback(STATUS_ERROR);
    }

    return true;
}

bool32 EOSUserStorage::TrySaveUserFile(const char *filename, void *buffer, uint32 size, void (*callback)(int32 status), bool32 compressed)
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

bool32 EOSUserStorage::TryDeleteUserFile(const char *filename, void (*callback)(int32 status))
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
