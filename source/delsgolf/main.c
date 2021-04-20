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

#ifdef WIN32
#include "win32_dsurface.h"
sys_win32_t s_win32;
#endif

const UInt8 rgui8_pel_tab[ 2 ][ 4 ] = { { 0x0, 0x01, 0x10, 0x11 }, { 0x0, 0x0f, 0xf0, 0xff } };
#define WITH_SOUND 1

#if PERFORMANCE_COUNTERS

UInt16 i_timer_map;
UInt16 i_timer_line_segment;
UInt16 i_timer_scan_vspan;
UInt16 i_timer_render_vspan;
UInt16 i_timer_render_vspan_inner;
UInt16 i_timer_scan_floor;
UInt16 i_timer_render_floor;
UInt16 i_timer_render_floor_inner;
UInt16 i_timer_flush_floor;
UInt16 i_timer_input;
UInt16 i_timer_sprites;
UInt16 i_timer_buffer;

#endif

#ifdef WIN32

UInt16 eng_get_time( )
{
//	return ps_eng->i_int5_counter;
	return ( UInt16 )GetTickCount();
}

#else

volatile UInt16 i_int5_counter = 0;

unsigned long i_int5_to_ms = 0;

extern volatile UInt16 i_sound_enabled;

DEFINE_INT_HANDLER (eng_int5_handler)
{
#if WITH_SOUND
	on_sound_interrupt();
#endif
	i_int5_counter++;
}

#define TIMER_DIVIDER 32
unsigned int eng_get_time( )
{
	return mul_u16_32_ns( i_int5_counter, i_int5_to_ms ) >> 8;
}


#endif


void eng_generate_commit_drawbuffer_lut( engine_t *ps_eng )
{
	Int16 i_idx;

	for( i_idx = 0; i_idx < 256; i_idx++ )
	{
		UInt8 ui8_pel;
		ui8_pel  = rgui8_pel_tab[ 0 ][ ( i_idx & 0xc0 ) >> 6 ] << 3;
		ui8_pel |= rgui8_pel_tab[ 0 ][ ( i_idx & 0x30 ) >> 4 ] << 2;
		ui8_pel |= rgui8_pel_tab[ 0 ][ ( i_idx & 0x0c ) >> 2 ] << 1;
		ui8_pel |= rgui8_pel_tab[ 0 ][ ( i_idx & 0x03 ) ];
		ps_eng->rgui8_bitmap_translation_tab[ i_idx ] = ui8_pel;
	}

	for( i_idx = 0; i_idx < DRAWBUFFER_SCREEN_HEIGHT; i_idx++ )
	{
		ps_eng->rgui16_ylut[ i_idx ] = i_idx * ( DRAWBUFFER_SCREEN_WIDTH >> 2 );
	}
	for( i_idx = 0; i_idx < REND_SCREEN_HEIGHT; i_idx++ )
	{
		Int16 i_y, i_tan;
		
		i_y = i_idx - ( REND_SCREEN_HEIGHT / 2 );
		ps_eng->rgi16_ritab[ i_idx ] = mul_16_16( i_y, INVERSE_VSCALE_SCALE ) >> 4;

		i_tan = ( REND_SCREEN_HEIGHT / 2 ) - i_idx;
		if( i_tan == 0 )
		{
			i_tan = 1;
		}
		ps_eng->rgi16_atan[ i_idx ] = div_16_16( YSCALE_CONST, i_tan );
	}
}



#ifndef WIN32


__attribute__((__stkparm__)) void eng_commit_drawbuffer_line( UInt8 *p_src, UInt8 *p_light, UInt8 *p_dark, Int16 i_bytes );
__attribute__((__stkparm__)) void eng_commit_drawbuffer_wh( UInt8 *pui8_src, UInt8 *pui8_plight, UInt8 *pui8_pdark, Int16 i_width, Int16 i_height );

void sys_ti89_commit_drawbuffer( engine_t *ps_eng, Int16 i_offs_x, Int16 i_offs_y, Int16 i_width, Int16 i_height )
{
	UInt8 *p_src, *p_light, *p_dark;

	p_light = GrayGetPlane( LIGHT_PLANE );
	p_dark = GrayGetPlane( DARK_PLANE );
	p_light += ( i_offs_x >> 3 ) + ( i_offs_y * ( 240 >> 3 ) );
	p_dark += ( i_offs_x >> 3 ) + ( i_offs_y * ( 240 >> 3 ) );

#if 1
	p_src = ps_eng->p_drawbuffer;
	eng_commit_drawbuffer_wh( p_src, p_light, p_dark, i_width >> 2, i_height );
#else
	{
		Int16 i_y;
		p_src = ps_eng->p_drawbuffer;
		for( i_y = 0; i_y < i_height; i_y++ )
		{
			eng_commit_drawbuffer_line( p_src, p_light, p_dark, i_width >> 2 );
			p_src += DRAWBUFFER_SCREEN_WIDTH >> 2;
			p_light += 240 >> 3;
			p_dark += 240 >> 3;
		}
	}
#endif
}
#endif

void eng_draw_commit_drawbuffer( engine_t *ps_eng )
{
#ifdef WIN32
	sys_win32_commit_drawbuffer( &s_win32, (UInt8*)ps_eng->p_drawbuffer, 0, 0, DRAWBUFFER_SCREEN_WIDTH, DRAWBUFFER_SCREEN_HEIGHT );
#else
	sys_ti89_commit_drawbuffer( ps_eng, 0, 0, DRAWBUFFER_SCREEN_WIDTH, DRAWBUFFER_SCREEN_HEIGHT );
#endif
}
void eng_draw_commit_renderbuffer( engine_t *ps_eng )
{
#ifdef WIN32
	sys_win32_commit_drawbuffer( &s_win32, (UInt8*)ps_eng->p_drawbuffer, REND_OFFSET_LEFT, REND_OFFSET_TOP, REND_SCREEN_WIDTH, REND_SCREEN_HEIGHT );
#else
	sys_ti89_commit_drawbuffer( ps_eng, REND_OFFSET_LEFT, REND_OFFSET_TOP, REND_SCREEN_WIDTH, REND_SCREEN_HEIGHT );
#endif
}

void eng_draw_commit_statusbar( engine_t *ps_eng )
{
#ifdef WIN32
	sys_win32_commit_drawbuffer( &s_win32, (UInt8*)ps_eng->p_drawbuffer, 48, 79, 64, 18 );
#else
	sys_ti89_commit_drawbuffer( ps_eng, 48, 79, 64, 18 );
#endif
}




//UInt16 i_backsector;

//UInt8 *pui8_tex, *pui8_drawbuffer;

//Int16 glob_pheight, glob_py, glob_x;
//Int16 i_pu;
//Int32 i_v, i_y_stepv;



/*
*/

#ifdef WIN32

UInt32 div_persp( UInt32 d )
{
	UInt32 dc, db, ui_dl;
	UInt32 q, n;

	n = 0xffffffff;

	db = 1;
	dc = d;

	if( dc < 0x1000000 )
	{
		db <<= 8;
		dc <<= 8;
	}
	if( dc < 0x1000000 )
	{
		db <<= 8;
		dc <<= 8;
	}

	while( !( dc & 0x80000000 ) )
	{
		dc <<= 1;
		db <<= 1;
	}

	q = 0;
	ui_dl = d << 15;

	while( db )
	{
		if( n >= dc )
		{
			n -= dc;
			q |= db;
		}
		dc >>= 1;
		db >>= 1;
	}


	return q;
}


UInt32 div_persp_combine( UInt32 i_persp, Int32 *pi_y_stepv, Int32 *pi_persp_u )
{
	UInt32 i_ipersp;

	i_ipersp = div_persp( i_persp );
	if( i_ipersp >= 0x10000 )
	{
		*pi_y_stepv = mul_32_u32( *pi_y_stepv, i_ipersp );
		*pi_persp_u = mul_16_u32( *pi_persp_u, i_ipersp );
	}
	else
	{
		*pi_y_stepv = mul_32_u16( *pi_y_stepv, i_ipersp );
		*pi_persp_u = mul_16_u16( *pi_persp_u, i_ipersp );
	}
	return i_ipersp;
}
#else
void div_persp_combine( UInt32 i_persp asm( "d0" ), Int32 *pi_y_stepv  asm( "a0" ), Int32 *pi_persp_u asm( "a1" ) );
#endif


#if 0

static void eng_draw_wall_segment_vlines( render_vline_gen_struct_t *ps_gstructv, render_vline_struct_t *ps_vlines, render_texture_params_t *ps_texture_paramsv )
{
	const render_texture_params_t *ps_texture_params = ( const render_texture_params_t *)ps_texture_paramsv;
	const render_vline_gen_struct_t *ps_gstruct = ( const render_vline_gen_struct_t * )ps_gstructv;
	UInt16 ui_timer_start;
	Int16 i_x, i_len;
	UInt8 *pui8_drawbuffer, *pui8_db, *pui8_tex;
	Int32 i_ru, i_rzi, i_v_offset;
	
	ps_vlines = ps_vlines + ps_gstruct->i16_x_start;
	i_len = ps_gstruct->i16_x_end - ps_gstruct->i16_x_start;
	
	pui8_drawbuffer = ps_gstruct->pui8_db;

	i_v_offset = ( ( Int32 )ps_gstruct->i_v_offset ) << 12;

	i_x = ps_gstruct->i16_x_start;
	i_rzi = ( mul_u16_32_ns( i_x, ps_texture_params->i_zx ) ) + ps_texture_params->i_zbase;
	i_ru = mul_u16_32_ns( i_x, ps_texture_params->i_ux ) + ps_texture_params->i_ubase; 

	while( i_len-- )
	{
		Int32 i_zi, i_u, i_tu;
		Int16 i_py, i_pheight, i_pu;
		UInt16 i_z;
		Int32 i_v, i_y_stepv;
		Int32 i_persp;
		Int32 i_persp_u;

		i_py = ps_vlines->i_py;
		i_pheight = ps_vlines->i_pheight;

		if( i_pheight > i_py )
		{
			i_zi = i_rzi;
			if( i_zi < 0x1000 )
			{
				i_zi = 0x1000;
			}
			i_z = div_u32_u16( 0x7ffffff>>8, i_zi >> 8 );
			
			i_u = i_ru;
			i_u = ( ( UInt16 )( ( mul_16_16( i_z, (i_u>>1) ) >> 16 ) << 2 ) ) + ( UInt16 )ps_texture_params->i_ut;

			i_pheight = ( i_pheight - i_py );


			i_pu = i_u;
			ps_vlines->i_saved_pu = i_pu;

			i_y_stepv = mul_u16_u16( ( INVERSE_VSCALE_SCALE >> 4 ), i_z );
			ps_vlines->i_saved_y_stepv = i_y_stepv;

			i_v = i_v_offset + mul_16_u32_ns( i_py - (REND_SCREEN_HEIGHT/2), i_y_stepv );


			pui8_db = pui8_drawbuffer + ps_gstruct->pui16_ylut[ i_py ] + ( i_x >> 2 );
			pui8_tex = ps_gstruct->pui8_tex + ( ( i_pu & 0x1f0 ) << 1 );
/*
#if PERFORMANCE_COUNTERS
			ui_timer_start = eng_get_time();
#endif
*/
			vspan_draw_funcs[ i_x & 3 ]( pui8_db, i_pheight, pui8_tex, i_v, i_y_stepv );
/*
#if PERFORMANCE_COUNTERS
			i_timer_vinner += eng_get_time() - ui_timer_start;
#endif
*/
		}
		else
		{
			ps_vlines->i_saved_y_stepv = 0;
		}
		i_x++;
		i_rzi += ps_texture_params->i_zx;
		i_ru += ps_texture_params->i_ux;
		ps_vlines++;
	}
}


