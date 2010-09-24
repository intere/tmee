//
// DAAudioStuff.cpp
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


#include "stdafx.h"
#include "windows.h"
#include "TalkMasterConsole.h"
#include ".\daaudiostuff.h"
#include "TalkMasterConsoleDlg.h"
#include "DACodec.h"

#define	USE_VIRTUAL_MEMORY	0

void CALLBACK WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	WAVEHDR	*hdr = (WAVEHDR *)dw1;

	if( uMsg == WOM_DONE )
	{
//		waveOutUnprepareHeader(hWave, hdr, sizeof(WAVEHDR));

		hdr->dwBytesRecorded = 0;
		hdr->dwUser = 0;
	}
	else if (uMsg == WIM_DATA )			// SND_FILENAME
	{
		if( dlg )
		{
			if( hdr->dwBytesRecorded && dlg->audioPlayer.m_bInStarted )
				dlg->serviceMicAudio(hdr->lpData, hdr->dwBytesRecorded);

			if( dlg->audioPlayer.m_bInStarted )			// Only re-add the buffer if the player is still started
				waveInAddBuffer((HWAVEIN)hWave, hdr, sizeof(WAVEHDR));
	//		else
	//			::OutputDebugString(".");
		}
	}
#if 0
	else
	{
		char buf[128];
		sprintf(buf, "other code %d", uMsg);
		::OutputDebugString(buf);
	}
#endif
}

DAAudioStuff::DAAudioStuff(void)
{
	m_hOut = NULL;
	m_hIn = NULL;

	filterAvg1 = 0;
	filterAvg2 = 0;
}

DAAudioStuff::~DAAudioStuff(void)
{
}

//
// Open( outOnly )
//
// Open the waveIn and waveOut devices.  If outOnly is specified, only open the waveOut device

void DAAudioStuff::Open(BOOL outOnly)
{
	WAVEFORMATEX waveFmt;
	MMRESULT mResult;
	char msgBuffer[200], buffer[256];
	int i;
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	memset(&waveFmt, 0, sizeof(waveFmt));

	waveFmt.wFormatTag = WAVE_FORMAT_PCM;
	waveFmt.nChannels = 1;
	waveFmt.wBitsPerSample = 16;
	waveFmt.nSamplesPerSec = 8000;
	waveFmt.nBlockAlign = 2;
	waveFmt.nAvgBytesPerSec = 16000;
	waveFmt.cbSize = 0;

	mResult = waveOutOpen( &m_hOut, theApp.UserOptions.waveOutputDevice, &waveFmt, (DWORD)WaveCallback, (DWORD)this, CALLBACK_FUNCTION );
	if( mResult != MMSYSERR_NOERROR )
	{
		if( dlg )
		{
			waveInGetErrorText(mResult, msgBuffer, sizeof(msgBuffer));
			dlg->outputDebug(_T("waveOutOpen: device=%d failed with %d"), theApp.UserOptions.waveOutputDevice, mResult);
			dlg->outputDebug(_T("waveOutOpen: '%s'"), msgBuffer);
		}
	}

	for(i=0;i<NUM_BUFS;i++)
		setupOutBuffer(i);

// Now open the Microphone

	if( !outOnly )
	{
		memset(&waveFmt, 0, sizeof(waveFmt));

		if( dlg->bUseUlaw )
		{
			dlg->outputDebug(_T("Open: Using uLaw for Microphone"));

			waveFmt.wFormatTag = WAVE_FORMAT_PCM;
			waveFmt.nChannels = 1;
			waveFmt.wBitsPerSample = 16;
			waveFmt.nSamplesPerSec = 8000;
			waveFmt.nBlockAlign = 2;
			waveFmt.nAvgBytesPerSec = 16000;
			waveFmt.cbSize = 0;

			m_micBufferSize = 1024;						// 512 samples
		}
		else
		{
			dlg->outputDebug(_T("Open: Using PCM for Microphone"));

			waveFmt.wFormatTag = WAVE_FORMAT_PCM;
			waveFmt.nChannels = 1;
			waveFmt.wBitsPerSample = 8;
			waveFmt.nSamplesPerSec = 8000;
			waveFmt.nBlockAlign = 1;
			waveFmt.nAvgBytesPerSec = 8000;
			waveFmt.cbSize = 0;

			m_micBufferSize = 512;						// 512 samples
		}

		mResult = waveInOpen( &m_hIn, theApp.UserOptions.waveInputDevice, &waveFmt, (DWORD)WaveCallback, (DWORD)this, WAVE_FORMAT_QUERY );
		if( mResult != MMSYSERR_NOERROR )
		{
			if( dlg )
			{
				dlg->outputDebug(_T("Open: Wave form test for audio Type %d failed"), waveFmt.wFormatTag);
				
				MessageBox(NULL, dlg->getResourceString(IDS_STRING_ULAW_NOT_AVAIL), dlg->getResourceString(IDS_STRING_MIC_CODEC_NOT_AVAIL), MB_OK);
				waveFmt.wFormatTag = WAVE_FORMAT_PCM;

				theApp.UserOptions.useUlaw = FALSE;			// Don't use uLaw
				dlg->bUseUlaw = FALSE;
			}
		}

		mResult = waveInOpen( &m_hIn, theApp.UserOptions.waveInputDevice, &waveFmt, (DWORD)WaveCallback, (DWORD)this, CALLBACK_FUNCTION );
		if( mResult != MMSYSERR_NOERROR )
		{
			if( dlg )
			{
				waveInGetErrorText(mResult, msgBuffer, sizeof(msgBuffer));
				
				dlg->outputDebug(_T("waveInOpen failed with %d - '%s'"), mResult, msgBuffer);

				sprintf(buffer, dlg->getResourceString(IDS_STRING104), msgBuffer);
				MessageBox(NULL, buffer, dlg->getResourceString(IDS_STRING105), MB_OK);
			}
		}

		for(i=0;i<NUM_BUFS;i++)
			setupInBuffer(i);
	}

	m_bInStarted = FALSE;				// Can not stop or reset what is not started

	m_currentBuffer = -1;				// No current buffer in use
	m_muteSaveVolume = -1;				// Not muted yet
}

