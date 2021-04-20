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
typedef UInt32 ( *draw_vspan_f ) ( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv );
#else
typedef UInt32 ( *draw_vspan_f ) ( UInt8 *pui8_drawbuffer asm("%a0"), UInt16 i_height asm("%d0"), UInt8 *pui8_tex asm("%a1"), UInt32 i_v asm("%d1"), UInt32 i_y_stepv asm("%d2") );;
#endif

/*
draw_vspan_f vspan_draw_funcs[ 4 ] = {
	eng_draw_vspan0, eng_draw_vspan1, eng_draw_vspan2, eng_draw_vspan3
};
*/

#ifdef WIN32
static void eng_draw_wall_segment_vlines( render_vline_gen_struct_t *ps_gstructv, render_vline_struct_t *ps_vlines, render_texture_params_t *ps_texture_paramsv )
{
	const render_texture_params_t *ps_texture_params = ( const render_texture_params_t *)ps_texture_paramsv;
	const render_vline_gen_struct_t *ps_gstruct = ( const render_vline_gen_struct_t * )ps_gstructv;
	UInt16 ui_timer_start;
	Int16 i_x, i_ru;
	UInt8 *pui8_drawbuffer, *pui8_db, *pui8_tex;
	Int32 i_rzi, i_v_offset;
	
	i_x = ps_gstruct->i16_x_start;
	ps_vlines = ps_vlines + i_x;

	i_v_offset = ( ( Int32 )ps_gstruct->i_v_offset ) << 12;
	
	pui8_drawbuffer = ps_gstruct->pui8_db;
	i_rzi = ( mul_u16_32_ns( i_x, ps_texture_params->i_zx ) ) + ps_texture_params->i_zbase;
	i_ru = mul_16_16( i_x, ps_texture_params->i_ux ) + ps_texture_params->i_ubase; 

	do
	{
		Int32 i_zi, i_tu;
		Int16 i_py, i_pheight, i_pu, i_u;
		UInt16 i_z;
		Int32 i_v, i_y_stepv;

		i_py = ps_vlines->i_py;
		i_pheight = ps_vlines->i_pheight;
		ps_vlines++;

		i_zi = i_rzi;
		i_u = i_ru;
		i_rzi += ps_texture_params->i_zx;
		i_ru += ps_texture_params->i_ux;

		if( i_pheight > i_py )
		{
			i_pheight = ( i_pheight - i_py );

			if( i_zi < 0x1000 )
			{
				i_zi = 0x1000;
			}
			i_z = div_u32_u16_u16r( 0x7ffffff>>8, i_zi >> 8 );
			
			i_u = ( ( UInt16 )( ( mul_16_16( i_z, i_u ) >> 16 ) ) ) + ( UInt16 )( ps_texture_params->i_ut );

			i_pu = i_u;
			pui8_tex = ps_gstruct->pui8_tex + ( ( ( i_pu & 0x3e ) << 7 ) );
			/*ps_vlines->i_saved_pu = i_pu;*/

			i_y_stepv = mul_u16_u16( i_z, ( INVERSE_VSCALE_SCALE >> 4 ) );
			i_v = ps_gstruct->pi16_ritab[ i_py ];
			/*i_v = mul_16_16( i_py - ( REND_SCREEN_HEIGHT / 2 ), ( INVERSE_VSCALE_SCALE >> 4 ) );*/
			i_v = ( mul_16_16( i_v, i_z ) );
			i_v += i_v_offset;

			/*i_v = i_v_offset + mul_16_u32_ns( i_py - (REND_SCREEN_HEIGHT/2), i_y_stepv );*/


			pui8_db = ps_gstruct->pui8_db + ps_gstruct->pui16_ylut[ i_py ] + ( i_x >> 2 );
			

/*
#if PERFORMANCE_COUNTERS
			ui_timer_start = eng_get_time();
#endif
*/
			eng_draw_vspanX( pui8_db, i_pheight, pui8_tex, i_v, i_y_stepv, i_x & 3 );
/*
#if PERFORMANCE_COUNTERS
			i_timer_render_vspan_inner += eng_get_time() - ui_timer_start;
#endif
*/
		}
		i_x++;
	}
	while( i_x != ps_gstruct->i16_x_end );
}

