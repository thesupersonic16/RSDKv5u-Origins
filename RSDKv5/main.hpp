#ifndef MAIN_H
#define MAIN_H

#if !RETRO_STANDALONE
#ifdef _MSC_VER
#define DLLExport __declspec(dllexport)
#else
#define DLLExport
#endif

extern char modPath[MAX_PATH];
#if RETRO_RENDERDEVICE_DIRECTX9 || RETRO_RENDERDEVICE_DIRECTX11
extern HMODULE DllHandle;
#endif

extern "C" {
DLLExport int32 RSDK_main(int32 argc, char **argv, void *linkLogicPtr);
}
#else
int32 RSDK_main(int32 argc, char **argv, void *linkLogicPtr);
#endif

#endif // !ifdef MAIN_H