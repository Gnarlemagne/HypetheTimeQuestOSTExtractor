/*
	Modified work of CTPAX-X - See README.md for proper credits
	If you want to support this work, please consider supporting the actual CTPAX-X team who did the heavy lifting (www.CTPAX-X.org)
	If you go to their site and think it's gotta be a dead project with the way it looks, you're not the only one. That's just what their site looks like, they're alive and kickin'
*/

/*	ORIGINAL HEADER COMMENT
	If you like our work please visit www.CTPAX-X.org and consider a donation to support us.
	Any amount will be appreciated! All raised funds will go to pay our site hosting bills.
	We also accept BitCoins - write us feedback at site or send an e-mail at support for details.
	Thank you!
*/

#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include "tonictac_lib.h"
#include "_snd_dll_link.h"

#ifdef GCC32HACK
#include "win32gcc.h"
#endif

#define SAMPLE_PCM 1
#define SAMPLE_MPEG 2
#define SAMPLE_ADPCM1 4
#define SAMPLE_ADPCM2 5

#pragma pack(push, 1)

/* should be enough - it's always around 36893 samples */
#define LASTSIZE 44100
typedef struct
{
	wav_head head;
	/* add one sample for silence at the end */
	DWORD data[LASTSIZE + 1];
	DWORD size;
} end_data;

#pragma pack(pop)

/* float[4] (65536.0 (512.0 <<7 ) - max), char[1] (0x7F - max), char[1] (0x40 - max), unused[1], unused[1] */
static BYTE MasterVolume[8] = { 0x00, 0x00, 0x01, 0x00, 0x7F, 0x40, 0x00, 0x00 };
static char SndType[] = "PCM\0MPG\0APM";

/* ------------------------------------------------------------------------
		Bank files and sound decoding routines
	 ------------------------------------------------------------------------ */

void InitWaveHeader(wav_head* wh, DWORD sz, DWORD hz, WORD c, WORD b)
{
	memset(wh, 0, sizeof(wh[0]));
	wh->dwRIFF = MAKEFOURCC('R', 'I', 'F', 'F');
	wh->dwRIFFSize = sz + sizeof(wh[0]) - 8;
	wh->dwWAVE = MAKEFOURCC('W', 'A', 'V', 'E');
	wh->dwfmt_ = MAKEFOURCC('f', 'm', 't', ' ');
	wh->dwfmt_Size = sizeof(wh->pwf);
	wh->pwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	wh->pwf.wf.nChannels = c;
	wh->pwf.wf.nSamplesPerSec = hz;
	wh->pwf.wf.nBlockAlign = (b / 8) * c;
	wh->pwf.wf.nAvgBytesPerSec = wh->pwf.wf.nBlockAlign * hz;
	wh->pwf.wBitsPerSample = b;
	wh->dwdata = MAKEFOURCC('d', 'a', 't', 'a');
	wh->dwdataSize = sz;
}

BOOL FlRead(char* name, void* data, DWORD size, DWORD offs)
{
	HANDLE hFile;
	hFile = _SND_fn_hOpenFileReadSnd(name);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		_SND_fn_dwSeekFileSnd(hFile, offs, FILE_BEGIN);
		_SND_fn_dwReadFileSnd(hFile, size, data);
		_SND_fn_vCloseFileSnd(hFile);
	}
	return (hFile != INVALID_HANDLE_VALUE);
}

BOOL FlDump(char* name, void* data, DWORD size)
{
#ifndef DEBUG_NOWRITE
	HANDLE hFile;
	hFile = CreateFile(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, data, size, &size, NULL);
		CloseHandle(hFile);
	}
	return (hFile != INVALID_HANDLE_VALUE);
#else
	return (TRUE);
#endif
}