static void eng_draw_wall_segment_vlines_lower( render_vline_gen_struct_t *ps_gstructv, render_vline_struct_t *ps_vlines, render_texture_params_t *ps_texture_paramsv )
{
	const render_texture_params_t *ps_texture_params = ( const render_texture_params_t *)ps_texture_paramsv;
	const render_vline_gen_struct_t *ps_gstruct = ( const render_vline_gen_struct_t * )ps_gstructv;
	UInt16 ui_timer_start;
	Int16 i_x, i_len;
	UInt8 *pui8_drawbuffer, *pui8_db, *pui8_tex;
	Int32 i_ru, i_rzi, i_v_offset;
	
	ps_vlines = ps_vlines + ps_gstruct->i16_x_start;
	i_len = ps_gstruct->i16_x_end - ps_gstruct->i16_x_start;
	
	pui8_drawbuffer = ps_gstruct->pui8_db;

	i_v_offset = ( ( Int32 )ps_gstruct->i_v_offset ) << 12;

	i_x = ps_gstruct->i16_x_start;
	i_rzi = ( mul_u16_32_ns( i_x, ps_texture_params->i_zx ) ) + ps_texture_params->i_zbase;
	i_ru = mul_u16_32_ns( i_x, ps_texture_params->i_ux ) + ps_texture_params->i_ubase; 

	while( i_len-- )
	{
		Int32 i_zi, i_z, i_u, i_tu;
		Int16 i_py, i_pheight, i_pu;
		Int32 i_v, i_y_stepv;
		Int32 i_persp;
		Int32 i_persp_u;

		i_py = ps_vlines->i_py;
		i_pheight = ps_vlines->i_pheight;

		if( i_pheight > i_py )
		{
			i_y_stepv = ps_vlines->i_saved_y_stepv;
			if( i_y_stepv != 0 )
			{
				i_pu = ps_vlines->i_saved_pu;
			}
			else
			{
				i_zi = i_rzi;
				if( i_zi < 0x1000 )
				{
					i_zi = 0x1000;
				}
				i_z = div_u32_u16( 0x7ffffff>>8, i_zi >> 8 );
			
				i_u = i_ru;
				i_u = ( ( mul_u16_32_ns( i_z, i_u ) >> 16 ) << 1 ) + ps_texture_params->i_ut;

				i_pheight = ( i_pheight - i_py );


				i_pu = i_u;
				ps_vlines->i_saved_pu = i_pu;

				i_z = mul_u16_32_ns( INVERSE_VSCALE_SCALE, i_z );
				i_y_stepv = i_z >> 4;
				ps_vlines->i_saved_y_stepv = i_y_stepv;
			}

			i_v = i_v_offset + mul_16_u32_ns( i_py - (REND_SCREEN_HEIGHT/2), i_y_stepv );


			pui8_db = pui8_drawbuffer + ps_gstruct->pui16_ylut[ i_py ] + ( i_x >> 2 );
			pui8_tex = ps_gstruct->pui8_tex + ( ( i_pu & 0x1f0 ) << 1 );
/*
#if PERFORMANCE_COUNTERS
			ui_timer_start = eng_get_time();
#endif
*/
			vspan_draw_funcs[ i_x & 3 ]( pui8_db, i_pheight, pui8_tex, i_v, i_y_stepv );
/*
#if PERFORMANCE_COUNTERS
			i_timer_vinner += eng_get_time() - ui_timer_start;
#endif
*/

		}
		i_x++;
		i_rzi += ps_texture_params->i_zx;
		i_ru += ps_texture_params->i_ux;
		ps_vlines++;
	}
}

static void eng_draw_wall_segment( engine_t *ps_eng )
{
	Int32 i_x_stepy0, i_x_stepy1, i_x_stepy2 = 0, i_x_stepy3 = 0;
	Int32 i_y0, i_y1, i_y2, i_y3, i_persp_u;
	Int16 i_width, i_x_seg_end, i_x_start, i_x_end;
	Int16 i_floor_start, i_floor_end, i_ceil_start, i_ceil_end;
#if PERFORMANCE_COUNTERS
	Int16 i_timer_start;
#endif
	Int16 i_idx, i_left_x, i_x;
	Int16 rgi_skiplist[ REND_SCREEN_WIDTH + 1 ], i_skiplist_length;
	UInt16 i_clip_left;
	UInt32 i_iwidth;
	Int16 b_any;

	line_t *ps_gline = ps_eng->ps_gline;

	i_width = ( ps_eng->s_line_segment.x2 - ps_eng->s_line_segment.x1 );
	if( i_width < 1 )
	{
		return;
	}

	i_left_x = i_x_start = ps_eng->s_line_segment.x1;
	i_x_seg_end = ps_eng->s_line_segment.x2;

	if( clip_span( ps_eng, &i_x_start, &i_x_seg_end, ps_eng->s_line_segment.ui8_flags & MAP_LINE_FLAGS_MIDDLE, &rgi_skiplist[ 0 ], &i_skiplist_length ) )
	{
		return;
	}

	i_iwidth = div_u32_u16( 0xffffffffUL, i_width );

	i_iwidth >>= 16;
	i_x_stepy0 = mul_32_u32( ps_eng->s_line_segment.y02 - ps_eng->s_line_segment.y01, i_iwidth );
	i_x_stepy1 = mul_32_u32( ps_eng->s_line_segment.y12 - ps_eng->s_line_segment.y11, i_iwidth );
	if( ps_eng->s_line_segment.ui8_flags & ( MAP_LINE_FLAGS_UPPER | MAP_LINE_FLAGS_LOWER ) )
	{
		i_x_stepy2 = mul_32_u32( ps_eng->s_line_segment.y22 - ps_eng->s_line_segment.y21, i_iwidth );
		i_x_stepy3 = mul_32_u32( ps_eng->s_line_segment.y32 - ps_eng->s_line_segment.y31, i_iwidth );
	}

	i_y0 = ps_eng->s_line_segment.y01;
	i_y1 = ps_eng->s_line_segment.y11;
	i_y2 = ps_eng->s_line_segment.y21;
	i_y3 = ps_eng->s_line_segment.y31;

	i_idx = 0;
	while( i_x_start < i_x_seg_end )
	{
		i_clip_left = i_x_start - i_left_x;
		i_x_end = i_x_start + rgi_skiplist[ i_idx++ ];

		i_left_x = i_x_end;

		if( ps_eng->i_mark_ceiling && !can_join_ceiling( ps_eng, i_x_start, i_x_end ) )
		{
			eng_flush_ceil( ps_eng );
		}
		if( ps_eng->i_mark_floor && !can_join_floor( ps_eng, i_x_start, i_x_end ) )
		{
			eng_flush_floor( ps_eng );
		}

		if( i_clip_left )
		{
			i_y0 += mul_u16_32_ns( i_clip_left, i_x_stepy0 );
			i_y1 += mul_u16_32_ns( i_clip_left, i_x_stepy1 );
			if( ps_eng->s_line_segment.ui8_flags & ( MAP_LINE_FLAGS_UPPER | MAP_LINE_FLAGS_LOWER ) )
			{
				i_y2 += mul_u16_32_ns( i_clip_left, i_x_stepy2 );
				i_y3 += mul_u16_32_ns( i_clip_left, i_x_stepy3 );
			}
		}


		i_floor_start = i_ceil_start = REND_SCREEN_WIDTH + 1;
		i_floor_end = i_ceil_end = 0;
		if( ps_eng->s_line_segment.ui8_flags & MAP_LINE_FLAGS_MIDDLE )
		{
			UInt8 *pui8_ceiling, *pui8_floor;
			pui8_ceiling = &ps_eng->rgui8_ceiling[ i_x_start ][ 0 ];
			pui8_floor = &ps_eng->rgui8_floor[ i_x_start ][ 0 ];

#if PERFORMANCE_COUNTERS
			i_timer_start = eng_get_time();
#endif
			b_any = FALSE;
			for( i_x = i_x_start; i_x < i_x_end; i_x++ )
			{
				Int16 i_yclip;
				Int16 i_py;
				Int16 i_pheight;
				//glob_x = i_x;

				i_yclip = ps_eng->rgui8_yclip[ i_x ][ 0 ];
				i_py = ( i_y0 + 0xffff ) >> 16;
				if( i_py < i_yclip )
				{
					i_py = i_yclip;
					*(pui8_ceiling++) = i_yclip;
					*(pui8_ceiling++) = i_yclip;
				}
				else
				{
					Int16 i_ceil_py;

					i_ceil_py = i_py;
					if( i_ceil_py > ps_eng->rgui8_yclip[ i_x ][ 1 ] )
					{
						i_ceil_py = ps_eng->rgui8_yclip[ i_x ][ 1 ];
					}
					*(pui8_ceiling++) = ( UInt8 )i_yclip;
					*(pui8_ceiling++) = ( UInt8 )i_ceil_py;
				}

				i_yclip = ps_eng->rgui8_yclip[ i_x ][ 1 ];
				i_pheight = i_y1 >> 16;
				if( i_pheight > i_yclip )
				{
					i_pheight = i_yclip;
					*(pui8_floor++) = ( UInt8 )i_yclip;
					*(pui8_floor++) = ( UInt8 )i_yclip;
				}
				else
				{
					Int16 i_floor_pheight;

					i_floor_pheight = i_pheight;
					if( i_floor_pheight < ps_eng->rgui8_yclip[ i_x ][ 0 ] )
					{
						i_floor_pheight = ps_eng->rgui8_yclip[ i_x ][ 0 ];
					}
					*(pui8_floor++) = ( UInt8 )i_floor_pheight;
					*(pui8_floor++) = ( UInt8 )i_yclip;
				}

				ps_eng->rgs_vlines[ i_x ].i_py = i_py;
				ps_eng->rgs_vlines[ i_x ].i_pheight = i_pheight;

				if( i_pheight > i_py )
				{
					ps_eng->rgui8_yclip[ i_x ][ 0 ] = ( UInt8 )i_pheight;
					ps_eng->rgui8_yclip[ i_x ][ 1 ] = ( UInt8 )i_py;
					b_any = TRUE;
				}

				i_y0 += i_x_stepy0;
				i_y1 += i_x_stepy1;
			}

			if( b_any )
			{
				render_vline_gen_struct_t s_gs;
				s_gs.i16_x_start = i_x_start;
				s_gs.i16_x_end = i_x_end;
				s_gs.pui16_ylut = ( UInt16 *)ps_eng->rgui16_ylut;
				s_gs.i_v_offset = ps_eng->world_transform.translation[ 2 ];
				s_gs.pui8_db = ( UInt8 * )ps_eng->p_drawbuffer;
				s_gs.pui8_tex = texture_get( ps_eng, ps_gline->ui8_texture_upper, TRUE );

				eng_draw_wall_segment_vlines( &s_gs, ps_eng->rgs_vlines, &ps_eng->s_tex_params );
			}


#if PERFORMANCE_COUNTERS
			i_timer_scan_espan += eng_get_time() - i_timer_start;
#endif
		}
		else
		{
			UInt8 *pui8_ceiling, *pui8_floor;
			UInt8 *pui8_tex_upper = NULL, *pui8_tex_lower = NULL;
			UInt16 i_scan_upper, i_scan_lower;

			pui8_ceiling = &ps_eng->rgui8_ceiling[ i_x_start ][ 0 ];
			pui8_floor = &ps_eng->rgui8_floor[ i_x_start ][ 0 ];

			i_scan_upper = ( ps_eng->s_line_segment.ui8_flags & MAP_LINE_FLAGS_UPPER ) && ( ps_eng->s_line_segment.y21 > 0 || ps_eng->s_line_segment.y22 > 0 ) && ( ps_eng->s_line_segment.y01 <= ( (long)REND_SCREEN_WIDTH << 16 ) || ps_eng->s_line_segment.y02 <= ( (long)REND_SCREEN_WIDTH << 16 ) );
			i_scan_lower = ( ps_eng->s_line_segment.ui8_flags & MAP_LINE_FLAGS_LOWER ) && ( ps_eng->s_line_segment.y11 > 0 || ps_eng->s_line_segment.y12 > 0 ) && ( ps_eng->s_line_segment.y31 <= ( (long)REND_SCREEN_WIDTH << 16 ) || ps_eng->s_line_segment.y32 <= ( (long)REND_SCREEN_WIDTH << 16 ) );


#if PERFORMANCE_COUNTERS
			i_timer_start = eng_get_time();
#endif
			if( i_scan_upper )
			{
				b_any = FALSE;
				for( i_x = i_x_start; i_x < i_x_end; i_x++ )
				{
					Int16 i_py, i_pheight;
					Int16 i_yclip;

					i_yclip = ps_eng->rgui8_yclip[ i_x ][ 0 ];

					i_py = ( i_y0 + 0xffff ) >> 16;
					if( i_py < i_yclip )
					{
						i_py = i_yclip;
						*(pui8_ceiling++) = i_yclip;
						*(pui8_ceiling++) = i_yclip;
					}
					else
					{
						Int16 i_ceil_py;

						i_ceil_py = i_py;
						if( i_ceil_py > ps_eng->rgui8_yclip[ i_x ][ 1 ] )
						{
							i_ceil_py = ps_eng->rgui8_yclip[ i_x ][ 1 ];
						}
						*(pui8_ceiling++) = ( UInt8 )i_yclip;
						*(pui8_ceiling++) = ( UInt8 )i_ceil_py;
					}

					i_yclip = ps_eng->rgui8_yclip[ i_x ][ 1 ];
					i_pheight = ( i_y2 >> 16 );
					if( i_pheight > i_yclip )
					{
						i_pheight = i_yclip;
					}

					ps_eng->rgs_vlines[ i_x ].i_py = i_py;
					ps_eng->rgs_vlines[ i_x ].i_pheight = i_pheight;

					if( i_pheight > i_py )
					{
						ps_eng->rgui8_yclip[ i_x ][ 0 ] = ( UInt8 )i_pheight;
						b_any = TRUE;
					}

					i_y0 += i_x_stepy0;
					i_y2 += i_x_stepy2;
				}
				if( b_any )
				{
					render_vline_gen_struct_t s_gs;
					s_gs.i16_x_start = i_x_start;
					s_gs.i16_x_end = i_x_end;
					s_gs.pui16_ylut = ( UInt16 *)ps_eng->rgui16_ylut;
					s_gs.i_v_offset = ps_eng->world_transform.translation[ 2 ];
					s_gs.pui8_db = ( UInt8 * )ps_eng->p_drawbuffer;
					s_gs.pui8_tex = texture_get( ps_eng, ps_gline->ui8_texture_upper, TRUE );

					eng_draw_wall_segment_vlines( &s_gs, ps_eng->rgs_vlines, &ps_eng->s_tex_params );
				}
			}
			else if( ps_eng->i_mark_ceiling )
			{
				Int32 i_py, i_pheight;
				Int32 i_yclip;

				for( i_x = i_x_start; i_x < i_x_end; i_x++ )
				{
					i_yclip = ps_eng->rgui8_yclip[ i_x ][ 0 ];
					i_py = ( i_y0 + 0xffff ) >> 16;
					if( i_py < i_yclip )
					{
						*(pui8_ceiling++) = ( UInt8 )i_yclip;
						*(pui8_ceiling++) = ( UInt8 )i_yclip;
					}
					else
					{
						if( i_py > ps_eng->rgui8_yclip[ i_x ][ 1 ] )
						{
							i_py = ps_eng->rgui8_yclip[ i_x ][ 1 ];
						}
						*(pui8_ceiling++) = ( UInt8 )i_yclip;
						*(pui8_ceiling++) = ( UInt8 )i_py;
						ps_eng->rgui8_yclip[ i_x ][ 0 ] = ( UInt8 )i_py;
					}
				}
				i_y0 += i_x_stepy0;
			}

			if( i_scan_lower )
			{
				b_any = FALSE;
				for( i_x = i_x_start; i_x < i_x_end; i_x++ )
				{
					Int16 i_py, i_pheight;
					Int16 i_yclip;

					i_yclip = ps_eng->rgui8_yclip[ i_x ][ 0 ];
					i_py = ( i_y3 + 0xffff ) >> 16;
					if( i_py < i_yclip )
					{
						i_py = i_yclip;
					}

					i_yclip = ps_eng->rgui8_yclip[ i_x ][ 1 ];
					i_pheight = ( i_y1 >> 16 );
					if( i_pheight > i_yclip )
					{
						i_pheight = i_yclip;
						*(pui8_floor++) = i_yclip;
						*(pui8_floor++) = i_yclip;
					}
					else
					{
						Int16 i_floor_pheight;

						i_floor_pheight = i_pheight;
						if( i_floor_pheight < ps_eng->rgui8_yclip[ i_x ][ 0 ] )
						{
							i_floor_pheight = ps_eng->rgui8_yclip[ i_x ][ 0 ];
						}
						*(pui8_floor++) = i_floor_pheight;
						*(pui8_floor++) = i_yclip;
					}

					ps_eng->rgs_vlines[ i_x ].i_py = i_py;
					ps_eng->rgs_vlines[ i_x ].i_pheight = i_pheight;

					if( i_pheight > i_py )
					{
						ps_eng->rgui8_yclip[ i_x ][ 1 ] = ( UInt8 )i_py;
						b_any = TRUE;
					}
					i_y1 += i_x_stepy1;
					i_y3 += i_x_stepy3;
				}
				if( b_any )
				{
					render_vline_gen_struct_t s_gs;
					s_gs.i16_x_start = i_x_start;
					s_gs.i16_x_end = i_x_end;
					s_gs.pui16_ylut = ( UInt16 *)ps_eng->rgui16_ylut;
					s_gs.i_v_offset = ps_eng->world_transform.translation[ 2 ];
					s_gs.pui8_db = ( UInt8 * )ps_eng->p_drawbuffer;
					s_gs.pui8_tex = texture_get( ps_eng, ps_gline->ui8_texture_lower, TRUE );

					if( i_scan_upper )
					{
						eng_draw_wall_segment_vlines_lower( &s_gs, ps_eng->rgs_vlines, &ps_eng->s_tex_params );
					}
					else
					{
						eng_draw_wall_segment_vlines( &s_gs, ps_eng->rgs_vlines, &ps_eng->s_tex_params );
					}
				}
			}
			else if( ps_eng->i_mark_floor )
			{
				for( i_x = i_x_start; i_x < i_x_end; i_x++ )
				{
					Int16 i_py, i_pheight;
					Int16 i_yclip;

					if( ps_eng->i_mark_floor )
					{
						i_yclip = ps_eng->rgui8_yclip[ i_x ][ 1 ];
						i_py = ( i_y1 >> 16 );
						if( i_py > ( Int16 )i_yclip )
						{
							*(pui8_floor++) = i_yclip;
							*(pui8_floor++) = i_yclip;
						}
						else
						{
							if( i_py < ps_eng->rgui8_yclip[ i_x ][ 0 ] )
							{
								i_py = ps_eng->rgui8_yclip[ i_x ][ 0 ];
							}
							*(pui8_floor++) = i_py;
							*(pui8_floor++) = i_yclip;
							ps_eng->rgui8_yclip[ i_x ][ 1 ] = ( UInt8 )i_py;
						}
					}
					i_y1 += i_x_stepy1;
				}
			}
#if PERFORMANCE_COUNTERS
			i_timer_scan_espan += eng_get_time() - i_timer_start;
#endif
		}

		if( ps_eng->i_mark_floor )
		{
			if( ps_eng->i_floor_minx > i_x_start )
				ps_eng->i_floor_minx = i_x_start;
			if( ps_eng->i_floor_maxx < i_x_end )
				ps_eng->i_floor_maxx = i_x_end ;
		}

		if( ps_eng->i_mark_ceiling )
		{
			if( ps_eng->i_ceil_minx > i_x_start )
				ps_eng->i_ceil_minx = i_x_start;
			if( ps_eng->i_ceil_maxx < i_x_end )
				ps_eng->i_ceil_maxx = i_x_end;
		}

		/* advance */
		i_x_start = i_x_end;
		if( i_idx < i_skiplist_length )
		{
			i_x_start += rgi_skiplist[ i_idx++ ];
		}
	}
}