void DAAudioStuff::Close()
{
	if( m_hOut == NULL )
		return;

	waveOutReset( m_hOut );

	if(m_bInStarted)
	{
//		waveInReset( m_hIn );
		waveInStop( m_hIn );
	}

	for(int i=0;i<NUM_BUFS;i++)
	{
		waveOutUnprepareHeader(m_hOut, &m_hdrOut[i], sizeof(WAVEHDR));
#if USE_VIRTUAL_MEMORY
		VirtualFree(m_hdrOut[i].lpData, WAVE_OUT_BUF_SIZE, MEM_FREE);
#else
		free(m_hdrOut[i].lpData);
#endif

		if( m_hIn )
		{
			waveInUnprepareHeader(m_hIn, &m_hdrIn[i], sizeof(WAVEHDR));
			free(m_hdrIn[i].lpData);
		}
	}

	waveOutClose( m_hOut );

	if( m_hIn )
		waveInClose( m_hIn );

	m_hOut = NULL;
	m_hIn = NULL;
}

void DAAudioStuff::Flush()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	if( m_currentBuffer != -1 &&
		m_hdrOutPos[m_currentBuffer]>0 &&
		m_hdrOut[m_currentBuffer].dwUser == 0 )
	{
		m_hdrOut[m_currentBuffer].dwUser = m_currentBuffer+1;			// This header is in use
//		m_hdrOut[m_currentBuffer].dwBufferLength = WAVE_OUT_BUF_SIZE;
		m_hdrOut[m_currentBuffer].dwBufferLength =  m_hdrOutPos[m_currentBuffer];
		m_hdrOut[m_currentBuffer].dwBytesRecorded = m_hdrOutPos[m_currentBuffer];
		m_hdrOut[m_currentBuffer].dwFlags &= ~(WHDR_DONE|WHDR_INQUEUE);
//		m_hdrOut[m_currentBuffer].dwFlags = 0;
		m_hdrOutPos[m_currentBuffer] = 0;

//		waveOutPrepareHeader(m_hOut, &m_hdrOut[m_currentBuffer], sizeof(WAVEHDR));
		waveOutWrite(m_hOut, &m_hdrOut[m_currentBuffer], sizeof(WAVEHDR));

		m_currentBuffer = -1;			// None selected
	}
}