#else
void eng_draw_wall_segment_vlines( render_vline_gen_struct_t *ps_gstructv asm( "%a0" ), render_vline_struct_t *ps_vlines asm( "%a1" ), render_texture_params_t *ps_texture_paramsv asm( "%a2" ) );
#endif

#if 0
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
				i_z = div_u32_u16_u16r( 0x7ffffff>>8, i_zi >> 8 );
			
				i_u = i_ru;
				i_u = ( ( UInt16 )( ( mul_16_16( i_z, (i_u>>1) ) >> 16 ) << 2 ) ) + ( UInt16 )ps_texture_params->i_ut;

				i_pheight = ( i_pheight - i_py );


				i_pu = i_u;
				i_y_stepv = mul_u16_u16( ( INVERSE_VSCALE_SCALE >> 4 ), i_z );
			}

			i_v = mul_16_16( i_py - ( REND_SCREEN_HEIGHT / 2 ), ( INVERSE_VSCALE_SCALE >> 4 ) );
			i_v = ( mul_16_16( i_v, i_z ) );
			i_v += i_v_offset;


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
#endif

static Int16 clip_span( engine_t *ps_eng, Int16 *pi_x_start, Int16 *pi_x_end, Int16 update_clip, Int16 *pi_skiplist, Int16 *pi_skiplist_length )
{
	Int16 i_x_start, i_x_end, i_rightjoin, i_x, i_skiplist_length = 0;
	cliplist_t *p_clip, *p_clipleft, *p_clipright;

	p_clip = ps_eng->i_cliplist != 0xff ? &ps_eng->rgs_cliplist[ ps_eng->i_cliplist ] : 0;
	p_clipleft = p_clipright = 0;

	i_x_start = *pi_x_start;
	i_x_end = *pi_x_end;

	while( p_clip )
	{
		if( p_clip->i_x1 <= i_x_start && p_clip->i_x2 >= i_x_start )
		{
			p_clipleft = p_clip;
			break;
		}
		if( p_clip->i_x1 > i_x_start )
		{
			break;
		}
		p_clip = ( p_clip->i_next != 0xff ) ? &ps_eng->rgs_cliplist[ p_clip->i_next ] : 0;
	}

	if( p_clipleft )
	{
		i_x_start = p_clipleft->i_x2;
		/* advance for skiplist */
		p_clip = ( p_clip->i_next != 0xff ) ? &ps_eng->rgs_cliplist[ p_clip->i_next ] : 0;
	}
	if( i_x_start >= i_x_end )
	{
		return 1;
	}

	i_x = i_x_start;

	while( p_clip )
	{
		if( p_clip->i_x1 <= i_x_end && p_clip->i_x2 >= i_x_end )
		{
			p_clipright = p_clip;
			break;
		}
		if( p_clip->i_x1 > i_x_end )
		{
			break;
		}
		pi_skiplist[ i_skiplist_length++ ] = p_clip->i_x1 - i_x;
		pi_skiplist[ i_skiplist_length++ ] = p_clip->i_x2 - p_clip->i_x1;
		i_x = p_clip->i_x2;

		p_clip = ( p_clip->i_next != 0xff ) ? &ps_eng->rgs_cliplist[ p_clip->i_next ] : 0;
	}
	if( p_clipright )
	{
		i_x_end = p_clipright->i_x1;
	}
	if( i_x_start >= i_x_end )
	{
		return 1;
	}

	*pi_x_start = i_x_start;
	*pi_x_end = i_x_end;

	pi_skiplist[ i_skiplist_length++ ] = i_x_end - i_x;
	*pi_skiplist_length = i_skiplist_length;

	if( update_clip )
	{
		Int16 i_clip, i_leftjoin;

		i_leftjoin = 0xff;
		i_rightjoin = ps_eng->i_cliplist;
		while( i_rightjoin != 0xff )
		{
			if( ps_eng->rgs_cliplist[ i_rightjoin ].i_x2 <= i_x_start )
			{
				i_leftjoin = i_rightjoin;
				i_rightjoin = ps_eng->rgs_cliplist[ i_rightjoin ].i_next;
			}
			else
			{
				break;
			}
		}
		while( i_rightjoin != 0xff )
		{
			if( ps_eng->rgs_cliplist[ i_rightjoin ].i_x1 >= i_x_end )
			{
				break;
			}
			i_rightjoin = ps_eng->rgs_cliplist[ i_rightjoin ].i_next;
		}

		if( i_leftjoin != 0xff )
		{
			if( ps_eng->rgs_cliplist[ i_leftjoin ].i_x2 == i_x_start )
			{
				i_clip = i_leftjoin;
				ps_eng->rgs_cliplist[ i_leftjoin ].i_x2 = ( UInt8 )i_x_end;
			}
			else
			{
				i_clip = ps_eng->i_num_cliplist++;
				ps_eng->rgs_cliplist[ i_clip ].i_x1 = ( UInt8 )i_x_start;
				ps_eng->rgs_cliplist[ i_clip ].i_x2 = ( UInt8 )i_x_end;
				ps_eng->rgs_cliplist[ i_leftjoin ].i_next = ( UInt8 )i_clip;
			}
		}
		else
		{
			i_clip = ps_eng->i_num_cliplist++;
			ps_eng->rgs_cliplist[ i_clip ].i_x1 = ( UInt8 )i_x_start;
			ps_eng->rgs_cliplist[ i_clip ].i_x2 = ( UInt8 )i_x_end;
			ps_eng->rgs_cliplist[ i_clip ].i_next = ( UInt8 )ps_eng->i_cliplist;
			ps_eng->i_cliplist = i_clip;
		}
		if( i_rightjoin != 0xff )
		{
			if( ps_eng->rgs_cliplist[ i_rightjoin ].i_x1 == i_x_end )
			{
				ps_eng->rgs_cliplist[ i_clip ].i_next = ps_eng->rgs_cliplist[ i_rightjoin ].i_next;
				ps_eng->rgs_cliplist[ i_clip ].i_x2 = ps_eng->rgs_cliplist[ i_rightjoin ].i_x2;
			}
			else
			{
				ps_eng->rgs_cliplist[ i_clip ].i_next = ( UInt8 )i_rightjoin;
			}
		}
		else
		{
			ps_eng->rgs_cliplist[ i_clip ].i_next = 0xff;
		}
	}

	return 0;
}