static void eng_mark_ceil_floor_segment( engine_t *ps_eng )
{
	Int32 i_x_stepy0, i_x_stepy1;
	Int32 i_x, i_y0, i_y1;
	Int16 i_width, i_x_start, i_x_end;
	UInt16 i_clip_left;
	UInt8 *pui8_ceiling, *pui8_floor;
	UInt16 i_iwidth;
	Int16 rgi_skiplist[ REND_SCREEN_WIDTH ], i_skiplist_length;

	i_width = ( ps_eng->s_line_segment.x2 - ps_eng->s_line_segment.x1 );
	if( i_width < 1 )
	{
		return;
	}

	i_x_start = ps_eng->s_line_segment.x1;
	i_x_end = ps_eng->s_line_segment.x2;

	if( clip_span( ps_eng, &i_x_start, &i_x_end, ps_eng->s_line_segment.ui8_flags & MAP_LINE_FLAGS_MIDDLE, &rgi_skiplist[ 0 ], &i_skiplist_length ) )
	{
		return;
	}

	if( ps_eng->i_mark_ceiling && !can_join_ceiling( ps_eng, i_x_start, i_x_end ) )
	{
		eng_flush_ceil( ps_eng );
	}
	if( ps_eng->i_mark_floor && !can_join_floor( ps_eng, i_x_start, i_x_end ) )
	{
		eng_flush_floor( ps_eng );
	}

	i_clip_left = i_x_start - ps_eng->s_line_segment.x1;

	i_iwidth = /*0xffffU / i_width;*/ div_u32_u16( 0xffff, i_width );
	i_x_stepy0 = mul_32_u16( ps_eng->s_line_segment.y02 - ps_eng->s_line_segment.y01, i_iwidth );
	i_x_stepy1 = mul_32_u16( ps_eng->s_line_segment.y12 - ps_eng->s_line_segment.y11, i_iwidth );

	i_y0 = ps_eng->s_line_segment.y01;
	i_y1 = ps_eng->s_line_segment.y11;

	if( i_clip_left )
	{
		i_y0 += mul_u16_32_ns( i_clip_left, i_x_stepy0 );
		i_y1 += mul_u16_32_ns( i_clip_left, i_x_stepy1 );
	}


	if( ps_eng->i_mark_ceiling )
	{
		pui8_ceiling = &ps_eng->rgui8_ceiling[ i_x_start ][ 0 ];
		for( i_x = i_x_start; i_x < i_x_end; i_x++ )
		{
			UInt16 i_py;
			UInt32 i_clip;
			Int32 i_yclip;

			i_clip = ps_eng->rgui8_yclip[ i_x ][ 0 ];
			i_yclip = i_clip << 16;
			if( i_y0 < i_yclip )
			{
				*(pui8_ceiling++) = i_clip;
				*(pui8_ceiling++) = i_clip;
			}
			else
			{
				i_py = ( i_y0 + 0xffff ) >> 16;
				if( i_py > ps_eng->rgui8_yclip[ i_x ][ 1 ] )
				{
					i_py = ps_eng->rgui8_yclip[ i_x ][ 1 ];
				}
				*(pui8_ceiling++) = ( UInt8 )i_clip;
				*(pui8_ceiling++) = ( UInt8 )i_py;
				ps_eng->rgui8_yclip[ i_x ][ 0 ] = ( UInt8 )i_py;
			}
			i_y0 += i_x_stepy0;
		}
		if( ps_eng->i_ceil_minx > i_x_start )
			ps_eng->i_ceil_minx = i_x_start;
		if( ps_eng->i_ceil_maxx < i_x_end )
			ps_eng->i_ceil_maxx = i_x_end;
	}

	if( ps_eng->i_mark_floor )
	{
		pui8_floor = &ps_eng->rgui8_floor[ i_x_start ][ 0 ];
		for( i_x = i_x_start; i_x < i_x_end; i_x++ )
		{
			Int16 i_py;
			UInt32 i_clip;

			i_py = ( i_y1 >> 16 );
			i_clip = ps_eng->rgui8_yclip[ i_x ][ 1 ];
			if( i_py > ( Int16 )i_clip )
			{
				*(pui8_floor++) = i_clip;
				*(pui8_floor++) = i_clip;
			}
			else
			{
				if( i_py < ps_eng->rgui8_yclip[ i_x ][ 0 ] )
				{
					i_py = ps_eng->rgui8_yclip[ i_x ][ 0 ];
				}
				*(pui8_floor++) = ( UInt8 )i_py;
				*(pui8_floor++) = ( UInt8 )i_clip;
				ps_eng->rgui8_yclip[ i_x ][ 1 ] = ( UInt8 )i_py;
			}
			i_y1 += i_x_stepy1;
		}
		if( ps_eng->i_floor_minx > i_x_start )
			ps_eng->i_floor_minx = i_x_start;
		if( ps_eng->i_floor_maxx < i_x_end )
			ps_eng->i_floor_maxx = i_x_end;
	}
}

#endif

void eng_clip_interp( Int16 i_cnt, vec3_t v1, Int16 i_dist1, vec3_t v2, Int16 i_dist2 )
{
	Int16 i_idx, i_fdist;
	UInt16 ui_dist;

#ifdef WIN32
	assert( abs( i_dist2 - i_dist1 ) <= 0x7fff );
#endif
	i_fdist = ( Int16 )( i_dist2 - i_dist1 );
	if( i_fdist < 0 )
	{
#ifdef WIN32
		assert( abs( div_32_u16( ( (Int32)-i_dist2 ) << 16, -i_fdist ) ) <= 0xffff ); 
		assert( div_32_u16( ( (Int32)-i_dist2 ) << 16, -i_fdist ) >= 0 ); 
#endif
		ui_dist = div_32_u16( ( (Int32)-i_dist2 ) << 16, -i_fdist );
	}
	else
	{
#ifdef WIN32
		assert( abs( div_32_u16( ( (Int32)-i_dist2 ) << 16, i_fdist ) ) <= 0xffff ); 
		assert( div_32_u16( ( (Int32)i_dist2 ) << 16, i_fdist ) >= 0 ); 
#endif
		ui_dist = div_32_u16( ( (Int32)i_dist2 ) << 16, i_fdist );
	}
	for( i_idx = 0; i_idx < i_cnt; i_idx++ )
	{
		v1[ i_idx ] = v2[ i_idx ] + ( ( mul_u16_32_ns( ui_dist, ( ( Int32 )v1[ i_idx ] - v2[ i_idx ] ) ) + 0x7fff ) >> 16 );
	}
}

Int16 eng_clip_vector2( vec3_t v1, vec3_t v2, vec2_t clip, Int16 i_sign )
{
 	Int16 i_dist1, i_dist2;

	if( i_sign )
	{
		i_dist1 = v1[ 0 ] - -clip[ 0 ];
		i_dist2 = v2[ 0 ] - -clip[ 1 ];
	}
	else
	{
		i_dist1 = v1[ 0 ] - clip[ 0 ];
		i_dist2 = v2[ 0 ] - clip[ 1 ];
	}

	/* round down from safe to clip coord */
	if( i_dist2 == i_dist1 || i_dist2 == 0 )
	{
		/* target on edge or all out */
		return 1;
	}
	eng_clip_interp( 2, v1, i_dist1, v2, i_dist2 );
	return 0;
}