void DAAudioStuff::Reset()
{
	waveOutReset(m_hOut);
}

void DAAudioStuff::Mute(BOOL bMuteOn)
{
	if( bMuteOn )
	{
		if( m_muteSaveVolume == -1 )			// No value yet
		{
			waveOutGetVolume(m_hOut, (LPDWORD)&m_muteSaveVolume);
			waveOutSetVolume(m_hOut, 0);
		}
	}
	else
	{
		if( m_muteSaveVolume != -1 )			// Saved Value
		{
			waveOutSetVolume(m_hOut, m_muteSaveVolume);
			m_muteSaveVolume = -1;
		}
	}
}

void DAAudioStuff::Write( PUCHAR data, int nBytes, int audioFormat )
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	int	copySize, writePos=0;
	MMRESULT rc;
//	char d[WAVE_OUT_BUF_SIZE];

	static int guard=0;

//	dlg->outputDebug("Write of %d Bytes", nBytes);

	guard++;
	while( guard > 1 )
	{
		::OutputDebugString("ARGH: Multiple calls");
		dlg->DoEvents();
	}

	while( nBytes )
	{
		if( m_currentBuffer == -1 )
			m_currentBuffer = GetBuffer();

		copySize = min( nBytes, (WAVE_OUT_BUF_SIZE-m_hdrOutPos[m_currentBuffer])/2 );

		if( m_hdrOutPos[m_currentBuffer] + (copySize*2) > WAVE_OUT_BUF_SIZE  )//||
//			writePos + copySize > (WAVE_OUT_BUF_SIZE) )		// Input buffer is also 4K
		{
			dlg->outputDebug(_T("Size > MAX Size"));
			DebugBreak();
		}

		cvtBuffer(&m_hdrOut[m_currentBuffer].lpData[m_hdrOutPos[m_currentBuffer]],
			&data[writePos],
			copySize, audioFormat );

//		memcpy(d, data, copySize);

		if( theApp.UserOptions.noiseFilter )
			filterData( (short*)&m_hdrOut[m_currentBuffer].lpData[m_hdrOutPos[m_currentBuffer]], copySize );

		writePos += copySize;
		m_hdrOutPos[m_currentBuffer] += (copySize*2);						// We copied shorts, not chars

		nBytes -= copySize;													// advance the input buff by chars

		if( m_hdrOutPos[m_currentBuffer] == WAVE_OUT_BUF_SIZE )
		{
			m_hdrOutPos[m_currentBuffer] = 0;
			m_hdrOut[m_currentBuffer].dwUser = m_currentBuffer+1;			// This header is in use
			m_hdrOut[m_currentBuffer].dwBufferLength = WAVE_OUT_BUF_SIZE;
			m_hdrOut[m_currentBuffer].dwBytesRecorded = WAVE_OUT_BUF_SIZE;
			m_hdrOut[m_currentBuffer].dwFlags &= ~(WHDR_DONE|WHDR_INQUEUE);
//			m_hdrOut[m_currentBuffer].dwFlags = 0;
#if 1
//			waveOutPrepareHeader(m_hOut, &m_hdrOut[m_currentBuffer], sizeof(WAVEHDR));
			rc = waveOutWrite(m_hOut, &m_hdrOut[m_currentBuffer], sizeof(WAVEHDR));	// MMSYSERR_NODRIVER WAVERR_UNPREPARED
#else
			waveOutPrepareHeader(m_hOut, &m_hdrOut[m_currentBuffer], sizeof(WAVEHDR));
			m_hdrOut[m_currentBuffer].dwUser = 0;
			m_hdrOut[m_currentBuffer].dwBytesRecorded = 0;
			waveOutUnprepareHeader(m_hOut, &m_hdrOut[m_currentBuffer], sizeof(WAVEHDR));
#endif
			m_currentBuffer = -1;			// None selected
		}
	}

	guard--;
//	dlg->outputDebug("Write done of %d Bytes", nBytes);
}

