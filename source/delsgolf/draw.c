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

void draw_string( engine_t *ps_eng, Int16 i_bigfont, Int16 i_x, Int16 i_y, Int16 i_bg, UInt8 *pui8_char )
{
	UInt8 *pui8_drawbuffer_top, *pui8_drawbuffer, ui_charline, *pui8_cdata;
	Int16 i_cx_frac, i_cy, i_cwidth, i_char_width, i_char_height;
	char_t *ps_char;

	pui8_drawbuffer_top = ( UInt8 * )ps_eng->p_drawbuffer;
	pui8_drawbuffer_top += ps_eng->rgui16_ylut[ i_y ];

	while( *( pui8_char ) )
	{
		if( i_bigfont )
		{
			Int16 i_c = 38;
			if( *pui8_char >= '0' && *pui8_char <= '9' )
			{
				i_c = *pui8_char - '0';
			}
			else if( *pui8_char >= 'A' && *pui8_char <= ( 'Z' + 2 ) )
			{
				i_c = *pui8_char - 'A' + 10;
			}
			pui8_cdata = &ps_eng->rgui8_big_characters[ i_c * 8 ];
			i_char_width = 8;
			i_char_height = 8;
		}
		else
		{
			ps_char = &ps_eng->rgs_characters[( *pui8_char ) & 0x7f ];
			pui8_cdata = &ps_char->rgui8_data[ 0 ];
			i_char_width = ps_char->ui8_width;
			i_char_height = 6;
		}
		pui8_char++;

		for( i_cy = 0; i_cy < i_char_height; i_cy++ )
		{
			i_cwidth = i_char_width;
			ui_charline = *(pui8_cdata++);

			pui8_drawbuffer = pui8_drawbuffer_top + ( i_x >> 2 ) + ( ps_eng->rgui16_ylut[ i_cy ] );

			i_cx_frac = 3 - ( i_x & 3 );

			while( i_cwidth-- )
			{
				UInt8 ui8_mask, ui8_mask_inv;
				ui8_mask_inv = ( ( 0x11 ) << ( i_cx_frac ) );
				ui8_mask = ~ui8_mask_inv;

				if( ui_charline & 0x80 )
				{
					*( pui8_drawbuffer ) = *pui8_drawbuffer | ui8_mask_inv;
				}
				else if( i_bg > 0 )
				{
					*( pui8_drawbuffer ) = ( *pui8_drawbuffer & ui8_mask );
				}
				if( i_cx_frac-- == 0 )
				{
					i_cx_frac = 3;
					pui8_drawbuffer++;
				}
				ui_charline <<= 1;
			}
		}
		i_x += i_char_width;
	}
}


#ifdef WIN32

void eng_draw_texture_hspan( UInt8 *pui8_drawbuffer, Int16 frac_x, Int16 i_length, UInt8 *pui8_tex, Int32 i_tu, Int32 i_u_xstep, Int32 i_tv, Int32 i_v_xstep )
{
	while( i_length > 0 )
	{
		UInt8 ui8_texel;

		ui8_texel = pui8_tex[ ( ( i_tu >> 16 ) & 0x1f ) + ( ( ( i_tv >> 16 ) & 0x1f ) * 256 ) ];
		i_tu += i_u_xstep;
		i_tv += i_v_xstep;

		switch( frac_x )
		{
			case 0:
				*pui8_drawbuffer = ( *pui8_drawbuffer & 0x77 ) | ( ui8_texel << 3 );
			break;
			case 1:
				*pui8_drawbuffer = ( *pui8_drawbuffer & 0xbb ) | ( ui8_texel << 2 );
			break;
			case 2:
				*pui8_drawbuffer = ( *pui8_drawbuffer & 0xdd ) | ( ui8_texel << 1 );
			break;
			case 3:
				*pui8_drawbuffer = ( *pui8_drawbuffer & 0xee ) | ui8_texel;
				pui8_drawbuffer++;
				frac_x = -1;
			break;
		}
		frac_x++;
		i_length--;
	}
}


