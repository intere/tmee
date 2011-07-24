//
// DAAudioArchive.cpp
//
//
// =======================================================================================
//
//	Copyright 2005-2006 Digital Acoustics
//
//	The source code included in this file is created as an example.  There is no specific
//	fitness or purpose for this code, and it is presented in an AS-IS basis.
//
//	Methods and processes specified by these files are purely for the purpose of example.
//
// =======================================================================================
//

#include "StdAfx.h"
#include ".\daaudioarchive.h"
#include "TalkMasterConsoleDlg.h"
#include "direct.h"

DAAudioArchive::DAAudioArchive(void)
{
	InitializeCriticalSection(&CriticalSection);
	m_archives = NULL;
}

DAAudioArchive::~DAAudioArchive(void)
{
	struct _archives *next;

	while( m_archives )
	{
		next = m_archives->next;
		if( m_archives->fp )
			Close(m_archives);

//		remove( m_archives->fileName );
		
		free(m_archives);
		m_archives = next;
	}

	DeleteCriticalSection(&CriticalSection);
}

HANDLE DAAudioArchive::getRecordingHandle( int socket, char *szInOut, BOOL callQueue, int audioFormat )
{
	struct _archives *archive = m_archives;

	while(archive)
	{
		if( archive->socket == socket &&
			!(strcmp(archive->inOutStr, szInOut)) &&
			archive->callQueue == callQueue)
			break;

		archive = archive->next;
	}

	if( !archive )
	{
		archive = (struct _archives *)calloc(1, sizeof(struct _archives));

		archive->socket = socket;
		archive->callQueue = callQueue;
		archive->audioFormat = audioFormat;						// We are opening this format of file

		strcpy(archive->inOutStr, szInOut);

		strcpy(archive->fileName, makeArchiveName( socket, szInOut, callQueue ));

		archive->fp = fopen( archive->fileName, "wb" );
		WriteHeader(archive->fp, 0, 8000, audioFormat);

		EnterCriticalSection( &CriticalSection );

		archive->next = m_archives;
		m_archives = archive;

		LeaveCriticalSection( &CriticalSection );

	}
	else if( !archive->fp )
	{
		archive->fp = fopen( archive->fileName, "rb+" );
		fseek(archive->fp, 0L, SEEK_END);
	}

	archive->openWrite = TRUE;

	return(archive);
}

int DAAudioArchive::Write( HANDLE hArchive, char *data, int len )
{
	struct _archives *archive = (struct _archives *)hArchive;

	archive->totalBytes += len;

	fwrite(data, 1, len, archive->fp);

	return(archive->totalBytes);
}

int DAAudioArchive::Close( HANDLE hArchive )
{
	struct _archives *archive = (struct _archives *)hArchive;

	if( !archive->fp )
		return(archive->totalBytes);

	if( archive->openWrite )
		WriteHeader(archive->fp, archive->totalBytes, 8000, archive->audioFormat);

	fclose(archive->fp);
	archive->fp = NULL;

	return(archive->totalBytes);
}

// http://ccrma.stanford.edu/courses/422/projects/WaveFormat/

BOOL DAAudioArchive::WriteHeader( FILE *fp, int dataSize, int sampleRate, int audioType )
{
	struct _waveHeader header = {{'R','I','F','F'},
		dataSize+sizeof(struct _waveHeader),
	{'W','A','V','E'},
	{'f','m','t',' '},
	18,
	audioType,1,
	sampleRate,
	sampleRate,
	1, 8, 0,						// last 0 is for the spacer "extra bytes"
	{'d','a','t','a'},
	dataSize};

	fseek(fp, 0L, 0);
	fwrite(&header, 1, sizeof(struct _waveHeader), fp);

	return(TRUE);
}

BOOL DAAudioArchive::Delete( HANDLE hArchive, BOOL delFile )
{
	struct _archives *archive = (struct _archives *)hArchive;

// See if the 1st archive is ours

	struct _archives *delArch = m_archives;
	if( delArch == archive )		// If so, treat it as a special case
	{
		m_archives = archive->next;
		if( archive->fp )
			Close(archive);
		if( delFile )
			remove(archive->fileName);
		free(archive);
		return(TRUE);
	}
	else
	{
// Find the archive in the rest of the list
		EnterCriticalSection( &CriticalSection );

		while( delArch )
		{
			if( delArch->next == archive )
			{
				if( archive->fp )
					Close(archive);
				delArch->next = archive->next;
				if( delFile )
					remove(archive->fileName);

				LeaveCriticalSection( &CriticalSection );

				free(archive);
				return(TRUE);
			}

			delArch = delArch->next;
		}
	}

	LeaveCriticalSection( &CriticalSection );

	return(FALSE);
}

