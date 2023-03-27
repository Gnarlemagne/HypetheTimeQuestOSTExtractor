
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include "_snd_dll_link.h"

static LPFEEDDATA TMP_FeedData;

/* ------------------------------------------------------------------------
		Hooked routines and stubs
	 ------------------------------------------------------------------------ */

HMODULE WINAPI _SND_fn_vGetHModuleMpeg(void)
{
	return ((HMODULE)0xF0E9);
}

HMODULE WINAPI _SND_fn_vGetHModuleWav(void)
{
	return ((HMODULE)0xF0AE);
}

HMODULE WINAPI _SND_fn_vGetHModuleCd(void)
{
	return ((HMODULE)0xF0CD);
}

HMODULE WINAPI _SND_fn_vGetHModuleAdpcm(void)
{
	return ((HMODULE)0xF0AD);
}

HMODULE WINAPI _SND_fn_vGetHModuleDbg(void)
{
	return ((HMODULE)0xF0DB);
}

HMODULE WINAPI _SND_fn_vGetHModuleRecord(void)
{
	return ((HMODULE)0xF0EC);
}

/* ------------------------------------------------------------------------
		Hook and library loader routines
	 ------------------------------------------------------------------------ */

DWORD *GetImportProcAddr(HMODULE hmod, CCHAR *slib, CCHAR *sprc)
{
	IMAGE_DOS_HEADER *dh;
	IMAGE_NT_HEADERS *nt;
	IMAGE_IMPORT_DESCRIPTOR *id;
	IMAGE_THUNK_DATA *tdo, *tdm;
	BYTE *p;
	if ((!hmod) || (!slib) || (!sprc))
	{
		return (NULL);
	}
	p = (BYTE *)hmod;
	dh = (IMAGE_DOS_HEADER *)p;
	if (dh->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return (NULL);
	}
	nt = (IMAGE_NT_HEADERS *)&p[dh->e_lfanew];
	if ((nt->Signature != IMAGE_NT_SIGNATURE) || (nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC))
	{
		return (NULL);
	}
	id = (IMAGE_IMPORT_DESCRIPTOR *)&p[nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress];
	while (id->Name)
	{
		if (!lstrcmpiA((CCHAR *)&p[id->Name], slib))
		{
			tdo = (IMAGE_THUNK_DATA *)&p[(DWORD)id->OriginalFirstThunk];
			tdm = (IMAGE_THUNK_DATA *)&p[(DWORD)id->FirstThunk];
			while (tdo->u1.Ordinal)
			{
				if (tdo->u1.Ordinal & IMAGE_ORDINAL_FLAG)
				{
					if (tdo->u1.Ordinal == (DWORD)sprc)
					{
						return ((DWORD *)tdm);
					}
				}
				else
				{
					if (!lstrcmpiA((CCHAR *)&p[(DWORD)((IMAGE_IMPORT_BY_NAME *)((DWORD)tdo->u1.AddressOfData))->Name], sprc))
					{
						return ((DWORD *)tdm);
					}
				}
				tdo++;
				tdm++;
			}
		}
		id++;
	}
	return (NULL);
}

void MemWrite(void *d, void *s, DWORD l)
{
	DWORD dx;
	if (VirtualProtect(d, l, PAGE_READWRITE, &dx))
	{
		CopyMemory(d, s, l);
		VirtualProtect(d, l, dx, &dx);
		FlushInstructionCache(GetCurrentProcess(), d, l);
	}
}

BOOL WINAPI _SND_fn_bTestSnd_MMX(void)
{
	return (TRUE);
}

BOOL WINAPI _SND_fn_bTestSnd_WinMM(DWORD parm)
{
	return (TRUE);
}

void WINAPI _SND_fn_vDisplayError(DWORD code, CCHAR *text)
{
#ifdef DEBUG_LOGINFO
	DebugOut("ERROR-%08X: %s", code, text);
#endif
}

void WINAPI *_SND_fn_pvMallocSndEx(DWORD size1, DWORD size2)
{
	return ((void *)LocalAlloc(LMEM_FIXED, size2));
}

void WINAPI *_SND_fn_pvMallocSnd(DWORD size)
{
	return ((void *)LocalAlloc(LMEM_FIXED, size));
}

