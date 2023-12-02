#if RETRO_REV02
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
    bool32 success = false;
    memset(buffer, 0, size);

    if (!SteamRemoteStorage()->FileExists(filename))
    {
        if (callback)
            callback(STATUS_NOTFOUND);
        return true;
    }

    int fileSize = SteamRemoteStorage()->GetFileSize(filename);
    if (fileSize > size)
    {
        std::string str = __FILE__;
        str += ": TryLoadUserFile() # TryLoadUserFile(";
        str += filename;
        str += ", ...) file is larger than buffer \r\n";
        RSDK::PrintLog(PRINT_NORMAL, str.c_str());
    }
    else
    {
        size = fileSize;
    }

    int bytesRead = SteamRemoteStorage()->FileRead(filename, buffer, size);

    // Check if any bytes are read
    success = bytesRead != 0;
    if (callback)
        callback(success ? STATUS_OK : STATUS_ERROR);

    return success;
}

bool32 SteamUserStorage::TrySaveUserFile(const char *filename, void *buffer, uint32 size, void (*callback)(int32 status), bool32 compressed)
{
    bool32 success = SteamRemoteStorage()->FileWrite(filename, buffer, size);

    if (callback)
        callback(success ? STATUS_OK : STATUS_ERROR);

    return success;
}

bool32 SteamUserStorage::TryDeleteUserFile(const char *filename, void (*callback)(int32 status))
{
    bool32 success = SteamRemoteStorage()->FileDelete(filename);

    if (callback)
        callback(success ? STATUS_OK : STATUS_ERROR);

    return success;
}
#endif
