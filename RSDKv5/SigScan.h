#pragma once

extern bool SigValid;
extern const char* InvalidSig;

extern void *SigusePathTracer();
extern void *SigLinkGameLogic();
extern void *SigNukeSystemReq();
extern void *SigEngine_LoadFile();
extern void *SigStateMachineRun();

extern void *SigProcessEngine_0A();
extern void *SigRunCore_0A();


extern void *SigAVXPatch();