BOOL FlGlue(char* name, void* data, DWORD size, BOOL first)
{
#ifndef DEBUG_NOWRITE
	wav_head wh, * ws;
	HANDLE hFile;
	DWORD dw;
	BYTE* p;
	hFile = CreateFile(name, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, first ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		/* first file - write as is */
		if (first)
		{
			WriteFile(hFile, data, size, &dw, NULL);
		}
		else
		{
			/* file exists - append data */
			ws = (wav_head*)data;
			/* read and check header */
			memset(&wh, 0, sizeof(wh));
			_SND_fn_dwReadFileSnd(hFile, sizeof(wh), &wh);
			if (
				(ws->dwRIFF == wh.dwRIFF) && (ws->dwWAVE == wh.dwWAVE) && (ws->dwfmt_ == wh.dwfmt_) &&
				(ws->dwfmt_Size == wh.dwfmt_Size) && (ws->dwdata == wh.dwdata) &&
				(ws->pwf.wf.wFormatTag == wh.pwf.wf.wFormatTag) &&
				(ws->pwf.wf.nChannels == wh.pwf.wf.nChannels) &&
				(ws->pwf.wf.nSamplesPerSec == wh.pwf.wf.nSamplesPerSec) &&
				(ws->pwf.wf.nBlockAlign == wh.pwf.wf.nBlockAlign) &&
				(ws->pwf.wf.nAvgBytesPerSec == wh.pwf.wf.nAvgBytesPerSec) &&
				(ws->pwf.wBitsPerSample == wh.pwf.wBitsPerSample))
			{
				/* add data */
				size -= sizeof(wav_head);
				_SND_fn_dwSeekFileSnd(hFile, 0, FILE_END);
				p = (BYTE*)data;
				WriteFile(hFile, &p[sizeof(wav_head)], size, &dw, NULL);
				/* fix header */
				wh.dwRIFFSize += size;
				wh.dwdataSize += size;
				_SND_fn_dwSeekFileSnd(hFile, 0, FILE_BEGIN);
				WriteFile(hFile, &wh, sizeof(wh), &dw, NULL);
			}
			else
			{
				printf("Error: file merge data mismatch!\n");
			}
		}
		_SND_fn_vCloseFileSnd(hFile);
	}
	return (hFile != INVALID_HANDLE_VALUE);
#else
	return (TRUE);
#endif
}

void newext(CCHAR* name, CCHAR* ext)
{
	char* p;
	if (name && ext)
	{
		for (; (*name == '.'); name++)
			;
		for (p = NULL; *name; name++)
		{
			if (*name == '.')
			{
				p = name;
			}
		}
		lstrcpyA(p ? p : name, ext);
	}
}

char* basename(char* s)
{
	char* r;
	if (s)
	{
		for (r = s; *r; r++)
		{
			if ((*r == '/') || (*r == '\\'))
			{
				s = &r[1];
			}
		}
	}
	return (s);
}

/*
	tiny routine that needs some huge explanation
	decoders have some sort of internal buffer
	decoding a frame leaves small amount of data inside this buffer
	so when the next frame comes decoder can continue playing without any hiccup
	but if this buffer not dropped before next separate file starts playing
	bad things will happen:
	- new file starts with last data from previous file
	- since this new data at start shifts file back tail part will be lost
	solution: if it's single file or last block - drain internal buffer
	until it empty and save this as the last decoded block
	seems like only MPEG decoder have this buffer, since ADPCM always empty
	phew!
*/

/* buffer for last block */
static end_data lstblock;

void DevastateBuffer(LPFEEDDATA FeedProc, bnm_entry* bank_entry)
{
	DWORD i, l;
	lstblock.size = 0;
	if (FeedProc && bank_entry && (bank_entry->BitsPSam == 16))
	{
		l = 0;
		for (i = 0; i < LASTSIZE; i++)
		{
			lstblock.data[i] = 0;
			if (!FeedProc(0, 0, 1, (BYTE*)&lstblock.data[i]))
			{
				break;
			}
			/* size of non-silence part */
			if (lstblock.data[i])
			{
				l = i + 1;
			}
		}
		/* this shouldn't happend, but just in case */
		while (FeedProc(0, 0, 1, (BYTE*)&i))
			;
		/* at least one non-zero sample */
		if (l)
		{
			/* add last sample for silence */
			lstblock.data[l] = 0;
			l++;
			/* block size */
			lstblock.size = l * 4;
			/* init wave header for block */
			InitWaveHeader(&lstblock.head, lstblock.size, bank_entry->SoundHz, bank_entry->Channels, bank_entry->BitsPSam);
		}
	}
}

static LPFEEDDATA ADPCM_FeedData;
static LPFEEDDATA MPEG_FeedData;

BOOL DecodeADPCM(bnm_entry* bank_entry, BYTE* p, DWORD samples, DWORD offs, DWORD part)
{
	/* same size as bnm_entry but different layout */
	BYTE parm[sizeof(bnm_entry)];
	void* l;
	memset(parm, 0, sizeof(bnm_entry));
	/* return: TRUE - ok / FALSE - error */
	_SND_fn_bLoadResBinarySxdADPCM(bank_entry, (void*)parm, offs);
	/* this one return pointer, NULL or -1 */
	l = _SND_fn_lPlaySxdADPCM(&parm, MasterVolume, 0, NULL, NULL);
	if (l && ((int)l != -1))
	{
		/*printf("ADPCM: %lu\n", ((DWORD **) l)[0][19] == samples);*/
		ADPCM_FeedData(0, 0, samples, p);
		/* single or last part */
		if (part < 2)
		{
			DevastateBuffer(ADPCM_FeedData, bank_entry);
		}
		_SND_fn_vStopSxdADPCM((void*)l);
	}
	else
	{
		l = NULL;
	}
	_SND_fn_vUnLoadResSxdADPCM((void*)parm);
	return (l != NULL);
}

