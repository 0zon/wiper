#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define MIN(x, y) ((x) > (y) ? (y) : (x))

int GetRndFileName(const char *Name, char *RndFileName)
{
	unsigned int i, k;
	int lastpos;
	char TempName[MAX_PATH];

	k = strlen(Name);
	lastpos = 0;
	for (i=0;i<k-1;i++) {
		// find separator '\'
		if ((unsigned char)(*(Name+i)) == 92) {
			lastpos = i;
		}
	}
	if (!lastpos) 
		lastpos--;

	TempName[lastpos+1] = 0;
	memcpy(TempName, Name, lastpos+1);
	if (GetTempFileName(TempName, ".", 0, RndFileName) == 0)
		return FALSE;

	return TRUE;
}

int GetFileSize64(HANDLE f, UINT64 *FileSize) {
	LARGE_INTEGER DataTemp;

	if (GetFileSizeEx(f, &DataTemp) != 0) {
		*FileSize = DataTemp.QuadPart;
		return TRUE;
	}

	return FALSE; 
}

int WipeFile(const char *FileName)
{
	HANDLE f;
	unsigned __int64 FileSize;
	const unsigned int BufSize=65536;
	static BYTE Buf[65536];
	DWORD Written;
	char TempName[MAX_PATH];

	// reset the file attributes
	if (!SetFileAttributes(FileName, 0))
		return GetLastError();
	
	// open file
	f = CreateFile(FileName,
		GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_SEQUENTIAL_SCAN, 
		NULL);
	if(f == INVALID_HANDLE_VALUE)
		return GetLastError();

	// get size of file
	FileSize = 0;
	if (!GetFileSize64(f, &FileSize))
		return GetLastError();
	
	// nulling file
	if(FileSize)
	{
		memset(Buf, 0, BufSize);
		
		while (FileSize>0)
		{
			DWORD WriteSize = (DWORD)MIN((UINT64)BufSize, FileSize);
			if (!WriteFile(f, Buf, WriteSize, &Written, NULL)) {
				DWORD Err = GetLastError();
				CloseHandle(f);
				return Err;
			}
			FileSize-=WriteSize;
		}
		
		if (SetFilePointer(f, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
			DWORD Err = GetLastError();
			CloseHandle(f);
			return Err;
		}
	}

	if (!CloseHandle(f))
		return GetLastError();
	
	// rename file
	memset(TempName, 0, MAX_PATH);
	if (GetRndFileName(FileName, &TempName[0])) {
		if (!MoveFileEx(FileName, TempName, MOVEFILE_REPLACE_EXISTING))
			return GetLastError();
	} else {
		memcpy(&TempName[0], FileName, strlen(FileName));
	}
	
	// delete file
	if (!DeleteFile(TempName))
		return GetLastError();

	// success
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: wipe <fullfilename>\r\n");
		
		return 1;
	}
	
	return WipeFile(argv[1]);
}