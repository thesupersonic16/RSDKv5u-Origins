#if RETRO_REV02
struct EOSUserStorage : UserStorage {
    char basePath[MAX_PATH];
    EOSCore *core = nullptr;
    
    EOSUserStorage(EOSCore *eosCore);
    int32 TryAuth();
    int32 TryInitStorage();
    bool32 GetUsername(RSDK::String *name);
    bool32 TryLoadUserFile(const char *filename, void *buffer, uint32 size, void (*callback)(int32 status));
    bool32 TrySaveUserFile(const char *filename, void *buffer, uint32 size, void (*callback)(int32 status), bool32 compressed);
    bool32 TryDeleteUserFile(const char *filename, void (*callback)(int32 status));
    void ClearPrerollErrors() {}
};

#endif