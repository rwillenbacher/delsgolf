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

#include "eng.h"

volatile UInt16 i_sound_enabled = 0;

tones_t s_tones;

static const UInt16 rgui16_sndtab[ 9 ] = { 0, 0xaaaa >> 1, 0x6db6 >> 1, 0xcccc >> 2, 0x739c >> 2, 0xe38 >> 3, 0x3c78 >> 3, ( 0x3c78 << 1 ) | 1, 0x80aa };
static const UInt8 rgui8_sndtab_length[ 9 ] = { 16, 16, 15, 16, 15, 12, 14, 16, 16 };

static const Int8 rgui8_sounds[][ 5 ] = { { 0, 0, 0, 0, 0 }, { 5, 1, 3, 4, 0 }, { 1, 2, 1, 0, 0 }, { 7, 7, 0, 0, 0 }, { 8, 0, 0, 0, 0 }, { 1, 1, 1, 5, 0 } };

void enable_sound( void )
{
	volatile UInt8 *pui8_link_control = ( UInt8 *)0x60000c;
	volatile UInt8 *pui8_sound_port = ( UInt8 *)0x60000e;

	memset( &s_tones, 0, sizeof( s_tones ) );

#ifndef WIN32
	pokeIO_bset( pui8_link_control, 5 );
	pokeIO_bset( pui8_link_control, 6 );
#endif

	*pui8_sound_port &= 0xfc;
	i_sound_enabled = 1;
}

void disable_sound( void )
{
	volatile UInt8 *pui8_link_control = ( UInt8 *)0x60000c;
	volatile UInt8 *pui8_sound_port = ( UInt8 *)0x60000e;

	i_sound_enabled = 2;

	while( i_sound_enabled != 0 )
	{
	}

	*pui8_sound_port &= 0xfc;
#ifndef WIN32
	pokeIO_bclr( pui8_link_control, 5 );
	pokeIO_bclr( pui8_link_control, 6 );
	OSLinkReset();
#endif
}

void on_sound_interrupt( void )
{
	Int16 i_c1, i_c2, i_idx;
	volatile UInt8 *pui8_sound_port = ( UInt8 *)0x60000e;
	tones_t *ps_tones = &s_tones;

	if( i_sound_enabled == 1 )
	{
		for( i_idx = 0; i_idx < 2; i_idx++ )
		{
			Int16 i_ticks;
			i_ticks = ps_tones->rgi8_channel_remain[ i_idx ];
			if( i_ticks == 0 )
			{
				Int16 i_sound;
				Int16 i_tone = 0;
				i_sound = ps_tones->rgi8_channel_sounds[ i_idx ];
				if( i_sound > 0 )
				{
					Int16 i_step = ps_tones->rgi8_channel_sounds_pos[ i_idx ];
					i_tone = rgui8_sounds[ i_sound ][ i_step++ ];
					if( i_tone == 0 )
					{
						ps_tones->rgi8_channel_sounds[ i_idx ] = 0;
					}
					else
					{
						ps_tones->rgi8_channel_sounds_pos[ i_idx ] = i_step;
					}
				}
				i_ticks = rgui8_sndtab_length[ i_tone ];
				ps_tones->rgui16_channel_remainder[ i_idx ] = rgui16_sndtab[ i_tone ];
			}
			i_ticks--;
			ps_tones->rgi8_channel_remain[ i_idx ] = i_ticks;
		}

		i_c1 = ps_tones->rgui16_channel_remainder[ 0 ];
		i_c2 = ps_tones->rgui16_channel_remainder[ 1 ];
		ps_tones->rgui16_channel_remainder[ 0 ] = i_c1 >> 1;
		ps_tones->rgui16_channel_remainder[ 1 ] = i_c2 >> 1;

		i_c1 = ( i_c1 & 1 ) | ( ( i_c2 & 1 ) << 1 );
		*pui8_sound_port = i_c1;
	}
	else if( i_sound_enabled == 2 )
	{
		i_sound_enabled = 0;
	}
}

void play_sound( Int16 i_sound, Int16 i_channel )
{
	tones_t *ps_tones = &s_tones;

	ps_tones->rgi8_channel_sounds[ i_channel ] = -1;
	ps_tones->rgi8_channel_sounds_pos[ i_channel ] = 0;
	ps_tones->rgi8_channel_sounds[ i_channel ] = i_sound;
}