void eng_flush_ceil( engine_t *ps_eng );
void eng_flush_floor( engine_t *ps_eng );

Int16 can_join_ceiling( engine_t *ps_eng, Int16 i_x_l, Int16 i_x_r )
{
	Int16 i_left, i_right, i_idx;

	if( ps_eng->i_ceiling_z != ps_eng->s_map.p_vertices[ ps_eng->ps_gsector->i_zvert ][ 1 ] ||
		ps_eng->i_ceiling_texture != ps_eng->ps_gsector->ui8_texture_ceiling )
	{
		return 0;
	}

	if( i_x_l < ps_eng->i_ceil_minx )
	{
		i_left = ps_eng->i_ceil_minx;
	}
	else
	{
		i_left = i_x_l;
	}
	if( i_x_r < ps_eng->i_ceil_maxx )
	{
		i_right = i_x_r;
	}
	else
	{
		i_right = ps_eng->i_ceil_maxx;
	}
	if( i_right <= i_left )
	{
		return 1;
	}
	for( i_idx = i_left; i_idx < i_right; i_idx++ )
	{
		if( ps_eng->rgui8_ceiling[ i_idx ][ 0 ] != REND_SCREEN_HEIGHT )
		{
			return 0;
		}
	}
	return 1;
}

Int16 can_join_floor( engine_t *ps_eng, Int16 i_x_l, Int16 i_x_r )
{
	Int16 i_left, i_right, i_idx;

	if( ps_eng->i_floor_z != ps_eng->s_map.p_vertices[ ps_eng->ps_gsector->i_zvert ][ 0 ] ||
	    ps_eng->i_floor_texture != ps_eng->ps_gsector->ui8_texture_floor )
	{
		return 0;
	}

	if( i_x_l < ps_eng->i_floor_minx )
	{
		i_left = ps_eng->i_floor_minx;
	}
	else
	{
		i_left = i_x_l;
	}
	if( i_x_r < ps_eng->i_floor_maxx )
	{
		i_right = i_x_r;
	}
	else
	{
		i_right = ps_eng->i_floor_maxx;
	}
	if( i_right <= i_left )
	{
		return 1;
	}

	for( i_idx = i_left; i_idx < i_right; i_idx++ )
	{
		if( ps_eng->rgui8_floor[ i_idx ][ 1 ] != 0 )
		{
			return 0;
		}
	}

	return 1;
}