BOOL DecodeMPEG(bnm_entry* bank_entry, BYTE* p, DWORD samples, DWORD offs, DWORD part)
{
	/* same size as bnm_entry but different layout */
	BYTE parm[sizeof(bnm_entry)];
	void* l;
	memset(parm, 0, sizeof(bnm_entry));
	/* always return 0 */
	_SND_fn_bLoadResBinaryMPEG(bank_entry, (void*)parm, offs);
	/* this one return pointer, NULL or -1 */
	l = _SND_fn_lPlayMPEG(&parm, MasterVolume, 0, NULL, NULL);
	if (l && ((int)l != -1))
	{
		/*printf("MPEG: %lu\n", ((DWORD *) l)[22] == samples);*/
		MPEG_FeedData(0, 0, samples, p);
		/* single or last part */
		if (part < 2)
		{
			DevastateBuffer(MPEG_FeedData, bank_entry);
		}
		_SND_fn_vStopMPEG((void*)l);
	}
	else
	{
		l = NULL;
	}
	_SND_fn_vUnLoadResMPEG((void*)parm);
	return (l != NULL);
}

int ExtractBank(char* filename)
{
	DWORD index, data_length, sz, num_samples;
	HANDLE bank_file;
	bnm_header bh;
	bnm_entry bank_entry;
	WORD merge_count, current_group, first_item_in_merge, file_number;
	char output_filename[32];
	BYTE* data_buffer;
	if (!filename)
	{
		return (10);
	}
	bank_file = _SND_fn_hOpenFileReadSnd(filename);
	if (bank_file == INVALID_HANDLE_VALUE)
	{
		printf("Error: can't open input file for read.\n\n");
		return (11);
	}
	ZeroMemory(&bh, sizeof(bh));
	/* read only 5 first fields of the header */
	_SND_fn_dwReadFileSnd(bank_file, 5 * 4, &bh);
	/* check header */
	if (
		(!bh.table_offset) || (!bh.table_count) ||
		(!bh.content_offset) || (!bh.bnm_count) ||
		(bh.table_offset > (11 * 4)) || /* 44 */
		((bh.table_offset + (bh.table_count * sizeof(table_item))) != bh.content_offset))
	{
		/* skip for() loop later */
		bh.bnm_count = 0;
		printf("Error: invalid input file format - not a .BNM bank?\n\n");
	}
	else
	{
		/* total offsets */
		bh.unused = (bh.table_offset - (5 * 4)) / 4;
		_SND_fn_dwReadFileSnd(bank_file, bh.unused * 4, bh.offslist);
	}
	
	merge_count = 0;
	current_group = 0;
	first_item_in_merge = 0;
	file_number = 0;
	lstblock.size = 0;
	data_buffer = NULL;
	printf("Begin processing bank file\n");
	printf("HEADER DATA\n");
	printf("+------------+----------+-------------+-----------+-----------------------------------------------------+\n");
	printf("| TBL OFFSET | # TABLES | DATA OFFSET | # ENTRIES |                       OFFSETS                       |\n");
	printf("+------------+----------+-------------+-----------+--------+--------+--------+--------+--------+--------+\n");
	printf("|   %6d   |  %6d  |   %7d   |    %3d    | %6d | %6d | %6d | %6d | %6d | %6d |\n", bh.table_offset, bh.table_count, bh.content_offset, bh.bnm_count, bh.offslist[0], bh.offslist[1], bh.offslist[2], bh.offslist[3], bh.offslist[4], bh.offslist[5]);
	printf("+------------+----------+-------------+-----------+--------+--------+--------+--------+--------+--------+\n");

	for (index = 0; index < bh.bnm_count; index++)
	{
		data_buffer = NULL;
		printf("Reading data at offset %d\n", bh.content_offset + (index * sizeof(bank_entry)));
		_SND_fn_dwSeekFileSnd(bank_file, bh.content_offset + (index * sizeof(bank_entry)), FILE_BEGIN);
		ZeroMemory(&bank_entry, sizeof(bank_entry));
		_SND_fn_dwReadFileSnd(bank_file, sizeof(bank_entry), &bank_entry);

		printf("-= Investigating file :   %s   =-\n", bank_entry.Name);
		printf(" %d %d |", bank_entry.SoundResourceID, bank_entry.SoundGroupID);
		printf(" %d %d | %d %d |", bank_entry.DataType, bank_entry.Internal, bank_entry.DataSize, bank_entry.DataOffs);
		printf("| %d %d %d %d %d %d |", bank_entry.Flags[0], bank_entry.Flags[1], bank_entry.Flags[2], bank_entry.Flags[3], bank_entry.Flags[4], bank_entry.FileAbsoluteOffset);
		printf(" %d |", bank_entry.SoundVol);
		printf(" %d |\n\n", bank_entry.ZipFormat);

		/* demo - internal SFX group */
		if (bank_entry.DataType == 8)
		{
			bank_entry.DataType = 5;
		}
		/* demo - unknown, just skip */
		if (bank_entry.DataType == 10)
		{
			bank_entry.DataType = 6;
		}
		/* demo - ADPCM internal group */
		if (bank_entry.DataType == 11)
		{
			bank_entry.DataType = 7;
			bank_entry.Flags[1] = bank_entry.DataSize / 24;
		}
		/* invalid data type */
		if ((bank_entry.DataType < 1) || (bank_entry.DataType > 7))
		{
			printf("Error: invalid data type %lu.\n", bank_entry.DataType);
			break;
		}
		/*
			7 = theme - merge next bank_entry.Flags[1] files
			with (bank_entry.Flags[2] == -1)
			also (bank_entry.DataSize == (bank_entry.Flags[1] * 12))
		*/
		if (!merge_count) {
			file_number++;
		}
		if (bank_entry.DataType == 7)
		{
			merge_count = bank_entry.Flags[1];
			current_group = bank_entry.SoundGroupID;
			printf("====================== Begin Merge: %s - %d: %d files ======================\n", bank_entry.Name, current_group, merge_count);
			first_item_in_merge = 1;
			lstblock.size = 0;
		}
		/* skip any non-sound blocks - block 5 in "Bnk_0.bnm" (SFX group?) */
		if (bank_entry.DataType != 1)
		{
			continue;
		}
		/* merge music, but group mismatch - stop merge */
		if (merge_count && (bank_entry.SoundGroupID != current_group))
		{
			printf("Error: group mismatch while in merge mode (%u != %u, %u).\n", bank_entry.SoundGroupID, current_group, merge_count);
			
		}
		if (
			(bank_entry.ZipFormat != SAMPLE_PCM) && (bank_entry.ZipFormat != SAMPLE_MPEG) &&
			(bank_entry.ZipFormat != SAMPLE_ADPCM1) && (bank_entry.ZipFormat != SAMPLE_ADPCM2))
		{
			printf("Error: invalid compression method %lu.\n", bank_entry.ZipFormat);
			break;
		}
		/* generate filename */
		lstrcpyA(output_filename, bank_entry.Name);
		/* demo - the same name as input bank file - isn't it nice? */
		if (!lstrcmpiA(bank_entry.Name, basename(filename)))
		{
			newext(output_filename, ".");
			wsprintfA(output_filename, "%s%d.%03u.wav", output_filename, bank_entry.SoundGroupID, file_number);
		}
		else
		{
			newext(output_filename, ".wav");
		}

		printf("Tentative filename: %s\n", output_filename);

		/* demo - ugh... for files "Bnk_83.bnm" and "Bnk_103.bnm" */
		if (
			/* FIXME: find a nice way to fix this (missing some information from the bank header?) */
			(bank_entry.SoundGroupID == 0xBD) && (!bank_entry.FileAbsoluteOffset) && (bank_entry.ZipFormat == SAMPLE_ADPCM1))
		{
			/* absolute offset */
			bank_entry.FileAbsoluteOffset = 1;
			/* relative to absolute */
			/* FIXME: why offslist[3] used instead of offslist[0] (?) how this can be detected (?) */
			bank_entry.DataOffs += bh.offslist[3];
			/* FIXME: why this (?) or stereo instead of mono (?) only for dialogs in demo (?) */
			bank_entry.SoundHz = 44100;
		}
		/* convert internal file to (pseudo)external
			 or use memory pointer to data instead of offs in Decode####() calls */
		if (bank_entry.Internal)
		{
			/* external file */
			bank_entry.Internal = 0;
			/* relative to absolute */
			if (!bank_entry.FileAbsoluteOffset)
			{
				bank_entry.FileAbsoluteOffset = 1;
				bank_entry.DataOffs += bh.offslist[0];
			}
			/* fix filename (should be in 8.3 format) */
			ZeroMemory(bank_entry.Name, 13);
			sz = lstrlenA(basename(filename));
			CopyMemory(bank_entry.Name, basename(filename), (sz > 12) ? 12 : sz);
		}

		printf("AAAAAAA");

		if (bank_entry.SoundVol < 10) {
			bank_entry.SoundVol = 60;
		}

		/* convert file */
		switch (bank_entry.ZipFormat)
		{
		case SAMPLE_PCM:
			data_length = sizeof(wav_head) + bank_entry.DataSize;
			data_buffer = (BYTE*)_SND_fn_pvMallocSnd(data_length);
			if (data_buffer)
			{
				memset(data_buffer, 0, data_length);
				InitWaveHeader((wav_head*)data_buffer, bank_entry.DataSize, bank_entry.SoundHz, bank_entry.Channels, bank_entry.BitsPSam);
				if (bank_entry.Internal)
				{
					_SND_fn_dwSeekFileSnd(bank_file, (bank_entry.FileAbsoluteOffset ? 0 : bh.offslist[0]) + bank_entry.DataOffs, FILE_BEGIN);
					_SND_fn_dwReadFileSnd(bank_file, bank_entry.DataSize, &data_buffer[sizeof(wav_head)]);
				}
				else
				{
					if (!FlRead(bank_entry.Name, &data_buffer[sizeof(wav_head)], bank_entry.DataSize, bank_entry.DataOffs))
					{
						_SND_fn_vFreeSnd(data_buffer);
						data_buffer = NULL;
					}
				}
			}
			break;
		case SAMPLE_MPEG:
			/* read samples count */
			num_samples = 0;
			if (bank_entry.Internal)
			{
				_SND_fn_dwSeekFileSnd(bank_file, (bank_entry.FileAbsoluteOffset ? 0 : bh.offslist[0]) + bank_entry.DataOffs, FILE_BEGIN);
				_SND_fn_dwReadFileSnd(bank_file, 4, &num_samples);
			}
			else
			{
				FlRead(bank_entry.Name, &num_samples, 4, bank_entry.DataOffs);
			}
			printf("BBBBBBB");

			/* got samples */
			if (num_samples)
			{
				data_length = (num_samples * (bank_entry.BitsPSam / 8) * bank_entry.Channels);
				data_buffer = (BYTE*)_SND_fn_pvMallocSnd(data_length + sizeof(wav_head));
				if (data_buffer)
				{
					memset(data_buffer, 0, data_length + sizeof(wav_head));
					InitWaveHeader((wav_head*)data_buffer, data_length, bank_entry.SoundHz, bank_entry.Channels, bank_entry.BitsPSam);
					DecodeMPEG(&bank_entry, &data_buffer[sizeof(wav_head)], num_samples, (bank_entry.FileAbsoluteOffset ? 0 : bh.offslist[0]), merge_count);
					bank_entry.DataSize = data_length;
				}
			} else {
				printf("Didn't ready anything from file %s\n", bank_entry.Name);
			}
			break;
		case SAMPLE_ADPCM1:
		case SAMPLE_ADPCM2:
			/* samples */
			num_samples = 0;

			/*Don't need this anymore since the bank_entry.Internal value is adjusted before we reach this point
			if (bank_entry.Internal)
			{
				_SND_fn_dwSeekFileSnd(bank_file, (bank_entry.FileAbsoluteOffset ? 0 : bh.offslist[0]) + bank_entry.DataOffs + 28, FILE_BEGIN);
				_SND_fn_dwReadFileSnd(bank_file, 4, &num_samples);
			} */

			FlRead(bank_entry.Name, &num_samples, 4, bank_entry.DataOffs + 28);

			/* got samples */
			if (num_samples != NULL)
			{
				data_length = sizeof(wav_head) + (num_samples * 4);

				if (num_samples < bank_entry.DataSize - 500 || num_samples > bank_entry.DataSize + 500) {					
					/* Load the alternate data offset */
					FlRead(bank_entry.Name, &num_samples, 4, bank_entry.DataOffs - bh.offslist[0] + bh.offslist[3] + 24);
					data_length = sizeof(wav_head) + (num_samples * 4);
				}
				data_buffer = (BYTE*)_SND_fn_pvMallocSnd(data_length);
				if (data_buffer)
				{
					memset(data_buffer, 0, data_length);
					InitWaveHeader((wav_head*)data_buffer, data_length - sizeof(wav_head), bank_entry.SoundHz, bank_entry.Channels, bank_entry.BitsPSam);
					DecodeADPCM(&bank_entry, &data_buffer[sizeof(wav_head)], num_samples, (bank_entry.FileAbsoluteOffset ? 0 : bh.offslist[0]), merge_count);
					bank_entry.DataSize = data_length - sizeof(wav_head);
				}
			}
			printf("nSamples: %d - Data length: %d\n", num_samples, data_length);
			break;
		}
		if (data_buffer)
		{
			if (merge_count)
			{
				/* merge music */
				FlGlue(output_filename, data_buffer, bank_entry.DataSize + sizeof(wav_head), first_item_in_merge);
				printf("============ Group %d (Remaining parts: %d) ============\n", current_group, merge_count);
			}
			else
			{
				/* single file */
				FlDump(output_filename, data_buffer, bank_entry.DataSize + sizeof(wav_head));
			}
			/* last block */
			if (merge_count < 2)
			{
				/* block not zero */
				if (lstblock.size)
				{
					FlGlue(output_filename, (BYTE*)&lstblock, lstblock.size + sizeof(wav_head), 0);
					lstblock.size = 0;
				}
				printf("====================== [%s] %s ======================\n", &SndType[((bank_entry.ZipFormat > 2) ? 2 : (bank_entry.ZipFormat - 1)) * 4], output_filename);
			}
			_SND_fn_vFreeSnd(data_buffer);
			printf("%s", (merge_count > 1) ? "\r" : "\n");
		}
		else
		{
			printf("\nError: can't allocate memory or missing external file.\n");
		}
		/* music merge mode */
		merge_count -= merge_count ? 1 : 0;
		/* not first item in merge mode */
		first_item_in_merge = 0;
	}
	_SND_fn_vCloseFileSnd(bank_file);
	return (data_buffer ? 0 : 12);
}

