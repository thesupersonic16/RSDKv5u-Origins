#pragma once

extern bool SigValid;
extern const char* InvalidSig;

extern void *sigScan(const char *signature, const char *mask, void *hint);

extern void *SigusePathTracer();
extern void *SigLinkGameLogic();
extern void *SigEngine_LoadFile();
extern void *SigStateMachineRun();

extern void *SigProcessEngine_0A();
extern void *SigRunCore_0A();

extern void *SigAVXPatch();

extern void *SigS3K_SS_Player_Input_Following();
