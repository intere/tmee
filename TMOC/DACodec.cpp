//
// DACodec.cpp : implementation file
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
#include "windowsx.h"
#include "TalkMasterConsole.h"
#include "TalkMasterConsoleDlg.h"
#include "DACodec.h"
#include "shellapi.h"

#define BIAS 0x84

static short seg_end[] = 
      {0xFF, 
       0x1FF, 
       0x3FF, 
       0x7FF, 
       0xFFF, 
       0x1FFF, 
       0x3FFF, 
       0x7FFF}; 


/*
 * ulaw_to_s16() - Convert a u-law value to 16-bit linear PCM
 *
 * First, a biased linear code is derived from the code word. An unbiased
 * output can then be obtained by subtracting 33 from the biased code.
 *
 * Note that this function expects to be passed the complement of the
 * original code word. This is in keeping with ISDN conventions.
 */
short ulaw_to_s16(unsigned char u_val)
{
	short t;

	/* Complement to obtain normal u-law value. */
	u_val = ~u_val;

	/*
	 * Extract and bias the quantization bits. Then
	 * shift up by the segment number and subtract out the bias.
	 */
	t = ((u_val & 0x0f) << 3) + (unsigned)0x84;
	t <<= (u_val & 0x70) >> 4;

	return ((u_val & (unsigned)0x80) ? ((unsigned)0x84 - t) : (t - (unsigned)0x84));
}

/* 
 * Convert a linear 16-bit PCM value to 8-bit u-law 
 * 
 * In order to simplify the encoding process, the original linear magnitude 
 * is biased by adding 33 which shifts the encoding range from (0 - 8158) to 
 * (33 - 8191). The result can be seen in the following encoding table: 
 * 
 *      Biased Linear Input Code        Compressed Code 
 *      ------------------------        --------------- 
 *      00000001wxyza                   000wxyz 
 *      0000001wxyzab                   001wxyz 
 *      000001wxyzabc                   010wxyz 
 *      00001wxyzabcd                   011wxyz 
 *      0001wxyzabcde                   100wxyz 
 *      001wxyzabcdef                   101wxyz 
 *      01wxyzabcdefg                   110wxyz 
 *      1wxyzabcdefgh                   111wxyz 
 *  
 * Each biased linear code has a leading 1 which identifies the segment 
 * number. The value of the segment number is equal to 7 minus the number 
 * of leading 0's. The quantization interval is directly available as the 
 * four bits wxyz.  * The trailing bits (a - h) are ignored. 
 * 
 * Ordinarily the complement of the resulting code word is used for 
 * transmission, and so the code word is complemented before it is returned. 
 *  
 * For further information see John C. Bellamy's Digital Telephony, 1982, 
 * John Wiley & Sons, pps 98-111 and 472-476. 
 */ 

unsigned char s16_to_ulaw(short pcm_val) // from 16 bit linear 
{ 
	unsigned char mask, retData, uval;
	int seg;

    /* Get the sign and the magnitude of the value. */ 
	if (pcm_val < 0) 
	{ 
		pcm_val = (short)(BIAS - pcm_val); 
		mask = 0x7F; 
	} 
	else 
	{ 
		pcm_val += BIAS; 
		mask = 0xFF; 
	} 


/* Convert the scaled magnitude to segment number. */ 
    seg = calcSegment(pcm_val); 

/* 
 * Combine the sign, segment, quantization bits; 
 * and complement the code word. 
 */ 
    if (seg >= 8)  /* out of range, return maximum value. */ 
        retData =  (unsigned char)(0x7F ^ mask); 
    else 
	{ 
		uval = (unsigned char)((seg << 4) | ((pcm_val >> (seg + 3)) & 0xF)); 
		retData = (unsigned char)(uval ^ mask); 
	} 

	return(retData);
}

int calcSegment(int val) 
{ 
	for (int i = 0; i < 8; i++) 
		if (val <= seg_end[i])
			return (i); 

	return 8; 
} // end calcSegment 

unsigned char s16_to_pcm8(short pcm_val) // from 16 bit linear 
{ 
	return((pcm_val>>8) ^ 0x80);
}

short pcm8_to_s16(unsigned char u_val)
{
	return((u_val^0x80) << 8);
}

void cvtBuffer( void *outputBuffer, void *inputBuffer, int samples, int type )
{
	short *pOutput = (short *)outputBuffer;
	unsigned char *pInput = (unsigned char *)inputBuffer;

	while( samples-- )
	{
		if( type == WAVE_FORMAT_PCM )
			*(pOutput++) = pcm8_to_s16( *(pInput++) );
		else if( type == WAVE_FORMAT_ULAW )
			*(pOutput++) = ulaw_to_s16( *(pInput++) );
	}
}