/* ------------------------------------------------------------------------
		File to memory mapping routines
	 ------------------------------------------------------------------------ */

void MapFileFree(map_file* mf)
{
	if (mf)
	{
		if (mf->pbMem)
		{
			UnmapViewOfFile(mf->pbMem);
		}
		if (mf->hMap)
		{
			CloseHandle(mf->hMap);
		}
		if (mf->hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(mf->hFile);
		}
		ZeroMemory(mf, sizeof(mf[0]));
		mf->hFile = INVALID_HANDLE_VALUE;
	}
}

BOOL MapFileOpen(char* filename, map_file* mf)
{
	if (filename && mf)
	{
		ZeroMemory(mf, sizeof(mf[0]));
		mf->hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (mf->hFile != INVALID_HANDLE_VALUE)
		{
			mf->lSize = GetFileSize(mf->hFile, NULL);
			if (mf->lSize)
			{
				mf->hMap = CreateFileMapping(mf->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
				if (mf->hMap)
				{
					mf->pbMem = (BYTE*)MapViewOfFile(mf->hMap, FILE_MAP_READ, 0, 0, 0);
				}
			}
			if (!mf->pbMem)
			{
				MapFileFree(mf);
			}
		}
		mf = mf->pbMem ? mf : NULL;
	}
	else
	{
		mf = NULL;
	}
	return (mf ? TRUE : FALSE);
}

/*
	and this routines below here since few .MPX files doesn't have any references
	anywhere inside the .BNM files - leftovers from the earlier development stages (?)
*/

DWORD BoyerMooreHorspool(BYTE* p, DWORD ps, BYTE* q, BYTE qs, DWORD* list, DWORD back)
{
	BYTE t[256];
	DWORD i, n, s;
	n = 0;
	if (p && ps && q && qs)
	{
		FillMemory(t, 256, qs);
		qs--;
		for (i = 0; i < qs; i++)
		{
			t[q[i]] = qs - i;
		}
		s = 0;
		while ((ps - s) > qs)
		{
			for (i = qs; (p[s + i] == q[i]); i--)
			{
				if (!i)
				{
					if (list)
					{
						*list = s - back;
						list++;
					}
					n++;
					break;
				}
			}
			s += t[p[s + qs]];
		}
	}
	return (n);
}

void BuildBNM(bnm_entry* bank_entry, char* s, DWORD l, DWORD h, DWORD c, DWORD z)
{
	if (bank_entry && s)
	{
		ZeroMemory(bank_entry, sizeof(bank_entry[0]));
		bank_entry->DataType = 1;
		bank_entry->DataSize = l;
		bank_entry->SoundVol = 0x7F;
		bank_entry->SoundHz = h;
		bank_entry->BitsPSam = 16;
		bank_entry->Channels = c;
		bank_entry->ZipFormat = z;
		for (l = 1; l < 5; l++)
		{
			bank_entry->Flags[l] = 1;
		}
		bank_entry->FileAbsoluteOffset = 1;
		s = basename(s);
		l = lstrlenA(s);
		CopyMemory(bank_entry->Name, s, (l > 12) ? 12 : l);
	}
}

int ExtractADPCM(char* filename)
{
	apm_head* ah;
	map_file mf;
	bnm_entry bank_entry;
	char s[32];
	DWORD sz;
	BYTE* p;
	if (!MapFileOpen(filename, &mf))
	{
		printf("Error: can't open input file for read or create file mapping.\n\n");
		return (10);
	}
	ah = (apm_head*)mf.pbMem;
	if (
		/* file too short */
		(mf.lSize < sizeof(ah[0])) ||
		/* invalid signatures */
		(ah->Signvs12 != MAKEFOURCC('v', 's', '1', '2')) || (ah->SignData != MAKEFOURCC('D', 'A', 'T', 'A')))
	{
		MapFileFree(&mf);
		printf("Error: invalid header - not an UBI-ADPCM file.\n\n");
		return (11);
	}
	/* prepare bnm_entry struct */
	BuildBNM(&bank_entry, filename, mf.lSize, ah->SoundHz, ah->Channels, SAMPLE_ADPCM1);
	bank_entry.LoopTill = ah->nSamples * 4;
	/* mapping not needed anymore */
	MapFileFree(&mf);
	/* output new name */
	lstrcpyA(s, bank_entry.Name);
	newext(s, ".wav");
	printf("[%s] %s", &SndType[((bank_entry.ZipFormat > 2) ? 2 : (bank_entry.ZipFormat - 1)) * 4], s);
	/* decode */
	sz = sizeof(wav_head) + bank_entry.LoopTill;
	p = (BYTE*)_SND_fn_pvMallocSnd(sz);
	if (p)
	{
		memset(p, 0, sz);
		InitWaveHeader((wav_head*)p, sz - sizeof(wav_head), bank_entry.SoundHz, bank_entry.Channels, bank_entry.BitsPSam);
		DecodeADPCM(&bank_entry, &p[sizeof(wav_head)], bank_entry.LoopTill / 4, 0, 0);
		FlGlue(s, p, sz, 1);
		_SND_fn_vFreeSnd(p);
		/* block not zero */
		if (lstblock.size)
		{
			FlGlue(s, (BYTE*)&lstblock, lstblock.size + sizeof(wav_head), 0);
			lstblock.size = 0;
		}
		printf("\n");
		sz = 0;
	}
	else
	{
		printf("\n\nError: no enough memory for output buffer.\n\n");
		sz = 12;
	}
	return (sz);
}

/* this routine can convert 1RUS/2RUS .MPX files and .MPU as single file
	 since for later one size and offset of each data chunk can't be calculated
	 (no fixed signatures or any other way to detect chunk size because game
	 variation of MPEG codec uses variable-length blocks without proper header) */

int ExtractMPEG(char* filename)
{
	DWORD index, data_length, sz, * list, * dw;
	map_file mf;
	bnm_entry bank_entry;
	char s[32];
	BYTE* p;
	if (!MapFileOpen(filename, &mf))
	{
		printf("Error: can't open input file for read or create file mapping.\n\n");
		return (10);
	}
	dw = (DWORD*)mf.pbMem;
	if (
		/* file too short */
		(mf.lSize < 8) ||
		/* zero samples */
		(!dw[0]) ||
		/* invalid signature */
		(
			(dw[1] != MAKEFOURCC('1', 'R', 'U', 'S')) &&
			(dw[1] != MAKEFOURCC('2', 'R', 'U', 'S')) &&
			/* single-block file */
			((dw[1] & 0xFF) != 0xFF)))
	{
		MapFileFree(&mf);
		printf("Error: invalid header - not an UBI-MPEG file.\n\n");
		return (11);
	}
	/* parse and find all MPEG blocks */
	printf("- Find MPG offsets");
	if ((dw[1] & 0xFF) != 0xFF)
	{
		/* assuming all blocks will be 1RUS or 2RUS and didn't change in the middle */
		data_length = BoyerMooreHorspool(mf.pbMem, mf.lSize, (BYTE*)&dw[1], 4, NULL, 4);
	}
	else
	{
		/* headerless - probably single file */
		printf(", warning no signatures - assuming single-block file");
		data_length = 1;
	}
	/* allocate memory */
	list = (DWORD*)_SND_fn_pvMallocSnd(sizeof(list[0]) * (data_length + 1));
	if (!list)
	{
		MapFileFree(&mf);
		printf("...error\nCan't allocate memory for offsets list.\n\n");
		return (13);
	}
	printf(".");
	/* fill in list */
	if ((dw[1] & 0xFF) != 0xFF)
	{
		BoyerMooreHorspool(mf.pbMem, mf.lSize, (BYTE*)&dw[1], 4, list, 4);
	}
	else
	{
		/* headerless file */
		list[0] = 0;
	}
	printf(".");
	/* file size - need to calulate block size as (next - curent) */
	list[data_length] = mf.lSize;
	/* prepare bnm_entry struct */
	BuildBNM(&bank_entry, filename, 0, 44100, 2, SAMPLE_MPEG);
	/* output new name */
	lstrcpyA(s, bank_entry.Name);
	newext(s, ".wav");
	printf(".ok\n\n");
	printf("[%s] %s", &SndType[((bank_entry.ZipFormat > 2) ? 2 : (bank_entry.ZipFormat - 1)) * 4], s);
	sz = 0;
	for (index = 0; index < data_length; index++)
	{
		/* update item information */
		bank_entry.SoundResourceID++;
		bank_entry.DataSize = list[index + 1] - list[index];
		bank_entry.DataOffs = list[index];
		/* read samples */
		bank_entry.LoopTill = *((DWORD*)&mf.pbMem[list[index]]);
		bank_entry.LoopTill *= 4;
		/* decode */
		sz = sizeof(wav_head) + bank_entry.LoopTill;
		p = (BYTE*)_SND_fn_pvMallocSnd(sz);
		if (p)
		{
			memset(p, 0, sz);
			InitWaveHeader((wav_head*)p, sz - sizeof(wav_head), bank_entry.SoundHz, bank_entry.Channels, bank_entry.BitsPSam);
			DecodeMPEG(&bank_entry, &p[sizeof(wav_head)], bank_entry.LoopTill / 4, 0, data_length - index);
			FlGlue(s, p, sz, (index == 0));
			_SND_fn_vFreeSnd(p);
		}
		else
		{
			sz = 12;
			break;
		}
		sz = 0;
	}
	/* mapping not needed anymore */
	MapFileFree(&mf);
	if (!sz)
	{
		/* last block present */
		if (lstblock.size)
		{
			FlGlue(s, (BYTE*)&lstblock, lstblock.size + sizeof(wav_head), 0);
			lstblock.size = 0;
		}
		printf("\n");
	}
	else
	{
		printf("\n\nError: no enough memory for output buffer.\n\n");
	}
	return (sz);
}

int main(int argc, char* argv[])
{
	int c;
	printf("Tonic Trouble audio converter v1.0\n(c) CTPAX-X Team 2019\nhttp://www.CTPAX-X.org/\n\n");
	if (argc != 3)
	{
		printf(
			"Usage: tonictac <#> <filename.ext>\n"
			"Where:\n"
			"	# - how input file should be treated (it's NOT detected automatically!):\n"
			"		 b - Bank	file (.BNM)\n"
			"		 a - ADPCM file (.APM)\n"
			"		 m - MPEG	file (.MPX / .MPU)\n"
			"	filename.ext - input filename\n\n"
			"Note that for some bank files you're also may need external .MPX, .APM, etc.\n"
			"files in the same folder as this tool and source bank file.\n\n"
			"Examples:\n"
			"	tonictac b Bnk_0.bnm\n"
			"	tonictac a Usdolby.apm\n"
			"	tonictac m Ski.mpx\n"
			"\n\n");
		return (1);
	}

	ADPCM_FeedData = InitAPMDecoder();
	MPEG_FeedData = InitMPEGDecoder();

	/* convert input file */
	c = argv[1][0] | 0x20;
	/* switch() make unnecessary large jump table - so much space wasted */
	do
	{
		if (c == 'b')
		{
			c = ExtractBank(argv[2]);
			break;
		}
		if (c == 'a')
		{
			c = ExtractADPCM(argv[2]);
			break;
		}
		if (c == 'm')
		{
			c = ExtractMPEG(argv[2]);
			break;
		}
		printf("Error: unknown input mode: '%c'.\n\n", argv[1][0]);
		c = 4;
	} while (0);

	UnloadDecoders();

	/* no errors */
	if (!c)
	{
		printf("\ndone\n\n");
	}
	return (c);
}