Int16 eng_clip_vector2_near( vec3_t v1, vec3_t v2 )
{
 	Int16 i_dist1, i_dist2;

	i_dist1 = v1[ 1 ] - ( 3 << 4 );
	i_dist2 = v2[ 1 ] - ( 3 << 4 );

	if( i_dist1 == i_dist2 || i_dist2 == 0 )
	{
		return 1;
	}

	eng_clip_interp( 2, v1, i_dist1, v2, i_dist2 );

	return 0;
}


void eng_draw_map_line_segment( engine_t *ps_eng, eng_map_line_segment_t *ps_line_segment )
{
	UInt16 i_clip_v1, i_clip_v2;
#if PERFORMANCE_COUNTERS
	UInt16 i_timer_start;
#endif
	UInt32 i_tmp, i_itv1, i_itv2;
	vec2_t v1, v2;
	vec2_t tv1, tv2;
	vec2_t vclip;
	Int16 tz[ 4 ];

#if PERFORMANCE_COUNTERS
		i_timer_start = eng_get_time();
#endif

	v1[ 0 ] = ps_line_segment->i_x1;
	v1[ 1 ] = ps_line_segment->i_y1;
	v2[ 0 ] = ps_line_segment->i_x2;
	v2[ 1 ] = ps_line_segment->i_y2;


	transform_vec2( v1, &ps_eng->world_transform, tv1 );
	transform_vec2( v2, &ps_eng->world_transform, tv2 );

	while( 1 )
	{
		i_clip_v1 = i_clip_v2 = 0;

		if( ps_eng->i_sector_clipflags & 1 )
		{
			if( tv1[ 0 ] < -tv1[ 1 ] )
			{
				i_clip_v1 |= 1;
			}
			if( tv2[ 0 ] < -tv2[ 1 ] )
			{
				i_clip_v2 |= 1;
			}


			if( i_clip_v1 & i_clip_v2 )
			{
				return; /* fully clipped */
			}

			if( i_clip_v1 & 1 )
			{
				vclip[ 0 ] = tv1[ 1 ];
				vclip[ 1 ] = tv2[ 1 ];
				if( eng_clip_vector2( tv1, tv2, vclip, 1 ) )
				{
					return;
				}
			}

			if( i_clip_v2 & 1 )
			{
				vclip[ 0 ] = tv2[ 1 ];
				vclip[ 1 ] = tv1[ 1 ];
				if( eng_clip_vector2( tv2, tv1, vclip, 1 ) )
				{
					return;
				}
			}

			if( ( i_clip_v1 | i_clip_v2 ) & 1 )
			{
				continue;
			}
		}
		if( ps_eng->i_sector_clipflags & 2 )
		{
			if( tv1[ 0 ] > tv1[ 1 ] )
			{
				i_clip_v1 |= 2;
			}

			if( tv2[ 0 ] > tv2[ 1 ] )
			{
				i_clip_v2 |= 2;
			}

			if( i_clip_v1 & i_clip_v2 )
			{
				return; /* fully clipped */
			}

			if( i_clip_v1 & 2 )
			{
				vclip[ 0 ] = tv1[ 1 ];
				vclip[ 1 ] = tv2[ 1 ];
				if( eng_clip_vector2( tv1, tv2, vclip, 0 ) )
				{
					return;
				}
			}

			if( i_clip_v2 & 2 )
			{
				vclip[ 0 ] = tv2[ 1 ];
				vclip[ 1 ] = tv1[ 1 ];
				if( eng_clip_vector2( tv2, tv1, vclip, 0 ) )
				{
					return;
				}
			}

			if( ( i_clip_v1 | i_clip_v2 ) & 2 )
			{
				continue;
			}
		}

		if( ps_eng->i_sector_clipflags & 4 )
		{
			if( tv1[ 1 ] < ( 3 << 4 ) )
			{
				i_clip_v1 |= 4;
			}
			if( tv2[ 1 ] < ( 3 << 4 ) )
			{
				i_clip_v2 |= 4;
			}

			if( i_clip_v1 & i_clip_v2 )
			{
				return; /* fully clipped */
			}

			if( i_clip_v1 & 4 )
			{
				if( eng_clip_vector2_near( tv1, tv2 ) )
				{
					return;
				}
			}

			if( i_clip_v2 & 4 )
			{
				if( eng_clip_vector2_near( tv2, tv1 ) )
				{
					return;
				}
			}

			if( ( i_clip_v1 | i_clip_v2 ) & 4 )
			{
				continue;
			}
		}

		break;
	}
	/*
	sprintf( rgui8_buf, "%d,%d", tv1[ 2 ], tv2[ 2 ] );
	DrawStr( 50, 32 + ( i_idx * 10 ), rgui8_buf, A_NORMAL );
	*/

	{
		UInt16 i_texture, i_backsector;

#ifdef WIN32
		assert( YSCALE_CONST == 0x4800 );
#endif

		i_itv1 = div_u32_u16_u16r( 0x47fff, tv1[ 1 ] );
		i_itv2 = div_u32_u16_u16r( 0x47fff, tv2[ 1 ] );

		ps_eng->s_line_segment.ui8_flags = ps_line_segment->ui8_flags;
		i_texture = ps_line_segment->ui8_flags & ( MAP_LINE_FLAGS_MIDDLE | MAP_LINE_FLAGS_UPPER | MAP_LINE_FLAGS_LOWER );
		i_backsector = ps_line_segment->ui8_flags & ( MAP_LINE_FLAGS_UPPER | MAP_LINE_FLAGS_LOWER );


		tv1[ 0 ] = ( ( mul_16_16( tv1[ 0 ], i_itv1 ) + (1<<11) ) >> 12 ) + (REND_SCREEN_WIDTH/2);
		tv2[ 0 ] = ( ( mul_16_16( tv2[ 0 ], i_itv2 ) + (1<<11) ) >> 12 ) + (REND_SCREEN_WIDTH/2);
		if( tv1[ 0 ] < 0 )
		{
			tv1[ 0 ] = 0;
		}
		else if( tv1[ 0 ] > REND_SCREEN_WIDTH )
		{
			tv1[ 0 ] = REND_SCREEN_WIDTH;
		}
		if( tv2[ 0 ] < 0 )
		{
			tv2[ 0 ] = 0;
		}
		else if( tv2[ 0 ] > REND_SCREEN_WIDTH )
		{
			tv2[ 0 ] = REND_SCREEN_WIDTH;
		}

		if( i_texture )
		{
			render_texture_params_t *ps_texture_params = &ps_eng->s_tex_params;
			Int32 zbase, zx, idist;
			Int16 ut, ubase, ux;
			vec2_t v_norm, t_norm/*, t_normb*/;

			rotate_vec2( ps_texture_params->v_norm, &ps_eng->world_transform, v_norm );

			/* fixme: increase precision throughout and shift down in renderer ? */
#if 0
			if( ps_line_segment->ui8_flags & MAP_LINE_FLAGS_TMAP_Y )
			{
				t_normb[ 0 ] = 0;
				t_normb[ 1 ] = 0x2000;
			}
			else
			{
				t_normb[ 0 ] = 0x2000;
				t_normb[ 1 ] = 0;
			}
			rotate_vec2( t_normb, &ps_eng->world_transform, t_norm );
#else
/*void rotate_vec2( vec2_t v1, transform_t *t, vec2_t dst )
{
	dst[ 0 ] = ( mul_16_16( t->right[ 0 ], v1[ 0 ] ) + mul_16_16( t->right[ 1 ], v1[ 1 ] ) ) >> 15;
	dst[ 1 ] = ( mul_16_16( t->front[ 0 ], v1[ 0 ] ) + mul_16_16( t->front[ 1 ], v1[ 1 ] ) ) >> 15;
}*/
			if( ps_line_segment->ui8_flags & MAP_LINE_FLAGS_TMAP_Y )
			{
				t_norm[ 0 ] = ps_eng->world_transform.right[ 1 ] >> 2;
				t_norm[ 1 ] = ps_eng->world_transform.front[ 1 ] >> 2;
			}
			else
			{
				t_norm[ 0 ] = ps_eng->world_transform.right[ 0 ] >> 2;
				t_norm[ 1 ] = ps_eng->world_transform.front[ 0 ] >> 2;
			}
#endif

			idist = div_u32_u16( 0xfffffffUL, ps_texture_params->v_dist );
			zx = -mul_16_u32( v_norm[ 0 ], idist );
			zbase = (-mul_16_u32( v_norm[ 1 ], idist ) ) - ( zx );
			zx = mul_32_u16( ( zx + REND_SCREEN_WIDTH / 4 ), INV_HALF_SCREEN_WIDTH );

			ps_texture_params->i_zx = zx;
			ps_texture_params->i_zbase = zbase;

			ux = t_norm[ 0 ];
			ubase = t_norm[ 1 ] - ( ux );
			ux = ( mul_16_16( ux + REND_SCREEN_WIDTH / 4, INV_HALF_SCREEN_WIDTH ) ) >> 16;
			if( ps_line_segment->ui8_flags & MAP_LINE_FLAGS_TMAP_Y )
			{
				ut = ps_eng->origin[ 1 ] >> 3;
			}
			else
			{
				ut = ps_eng->origin[ 0 ] >> 3;
			}

			ps_texture_params->i_ux = ux;
			ps_texture_params->i_ubase = ubase;
			ps_texture_params->i_ut = ut;
		}

		if( tv1[ 0 ] < tv2[ 0 ] )
		{
			ps_eng->s_line_segment.x1 = tv1[ 0 ];
			ps_eng->s_line_segment.x2 = tv2[ 0 ];
		}
		else
		{
			i_tmp = i_itv1;
			i_itv1 = i_itv2;
			i_itv2 = i_tmp;;
			ps_eng->s_line_segment.x1 = tv2[ 0 ];
			ps_eng->s_line_segment.x2 = tv1[ 0 ];
		}


		tz[ 0 ] = ( ps_line_segment->i_z1 + ps_eng->world_transform.translation[ 2 ] );
		tz[ 1 ] = ( ps_line_segment->i_z2 + ps_eng->world_transform.translation[ 2 ] );
		tz[ 2 ] = ( ps_line_segment->i_z3 + ps_eng->world_transform.translation[ 2 ] );
		tz[ 3 ] = ( ps_line_segment->i_z4 + ps_eng->world_transform.translation[ 2 ] );


		ps_eng->s_line_segment.y01 = ( mul_16_16( -tz[ 0 ], i_itv1 ) << 4 ) + ( (Int32)REND_SCREEN_HEIGHT<<15 ) + ( (Int32)1<<15 );
		ps_eng->s_line_segment.y02 = ( mul_16_16( -tz[ 0 ], i_itv2 ) << 4 ) + ( (Int32)REND_SCREEN_HEIGHT<<15 ) + ( (Int32)1<<15 );
		ps_eng->s_line_segment.y11 = ( mul_16_16( -tz[ 1 ], i_itv1 ) << 4 ) + ( (Int32)REND_SCREEN_HEIGHT<<15 ) + ( (Int32)1<<15 );
		ps_eng->s_line_segment.y12 = ( mul_16_16( -tz[ 1 ], i_itv2 ) << 4 ) + ( (Int32)REND_SCREEN_HEIGHT<<15 ) + ( (Int32)1<<15 );

		if( i_backsector )
		{
			ps_eng->s_line_segment.y21 = ( mul_16_16( -tz[ 2 ], i_itv1 ) << 4 ) + ( (Int32)REND_SCREEN_HEIGHT<<15 ) + ( (Int32)1<<15 );
			ps_eng->s_line_segment.y22 = ( mul_16_16( -tz[ 2 ], i_itv2 ) << 4 ) + ( (Int32)REND_SCREEN_HEIGHT<<15 ) + ( (Int32)1<<15 );
			ps_eng->s_line_segment.y31 = ( mul_16_16( -tz[ 3 ], i_itv1 ) << 4 ) + ( (Int32)REND_SCREEN_HEIGHT<<15 ) + ( (Int32)1<<15 );
			ps_eng->s_line_segment.y32 = ( mul_16_16( -tz[ 3 ], i_itv2 ) << 4 ) + ( (Int32)REND_SCREEN_HEIGHT<<15 ) + ( (Int32)1<<15 );
		}

		if( i_texture )
		{
			eng_draw_wall_segment( ps_eng );
		}
		else
		{
			eng_mark_ceil_floor_segment( ps_eng );
		}
	}
#if PERFORMANCE_COUNTERS
	i_timer_line_segment += eng_get_time() - i_timer_start;
#endif
}



static void eng_start_new_sector()
{
}




