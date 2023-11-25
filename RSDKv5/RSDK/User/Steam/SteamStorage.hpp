#if RETRO_REV02
struct SteamUserStorage : UserStorage {
    int32 TryAuth();
    int32 TryInitStorage();
    bool32 GetUsername(String *userName);
    bool32 TryLoadUserFile(const char *filename, void *buffer, uint32 size, void (*callback)(int32 status));
    bool32 TrySaveUserFile(const char *filename, void *buffer, uint32 size, void (*callback)(int32 status), bool32 compressed);
    bool32 TryDeleteUserFile(const char *filename, void (*callback)(int32 status));
    void ClearPrerollErrors() {}
};

#endif