void eng_draw_spritespan0( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev )
{
	Int16 i_y;

	i_y = 0;
	while( i_y++ < i_height )
	{
		UInt8 ui8_texel;
		ui8_texel = pui8_tex[ i_v >> 16 ];
		i_v += i_scalev;

		if( !(ui8_texel & 0x4) )
		{
			*pui8_drawbuffer = ( *pui8_drawbuffer & 0x77 ) | ui8_texel << 3;
		}
		pui8_drawbuffer += DRAWBUFFER_SCREEN_WIDTH / 4;
	}
}

void eng_draw_spritespan1( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev )
{
	Int16 i_y;

	i_y = 0;
	while( i_y++ < i_height )
	{
		UInt8 ui8_texel;
		ui8_texel = pui8_tex[ i_v >> 16 ];
		i_v += i_scalev;

		if( !(ui8_texel & 0x4) )
		{
			*pui8_drawbuffer = ( *pui8_drawbuffer & 0xbb ) | ( ui8_texel << 2 );
		}
		pui8_drawbuffer += DRAWBUFFER_SCREEN_WIDTH / 4;
	}
}

void eng_draw_spritespan2( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev )
{
	Int16 i_y;

	i_y = 0;
	while( i_y++ < i_height )
	{
		UInt8 ui8_texel;
		ui8_texel = pui8_tex[ i_v >> 16 ];
		i_v += i_scalev;

		if( !(ui8_texel & 0x4) )
		{
			*pui8_drawbuffer = ( *pui8_drawbuffer & 0xdd ) | ( ui8_texel << 1 );
		}
		pui8_drawbuffer += DRAWBUFFER_SCREEN_WIDTH / 4;
	}
}

void eng_draw_spritespan3( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev )
{
	Int16 i_y;

	i_y = 0;
	while( i_y++ < i_height )
	{
		UInt8 ui8_texel;
		ui8_texel = pui8_tex[ i_v >> 16 ];
		i_v += i_scalev;

		if( !( ui8_texel & 0x4 ) )
		{
			*pui8_drawbuffer = ( *pui8_drawbuffer & 0xee ) | ( ui8_texel );
		}
		pui8_drawbuffer += DRAWBUFFER_SCREEN_WIDTH / 4;
	}
}


UInt32 eng_draw_vspan_c( UInt8 *pui8_drawbuffer, Int16 i_xfrac, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv )
{
	Int16 i_y;
	UInt8 rgui8_shift[ 4 ] = { 3, 2, 1, 0 };
	UInt8 rgui8_and[ 4 ] = { 0x88, 0x44, 0x22, 0x11};

	i_y = 0;
	while( i_y++ < i_height )
	{
		UInt8 ui8_texel;
		ui8_texel = pui8_tex[ ( i_v >> 16 ) & 0x1f ];
		i_v += i_y_stepv;

		*pui8_drawbuffer = *pui8_drawbuffer | ( ui8_texel & rgui8_and[ i_xfrac ] );

		pui8_drawbuffer += DRAWBUFFER_SCREEN_WIDTH / 4;
	}
	return i_v;
}

UInt32 eng_draw_vspan0( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv )
{
	return eng_draw_vspan_c( pui8_drawbuffer, 0, i_height, pui8_tex, i_v, i_y_stepv );
}

UInt32 eng_draw_vspan1( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv )
{
	return eng_draw_vspan_c( pui8_drawbuffer, 1, i_height, pui8_tex, i_v, i_y_stepv );
}

UInt32 eng_draw_vspan2( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv )
{
	return eng_draw_vspan_c( pui8_drawbuffer, 2, i_height, pui8_tex, i_v, i_y_stepv );
}

UInt32 eng_draw_vspan3( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv )
{
	return eng_draw_vspan_c( pui8_drawbuffer, 3, i_height, pui8_tex, i_v, i_y_stepv );
}

UInt32 eng_draw_vspanX( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv, Int16 i_xf )
{
	return eng_draw_vspan_c( pui8_drawbuffer, i_xf, i_height, pui8_tex, i_v, i_y_stepv );
}



#endif