Int16 eng_draw_wall_segment_scan_middle( Int16 i_x, Int16 i_x_end, Int32 i_y0, Int32 i_x_stepy0, Int32 i_y1, Int32 i_x_stepy1, UInt8 (*pui8_yclip )[ 2 ], render_vline_struct_t *ps_vlines, UInt8 *pui8_ceiling, UInt8 *pui8_floor )
{
	Int16 b_any = FALSE;

	pui8_floor += i_x * 2;
	pui8_ceiling += i_x * 2;

	for( (void)i_x; i_x < i_x_end; i_x++ )
	{
		Int16 i_yclip, i_yclip2;
		Int16 i_py;
		Int16 i_pheight;
		//glob_x = i_x;

		i_py = i_y0 >> 16;
		i_pheight = i_y1 >> 16;

		i_y0 += i_x_stepy0;
		i_y1 += i_x_stepy1;

		i_yclip = pui8_yclip[ i_x ][ 0 ];

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
			if( i_ceil_py > pui8_yclip[ i_x ][ 1 ] )
			{
				i_ceil_py = pui8_yclip[ i_x ][ 1 ];
			}
			*(pui8_ceiling++) = ( UInt8 )i_yclip;
			*(pui8_ceiling++) = ( UInt8 )i_ceil_py;
		}

		i_yclip2 = pui8_yclip[ i_x ][ 1 ];
		if( i_pheight > i_yclip2 )
		{
			i_pheight = i_yclip2;
			*(pui8_floor++) = ( UInt8 )i_yclip2;
			*(pui8_floor++) = ( UInt8 )i_yclip2;
		}
		else
		{
			Int16 i_floor_pheight;

			i_floor_pheight = i_pheight;
			if( i_floor_pheight < i_yclip )
			{
				i_floor_pheight = i_yclip;
			}
			*(pui8_floor++) = ( UInt8 )i_floor_pheight;
			*(pui8_floor++) = ( UInt8 )i_yclip2;
		}

		ps_vlines[ i_x ].i_py = i_py;
		ps_vlines[ i_x ].i_pheight = i_pheight;

		if( i_pheight > i_py )
		{
			pui8_yclip[ i_x ][ 0 ] = ( UInt8 )i_pheight;
			pui8_yclip[ i_x ][ 1 ] = ( UInt8 )i_py;
			b_any = TRUE;
		}
	}
	return b_any;
}


