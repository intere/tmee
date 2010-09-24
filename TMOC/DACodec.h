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
#pragma once

short ulaw_to_s16(unsigned char u_val);
short pcm8_to_s16(unsigned char u_val);

unsigned char s16_to_ulaw(short b);			// from 16 bit linear 
unsigned char s16_to_pcm8(short b);

int calcSegment(int val);

void cvtBuffer( void *outputBuffer, void *inputBuffer, int samples, int type );
