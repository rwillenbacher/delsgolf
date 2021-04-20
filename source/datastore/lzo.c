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


#define MAGIC 0xfd

int datastore_lzo_enc(unsigned char *pui8_in, int i_len, unsigned char *pui8_out )
{
	int i_outbytes = 0;
	int i, j, k, i_search, i_skipped;
	int i_pos, i_bestpos, i_bestskip;
  
	for(i = 0; i < i_len; i += i_skipped)
	{
		i_pos = i - 0xfff;
		if( i_pos < 0)
		{
			i_pos = 0;
		}

		i_search = *(int *)&pui8_in[ i ];
		i_skipped = 0;

		i_bestpos = 0;
		i_bestskip = 0;

		for(j = i_pos; j < i-20; j++)
		{
			if( *(int*)&pui8_in[ j ] == i_search && ( i + 4 ) < i_len)
			{
				for(k = 0; k < 255 + 16 && ( j + k + 4 ) < i && ( i + k + 4 ) < i_len; k++)
				{
					if( pui8_in[ j + 4 + k ] != pui8_in[ i + 4 + k ] )
					{
						break;
					}
				}
				if(k > 255 + 15 )
				{
					k = 255 + 15;
				}

				if( k + 4 > i_bestskip )
				{
					i_bestskip = k + 4;
					i_bestpos = j;
				}
			}
		}

		if( i_bestskip )
		{
			int i_length_small, i_length_big;

			i_length_small = i_bestskip - 4 >= 15 ? 15 + 4 : i_bestskip;
			i_length_big = ( i_bestskip - 4 >= 15 ) ? ( i_bestskip - 4 - 15 ) : 0;
			//pui8_out[ i_outbytes ] = ( MAGIC >> 8 ) & 0xff;
			pui8_out[ i_outbytes + 0 ] = ( MAGIC ) & 0xff;
			pui8_out[ i_outbytes + 1 ] = ( ( ( ( i - i_bestpos) <<4) | ( i_length_small - 4 ) ) >> 8 ) & 0xff ;
			pui8_out[ i_outbytes + 2 ] = ( ( ( ( i - i_bestpos) <<4) | ( i_length_small - 4 ) ) ) & 0xff;
			if( i_length_small == 15 + 4  )
			{
				pui8_out[ i_outbytes + 3 ] = i_length_big & 0xff;
				i_outbytes += 1;
			}
			i_outbytes += 3;
			i_skipped = i_bestskip;
		}

		if( /*( pui8_in[i] ) == ( ( MAGIC >> 8 ) & 0xff ) && */ pui8_in[ i + 0 ] == ( MAGIC & 0xff ) && !i_skipped )
		{
			/*pui8_out[ i_outbytes ] = ( MAGIC >> 8 ) & 0xff; */
			pui8_out[ i_outbytes + 0 ] = ( MAGIC ) & 0xff;
			pui8_out[ i_outbytes + 1 ] = 0;
			pui8_out[ i_outbytes + 2 ] = 0;
			i_outbytes += 3;
			i_skipped = 1;
		}
		if( !i_skipped)
		{
			i_skipped = 1;
			pui8_out[ i_outbytes++ ] = pui8_in[ i ];
		}
	}
	return i_outbytes;
}


int datastore_lzo_dec( unsigned char *pui8_in, int len, unsigned char *pui8_out, int i_out_len )
{
  int i_outbytes = 0;
  int i, j, i_skipped;
  int i_pos;
  
	for(i = 0; i < len; i += i_skipped)
	{
		i_skipped = 0;
		if( i < len )
		{
			if( /*pui8_in[ i ] == ( MAGIC >> 8 ) && */ pui8_in[ i + 0 ] == ( MAGIC & 0xff ) )
			{
				int i_extra = 0;
				i_pos = pui8_in[ i + 1 ] << 8;
				i_pos |= pui8_in[ i + 2 ];
				if( i_pos )
				{
					i_skipped = 4 + ( i_pos & 0xf );
					if( ( i_pos & 0xf ) == 15 )
					{
						i_skipped += pui8_in[ i + 3 ];
						i_extra = 1;
					}
					i_pos = i_outbytes - ( i_pos >> 4 );
					for(j = 0; j < i_skipped; j++)
					{
						pui8_out[ i_outbytes++ ] = pui8_out[ i_pos + j ];
					}
				}
				else
				{
					/*pui8_out[ i_outbytes ] = MAGIC >> 8; */
					pui8_out[ i_outbytes + 0 ] = MAGIC & 0xff;
					i_outbytes += 1;
				}
				i_skipped = 3 + i_extra;
			}
		}
		if( !i_skipped )
		{
			i_skipped = 1;
			pui8_out[ i_outbytes++ ] = pui8_in[ i ];
		}
	}
	return i_outbytes;
}

