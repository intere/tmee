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

#include "mmsystem.h"

#define WAVE_OUT_BUF_SIZE	1024
#define WAVE_IN_BUF_SIZE	512*2			// 512 uLaw or PCM
#define	NUM_BUFS		20

class DAAudioStuff
{
public:
	DAAudioStuff(void);
	~DAAudioStuff(void);

	BOOL		m_bInStarted;

	void	Open(BOOL outOnly);
	void	Close();

	void	Write( PUCHAR data, int nBytes, int audioFormat );
	void	Flush();
	void	Reset();

	void	StartMic();
	void	StopMic();

	void	Mute( BOOL bMuteOn );

	BOOL	inQueue();

	void DAAudioStuff::filterData( short *data, int len );

protected:
	int		GetBuffer();
	void DAAudioStuff::setupOutBuffer(int bufnum);
	void DAAudioStuff::setupInBuffer(int bufnum);

	HWAVEOUT	m_hOut;
	WAVEHDR		m_hdrOut[NUM_BUFS];
	BOOL		m_hdrOutAvailable[NUM_BUFS];
	int			m_hdrOutPos[NUM_BUFS];

	HWAVEIN		m_hIn;
	WAVEHDR		m_hdrIn[NUM_BUFS];

	int			m_muteSaveVolume;

	int			m_currentBuffer;

	int			m_micBufferSize;

	unsigned	filterAvg1, filterAvg2;
};
