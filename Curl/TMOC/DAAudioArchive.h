#pragma once

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

#include "TalkMasterConsole.h"

#define	WAVE_FORMAT_ULAW	7
#define	WAVE_FORMAT_PCM		1

struct _archives
{
	struct _archives *next;

	HANDLE	hArchive;
	int		totalBytes;
	int		socket;

	char	inOutStr[10];

	int		headerSize;					// Bytes that make up the header so we can do a Tell()

	BOOL	callQueue;

	FILE	*fp;

	BOOL	openWrite;

	char	fileName[MAX_PATH];

	short	audioFormat;
};

// http://ccrma.stanford.edu/courses/422/projects/WaveFormat/
struct _waveHeader
{
	char	title[4];
	ULONG	fSize;
	char	waveTitle[4];
	char	section1Name[4];		// "fmt "
	ULONG	section1Len;
	short	audioFormat;			// 1=PCM
	short	numChannels;
	ULONG	sampleRate;
	ULONG	byteRate;
	short	blockAlign;
	short	bitsPerSample;
	short	extraBytes;				// Should be ZERO
	char	section2Name[4];		// 'data'
	ULONG	section2Len;
};

struct _fmtHeader
{
	ULONG	len;
	short	audioFormat;
	short	numChannels;
	ULONG	sampleRate;
	ULONG	byteRate;
	short	blockAlign;
	short	bitsPerSample;
};

class DAAudioArchive
{
protected:
	struct _archives *m_archives;

	char mergeData[2048];
	char mergeName[MAX_PATH];

	CRITICAL_SECTION CriticalSection; 

public:
	DAAudioArchive(void);
	~DAAudioArchive(void);

	HANDLE getRecordingHandle( int socket, char *csInOut, BOOL callQueue, int audioFormat );
	int Write( HANDLE hArchive, char *data, int len );
	int Close( HANDLE hArchive );

	BOOL WriteHeader( FILE *fp, int dataSize, int sampleRate, int audioFormat );
	BOOL Delete( HANDLE hArchive, BOOL bDelFile );

	int Open( HANDLE hArchive );			// Returns the # of bytes
	int	Read( HANDLE hArchive, char *data, int maxLen );

	BOOL isOpen( HANDLE hArchive );			// See if it is already open

	int	Tell( HANDLE hArchive );			// Tell the current position in the data
	int	Size( HANDLE hArchive );				// Tell the file size

	int	Seek( HANDLE hArchive, LONG pos );
	int	Merge( HANDLE hArchiveOut, HANDLE hArchiveIn );
	BOOL Rename( HANDLE hArchive, char *csInOut );

	CString makeArchiveName( int socket, char *szInOut, BOOL callQueue );

	short GetAudioFormat( HANDLE hArchive );
};
