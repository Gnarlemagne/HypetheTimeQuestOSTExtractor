
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include "tonictac_lib.h"

#ifndef _SND_DLL_LINK
#define _SND_DLL_LINK

/* ADPCM */
INT WINAPI _SND_fn_iInitSxdADPCM(HINSTANCE* hmod);
void WINAPI _SND_fn_vRestoreDriverSxdADPCM(void);
void WINAPI _SND_fn_vReleaseDriverSxdADPCM(void);
BOOL WINAPI _SND_fn_bLoadResBinarySxdADPCM(bnm_entry* bank_entry, void* parm, DWORD offs);
void WINAPI _SND_fn_vUnLoadResSxdADPCM(void* parm);
void WINAPI* _SND_fn_lPlaySxdADPCM(void* data, BYTE* vol, DWORD unused, void* proc, void* procdata);
void WINAPI _SND_fn_vStopSxdADPCM(void* data);
void WINAPI _SND_fn_vDesInitSxdADPCM(void);

/* MPEG */
INT WINAPI _SND_fn_iInitMPEG(HINSTANCE* hmod);
void WINAPI _SND_fn_vRestoreDriverMPEG(void);
void WINAPI _SND_fn_vReleaseDriverMPEG(void);
BOOL WINAPI _SND_fn_bLoadResBinaryMPEG(bnm_entry* bank_entry, void* parm, DWORD offs);
void WINAPI _SND_fn_vUnLoadResMPEG(void* parm);
void WINAPI* _SND_fn_lPlayMPEG(void* data, BYTE* vol, DWORD unused, void* proc, void* procdata);
void WINAPI _SND_fn_vStopMPEG(void* data);
void WINAPI _SND_fn_vDesInitMPEG(void);

typedef BOOL(WINAPI *LPFEEDDATA)(int unused1, int unused2, DWORD size, BYTE *data);

/* ------------------------------------------------------------------------
		Hooked routines and stubs
	 ------------------------------------------------------------------------ */

HMODULE WINAPI _SND_fn_vGetHModuleMpeg(void);
HMODULE WINAPI _SND_fn_vGetHModuleWav(void);
HMODULE WINAPI _SND_fn_vGetHModuleCd(void);
HMODULE WINAPI _SND_fn_vGetHModuleAdpcm(void);
HMODULE WINAPI _SND_fn_vGetHModuleDbg(void);
HMODULE WINAPI _SND_fn_vGetHModuleRecord(void);

/* ------------------------------------------------------------------------
		Hook and library loader routines
	 ------------------------------------------------------------------------ */

DWORD *GetImportProcAddr(HMODULE hmod, CCHAR *slib, CCHAR *sprc);
void MemWrite(void *d, void *s, DWORD l);

BOOL WINAPI _SND_fn_bTestSnd_MMX(void);
BOOL WINAPI _SND_fn_bTestSnd_WinMM(DWORD parm);
void WINAPI _SND_fn_vDisplayError(DWORD code, CCHAR *text);
void WINAPI *_SND_fn_pvMallocSndEx(DWORD size1, DWORD size2);
void WINAPI *_SND_fn_pvMallocSnd(DWORD size);
void WINAPI _SND_fn_vFreeSnd(void *ptr);
void WINAPI _SND_fn_vFreeSndEx(void *p1, void *p2);
void WINAPI _SND_fn_vResolveFileName(CCHAR *s, CCHAR *d);
HANDLE WINAPI _SND_fn_hOpenFileReadSnd(const CCHAR *s);
void WINAPI _SND_fn_vCloseFileSnd(HANDLE hFile);
DWORD WINAPI _SND_fn_dwSeekFileSnd(HANDLE hFile, LONG lDistanceToMove, DWORD dwMoveMethod);
DWORD WINAPI _SND_fn_dwReadFileSnd(HANDLE hFile, DWORD nNumberOfBytesToRead, LPVOID lpBuffer);
DWORD WINAPI _SND_fn_lCreateNewBufferExSxd(int p1, DWORD *p2, int p3, int p4);
DWORD WINAPI _SND_fn_rGetPosBufferSxd(int p1);
void WINAPI _SND_fn_vDeleteBufferSxd(int p1);
BOOL WINAPI _SND_fn_bLoadDataInMem(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPCSTR lpFileName, LONG lDistanceToMove);
void WINAPI _SND_fn_vParam3Dto2D(void *p1, void *p2);
typedef struct
{
	char *name;
	void *proc;
} prc_item;

DWORD WINAPI Hook_GetProcAddress(HMODULE hmod, char *sprc);

/* ----------------------------- DLL Loading API -------------------------- */
void InitSoundDecoder(CCHAR *slib);
LPFEEDDATA InitAPMDecoder();
LPFEEDDATA InitMPEGDecoder();
void UnloadDecoders();

#endif