static void init_floor_ceiling_cache( engine_t *ps_eng )
{
	Int16 i_idx, i_idx2;

	for( i_idx = 0; i_idx < REND_SCREEN_HEIGHT; i_idx++ )
	{
		for( i_idx2 = 0; i_idx2 < FLOOR_CEIL_CACHE_SIZE; i_idx2++ )
		{
			ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ i_idx ][ i_idx2 ].i_plane_y = REND_SCREEN_HEIGHT + 1;
			ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ i_idx ][ i_idx2 ].i_plane_z = 0;
		}
		ps_eng->s_floor_ceiling_cache.rgi_uv_cache_pos[ i_idx ] = 0;
	}

	/* force empty flush on first line */
	ps_eng->i_ceiling_texture = -1;
	ps_eng->i_floor_texture = -1;
	ps_eng->i_ceil_maxx = 0;
	ps_eng->i_floor_maxx = 0;
}

void draw_plane_span( engine_t *ps_eng )
{
	Int16 i_idx;
	Int32 i_sz, i_u_xstep, i_v_xstep, i_tu, i_tv;

	for( i_idx = 0; i_idx < FLOOR_CEIL_CACHE_SIZE; i_idx++ )
	{
		if( ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_plane_z == ps_eng->i_plane_z )
		{
			i_u_xstep = ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_u_xstep;
			i_v_xstep = ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_v_xstep;
			break;
		}
	}
	if( i_idx == FLOOR_CEIL_CACHE_SIZE )
	{
		Int32 i_product;
		i_idx = ++ps_eng->s_floor_ceiling_cache.rgi_uv_cache_pos[ ps_eng->i_plane_y ];
		if( i_idx >= FLOOR_CEIL_CACHE_SIZE )
		{
			i_idx = 0;
		}
		ps_eng->s_floor_ceiling_cache.rgi_uv_cache_pos[ ps_eng->i_plane_y ] = i_idx;
		
#ifdef WIN32
		if( ( YSCALE_CONST * INV_HALF_SCREEN_WIDTH ) == 0xfff000 ) /* FIXME: is 2^24 without rounding error */
		{
#endif
/*			i_tan = ( ( REND_SCREEN_HEIGHT/2) - ps_eng->i_plane_y );
			if( i_tan == 0 )
			{
				i_sz = ( (1024UL<<4) * YSCALE_CONST ) >> 11;
			}
			else */
#if 0
			{
				Int16 i_atan, i_pz;
				i_pz = ps_eng->i_plane_z;
				i_atan = ps_eng->rgi16_atan[ ps_eng->i_plane_y ];
				i_sz = mul_16_16( ps_eng->i_plane_z, i_atan ) >> 11;
			}
			/*i_sz = mul_16_16( i_sz, INV_HALF_SCREEN_WIDTH ) >> 16;*/
			i_product = mul_16_16( ps_eng->world_transform.front[ 1 ], i_sz );
			i_u_xstep = mul_32_u16( -i_product, INV_HALF_SCREEN_WIDTH );
			/*i_u_xstep = -i_product;*/
			i_tv = ps_eng->i_plane_v_offset + i_product;

			i_product = mul_16_16( ps_eng->world_transform.front[ 0 ], i_sz );
			i_v_xstep = mul_32_u16( i_product, INV_HALF_SCREEN_WIDTH );
			/*i_v_xstep = i_product;*/
			i_tu = ps_eng->i_plane_u_offset + i_product;
#else
			{
				Int16 i_atan, i_pz, i_front1z, i_front0z;
				i_pz = ps_eng->i_plane_z;
				i_atan = ps_eng->rgi16_atan[ ps_eng->i_plane_y ];
				/*
				i_sz = mul_16_16( ps_eng->i_plane_z, i_atan ) >> 11;
				*/

				/*i_sz = mul_16_16( i_sz, INV_HALF_SCREEN_WIDTH ) >> 16;*/
				i_sz = mul_16_16( ps_eng->i_plane_z, i_atan ) >> 6;
				if( i_sz < 1 )
				{
					i_sz = 1;
				}
				else if( i_sz > 0x7fff )
				{
					i_sz = 0x7fff;
				}
				i_front1z = ( ( mul_16_16( i_sz, ps_eng->world_transform.front[ 1 ] ) ) >> 16 );
				i_product = i_front1z;
				i_u_xstep = mul_16_16( -i_product, INV_HALF_SCREEN_WIDTH >> 5 );
				i_product <<= 11;
				i_tv = ps_eng->i_plane_v_offset + i_product;

				i_front0z = ( ( mul_16_16( i_sz, ps_eng->world_transform.front[ 0 ] ) ) >> 16 );
				i_product = i_front0z;
				i_v_xstep = mul_16_16( i_product, INV_HALF_SCREEN_WIDTH >> 5 );
				i_product <<= 11;
				i_tu = ps_eng->i_plane_u_offset + i_product;
			}
#endif
#ifdef WIN32
			
		}
		else
		{
			Int16 i_tan;
			Int32 *pi_null = 0;
			*pi_null = 0;

			i_tan = ( ( REND_SCREEN_HEIGHT/2) - ps_eng->i_plane_y );
			if( i_tan == 0 )
			{
				i_sz = ( (1024UL<<4) * YSCALE_CONST ) >> 11;
			}
			else
			{
				i_sz = ( mul_16_16( ps_eng->i_plane_z, YSCALE_CONST ) / i_tan ) >> 11;
			}
			i_product = mul_16_16( ps_eng->world_transform.front[ 1 ], i_sz );
			i_u_xstep = mul_32_u16( -i_product, INV_HALF_SCREEN_WIDTH );
			i_tv = ps_eng->i_plane_v_offset + i_product;

			i_product = mul_16_16( ps_eng->world_transform.front[ 0 ], i_sz );
			i_v_xstep = mul_32_u16( i_product, INV_HALF_SCREEN_WIDTH );
			i_tu = ps_eng->i_plane_u_offset + i_product;
		}
#endif

		ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_plane_z = ps_eng->i_plane_z;
		ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_u_xstep = i_u_xstep;
		ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_v_xstep = i_v_xstep;
		ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_tu = i_tu;
		ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_tv = i_tv;
	}
	/* fixme: argh */
	{
		Int16 i16_xoff;
		Int32 i_offu, i_offv;
		i16_xoff = ( ps_eng->i_plane_x - (REND_SCREEN_WIDTH/2) );

		if( i16_xoff < 0 )
		{
			i_offu = mul_u16_32_ns( -i16_xoff, -i_u_xstep );
			i_offv = mul_u16_32_ns( -i16_xoff, -i_v_xstep );
		}
		else
		{
			i_offu = mul_u16_32_ns( i16_xoff, i_u_xstep );
			i_offv = mul_u16_32_ns( i16_xoff, i_v_xstep );
		}
		i_tu = ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_tu + i_offu;
		i_tv = ps_eng->s_floor_ceiling_cache.rgs_uv_cache[ ps_eng->i_plane_y ][ i_idx ].i_tv + i_offv;
	}

#if PERFORMANCE_COUNTERS && 0
	{ UInt16 i_timer_start = eng_get_time();
#endif

	eng_draw_texture_hspan( ( UInt8 *)ps_eng->p_drawbuffer + ps_eng->rgui16_ylut[ ps_eng->i_plane_y ] + (ps_eng->i_plane_x>>2), ps_eng->i_plane_x & 0x3, ps_eng->i_plane_span_length, ps_eng->pui8_tex, i_tu, i_u_xstep, i_tv, i_v_xstep );

#if PERFORMANCE_COUNTERS  && 0
	i_timer_render_floor_inner += eng_get_time() - i_timer_start;
	}
#endif


}



static void eng_draw_floor_ceiling_extends( engine_t *ps_eng, Int16 i_minx, Int16 i_maxx, UInt8 (*pui8_extends)[ 2 ], Int16 i_z )
{
	UInt8 rgui8_span_start[ REND_SCREEN_HEIGHT ];
	Int16 i_x, i_from_top, i_from_bottom;

	i_from_top = REND_SCREEN_HEIGHT;
	i_from_bottom = 0;

	ps_eng->i_plane_z = i_z;
	ps_eng->i_plane_u_offset = (long)-ps_eng->world_transform.translation[ 0 ] << 12;
	ps_eng->i_plane_v_offset = (long)-ps_eng->world_transform.translation[ 1 ] << 12;

	for( i_x = i_minx; i_x <= i_maxx; i_x++ )
	{
		Int16 i_next_top, i_next_bottom;

		if( i_x >= 136 )
		{
			i_maxx = i_maxx;
		}

		i_next_top = pui8_extends[ i_x ][ 0 ];
		i_next_bottom = pui8_extends[ i_x ][ 1 ];

		while( i_from_top < i_next_top && i_from_top < i_from_bottom )
		{
			ps_eng->i_plane_x = rgui8_span_start[ i_from_top ];
			ps_eng->i_plane_y = i_from_top;
			ps_eng->i_plane_span_length = i_x - ps_eng->i_plane_x;

			draw_plane_span( ps_eng );
			i_from_top++;
		}

		while( i_from_bottom > i_next_bottom && i_from_bottom > i_from_top )
		{
			i_from_bottom--;

			ps_eng->i_plane_x = rgui8_span_start[ i_from_bottom ];
			ps_eng->i_plane_y = i_from_bottom;
			ps_eng->i_plane_span_length = i_x - ps_eng->i_plane_x;
			draw_plane_span( ps_eng );
		}

		while( i_from_top > i_next_top && i_next_top < i_next_bottom )
		{
			i_from_top--;
			rgui8_span_start[ i_from_top ] = ( UInt8 )i_x;
		}
		while( i_from_bottom < i_next_bottom && i_next_bottom > i_next_top )
		{
			rgui8_span_start[ i_from_bottom ] = ( UInt8 )i_x;
			i_from_bottom++;
		}

		i_from_top = i_next_top;
		i_from_bottom = i_next_bottom;
	}
}

void eng_flush_ceil( engine_t *ps_eng )
{
#if PERFORMANCE_COUNTERS
	UInt16 i_timer_start;
#endif
	Int16 i_new_texture_ceiling = -1, i_z, i_zvert = 0;

	if( ps_eng->ps_gsector )
	{
		i_zvert = ps_eng->ps_gsector->i_zvert;
		i_new_texture_ceiling = ps_eng->ps_gsector->ui8_texture_ceiling;
	}

//	DrawStr( i_dbg_counter++ * 8, 80, "a", A_REPLACE );
#if PERFORMANCE_COUNTERS
	i_timer_start = eng_get_time();
#endif

	if( ps_eng->i_ceil_minx < ps_eng->i_ceil_maxx )
	{
		ps_eng->pui8_tex = texture_get( ps_eng, ps_eng->i_ceiling_texture, FALSE );

		ps_eng->rgui8_ceiling[ ps_eng->i_ceil_maxx ][ 0 ] = REND_SCREEN_HEIGHT;
		ps_eng->rgui8_ceiling[ ps_eng->i_ceil_maxx ][ 1 ] = 0;

		i_z = ps_eng->i_ceiling_z + ps_eng->world_transform.translation[ 2 ];
		if( i_z > 0 )
		{
			eng_draw_floor_ceiling_extends( ps_eng, ps_eng->i_ceil_minx, ps_eng->i_ceil_maxx, &ps_eng->rgui8_ceiling[ 0 ], i_z );
		}
	}

	ps_eng->i_ceil_minx = REND_SCREEN_WIDTH + 1;
	ps_eng->i_ceil_maxx = 0;
	memset( &ps_eng->rgui8_ceiling[ 0 ][ 0 ], REND_SCREEN_HEIGHT, sizeof( ps_eng->rgui8_ceiling ) );
	ps_eng->i_ceiling_z = ps_eng->s_map.p_vertices[ i_zvert ][ 1 ];
	ps_eng->i_ceiling_texture = i_new_texture_ceiling;
#if PERFORMANCE_COUNTERS
	i_timer_render_floor += eng_get_time() - i_timer_start;
#endif

}

void eng_flush_floor( engine_t *ps_eng )
{
#if PERFORMANCE_COUNTERS
	UInt16 i_timer_start;
#endif
	Int16 i_new_texture_floor = -1, i_z, i_zvert = 0;

	if( ps_eng->ps_gsector )
	{
		i_zvert = ps_eng->ps_gsector->i_zvert;
		i_new_texture_floor = ps_eng->ps_gsector->ui8_texture_floor;
	}

//	DrawStr( i_dbg_counter++ * 8, 80, "b", A_REPLACE );

#if PERFORMANCE_COUNTERS
	i_timer_start = eng_get_time();
#endif

	if( ps_eng->i_floor_minx < ps_eng->i_floor_maxx )
	{
		ps_eng->pui8_tex = texture_get( ps_eng, ps_eng->i_floor_texture, FALSE );

		ps_eng->rgui8_floor[ ps_eng->i_floor_maxx ][ 0 ] = REND_SCREEN_HEIGHT;
		ps_eng->rgui8_floor[ ps_eng->i_floor_maxx ][ 1 ] = 0;

		i_z = ps_eng->i_floor_z + ps_eng->world_transform.translation[ 2 ];
		if( i_z < 0 )
		{
			eng_draw_floor_ceiling_extends( ps_eng, ps_eng->i_floor_minx, ps_eng->i_floor_maxx, &ps_eng->rgui8_floor[ 0 ], i_z );
		}
	}

	ps_eng->i_floor_minx = REND_SCREEN_WIDTH + 1;
	ps_eng->i_floor_maxx = 0;
	memset( &ps_eng->rgui8_floor[ 0 ][ 0 ], 0, sizeof( ps_eng->rgui8_floor ) );
	ps_eng->i_floor_z = ps_eng->s_map.p_vertices[ i_zvert ][ 0 ];
	ps_eng->i_floor_texture = i_new_texture_floor;
#if PERFORMANCE_COUNTERS
	i_timer_render_floor += eng_get_time() - i_timer_start;
#endif
}