int DAAudioArchive::Open( HANDLE hArchive )			// Returns the # of bytes
{
	struct _archives *archive = (struct _archives *)hArchive;
	int	messageLen=0;
	char sectionName[5] = {0,0,0,0,0};

	if( !(archive->fp = fopen( archive->fileName, "rb" ) ) )
		return(0);

	fseek(archive->fp, 12, SEEK_SET);

	while (1)			// Read until we get to "data" and then read the size
	{
		if( fread(sectionName, 4, 1, archive->fp) == 0 )	// did we read anything?
			break;
		fread(&messageLen, 4, 1, archive->fp);		// Read in the actual length to read
		if( !strcmp(sectionName, "data") )
			break;
		else if( !strcmp(sectionName, "fmt ") )				// If we read the format, get the audio format type
		{
			fread(&archive->audioFormat, 2, 1, archive->fp);			// Read the audio format type
			fseek(archive->fp, messageLen-2, SEEK_CUR);		// Now skip the rest
		}
		else
			fseek(archive->fp, messageLen, SEEK_CUR);		// Skip this section
	}

	if( !messageLen )
	{
		fclose(archive->fp);
		archive->fp = NULL;
		return(0);
	}

	archive->headerSize = fseek(archive->fp, 0L, SEEK_CUR);

	return(archive->totalBytes);
}

int	DAAudioArchive::Read( HANDLE hArchive, char *data, int maxLen )
{
	struct _archives *archive = (struct _archives *)hArchive;

	if( !archive || !archive->fp )
		return(0);

	return( fread(data, 1, maxLen, archive->fp) );
}

BOOL DAAudioArchive::isOpen( HANDLE hArchive )
{
	struct _archives *archive = (struct _archives *)hArchive;

	return((BOOL)archive->fp);
}

int	DAAudioArchive::Tell( HANDLE hArchive )
{
	struct _archives *archive = (struct _archives *)hArchive;
	return( ftell(archive->fp) - archive->headerSize );
}

int	DAAudioArchive::Size( HANDLE hArchive )
{
	struct _archives *archive = (struct _archives *)hArchive;
	return( archive->totalBytes - archive->headerSize );
}

int	DAAudioArchive::Merge( HANDLE hArchiveOut, HANDLE hArchiveIn )			// Copy the in to the end of the out
{																			// Closing, deleteing, releasing the "in" archive
	int len, i, readLen;
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	Close(hArchiveIn);														// Close it to be able to seek

	len = Open(hArchiveIn);													// Reopen the file to get size and reset position

	for(i=0 ; i<len ; )
	{
		readLen = Read(hArchiveIn, mergeData, 2048);
		Write(hArchiveOut, mergeData, readLen);
		i += readLen;
	}

	Close(hArchiveIn);

	return( Size(hArchiveOut) );
}

BOOL DAAudioArchive::Rename( HANDLE hArchive, char *szInOut )
{
	struct _archives *archive = (struct _archives *)hArchive;
	char newName[MAX_PATH];

	Close(hArchive);

	strcpy( newName, makeArchiveName( archive->socket, szInOut, archive->callQueue ) );

	rename( archive->fileName, newName );

	strcpy( archive->fileName, newName );
	strcpy( archive->inOutStr, szInOut );

	return(TRUE);
}

CString DAAudioArchive::makeArchiveName( int socket, char *szInOut, BOOL callQueue )
{
	struct _itemData *tItemData;
	CString csFolderName, csTemp;
//	int index, count;

	time_t now;
	struct tm	atime;

	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	time( &now );
	atime = *localtime( &now );

	csFolderName = theApp.UserOptions.pathTemp;

	if( callQueue )
	{
		csFolderName += "\\CallQueue\\";
		mkdir(csFolderName);
	}
	else
	{
		csFolderName += "\\iArchive\\";

		mkdir(csFolderName);

		csTemp.Format("%4d%02d%02d\\", atime.tm_year+1900, atime.tm_mon+1, atime.tm_mday );
		csFolderName += csTemp;

		mkdir(csFolderName);
	}

// Now add the filename

	tItemData = dlg->findItemData(socket);

	if( !tItemData )
		DebugBreak();

//		csFolderName += "\\";
	csFolderName += szInOut;

	csTemp.Format("-%02x%02x%02x%02x%02x%02x-", 
		tItemData->iCom.MAC.MAC1,
		tItemData->iCom.MAC.MAC2,
		tItemData->iCom.MAC.MAC3,
		tItemData->iCom.MAC.MAC4,
		tItemData->iCom.MAC.MAC5,
		tItemData->iCom.MAC.MAC6);
	csFolderName += csTemp;

	csTemp.Format("%02d%02d%02d.wav", atime.tm_hour, atime.tm_min, atime.tm_sec );
	csFolderName += csTemp;

	return(csFolderName);
}

short DAAudioArchive::GetAudioFormat(HANDLE hArchive)
{
	struct _archives *archive = (struct _archives *)hArchive;

	if( archive )
		return(archive->audioFormat);
	else
		return(-1);
}