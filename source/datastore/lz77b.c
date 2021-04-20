/*
Copyright (c) 2013-2015, Ralf Willenbacher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <stdio.h>

#define MIN_CLEN 2
#define MAX_CLEN ( 15 + MIN_CLEN )
#define WINDOW_SIZE 0xfff

int datastore_lz77_enc(unsigned char *pui8_in, int i_len, unsigned char *pui8_out )
{
	int i_outbytes = 0;
	int i, j, k, i_skipped;
	int i_cflags, i_cflagspos, i_cflagsbit;
	int i_pos, i_bestpos, i_bestskip;
  
	i_cflags = 0;
	i_cflagspos = 0;
	i_cflagsbit = 8;
	i_outbytes = 1;

	for(i = 0; i < i_len; i += i_skipped)
	{

		if( i_cflagsbit == 0 )
		{
			pui8_out[ i_cflagspos ] = i_cflags;
			i_cflags = 0;
			i_cflagsbit = 8;
			i_cflagspos = i_outbytes++;
		}

		i_pos = i - WINDOW_SIZE;
		if( i_pos < 0)
		{
			i_pos = 0;
		}

		i_skipped = 0;
		i_bestpos = 0;
		i_bestskip = 0;

		for(j = i_pos; j < i; j++)
		{
			k = 0;
			while( pui8_in[ j + k ] == pui8_in[ i + k ] && k < MAX_CLEN && i + k < i_len )
			{
				k++;
			}
			if( k > i_bestskip )
			{
				i_bestskip = k;
				i_bestpos = i - 1 - j;
			}
		}

		if( i_bestskip && i_bestskip >= MIN_CLEN )
		{
			i_cflags |= 1 << ( --i_cflagsbit );

			pui8_out[ i_outbytes ] = ( ( ( i_bestskip - MIN_CLEN ) << 4 ) | ( i_bestpos >> 8 ) ) & 0xff; 
			pui8_out[ i_outbytes + 1 ] = i_bestpos & 0xff;
			i_outbytes += 2;
			i_skipped = i_bestskip;
		}
		else
		{
			--i_cflagsbit;
			i_skipped = 1;
			pui8_out[ i_outbytes++ ] = pui8_in[ i ];
		}
	}
	if( i_cflagsbit != 8 )
	{
		pui8_out[ i_cflagspos ] = i_cflags;
	}
	return i_outbytes;
}


int datastore_lz77_dec( unsigned char *pui8_in, int len, unsigned char *pui8_out )
{
	int i_outbytes = 0;
	int i, j, cflags, cflagsbit, i_skipped;
	int i_pos;
  
	cflagsbit = 0;
	for(i = 0; i < len; (void)i )
	{
		if( cflagsbit == 0 )
		{
			cflags = pui8_in[ i++ ];
			cflagsbit = 8;
		}
		if( cflags & ( 1 << ( --cflagsbit ) ) )
		{
			int i_pos, i_len;

			i_len = pui8_in[ i ] >> 4;
			i_pos = ( ( ( pui8_in[ i ] & 0xf ) << 8 ) | pui8_in[ i + 1 ] ) + 1; 
			i += 2;
			i_len += MIN_CLEN;

			while( i_len-- )
			{
				pui8_out[ i_outbytes ] = pui8_out[ i_outbytes - i_pos ];
				i_outbytes++;
			}
		}
		else
		{
			pui8_out[ i_outbytes++ ] = pui8_in[ i++ ];
		}
	}
	return i_outbytes;
}