Int16 eng_draw_wall_segment_scan_upper( Int16 i_x, Int16 i_x_end, Int32 i_y0, Int32 i_x_stepy0, Int32 i_y2, Int32 i_x_stepy2, UInt8 (*pui8_yclip )[ 2 ], render_vline_struct_t *ps_vlines, UInt8 *pui8_ceiling )
{
	Int16 b_any = FALSE;

	pui8_ceiling += i_x * 2;

	for( (void)i_x; i_x < i_x_end; i_x++ )
	{
		Int16 i_py, i_pheight;
		Int16 i_yclip;

		i_yclip = pui8_yclip[ i_x ][ 0 ];

		i_py = ( i_y0 ) >> 16;
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
			if( i_ceil_py > pui8_yclip[ i_x ][ 1 ] )
			{
				i_ceil_py = pui8_yclip[ i_x ][ 1 ];
			}
			*(pui8_ceiling++) = ( UInt8 )i_yclip;
			*(pui8_ceiling++) = ( UInt8 )i_ceil_py;
		}

		i_yclip = pui8_yclip[ i_x ][ 1 ];
		i_pheight = ( i_y2 >> 16 );
		if( i_pheight > i_yclip )
		{
			i_pheight = i_yclip;
		}

		ps_vlines[ i_x ].i_py = i_py;
		ps_vlines[ i_x ].i_pheight = i_pheight;

		if( i_pheight > i_py )
		{
			pui8_yclip[ i_x ][ 0 ] = ( UInt8 )i_pheight;
			b_any = TRUE;
		}

		i_y0 += i_x_stepy0;
		i_y2 += i_x_stepy2;
	}
	return b_any;
}

Int16 eng_draw_wall_segment_scan_lower( Int16 i_x, Int16 i_x_end, Int32 i_y1, Int32 i_x_stepy1, Int32 i_y3, Int32 i_x_stepy3, UInt8 (*pui8_yclip )[ 2 ], render_vline_struct_t *ps_vlines, UInt8 *pui8_floor )
{
	Int16 b_any = FALSE;

	pui8_floor += i_x * 2;

	for( (void)i_x; i_x < i_x_end; i_x++ )
	{
		Int16 i_py, i_pheight;
		Int16 i_yclip;

		i_yclip = pui8_yclip[ i_x ][ 0 ];
		i_py = ( i_y3 ) >> 16;
		if( i_py < i_yclip )
		{
			i_py = i_yclip;
		}

		i_yclip = pui8_yclip[ i_x ][ 1 ];
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
			if( i_floor_pheight < pui8_yclip[ i_x ][ 0 ] )
			{
				i_floor_pheight = pui8_yclip[ i_x ][ 0 ];
			}
			*(pui8_floor++) = i_floor_pheight;
			*(pui8_floor++) = i_yclip;
		}

		ps_vlines[ i_x ].i_py = i_py;
		ps_vlines[ i_x ].i_pheight = i_pheight;

		if( i_pheight > i_py )
		{
			pui8_yclip[ i_x ][ 1 ] = ( UInt8 )i_py;
			b_any = TRUE;
		}
		i_y1 += i_x_stepy1;
		i_y3 += i_x_stepy3;
	}
	return b_any;
}

