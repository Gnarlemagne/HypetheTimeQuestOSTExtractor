
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>

#ifndef __UBI_BNM_STRUCTS

typedef struct
{
	DWORD dwRIFF;
	DWORD dwRIFFSize;
	DWORD dwWAVE;
	DWORD dwfmt_;
	DWORD dwfmt_Size;
	PCMWAVEFORMAT pwf;
	DWORD dwdata;
	DWORD dwdataSize;
} wav_head;


typedef struct
{
	WORD TableGroupID;
	WORD SoundGroupID;
	DWORD ItemStatus;		/* 1 - exists; 2 - deleted (ignore this item); */
	WORD Position;		 /* number in bnm_entry list */
	WORD BnkGrpID;		 /* bank number; can point to the EXTERNAL bank (Bnk_2.bnm => Bnk_13.bnm) */
	DWORD Unknown[3];	/* unused filename 8.3 buffer (?) */
	DWORD DataFlag[2]; /* usually 1, 1 */
} table_item;

typedef struct
{
	DWORD unused;
	DWORD table_offset; /* 0x24 - MPEG (?); 0x2C - ADPCM (?); see table_item below for table decription */
	DWORD table_count; /* The number of data tables */
	DWORD content_offset; /* The offset until the start of the bnm_entry content */
	DWORD bnm_count; /* The total number of entries in the bank file */
	DWORD offslist[6]; /* An offset - usually just stored at offslist[0], not sure why the other 5 are needed  */
} bnm_header;

typedef struct
{
	WORD SoundResourceID;
	WORD SoundGroupID;
	DWORD DataType; /* 1 = WAVE, 7 = Theme (multiple files will be merged) */
	DWORD Internal; /* 0 or 1 */
	DWORD DataSize; /* The size of the data (excluding header) - may not be used on MPEG files in favor of calculating size */
	DWORD DataOffs; /* The offset from the start of the entry to where the data for the decoder begins */
	BYTE SoundVol; /* Volume */
	BYTE Align[3];	/* unused, align to 32 bit */
	DWORD Flags[5];  /* If DataType is 7, the next <Flag[1]> entries will be merged with this*/
	DWORD FileAbsoluteOffset; /* 1/0 absolute/relative offset (only when Internal non-zero) */
	DWORD Unused1;
	DWORD LoopFrom;
	DWORD LoopTill;
	DWORD SoundHz;
	WORD BitsPSam;
	WORD Channels;
	DWORD ZipFormat; /* 1 = SAMPLE_PCM; 2 - SAMPLE_MPEG; 4,5 - ADPCM */
	CCHAR Name[16];
	DWORD Unused2;
} bnm_entry;


typedef struct
{
	WORD Unknown1; /* 0x2000 (?) */
	WORD Channels;
	DWORD SoundHz;
	DWORD BytePSec;		 /* Channels * SoundHz * 2 (16 bits) */
	WORD Unknown2;			/* 1 (?) */
	WORD Unknown3;			/* 4 (?) */
	DWORD HeadSize;		 /* include this */
	DWORD Signvs12;		 /* always "vs12" - codec id (?) */
	DWORD DataSize;		 /* whole .APM file	*/
	DWORD nSamples;		 /* total samples, usually (((DataSize - sizeof(apm_head)) * 2) / Channels) */
	DWORD Unknown5[3];	/* usually -1, 0, 0 */
	DWORD Coeffs[2][3]; /* for each channel, for mono only first one used */
	DWORD Unknown6[7];
	DWORD SignData; /* always "DATA" with packed stream followed */
} apm_head;

/* mapping file helper */
typedef struct
{
	HANDLE hFile;
	HANDLE hMap;
	BYTE* pbMem;
	DWORD lSize;
} map_file;

#define __UBI_BNM_STRUCTS
#endif