void WINAPI _SND_fn_vFreeSnd(void *ptr)
{
	LocalFree(ptr);
}

void WINAPI _SND_fn_vFreeSndEx(void *p1, void *p2)
{
	LocalFree(p2);
}

void WINAPI _SND_fn_vResolveFileName(CCHAR *s, CCHAR *d)
{
	lstrcpyA(d, s);
}

HANDLE WINAPI _SND_fn_hOpenFileReadSnd(const CCHAR *s)
{
	return (CreateFile(s, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0));
}

void WINAPI _SND_fn_vCloseFileSnd(HANDLE hFile)
{
	CloseHandle(hFile);
}

DWORD WINAPI _SND_fn_dwSeekFileSnd(HANDLE hFile, LONG lDistanceToMove, DWORD dwMoveMethod)
{
	return (SetFilePointer(hFile, lDistanceToMove, NULL, dwMoveMethod));
}

DWORD WINAPI _SND_fn_dwReadFileSnd(HANDLE hFile, DWORD nNumberOfBytesToRead, LPVOID lpBuffer)
{
	DWORD NumberOfBytesRead;
	NumberOfBytesRead = 0;
	ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &NumberOfBytesRead, NULL);
	return (NumberOfBytesRead);
}


DWORD WINAPI _SND_fn_lCreateNewBufferExSxd(int p1, DWORD *p2, int p3, int p4)
{
	TMP_FeedData = (LPFEEDDATA)p2[1];
	return (0xC0DE);
}

DWORD WINAPI _SND_fn_rGetPosBufferSxd(int p1)
{
	return (0);
}

void WINAPI _SND_fn_vDeleteBufferSxd(int p1)
{
}

BOOL WINAPI _SND_fn_bLoadDataInMem(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPCSTR lpFileName, LONG lDistanceToMove)
{
	HANDLE h;
	h = _SND_fn_hOpenFileReadSnd(lpFileName);
	if (h != INVALID_HANDLE_VALUE)
	{
		_SND_fn_dwSeekFileSnd(h, lDistanceToMove, FILE_BEGIN);
		nNumberOfBytesToRead ^= _SND_fn_dwReadFileSnd(h, nNumberOfBytesToRead, lpBuffer);
		_SND_fn_vCloseFileSnd(h);
	}
	return ((h != INVALID_HANDLE_VALUE) && (!nNumberOfBytesToRead));
}

void WINAPI _SND_fn_vParam3Dto2D(void *p1, void *p2)
{
	/* just copy MasterVolume back */
	CopyMemory(p2, p1, 8);
}

static prc_item loadlist[] = {
	{"_SND_fn_lCreateNewBufferExSxd@16", (void *)_SND_fn_lCreateNewBufferExSxd},
	{"_SND_fn_rGetPosBufferSxd@4", (void *)_SND_fn_rGetPosBufferSxd},
	{"_SND_fn_vDeleteBufferSxd@4", (void *)_SND_fn_vDeleteBufferSxd},
	{"_SND_fn_vDisplayError@8", (void *)_SND_fn_vDisplayError},
	{"_SND_fn_vGetHModuleMpeg@0", (void *)_SND_fn_vGetHModuleMpeg},
	{"_SND_fn_vGetHModuleWav@0", (void *)_SND_fn_vGetHModuleWav},
	{"_SND_fn_vGetHModuleCd@0", (void *)_SND_fn_vGetHModuleCd},
	{"_SND_fn_vGetHModuleAdpcm@0", (void *)_SND_fn_vGetHModuleAdpcm},
	{"_SND_fn_vGetHModuleDbg@0", (void *)_SND_fn_vGetHModuleDbg},
	{"_SND_fn_vGetHModuleRecord@0", (void *)_SND_fn_vGetHModuleRecord},
	{"_SND_fn_bTestSnd_MMX@0", (void *)_SND_fn_bTestSnd_MMX},
	{"_SND_fn_bTestSnd_WinMM@4", (void *)_SND_fn_bTestSnd_WinMM},
	{"_SND_fn_pvMallocSndEx@8", (void *)_SND_fn_pvMallocSndEx},
	{"_SND_fn_pvMallocSnd@4", (void *)_SND_fn_pvMallocSnd},
	{"_SND_fn_vFreeSnd@4", (void *)_SND_fn_vFreeSnd},
	{"_SND_fn_vFreeSndEx@8", (void *)_SND_fn_vFreeSndEx},
	{"_SND_fn_vResolveFileName@8", (void *)_SND_fn_vResolveFileName},
	{"_SND_fn_hOpenFileReadSnd@4", (void *)_SND_fn_hOpenFileReadSnd},
	{"_SND_fn_dwReadFileSnd@12", (void *)_SND_fn_dwReadFileSnd},
	{"_SND_fn_dwSeekFileSnd@12", (void *)_SND_fn_dwSeekFileSnd},
	{"_SND_fn_vCloseFileSnd@4", (void *)_SND_fn_vCloseFileSnd},
	{"_SND_fn_bLoadDataInMem@16", (void *)_SND_fn_bLoadDataInMem},
	{"_SND_fn_vParam3Dto2D@8", (void *)_SND_fn_vParam3Dto2D},
	{NULL, NULL}};