void eng_draw_wall_segment( engine_t *ps_eng )
{
	Int32 i_x_stepy0, i_x_stepy1, i_x_stepy2 = 0, i_x_stepy3 = 0;
	Int32 i_y0, i_y1, i_y2, i_y3;
	Int16 i_width, i_x_seg_end, i_x_start, i_x_end;
	Int16 i_floor_start, i_floor_end, i_ceil_start, i_ceil_end;
#if PERFORMANCE_COUNTERS
	Int16 i_timer_start, i_timer_start2;
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
			pui8_ceiling = &ps_eng->rgui8_ceiling[ 0 ][ 0 ];
			pui8_floor = &ps_eng->rgui8_floor[ 0 ][ 0 ];

#if PERFORMANCE_COUNTERS
			i_timer_start = eng_get_time();
#endif
			b_any = eng_draw_wall_segment_scan_middle( i_x_start, i_x_end, i_y0, i_x_stepy0, i_y1, i_x_stepy1, ps_eng->rgui8_yclip, ps_eng->rgs_vlines, &ps_eng->rgui8_ceiling[ 0 ][ 0 ], &ps_eng->rgui8_floor[ 0 ][ 0 ] );

#if PERFORMANCE_COUNTERS
			i_timer_start2 = eng_get_time();
			i_timer_scan_vspan += i_timer_start2 - i_timer_start;
#endif

			if( b_any )
			{
				render_vline_gen_struct_t s_gs;
				s_gs.i16_x_start = i_x_start;
				s_gs.i16_x_end = i_x_end;
				s_gs.pui16_ylut = ( UInt16 *)ps_eng->rgui16_ylut;
				s_gs.pi16_ritab = ( Int16 *)ps_eng->rgi16_ritab;
				s_gs.i_v_offset = ps_eng->world_transform.translation[ 2 ];
				s_gs.pui8_db = ( UInt8 * )ps_eng->p_drawbuffer;
				s_gs.pui8_tex = texture_get( ps_eng, ps_gline->ui8_texture_upper, TRUE );

				eng_draw_wall_segment_vlines( &s_gs, ps_eng->rgs_vlines, &ps_eng->s_tex_params );
			}


#if PERFORMANCE_COUNTERS
			i_timer_render_vspan += eng_get_time() - i_timer_start2;
#endif
		}
		else
		{
			UInt8 *pui8_ceiling, *pui8_floor;
			UInt16 i_scan_upper, i_scan_lower;

			pui8_ceiling = &ps_eng->rgui8_ceiling[ i_x_start ][ 0 ];
			pui8_floor = &ps_eng->rgui8_floor[ i_x_start ][ 0 ];

			i_scan_upper = ( ps_eng->s_line_segment.ui8_flags & MAP_LINE_FLAGS_UPPER ) && ( ps_eng->s_line_segment.y21 > 0 || ps_eng->s_line_segment.y22 > 0 ) && ( ps_eng->s_line_segment.y01 <= ( (Int32)REND_SCREEN_HEIGHT << 16 ) || ps_eng->s_line_segment.y02 <= ( (Int32)REND_SCREEN_HEIGHT << 16 ) );
			i_scan_lower = ( ps_eng->s_line_segment.ui8_flags & MAP_LINE_FLAGS_LOWER ) && ( ps_eng->s_line_segment.y11 > 0 || ps_eng->s_line_segment.y12 > 0 ) && ( ps_eng->s_line_segment.y31 <= ( (Int32)REND_SCREEN_HEIGHT << 16 ) || ps_eng->s_line_segment.y32 <= ( (Int32)REND_SCREEN_HEIGHT << 16 ) );


			if( i_scan_upper )
			{
				b_any = FALSE;

#if PERFORMANCE_COUNTERS
				i_timer_start = eng_get_time();
#endif

				b_any = eng_draw_wall_segment_scan_upper( i_x_start, i_x_end, i_y0, i_x_stepy0, i_y2, i_x_stepy2, ps_eng->rgui8_yclip, ps_eng->rgs_vlines, &ps_eng->rgui8_ceiling[ 0 ][ 0 ] );

#if PERFORMANCE_COUNTERS
				i_timer_start2 = eng_get_time();
				i_timer_scan_vspan += i_timer_start2 - i_timer_start;
#endif

				if( b_any )
				{
					render_vline_gen_struct_t s_gs;
					s_gs.i16_x_start = i_x_start;
					s_gs.i16_x_end = i_x_end;
					s_gs.pui16_ylut = ( UInt16 *)ps_eng->rgui16_ylut;
					s_gs.pi16_ritab = ( Int16 *)ps_eng->rgi16_ritab;
					s_gs.i_v_offset = ps_eng->world_transform.translation[ 2 ];
					s_gs.pui8_db = ( UInt8 * )ps_eng->p_drawbuffer;
					s_gs.pui8_tex = texture_get( ps_eng, ps_gline->ui8_texture_upper, TRUE );

					eng_draw_wall_segment_vlines( &s_gs, ps_eng->rgs_vlines, &ps_eng->s_tex_params );
				}

#if PERFORMANCE_COUNTERS
			i_timer_render_vspan += eng_get_time() - i_timer_start2;
#endif

			}
			else if( ps_eng->i_mark_ceiling )
			{
				Int32 i_py;
				Int32 i_yclip;

#if PERFORMANCE_COUNTERS
				i_timer_start = eng_get_time();
#endif
				for( i_x = i_x_start; i_x < i_x_end; i_x++ )
				{
					i_yclip = ps_eng->rgui8_yclip[ i_x ][ 0 ];
					i_py = ( i_y0 ) >> 16;
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
					i_y0 += i_x_stepy0;
				}
#if PERFORMANCE_COUNTERS
				i_timer_scan_floor += eng_get_time() - i_timer_start;
#endif
			}

			if( i_scan_lower )
			{
				b_any = FALSE;
#if PERFORMANCE_COUNTERS
				i_timer_start = eng_get_time();
#endif
				b_any = eng_draw_wall_segment_scan_lower( i_x_start, i_x_end, i_y1, i_x_stepy1, i_y3, i_x_stepy3, ps_eng->rgui8_yclip, ps_eng->rgs_vlines, &ps_eng->rgui8_floor[ 0 ][ 0 ] );

#if PERFORMANCE_COUNTERS
				i_timer_start2 = eng_get_time();
				i_timer_scan_vspan += i_timer_start2 - i_timer_start;
#endif
				if( b_any )
				{
					render_vline_gen_struct_t s_gs;
					s_gs.i16_x_start = i_x_start;
					s_gs.i16_x_end = i_x_end;
					s_gs.pui16_ylut = ( UInt16 *)ps_eng->rgui16_ylut;
					s_gs.pi16_ritab = ( Int16 *)ps_eng->rgi16_ritab;
					s_gs.i_v_offset = ps_eng->world_transform.translation[ 2 ];
					s_gs.pui8_db = ( UInt8 * )ps_eng->p_drawbuffer;
					s_gs.pui8_tex = texture_get( ps_eng, ps_gline->ui8_texture_lower, TRUE );

					/* FIXME: cache */
					/*if( i_scan_upper )
					{
						eng_draw_wall_segment_vlines_lower( &s_gs, ps_eng->rgs_vlines, &ps_eng->s_tex_params );
					}
					else */
					{
						eng_draw_wall_segment_vlines( &s_gs, ps_eng->rgs_vlines, &ps_eng->s_tex_params );
					}
				}
#if PERFORMANCE_COUNTERS
				i_timer_render_vspan += eng_get_time() - i_timer_start2;
#endif

			}
			else if( ps_eng->i_mark_floor )
			{
#if PERFORMANCE_COUNTERS
				i_timer_start = eng_get_time();
#endif
				for( i_x = i_x_start; i_x < i_x_end; i_x++ )
				{
					Int16 i_py;
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

#if PERFORMANCE_COUNTERS
				i_timer_scan_floor += eng_get_time() - i_timer_start;
#endif
			}
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

void eng_mark_ceil_floor_segment( engine_t *ps_eng )
{
	Int32 i_x_stepy0, i_x_stepy1;
	Int32 i_x, i_y0, i_y1;
	Int16 i_width, i_x_start, i_x_end;
	UInt16 i_clip_left;
#if PERFORMANCE_COUNTERS
	UInt16 i_timer_start;
#endif
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

#if PERFORMANCE_COUNTERS
	i_timer_start = eng_get_time();
#endif

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
#if PERFORMANCE_COUNTERS
	i_timer_scan_floor += eng_get_time() - i_timer_start;
#endif
}