static void eng_finish_sector()
{
	/* ... */
}


static void eng_draw_line( engine_t *ps_eng, UInt16 i_line )
{
	render_texture_params_t *ps_texture_params = &ps_eng->s_tex_params;
	UInt16 i_backsector_zvert = 0, i_backsector_floor = 0, i_backsector_ceiling = 0;
	Int16 i_dist, i_zvert;
	line_t *p_line;
	plane_t *p_plane;
	vec2_t v_norm;
	sector_t *ps_backsector;

	p_line = &ps_eng->s_map.p_lines[ i_line ];
	p_plane = &ps_eng->s_map.p_planes[ p_line->i_plane ];
	v_norm[ 0 ] = ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 0 ];
	v_norm[ 1 ] = ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 1 ];

	i_dist = ( ( mul_16_16( ps_eng->origin[ 0 ], v_norm[ 0 ] ) >> 15 ) + ( mul_16_16( ps_eng->origin[ 1 ], v_norm[ 1 ] ) >> 15 ) ) - p_plane->i_length;

	if( i_dist <= 0 )
	{
		return;
	}

	ps_texture_params->v_norm[ 0 ] = v_norm[ 0 ];
	ps_texture_params->v_norm[ 1 ] = v_norm[ 1 ];
	ps_texture_params->v_dist = i_dist;

	i_zvert = ps_eng->ps_gsector->i_zvert;

	ps_eng->s_map_line.i_x1 = ps_eng->s_map.p_vertices[ p_line->i_v1 ][ 0 ];
	ps_eng->s_map_line.i_y1 = ps_eng->s_map.p_vertices[ p_line->i_v1 ][ 1 ];
	ps_eng->s_map_line.i_x2 = ps_eng->s_map.p_vertices[ p_line->i_v2 ][ 0 ];
	ps_eng->s_map_line.i_y2 = ps_eng->s_map.p_vertices[ p_line->i_v2 ][ 1 ];

	ps_eng->s_map_line.i_z1 = ps_eng->s_map.p_vertices[ i_zvert ][ 1 ];
	ps_eng->s_map_line.i_z2 = ps_eng->s_map.p_vertices[ i_zvert ][ 0 ];

	
	if( p_line->i_backsector != 0xffff )
	{
		ps_backsector = &ps_eng->s_map.p_sectors[ p_line->i_backsector ];
		i_backsector_zvert = ps_backsector->i_zvert;
		i_backsector_floor = ps_backsector->ui8_texture_floor;
		i_backsector_ceiling = ps_backsector->ui8_texture_ceiling;
	}

	/* check if we need to mark floor/ceiling */
	if( p_line->ui8_flags & MAP_LINE_FLAGS_MIDDLE )
	{
		ps_eng->i_mark_floor = ps_eng->i_mark_ceiling = 1;
	}
	else
	{
		ps_eng->i_mark_ceiling = 0;
		if( ps_eng->s_map.p_vertices[ i_zvert ][ 1 ] != ps_eng->s_map.p_vertices[ i_backsector_zvert ][ 1 ] ||
			i_backsector_ceiling != ps_eng->ps_gsector->ui8_texture_ceiling )
		{
			ps_eng->i_mark_ceiling = 1;
		}
		ps_eng->i_mark_floor = 0;
		if( ps_eng->s_map.p_vertices[ i_zvert ][ 0 ] != ps_eng->s_map.p_vertices[ i_backsector_zvert ][ 0 ] ||
			i_backsector_floor != ps_eng->ps_gsector->ui8_texture_floor )
		{
			ps_eng->i_mark_floor = 1;
		}
	}
	if( ( p_line->ui8_flags & MAP_LINE_FLAGS_UPPER ) || ( p_line->ui8_flags & MAP_LINE_FLAGS_LOWER ) )
	{
		ps_eng->s_map_line.i_z3 = ps_eng->s_map.p_vertices[ i_backsector_zvert ][ 1 ];
		ps_eng->s_map_line.i_z4 = ps_eng->s_map.p_vertices[ i_backsector_zvert ][ 0 ];
	}
	ps_eng->s_map_line.ui8_flags = ( UInt8 )p_line->ui8_flags;

	if( ps_eng->i_mark_ceiling || ps_eng->i_mark_floor )
	{
		ps_eng->ps_gline = &ps_eng->s_map.p_lines[ i_line ];
		eng_draw_map_line_segment( ps_eng, &ps_eng->s_map_line );
	}

}


static void eng_draw_sector( engine_t *ps_eng, UInt16 i_sector )
{
	sector_t *p_sec;
	UInt16 i_idx, i_line;

	p_sec = &ps_eng->s_map.p_sectors[ i_sector ];

	ps_eng->ps_gsector = p_sec;

	eng_start_new_sector( );

	{
		UInt16 ui16_next_entity;

		ui16_next_entity = ps_eng->pui16_sector_entities[ i_sector ];
		while( ui16_next_entity != ENTITY_MAPPING_SENTINEL )
		{
			entity_t *ps_entity;
			Int16 i16_sprite_index;

			ps_entity = &ps_eng->rgs_entities[ ps_eng->rgs_entities_in_sector_frags[ ui16_next_entity ].ui8_entity ];
			ui16_next_entity = ps_eng->rgs_entities_in_sector_frags[ ui16_next_entity ].ui16_next_entity;

			if( ( ps_entity->ui_flags & ENTITY_FLAGS_DRAW ) && !( ps_entity->ui_flags & ENTITY_FLAGS_DRAWN ) )
			{
				i16_sprite_index = ps_entity->i_sprite_index;
				if( i16_sprite_index >= SPRITE_INDEX_SPECIAL_BEGIN )
				{
					i16_sprite_index = entities_resolve_sprite_index( ps_eng, ps_entity );
				}
				generate_drawsprite( ps_eng, ps_entity->v_origin, i16_sprite_index );
				ps_entity->ui_flags |= ENTITY_FLAGS_DRAWN;
			}
		}
	}

	i_line = p_sec->i_line_offset;

	for( i_idx = 0; i_idx < p_sec->i_num_lines; i_idx++ )
	{
		eng_draw_line( ps_eng, i_line + i_idx );
	}

	eng_finish_sector();
}


static Int16 eng_check_node_bbox( engine_t *ps_eng, Int16 i_node, Int16 *pi_clipplanes )
{
	Int16 i_idx;
	vec2_t v[ 2 ];

	node_t *p_node = &ps_eng->s_map.p_nodes[ i_node ];
	v[ 0 ][ 0 ] = ps_eng->s_map.p_vertices[ p_node->i_bbox_min ][ 0 ];
	v[ 0 ][ 1 ] = ps_eng->s_map.p_vertices[ p_node->i_bbox_min ][ 1 ];

	v[ 1 ][ 0 ] = ps_eng->s_map.p_vertices[ p_node->i_bbox_max ][ 0 ];
	v[ 1 ][ 1 ] = ps_eng->s_map.p_vertices[ p_node->i_bbox_max ][ 1 ];

	for( i_idx = 0; i_idx < 3; i_idx++ )
	{
		Int16 i_side_x, i_side_y, i_dist;

		if( !( *pi_clipplanes & ( 1 << i_idx ) ) )
		{
			continue;
		}

		i_side_x = ps_eng->rgi_clipplanes_minmax_idx[ i_idx ][ 0 ];
		i_side_y = ps_eng->rgi_clipplanes_minmax_idx[ i_idx ][ 1 ];
		i_dist = ( mul_16_16( ps_eng->s_clipplanes[ i_idx ].v_normal[ 0 ], v[ i_side_x ][ 0 ] ) + mul_16_16( ps_eng->s_clipplanes[ i_idx ].v_normal[ 1 ], v[ i_side_y ][ 1 ] ) ) >> 16;
		i_dist -= ps_eng->s_clipplanes[ i_idx ].i_length >> 1;
		if( i_dist < 0 )
		{
			return 0;
		}
		i_side_x = !i_side_x;
		i_side_y = !i_side_y;
		i_dist = ( mul_16_16( ps_eng->s_clipplanes[ i_idx ].v_normal[ 0 ], v[ i_side_x ][ 0 ] ) + mul_16_16( ps_eng->s_clipplanes[ i_idx ].v_normal[ 1 ], v[ i_side_y ][ 1 ] ) ) >> 16;
		i_dist -= ps_eng->s_clipplanes[ i_idx ].i_length >> 1;
		if( i_dist >= 32 ) /* need safety range to edge before skipping renderer clip */
		{
			*pi_clipplanes = *pi_clipplanes & ~( 1 << i_idx );
		}
	}

	return 1;
}

void eng_draw_map_r( engine_t *ps_eng, UInt16 i_node )
{
	Int16 i_dist;
	UInt16 i_sec, i_secnum;
	plane_t *p_plane;

	if( !( ps_eng->pui8_node_visibility[ i_node >> 3 ] & ( 1 << ( i_node & 0x7 ) ) ) )
	{
		return;
	}

	if( ps_eng->s_map.p_nodes[ i_node ].i_plane >= 0xfffe ) /* leaf */
	{
		UInt16 i_num_sectors = ps_eng->s_map.p_nodes[ i_node ].i_plane - 0xfffe;
		for( i_sec = 0; i_sec < i_num_sectors; i_sec++ )
		{
			i_secnum = ps_eng->s_map.p_nodes[ i_node ].i_sector_offset + i_sec;
			if( 1 || !ps_eng->pui8_visibility )
			{
				Int16 i_clipplanes = 0x7;
#ifdef WIN32
				assert( ( ps_eng->pui8_visibility[ i_secnum >> 3 ] & ( 1 << ( i_secnum & 0x7 ) ) ) );
#endif
				if( !( eng_check_node_bbox( ps_eng, i_node, &i_clipplanes ) ) )
				{
					return;
				}
				ps_eng->i_sector_clipflags = i_clipplanes;
				eng_draw_sector( ps_eng, i_secnum );
			}
		}
	}
	else
	{
		Int16 i_player_side;
		//entity_t *ps_entities_front, *ps_entities_back, *ps_next_entity;
		p_plane = &ps_eng->s_map.p_planes[ ps_eng->s_map.p_nodes[ i_node ].i_plane ];


		/*i_dist = map_point_plane_dist( ps_eng, p_plane, ps_eng->origin );*/
		i_dist = mul_16_16( ps_eng->origin[ 0 ], ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 0 ] ) >> 16;
		i_dist += mul_16_16(ps_eng->origin[ 1 ], ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 1 ] ) >> 16;
		i_dist -= p_plane->i_length >> 1;
		i_player_side = i_dist < 0 ? 0 : 1;

		//ps_entities_front = ps_entities_back = ps_next_entity = 0;
		/*
		while( ps_draw_entities )
		{
			ps_next_entity = ps_draw_entities->p_next;
			i_edist = map_point_plane_dist( ps_eng, p_plane, ps_draw_entities->v_origin );
			i_node_offset = ( ps_draw_entities->i_radius << 4 );
			if( i_player_side )
			{
				i_node_offset = -i_node_offset;
			}
			if( i_edist < i_node_offset )
			{
				ps_draw_entities->p_next = ps_entities_back;
				ps_entities_back = ps_draw_entities;
			}
			else
			{
				ps_draw_entities->p_next = ps_entities_front;
				ps_entities_front = ps_draw_entities;
			}
			ps_draw_entities = ps_next_entity;
		}
		*/

		if( i_dist >= 0 )
		{
			eng_draw_map_r( ps_eng, i_node + 1 );
			eng_draw_map_r( ps_eng, ps_eng->s_map.p_nodes[ i_node ].i_backnode );
		}
		else
		{
			eng_draw_map_r( ps_eng, ps_eng->s_map.p_nodes[ i_node ].i_backnode );
			eng_draw_map_r( ps_eng, i_node + 1 );
		}
	}
}