int DAAudioStuff::GetBuffer()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	time_t lastDisplayed=0;

	int bufNum=0;
	while( 1 )
	{
		if( m_hdrOut[bufNum].dwUser == 0 )
			break;

		bufNum = (bufNum + 1) % NUM_BUFS;

		if( bufNum == 0 )
		{
			if( lastDisplayed != time(NULL) )
			{
				::OutputDebugString("Waiting for buffer\n");
				lastDisplayed = time(NULL);
			}

			dlg->DoEvents();
		}
	}

//	static int cnt=1;
//	char buf[100];
//	sprintf(buf, "%3d) Using Buffer %d\n", cnt++, bufNum);
//	::OutputDebugString(buf);

	return(bufNum);
}

void DAAudioStuff::StartMic()
{
	if( m_bInStarted == FALSE )
	{
		for(int i=0;i<NUM_BUFS;i++)
			waveInAddBuffer(m_hIn, &m_hdrIn[i], sizeof(WAVEHDR));

		waveInStart(m_hIn);
	}

	m_bInStarted = TRUE;
}

void DAAudioStuff::StopMic()
{
	time_t timeout = time(NULL) + 2;			// Don't want to loop forever
//	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	if(m_bInStarted)
	{
		m_bInStarted = FALSE;

//		dlg->outputDebug("StopMic: Stopping Microphone");
//		waveInReset(m_hIn);

//		while( inQueue() && timeout > time(NULL) )		// Wait for all of the microphone buffers to be used up before we stop
//			Sleep(1);

		waveInStop(m_hIn);
//		dlg->outputDebug("StopMic: Microphone Stopped");

//		Close();
//		Open();
	}

	m_bInStarted = FALSE;
}

void DAAudioStuff::setupOutBuffer(int bufnum)
{
	MMRESULT	rc;

	memset(&m_hdrOut[bufnum], 0, sizeof(WAVEHDR));
	m_hdrOut[bufnum].dwBufferLength = WAVE_OUT_BUF_SIZE;
#if USE_VIRTUAL_MEMORY
	m_hdrOut[bufnum].lpData = (LPSTR)VirtualAlloc(0, WAVE_OUT_BUF_SIZE, MEM_COMMIT, PAGE_READWRITE);
#else
	m_hdrOut[bufnum].lpData = (LPSTR)malloc(WAVE_OUT_BUF_SIZE);
#endif
	rc = waveOutPrepareHeader(m_hOut, &m_hdrOut[bufnum], sizeof(WAVEHDR));
	m_hdrOutAvailable[bufnum] = TRUE;
	m_hdrOutPos[bufnum] = 0;
	
}

void DAAudioStuff::setupInBuffer(int bufnum)
{
	memset(&m_hdrIn[bufnum], 0, sizeof(WAVEHDR));
	m_hdrIn[bufnum].dwBufferLength = m_micBufferSize;
	m_hdrIn[bufnum].lpData = (LPSTR)malloc(m_micBufferSize+10);
	waveInPrepareHeader(m_hIn, &m_hdrIn[bufnum], sizeof(WAVEHDR));
//	waveInAddBuffer(m_hIn, &m_hdrIn[bufnum], sizeof(WAVEHDR));
}

BOOL DAAudioStuff::inQueue()
{
	int i;

	for(i=0;i<NUM_BUFS;i++)
		if( m_hdrIn[i].dwFlags & WHDR_INQUEUE )
			return(TRUE);

	return(FALSE);
}

void DAAudioStuff::filterData( short *data, int len )
{
	int i;
	unsigned limLo = 1024;					// Below this point it is noise
	unsigned limHi = 2048;
	unsigned avg = 0;
	BOOL	bFilter = FALSE;
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	static time_t lastTime=0;

	for(i=0;i<len;i++)
		avg += abs(data[i]);

	avg /= len;

	if( avg < limLo )
		bFilter = TRUE;
	else if( avg > limHi && filterAvg1 > limHi && filterAvg2 > limHi )
		bFilter = FALSE;
	else if( avg > 10*1024 )
	{
		filterAvg1 = filterAvg2 = avg;
		bFilter = FALSE;
	}

	filterAvg2 = filterAvg1;
	filterAvg1 = avg;

	if( bFilter )
	{
		for(i=0;i<len;i++)
			data[i] = ((rand()%5)-2)*256;
	}

	if( lastTime != time(NULL) )
	{
		lastTime = time(NULL);

		dlg->outputDebug("AVG = %d", avg);
	}
}