/* a neat way to have list of all procs in one place using DebugView by SysInternals
	 so when any of these was called an error with proc address will be shown by system
	 and since all address below 0x10000 invalid in Win32 all called proc names will be exposed */
static DWORD dwLoadNum = 0;

DWORD WINAPI Hook_GetProcAddress(HMODULE hmod, char *sprc)
{
	DWORD p;
	/* not stub and not our module */
	if (HIWORD(hmod) && (hmod != GetModuleHandle(NULL)))
	{
		return ((DWORD)GetProcAddress(hmod, sprc));
	}
	/* this proc always first - use to split logs */
	if (!lstrcmpiA(sprc, loadlist[0].name))
	{
		dwLoadNum = (dwLoadNum + 0x1000) & 0xF000;
	}
	/* determine if this proc should be really hooked */
	dwLoadNum++;
	for (p = 0; loadlist[p].name; p++)
	{
		if (!lstrcmpiA(sprc, loadlist[p].name))
		{
			return ((DWORD)loadlist[p].proc);
		}
	}
	return (dwLoadNum);
}

/* ----------------------------- DLL Loading API -------------------------- */
void InitSoundDecoder(CCHAR *slib)
{
	HMODULE hlib;
	DWORD *pold, *pnew;
	if (!slib)
	{
		return;
	}
	hlib = GetModuleHandle(slib);
	if (!hlib)
	{
		return;
	}
	/* hook kernel proc instead of fixed decoder loader proc address
		 this will allow to use any version of decoder libraries */
	pold = GetImportProcAddr(hlib, "KERNEL32.dll", "GetProcAddress");
	if (!pold)
	{
		return;
	}
	pnew = (void *)Hook_GetProcAddress;
	MemWrite(pold, &pnew, 4);
}


LPFEEDDATA InitAPMDecoder()
{
	HMODULE hm;
	LPFEEDDATA FeedData;
	/* init sound modules */
	hm = GetModuleHandle(NULL);
	/* ADPCM */
	printf("- Init APM decoder...");
	InitSoundDecoder("APMmxBVR.dll");
	if (!_SND_fn_iInitSxdADPCM(&hm))
	{
		printf("error\n\n");
		return;
	}
	printf("ok\n");
	FeedData = TMP_FeedData;
	_SND_fn_vReleaseDriverSxdADPCM();
	_SND_fn_vRestoreDriverSxdADPCM();
	return FeedData;
}

LPFEEDDATA InitMPEGDecoder()
{
	HMODULE hm;
	LPFEEDDATA FeedData;
	_SND_fn_vReleaseDriverSxdADPCM();
	_SND_fn_vRestoreDriverSxdADPCM();
	/* MPEG */
	printf("- Init MPG decoder...");
	InitSoundDecoder("MPGMXBVR.dll");
	if (!_SND_fn_iInitMPEG(&hm))
	{
		_SND_fn_vDesInitSxdADPCM();
		printf("error\n\n");
		return;
	}
	_SND_fn_vReleaseDriverMPEG();
	_SND_fn_vRestoreDriverMPEG();
	FeedData = TMP_FeedData;
	printf("ok\n\n");
	return FeedData;
}

void UnloadDecoders()
{
	/* unload decoders */
	_SND_fn_vDesInitMPEG();
	_SND_fn_vDesInitSxdADPCM();
}