void eng_setup_world_transform( engine_t *ps_eng )
{
	Int16 i_idx;
	ps_eng->world_transform.translation[ 0 ] = -ps_eng->origin[ 0 ];
	ps_eng->world_transform.translation[ 1 ] = -ps_eng->origin[ 1 ];
	ps_eng->world_transform.translation[ 2 ] = -ps_eng->origin[ 2 ];
	ps_eng->world_transform.front[ 0 ] = cos_d8( ps_eng, -ps_eng->yaw );
	ps_eng->world_transform.front[ 1 ] = -sin_d8( ps_eng, -ps_eng->yaw );
	ps_eng->world_transform.front[ 2 ] = 0;
	ps_eng->world_transform.right[ 0 ] = sin_d8( ps_eng, -ps_eng->yaw );
	ps_eng->world_transform.right[ 1 ] = cos_d8( ps_eng, -ps_eng->yaw );
	ps_eng->world_transform.right[ 2 ] = 0;

	ps_eng->s_clipplanes[ 0 ].v_normal[ 0 ] = -sin_d8( ps_eng, ps_eng->yaw - 0x20 );
	ps_eng->s_clipplanes[ 0 ].v_normal[ 1 ] = cos_d8( ps_eng, ps_eng->yaw - 0x20 );
	ps_eng->s_clipplanes[ 0 ].i_length = ( mul_16_16( ps_eng->s_clipplanes[ 0 ].v_normal[ 0 ], ps_eng->origin[ 0 ] ) + mul_16_16( ps_eng->s_clipplanes[ 0 ].v_normal[ 1 ], ps_eng->origin[ 1 ] ) ) >> 15;
	ps_eng->s_clipplanes[ 1 ].v_normal[ 0 ] = sin_d8( ps_eng, ps_eng->yaw + 0x20 );
	ps_eng->s_clipplanes[ 1 ].v_normal[ 1 ] = -cos_d8( ps_eng, ps_eng->yaw + 0x20 );
	ps_eng->s_clipplanes[ 1 ].i_length = ( mul_16_16( ps_eng->s_clipplanes[ 1 ].v_normal[ 0 ], ps_eng->origin[ 0 ] ) + mul_16_16( ps_eng->s_clipplanes[ 1 ].v_normal[ 1 ], ps_eng->origin[ 1 ] ) ) >> 15;
	ps_eng->s_clipplanes[ 2 ].v_normal[ 0 ] = -cos_d8( ps_eng, ps_eng->yaw + 0x80 );
	ps_eng->s_clipplanes[ 2 ].v_normal[ 1 ] = -sin_d8( ps_eng, ps_eng->yaw + 0x80 );
	ps_eng->s_clipplanes[ 2 ].i_length = ( mul_16_16( ps_eng->s_clipplanes[ 2 ].v_normal[ 0 ], ps_eng->origin[ 0 ] ) + mul_16_16( ps_eng->s_clipplanes[ 2 ].v_normal[ 1 ], ps_eng->origin[ 1 ] ) ) >> 15;
	ps_eng->s_clipplanes[ 2 ].i_length += 1 << 4;

	for( i_idx = 0; i_idx < 3; i_idx++ )
	{
		if( ps_eng->s_clipplanes[ i_idx ].v_normal[ 0 ] < 0 )
		{
			ps_eng->rgi_clipplanes_minmax_idx[ i_idx ][ 0 ] = 0;
		}
		else
		{
			ps_eng->rgi_clipplanes_minmax_idx[ i_idx ][ 0 ] = 1;
		}
		if( ps_eng->s_clipplanes[ i_idx ].v_normal[ 1 ] < 0 )
		{
			ps_eng->rgi_clipplanes_minmax_idx[ i_idx ][ 1 ] = 0;
		}
		else
		{
			ps_eng->rgi_clipplanes_minmax_idx[ i_idx ][ 1 ] = 1;
		}
	}

}



Int16 menu( engine_t *ps_eng )
{
	Int32 i_x, i_y;
	const UInt8 rgui8_minus[ 2 ] = { 'Z' + 1, 0 };


	if( ps_eng->ui8_menu_state <= MENU_STATE_QUIT )
	{
		if( !( ps_eng->s_input.ui16_keys2 & EKEY_UP ) && ( ps_eng->s_input.ui16_keys1 & EKEY_UP ) )
		{
			ps_eng->ui8_menu_state = ps_eng->ui8_menu_state - 1;
			if( ps_eng->ui8_menu_state < MENU_STATE_NEW_GAME )
			{
				ps_eng->ui8_menu_state = MENU_STATE_NEW_GAME;
			}
		}
		else if( !( ps_eng->s_input.ui16_keys2 & EKEY_DOWN ) && ( ps_eng->s_input.ui16_keys1 & EKEY_DOWN ) )
		{
			ps_eng->ui8_menu_state = ps_eng->ui8_menu_state + 1;
			if( ps_eng->ui8_menu_state > MENU_STATE_QUIT )
			{
				ps_eng->ui8_menu_state = MENU_STATE_QUIT;
			}
		}
		else if( ( ps_eng->s_input.ui16_keys1 & EKEY_0 ) && !( ps_eng->s_input.ui16_keys2 & EKEY_0 ) )
		{
			if( ps_eng->ui8_menu_state == MENU_STATE_QUIT )
			{
				return 1;
			}
			else if( ps_eng->ui8_menu_state == MENU_STATE_NEW_GAME )
			{
				ps_eng->ui8_menu_state = MENU_STATE_DIFFICULTY0;
			}
		}
	}
	else if( ps_eng->ui8_menu_state <= MENU_STATE_DIFFICULTY3 )
	{
		if( !( ps_eng->s_input.ui16_keys2 & EKEY_UP ) && ( ps_eng->s_input.ui16_keys1 & EKEY_UP ) )
		{
			ps_eng->ui8_menu_state = ps_eng->ui8_menu_state - 1;
			if( ps_eng->ui8_menu_state < MENU_STATE_DIFFICULTY0 )
			{
				ps_eng->ui8_menu_state = MENU_STATE_DIFFICULTY0;
			}
		}
		else if( !( ps_eng->s_input.ui16_keys2 & EKEY_DOWN ) && ( ps_eng->s_input.ui16_keys1 & EKEY_DOWN ) )
		{
			ps_eng->ui8_menu_state = ps_eng->ui8_menu_state + 1;
			if( ps_eng->ui8_menu_state > MENU_STATE_DIFFICULTY3 )
			{
				ps_eng->ui8_menu_state = MENU_STATE_DIFFICULTY3;
			}
		}
		else if( ( ps_eng->s_input.ui16_keys1 & EKEY_0 ) && !( ps_eng->s_input.ui16_keys2 & EKEY_0 ) )
		{
			return ps_eng->ui8_menu_state;
		}
	}

	if( ps_eng->ui8_menu_state <= MENU_STATE_QUIT )
	{
		i_x = REND_SCREEN_WIDTH / 3;
		i_y = 30;
		draw_string( ps_eng, 1, i_x, i_y, 1, ( UInt8 *)" NEW GAME" );
		draw_string( ps_eng, 1, i_x, i_y + 12, 1, ( UInt8 *)" QUIT" );

		i_y += ( ps_eng->ui8_menu_state - 1 ) * 12;
		draw_string( ps_eng, 1, i_x, i_y, 1, ( UInt8 * )rgui8_minus );
	}
	else if( ps_eng->ui8_menu_state <= MENU_STATE_DIFFICULTY3 )
	{
		i_x = 1;
		i_y = 30;
		draw_string( ps_eng, 1, i_x, i_y, 1, ( UInt8 *)" TOO YOUNG TO DIE" );
		draw_string( ps_eng, 1, i_x, i_y + 10, 1, ( UInt8 *)" HEY NOT TOO ROUGH" );
		draw_string( ps_eng, 1, i_x, i_y + 20, 1, ( UInt8 *)" HURT ME PLENTY" );
		draw_string( ps_eng, 1, i_x, i_y + 30, 1, ( UInt8 *)" ULTRA VIOLENCE" );

		i_y += mul_16_16( ( ps_eng->ui8_menu_state - 3 ), 10 );
		draw_string( ps_eng, 1, i_x, i_y, 1, ( UInt8 * )rgui8_minus );
	}

	return 0;
}


Int16 eng_change_map( engine_t *ps_eng, Int16 i_map, player_t *ps_player )
{
	player_t s_player;

	if( ps_player )
	{
		s_player = *ps_player;
	}

	if( !map_load( ps_eng, i_map ) )
	{
		return -1;
	}
	else
	{
		generate_texture_mapping_table( ps_eng );
		sprite_cache_init( ps_eng );
		rand_init( ps_eng );
		spawn_entities( ps_eng );
	}
	if( ps_player )
	{
		entities_fixup_player( ps_eng, &s_player );
	}

	ps_eng->ui8_current_map = ( UInt8 )i_map;

	return 0;
}


void determine_draw_node_visibility( engine_t *ps_eng )
{
	Int16 i_idx, i_node;
	UInt8 *pui8_sector_vis, *pui8_node_vis, ui8_vis, ui8_bx;
	node_t *ps_nodes = ps_eng->s_map.p_nodes;
	sector_t *ps_sectors = ps_eng->s_map.p_sectors;

	pui8_sector_vis = ps_eng->pui8_visibility;
	pui8_node_vis = ps_eng->pui8_node_visibility;

	if( pui8_sector_vis == NULL )
	{
		memset( pui8_node_vis, 0xff, sizeof( UInt8 ) * ps_eng->i_num_node_visibility );
		return;
	}
	else
	{
		memset( pui8_node_vis, 0, sizeof( UInt8 ) * ps_eng->i_num_node_visibility );
	}

	ui8_vis = *( pui8_sector_vis++ );
	ui8_bx = 1;
	for( i_idx = 0; i_idx < ps_eng->s_map.i_num_sectors; i_idx++ )
	{
		if( ui8_vis & ui8_bx )
		{
			UInt8 ui8_bit;

			i_node = ps_sectors[ i_idx ].i_connector;
			ui8_bit = ( 1 << ( i_node & 7 ) );
			pui8_node_vis[ i_node >> 3 ] |= ui8_bit;
			do
			{
				i_node = ps_nodes[ i_node ].i_parent_node;
				ui8_bit = ( 1 << ( i_node & 7 ) );
				if( pui8_node_vis[ i_node >> 3 ] & ui8_bit )
				{
					break;
				}
				pui8_node_vis[ i_node >> 3 ] |= ui8_bit;
			} while( i_node != 0 );
		}
		ui8_bx += ui8_bx;
		if( ui8_bx == 0 )
		{
			ui8_vis = *( pui8_sector_vis++ );
			ui8_bx = 1;
		}
	}
}


// Main Function
#ifdef WIN32
void main( int argc, char *argv[ ] )
#else
void _main(void)
#endif
{
  	Int16 i_offs = 0;
	Int16 i_run = 1, i_frames, i_idx, i_prev_esc = 0;
	Int16 i_repository_ret;
#if PERFORMANCE_COUNTERS || SIMPLE_PERFORMANCE_COUNTERS
	Int16 i_delt = 0;
	UInt16 i_ticks_start, i_ticks_end;
	UInt16 i_timer_start, i_time_prev = 0, i_time;
#endif
	UInt16 ui16_keys1;
	Int16 i_first;
	engine_t *ps_eng;
#ifndef WIN32
	UInt8 ui8_saved_rate;
	UInt16 ui16_saved_rate;
	INT_HANDLER saved_int1;
	INT_HANDLER saved_int5;
#endif

#ifdef WIN32
	sys_win32_init( &s_win32 );
#else
#if WITH_SOUND
	i_sound_enabled = 0;
#endif
	saved_int1 = GetIntVec (AUTO_INT_1);
	SetIntVec (AUTO_INT_1, DUMMY_HANDLER);

	saved_int5 = GetIntVec (AUTO_INT_5);
	SetIntVec (AUTO_INT_5, eng_int5_handler);

	ui8_saved_rate = PRG_getStart();
	ui16_saved_rate = PRG_getRate();
	PRG_setRate( 0 );
	PRG_setStart( 256 - TIMER_DIVIDER );
	{

		UInt32 osc2_rate;
		UInt32 ui_powerdown_ticks;

		ui_powerdown_ticks = OSTimerCurVal( 2 );

		if( HW_VERSION == 1 )
		{
			osc2_rate = 242688000UL;
		}
		else
		{
			osc2_rate = 162816000UL;
		}

		while( ui_powerdown_ticks > 0xffff )
		{
			ui_powerdown_ticks >>= 1;
			osc2_rate >>= 1;
		}
		osc2_rate = div_u32_u16( osc2_rate, ui_powerdown_ticks );
		osc2_rate *= 20;

		ui_powerdown_ticks = ( ( 1000UL * TIMER_DIVIDER ) << 8 );
		osc2_rate >>= 5;
		while( osc2_rate > 0xffff )
		{
			ui_powerdown_ticks >>= 1;
			osc2_rate >>= 1;
		}
		i_int5_to_ms = div_u32_u16( ui_powerdown_ticks, osc2_rate );
	}
	//FontSetSys( F_4x6 );

	GrayOn();
#if WITH_SOUND
	enable_sound();
#endif
#endif


	i_repository_ret = init_engine_and_data_repository( &ps_eng );
	if( !i_repository_ret )
	{
		goto eng_shutdown_norepo;
	}

	sprite_cache_init( ps_eng );
	eng_generate_commit_drawbuffer_lut( ps_eng );

	
	i_frames = 0;
	memset( ps_eng->p_drawbuffer, 0, DRAWBUFFER_SIZE );
	
	draw_decorations( ps_eng );
	eng_draw_commit_drawbuffer( ps_eng );

//	DrawStr( 16, 16, "4", A_REPLACE );
#if 0
	ps_eng->ui16_difficulty = 0;
	eng_change_map( ps_eng, MAP_05, 0 );
#else
	ps_eng->ui8_current_map = 254;
	ps_eng->ui8_menu_state = MENU_STATE_INGAME;
#endif
//	DrawStr( 16, 16, "5", A_REPLACE );

	i_first = 1;
	while( i_run )
	{
		Int32 i_ticks;

#if PERFORMANCE_COUNTERS
		i_ticks_start = eng_get_time();
#endif
		ps_eng->ui16_time = eng_get_time();
		if( i_first )
		{
			i_first = 0;
			ps_eng->ui16_last_time = ps_eng->ui16_time - TICK_MS;
		}

		i_offs = ( i_offs + 1 ) & 0xf;

#if PERFORMANCE_COUNTERS
		i_timer_map = i_timer_line_segment = i_timer_scan_vspan = i_timer_render_vspan = i_timer_render_vspan_inner = i_timer_scan_floor = 0;
		i_timer_render_floor = i_timer_render_floor_inner = i_timer_input = i_timer_sprites/* = i_timer_buffer*/ = 0;
#endif

		{
			ui16_keys1 = 0;
#if PERFORMANCE_COUNTERS
			i_timer_start = eng_get_time();
#endif
#ifndef WIN32
  	  		BEGIN_KEYTEST
#endif

#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_UP ) )
#else
			if( _keytest_optimized( RR_UP ) )
#endif
			{
				ui16_keys1 |= EKEY_UP;
			}
#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_DOWN ) )
#else
			if( _keytest_optimized( RR_DOWN ) )
#endif
			{
				ui16_keys1 |= EKEY_DOWN;
			}
#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_LEFT ) )
#else
			if( _keytest_optimized( RR_LEFT ) )
#endif
			{
				ui16_keys1 |= EKEY_LEFT;
			}
#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_RIGHT ) )
#else
			if( _keytest_optimized( RR_RIGHT ) )
#endif
			{
				ui16_keys1 |= EKEY_RIGHT;
			}

#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_0 ) )
#else
			if( _keytest_optimized( RR_2ND ) )
#endif
			{
				ui16_keys1 |= EKEY_0;
			}
#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_1 ) )
#else
			if( _keytest_optimized( RR_X ) )
#endif
			{
				ui16_keys1 |= EKEY_1;
			}
#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_2 ) )
#else
			if( _keytest_optimized( RR_Y ) )
#endif
			{
				ui16_keys1 |= EKEY_2;
			}
#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_3 ) )
#else
			if( _keytest_optimized( RR_Z ) )
#endif
			{
				ui16_keys1 |= EKEY_3;
			}

#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_6 ) )
#else
			if( _keytest_optimized( RR_6 ) )
#endif
			{
				ui16_keys1 |= EKEY_6;
			}

#ifdef WIN32
			if( sys_test_key( &s_win32, ENG_KEY_ESC ) )
#else
			if (_keytest_optimized (RR_ESC) )
#endif
			{
				if( i_prev_esc == 0 )
				{
					ps_eng->ui8_menu_state = ps_eng->ui8_menu_state == MENU_STATE_INGAME;
				}
				i_prev_esc = 1;
			}
			else
			{
				i_prev_esc = 0;
			}

			ps_eng->s_input.ui16_keys2 = ps_eng->s_input.ui16_keys1;
			ps_eng->s_input.ui16_keys1 = ui16_keys1;


#ifndef WIN32
	    	END_KEYTEST
#endif
			if( ps_eng->s_input.ui16_keys1 & EKEY_6 && !( ps_eng->s_input.ui16_keys2 & EKEY_6 ) )
			{
				ps_eng->ui8_666++;
				if( ps_eng->ui8_666 == 3 )
				{
					ps_eng->b_godmode = !ps_eng->b_godmode;
				}
			}
			if( ps_eng->s_input.ui16_keys1 & ~EKEY_6 )
			{
				ps_eng->ui8_666 = 0;
			}
		}



		i_ticks = div_u32_u16( ( UInt16 )( ps_eng->ui16_time - ps_eng->ui16_last_time ), ( ( UInt16 )TICK_MS ) );
		ps_eng->ui16_last_time += mul_16_16( TICK_MS, i_ticks );
		if( i_ticks > 4 )
		{
			i_ticks = 4; /* just make things slower if fps is really low */
		}

		if( ps_eng->ui8_menu_state != MENU_STATE_INGAME )
		{
			i_ticks = 0;
		}

		while( i_ticks-- )
		{
			entities_think( ps_eng );
		}
#if PERFORMANCE_COUNTERS
		i_timer_input += eng_get_time() - i_timer_start;
#endif


#if PERFORMANCE_COUNTERS || SIMPLE_PERFORMANCE_COUNTERS
		i_timer_start = eng_get_time();
#endif

	  	memset( ps_eng->p_drawbuffer, 0, ( DRAWBUFFER_SCREEN_WIDTH * REND_SCREEN_HEIGHT ) >> 2 );

		if( ps_eng->ps_player )
		{
			sector_t *p_visibility_sector;
			ps_eng->origin[ 0 ] = ps_eng->ps_player->v_origin[ 0 ];
			ps_eng->origin[ 1 ] = ps_eng->ps_player->v_origin[ 1 ];
			ps_eng->origin[ 2 ] = ps_eng->ps_player->v_origin[ 2 ] + ( ( ps_eng->ps_player->i_height - 4 ) << 4 );
			ps_eng->yaw = ps_eng->ps_player->d8_yaw;

			p_visibility_sector = eng_get_visibility_sector( ps_eng );
			if( p_visibility_sector )
			{
				ps_eng->pui8_visibility = &ps_eng->s_map.pui8_visibility[ p_visibility_sector->i_visibility ];
			}
			else
			{
				ps_eng->pui8_visibility = 0;
			}
			determine_draw_node_visibility( ps_eng );
		}

#if 0
		ps_eng->origin[ 0 ] = 3232;
		ps_eng->origin[ 1 ] = -1674;
		ps_eng->origin[ 2 ] = 448;
		ps_eng->yaw = 18;
#endif

		eng_setup_world_transform( ps_eng );

		for( i_idx = 0; i_idx < REND_SCREEN_WIDTH; i_idx++ )
		{
			ps_eng->rgui8_yclip[ i_idx ][ 0 ] = 0;
			ps_eng->rgui8_yclip[ i_idx ][ 1 ] = REND_SCREEN_HEIGHT;
		}

		ps_eng->i_cliplist = 0xff;
		ps_eng->i_num_cliplist = 0;


		if( ps_eng->ui8_current_map < 254 )
		{
#if PERFORMANCE_COUNTERS
			UInt16 i_timer_start, i_timer_flush;
#endif
			draw_sprites_start( ps_eng );
#if PERFORMANCE_COUNTERS
			i_timer_start = eng_get_time();
#endif

			init_floor_ceiling_cache( ps_eng );
			eng_draw_map_r( ps_eng, 0 );


#if PERFORMANCE_COUNTERS
			i_timer_flush = eng_get_time();
#endif
			eng_flush_ceil( ps_eng );
			eng_flush_floor( ps_eng );

#if PERFORMANCE_COUNTERS
			i_timer_flush_floor = eng_get_time() - i_timer_flush;
			i_timer_map += eng_get_time() - i_timer_start;
#endif
#if PERFORMANCE_COUNTERS
			i_timer_start = eng_get_time();
#endif
			draw_sprites_finish( ps_eng );

			if( ps_eng->ps_player->ui8_think_state < ENTITIES_THINK_STATE_DYING )
			{
				draw_viewmodel( ps_eng );
			}
#if PERFORMANCE_COUNTERS
			i_timer_sprites = eng_get_time() - i_timer_start;
#endif
		}
		else if( ps_eng->ui8_current_map == 254 )
		{
			draw_splash( ps_eng );
		}
		else if( ps_eng->ui8_current_map == 255 )
		{
			draw_end( ps_eng );
		}

#if PERFORMANCE_COUNTERS
		{
			UInt8 rgui8_buf[ 0x40 ];
			i_ticks_end = eng_get_time();
			i_timer_start = eng_get_time();
			sprintf( rgui8_buf, "%d m%d,s%d,b%d", i_delt, i_timer_map, i_timer_sprites, i_timer_buffer );
			draw_string( ps_eng, 0, 0, 0, 0, rgui8_buf );
			sprintf( rgui8_buf, "%d vs%d,vr%d,vri%d", i_timer_line_segment, i_timer_scan_vspan, i_timer_render_vspan, i_timer_render_vspan_inner );
			draw_string( ps_eng, 0, 0, 6, 0, rgui8_buf );
			sprintf( rgui8_buf, "%d fs%d,fr%d,fri%d", i_timer_line_segment + i_timer_flush_floor, i_timer_scan_floor, i_timer_render_floor, i_timer_render_floor_inner );
			draw_string( ps_eng, 0, 0, 12, 0, rgui8_buf );
			/*sprintf( rgui8_buf, "%d,%d,%d : %d", ps_eng->origin[ 0 ], ps_eng->origin[ 1 ], ps_eng->origin[ 2 ], ps_eng->ps_player->d8_yaw );
			draw_string( ps_eng, 0, 0, 12, 0, rgui8_buf );*/
		}
#endif

#if SIMPLE_PERFORMANCE_COUNTERS
		{
			UInt8 rgui8_buf[ 0x20 ];
			sprintf( rgui8_buf, "%d", i_delt );
			draw_string( ps_eng, 0, 0, 0, 0, rgui8_buf );
		}
#endif
		/*
		sprintf( rgui8_buf, "%d ( %d )", ( Int16 )div_u32_u16( 10000, ( i_delt ) + 1 ), i_frames );
		draw_string( ps_eng, 0, 100, 0, 0, rgui8_buf );
*/

		if( ps_eng->ui8_menu_state != MENU_STATE_INGAME )
		{
			if( ps_eng->ui8_menu_state > MENU_STATE_DIFFICULTY3 )
			{
				if( ps_eng->ui8_menu_state == MENU_STATE_CHANGELEVEL )
				{
					if( ps_eng->ui8_current_map >= MAP_05 )
					{
						ps_eng->ui8_current_map = 255;
						/*eng_change_map( ps_eng, MAP_01, &ps_eng->s_player );*/
					}
					else
					{
						eng_change_map( ps_eng, ps_eng->ui8_current_map + 1, &ps_eng->s_player );
					}
					ps_eng->ui8_menu_state = MENU_STATE_INGAME;
				}
			}
			else
			{
				Int16 ui_choosen;

				ui_choosen = menu( ps_eng );
				if( ui_choosen == 1 )
				{
					i_run = 0;
				}
				else if( ui_choosen >= MENU_STATE_DIFFICULTY0 && ui_choosen <= MENU_STATE_DIFFICULTY3 )
				{
					ps_eng->ui16_difficulty = ( ui_choosen - MENU_STATE_DIFFICULTY0 + 1 ) << 6;
					eng_change_map( ps_eng, MAP_01, 0 );
					ps_eng->ui8_menu_state = MENU_STATE_INGAME;
				}
			}
		}

		{
#if PERFORMANCE_COUNTERS
			UInt16 ui_buffer_time_start = eng_get_time();
#endif
			eng_draw_commit_renderbuffer( ps_eng );
#if PERFORMANCE_COUNTERS
			i_timer_buffer = eng_get_time() - ui_buffer_time_start;
#endif

			if( !ps_eng->b_clean_status_bar )
			{
				draw_statusbar( ps_eng );
				eng_draw_commit_statusbar( ps_eng );
				ps_eng->b_clean_status_bar = TRUE;
			}
		}

#if SIMPLE_PERFORMANCE_COUNTERS || PERFORMANCE_COUNTERS
		i_time = eng_get_time();
		i_delt = i_time - i_time_prev;
		i_time_prev = eng_get_time();
#endif

		i_frames++;
	}

	eng_shutdown_norepo:;

	deinit_data_repository( ps_eng );

#ifdef WIN32
	sys_win32_deinit( &s_win32 );
#else
	if( kbhit() )
	{
		ngetchx();
		i_run = 0;
	}

	GrayOff();
#if WITH_SOUND
	disable_sound();
#endif
	PRG_setRate( ui16_saved_rate );
	PRG_setStart( ui8_saved_rate );
	SetIntVec (AUTO_INT_1, saved_int1 );
	SetIntVec( AUTO_INT_5, saved_int5 );
#endif
}













