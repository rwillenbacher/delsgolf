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

void sprite_cache_init( engine_t *ps_eng )
{
	Int16 i_idx;
	cached_sprite_t **pp_spr;

	pp_spr = &ps_eng->ps_sprite_cache->p_free;
	for( i_idx = 0; i_idx < SPRITE_CACHE_ENTRIES; i_idx++ )
	{
		*pp_spr = &ps_eng->ps_sprite_cache->rgs_cached_sprites[ i_idx ];
		pp_spr = &ps_eng->ps_sprite_cache->rgs_cached_sprites[ i_idx ].p_next;
	}
	*pp_spr = 0;

	ps_eng->ps_sprite_cache->p_used = 0;
	ps_eng->ps_sprite_cache->ui_lru_counter = 0;
	ps_eng->ps_sprite_cache->ui_sprite_cache_free_data = SPRITE_CACHE_SIZE;
}

static void sprite_cache_evict_oldest( engine_t *ps_eng )
{
	UInt16 ui_oldest;
	cached_sprite_t **pp_prev, **pp_oldest, *p_oldest, *p_cached;

	pp_oldest = pp_prev = &ps_eng->ps_sprite_cache->p_used;
	ui_oldest = 0;

	for( p_cached = ps_eng->ps_sprite_cache->p_used; p_cached; p_cached = p_cached->p_next )
	{
		if( ( ps_eng->ps_sprite_cache->ui_lru_counter - p_cached->ui_lru ) >= ui_oldest )
		{
			pp_oldest = pp_prev;
			ui_oldest = ( ps_eng->ps_sprite_cache->ui_lru_counter - p_cached->ui_lru );
		}
		pp_prev = &p_cached->p_next;
	}

	p_oldest = *pp_oldest;
	*pp_oldest = p_oldest->p_next;

	ps_eng->ps_sprite_cache->ui_sprite_cache_free_data += p_oldest->i_size;
	p_oldest->p_next = ps_eng->ps_sprite_cache->p_free;
	ps_eng->ps_sprite_cache->p_free = p_oldest;
}

static void sprite_cache_compact( engine_t *ps_eng )
{
	UInt16 ui_offset;
	cached_sprite_t *p_cached;

	ui_offset = 0;
	for( p_cached = ps_eng->ps_sprite_cache->p_used; p_cached; p_cached = p_cached->p_next )
	{
		if( p_cached->ui_offset != ui_offset )
		{
			memcpy( &ps_eng->ps_sprite_cache->rgui8_sprite_data[ ui_offset ], &ps_eng->ps_sprite_cache->rgui8_sprite_data[ p_cached->ui_offset ], p_cached->i_size );
			p_cached->ui_offset = ui_offset;
		}
		ui_offset += p_cached->i_size;
	}
}

UInt8 *sprite_cache_get_sprite( engine_t *ps_eng, Int16 i_idx, repository_sprite_t **pp_sprite, Int16 i_raw )
{
	UInt16 i_sprite_size, i_modified;
	UInt16 ui_offset;
	repository_sprite_t *p_sprite;
	cached_sprite_t *p_cached, **pp_tail, *p_new;


	for( p_cached = ps_eng->ps_sprite_cache->p_used; p_cached; p_cached = p_cached->p_next )
	{
		if( p_cached->i_idx == i_idx )
		{
			*pp_sprite = p_cached->p_sprite;
			p_cached->ui_lru = ps_eng->ps_sprite_cache->ui_lru_counter;
			return &ps_eng->ps_sprite_cache->rgui8_sprite_data[ p_cached->ui_offset ];
		}
	}

	repo_get_sprite( ps_eng, i_idx, &p_sprite, i_raw, NULL );

	if( !i_raw )
	{
		i_sprite_size = ( ( p_sprite->ui8_width * p_sprite->ui8_height ) + 3 ) & ~3;
	}
	else
	{
		i_sprite_size = ( p_sprite->i_sprite_length + 3 ) & ~3;
	}

	i_modified = 0;
	while( !ps_eng->ps_sprite_cache->p_free || ps_eng->ps_sprite_cache->ui_sprite_cache_free_data < i_sprite_size )
	{
		i_modified = 1;
		sprite_cache_evict_oldest( ps_eng );
	}
	if( i_modified )
	{
		sprite_cache_compact( ps_eng );
	}
	p_new = ps_eng->ps_sprite_cache->p_free;
	ps_eng->ps_sprite_cache->p_free = p_new->p_next;

	p_new->i_idx = i_idx;
	p_new->p_sprite = p_sprite;
	*pp_sprite = p_sprite;
	p_new->ui_lru = ps_eng->ps_sprite_cache->ui_lru_counter;
	ui_offset = SPRITE_CACHE_SIZE - ps_eng->ps_sprite_cache->ui_sprite_cache_free_data;
	p_new->ui_offset = ui_offset;
	p_new->i_size = i_sprite_size;

	pp_tail = &ps_eng->ps_sprite_cache->p_used;
	while( *pp_tail ) pp_tail = &( *pp_tail )->p_next;
	p_new->p_next = 0;
	*pp_tail = p_new;

	ps_eng->ps_sprite_cache->ui_sprite_cache_free_data -= i_sprite_size;

	repo_get_sprite( ps_eng, i_idx, NULL, i_raw, &ps_eng->ps_sprite_cache->rgui8_sprite_data[ ui_offset ] );

	return &ps_eng->ps_sprite_cache->rgui8_sprite_data[ ui_offset ];
}



void draw_sprites_start( engine_t *ps_eng )
{
	Int16 i16_idx;
	ps_eng->i_num_drawsprites = 0;
	ps_eng->i_num_sprite_clip = 0;
	ps_eng->ps_sprite_cache->ui_lru_counter++;

	for( i16_idx = 0; i16_idx < ps_eng->i_num_entities; i16_idx++ )
	{
		ps_eng->rgs_entities[ i16_idx ].ui_flags &= ~ENTITY_FLAGS_DRAWN;
	}
}


void generate_drawsprite( engine_t *ps_eng, vec3_t v_world, Int16 i_sprite_idx )
{
	Int32 i_mid_x, i_low_y;
	Int32 i_hwidth, i_height, i_x1, i_x2, i_y1, i_y2, i_scale, i_abs_sprite_idx;
	Int16 i_nspriteclip, i_clip_left, i_clip_top;
	UInt16 inv_v_origin_1;
	drawsprite_t *ps_ds;
	vec3_t v_origin;
	repository_sprite_t *ps_sprite;

	if( ps_eng->i_num_drawsprites >= MAX_NUM_DRAWSPRITES )
	{
		return;
	}

	i_abs_sprite_idx = i_sprite_idx;
	if( i_sprite_idx < 0 )
	{
		i_abs_sprite_idx = -i_abs_sprite_idx;
	}
	repo_get_sprite( ps_eng, i_abs_sprite_idx, &ps_sprite, 0, 0 );

	transform_vec2( v_world, &ps_eng->world_transform, v_origin );
	v_origin[ 2 ] = v_world[ 2 ] + ps_eng->world_transform.translation[ 2 ];

	if( v_origin[ 1 ] < ( 1<<4 ) )
	{
		return; /* behind or too close */
	}

	inv_v_origin_1 = div_u16_u16( 0xffff, v_origin[ 1 ] );

	i_mid_x = (mul_32_u16( (((Int32)v_origin[ 0 ]) * (REND_SCREEN_WIDTH/2) * 256 ), inv_v_origin_1 ) >> 8) + (REND_SCREEN_WIDTH/2);
	i_hwidth = mul_32_u16( ( (Int32)ps_sprite->ui8_width * (REND_SCREEN_WIDTH/2) * 256 ), inv_v_origin_1 ) >> ( 5 + SPRITE_SCALE_SHIFT );

	i_x1 = ( i_mid_x - i_hwidth );
	i_x2 = ( i_mid_x + i_hwidth );

	if( i_x2 <= 0 || i_x1 > REND_SCREEN_WIDTH )
	{
		return;  /* outside view */
	}
	i_clip_left = 0;
	if( i_x1 < 0 )
	{
		i_clip_left = -i_x1;
		i_x1 = 0;
	}
	if( i_x2 > REND_SCREEN_WIDTH )
	{
		i_x2 = REND_SCREEN_WIDTH;
	}

	i_low_y = ( ( mul_32_u16( mul_16_16( -v_origin[ 2 ], YSCALE_CONST ), inv_v_origin_1 ) ) >> 8 ) + (REND_SCREEN_HEIGHT/2);
	i_height = ( mul_32_u16( mul_16_16( ps_sprite->ui8_height, YSCALE_CONST ), inv_v_origin_1 ) ) >> ( 4 + SPRITE_SCALE_SHIFT );
	i_y1 = i_low_y - i_height;
	i_y2 = i_low_y;


	if( i_y2 <= 0 || i_y1 >= REND_SCREEN_HEIGHT )
	{
		return; /* outside view */
	}
	i_clip_top = 0;
	if( i_y1 < 0 )
	{
		i_clip_top = -i_y1;
		i_y1 = 0;
	}
	if( i_y2 > REND_SCREEN_HEIGHT )
	{
		i_y2 = REND_SCREEN_HEIGHT;
	}

	ps_ds = &ps_eng->drawsprites[ ps_eng->i_num_drawsprites++ ];
	ps_ds->i_x1 = i_x1;
	ps_ds->i_x2 = i_x2;
	ps_ds->i_y1 = i_y1;
	ps_ds->i_y2 = i_y2;
	ps_ds->i_sprite_idx = i_sprite_idx;
	ps_ds->i_dist = v_origin[ 1 ]; /* isqrt( mul_16_16( v_origin[ 0 ], v_origin[ 0 ] ) + mul_16_16( v_origin[ 1 ], v_origin[ 1 ] ) ); */

	i_scale = mul_32_u16( ( Int32 )v_origin[ 1 ] << ( 12 + SPRITE_SCALE_SHIFT ), INV_HALF_SCREEN_WIDTH );
	if( i_clip_left )
	{
		ps_ds->i_u = mul_u16_32_ns( i_clip_left, i_scale );
	}
	else
	{
		ps_ds->i_u = 0;
	}
	ps_ds->i_scaleu = i_scale;

	i_scale = div_32_u16( ( Int32 )v_origin[ 1 ] << ( 12 + SPRITE_SCALE_SHIFT ), (((REND_SCREEN_HEIGHT/2)*REND_SCREEN_WIDTH)/REND_SCREEN_HEIGHT) );
	if( i_clip_top )
	{
		ps_ds->i_v = mul_u16_32_ns( i_clip_top, i_scale );
	}
	else
	{
		ps_ds->i_v = 0;
	}
	ps_ds->i_scalev = i_scale;

	ps_ds->i_clip_idx = ps_eng->i_num_sprite_clip;
	i_nspriteclip = ps_ds->i_x2 - ps_ds->i_x1;
	if( ps_eng->i_num_sprite_clip + i_nspriteclip > MAX_NUM_SPRITECLIP )
	{
		ps_eng->i_num_drawsprites--;
		return; /* cannot do, skip sprite :( */
	}

	memcpy( &ps_eng->pui8_sprite_clip[ ps_eng->i_num_sprite_clip ][ 0 ], &ps_eng->rgui8_yclip[ ps_ds->i_x1 ][ 0 ], i_nspriteclip * 2 );
	ps_eng->i_num_sprite_clip += i_nspriteclip;
}

#if WIN32
typedef void ( *draw_spritespan_f ) ( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
#else
typedef __attribute__((__stkparm__)) void ( *draw_spritespan_f ) ( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
#endif
draw_spritespan_f spritespan_draw_funcs[ 4 ] = {
	eng_draw_spritespan0, eng_draw_spritespan1, eng_draw_spritespan2, eng_draw_spritespan3
};


void draw_sprites_finish( engine_t *ps_eng )
{
	UInt8 rgui8_drawsprite_idx[ MAX_NUM_DRAWSPRITES ];
	Int16 i_idx, i_x, i_xidx, i_y1, i_y2, i_clip_top, i_flip, i_sprite_idx;
	UInt32 i_u, i_v;
	UInt8 *pui8_drawbuffer, *pui8_tex, *pui8_sprite_data;
	drawsprite_t *ps_ds;

	for( i_idx = 0; i_idx < ps_eng->i_num_drawsprites; i_idx++ )
	{
		rgui8_drawsprite_idx[ i_idx ] = ( UInt8 )i_idx;
	}
	for( i_idx = 0; i_idx < ps_eng->i_num_drawsprites; i_idx++ )
	{
		for( i_xidx = i_idx + 1; i_xidx < ps_eng->i_num_drawsprites; i_xidx++ )
		{
			if( ps_eng->drawsprites[ rgui8_drawsprite_idx[ i_xidx ] ].i_dist < ps_eng->drawsprites[ rgui8_drawsprite_idx[ i_idx ] ].i_dist )
			{
				UInt8 ui8_tmp;
				ui8_tmp = rgui8_drawsprite_idx[ i_xidx ];
				rgui8_drawsprite_idx[ i_xidx ] = rgui8_drawsprite_idx[ i_idx ];
				rgui8_drawsprite_idx[ i_idx ] = ui8_tmp;
			}
		}
	}

	for( i_idx = ps_eng->i_num_drawsprites - 1; i_idx >= 0; i_idx-- )
	{
		repository_sprite_t *ps_sprite;

		ps_ds = &ps_eng->drawsprites[ rgui8_drawsprite_idx[ i_idx ] ];

		i_u = ps_ds->i_u;
		i_xidx = ps_ds->i_clip_idx;

		if( ps_ds->i_sprite_idx < 0 )
		{
			i_flip = 1;
			i_sprite_idx = -ps_ds->i_sprite_idx;
		}
		else
		{
			i_flip = 0;
			i_sprite_idx = ps_ds->i_sprite_idx;
		}

		pui8_sprite_data = sprite_cache_get_sprite( ps_eng, i_sprite_idx, &ps_sprite, 0 );

		for( i_x = ps_ds->i_x1; i_x < ps_ds->i_x2; i_x++ )
		{
			Int16 i_fpu;

			i_y1 = ps_ds->i_y1;
			i_y2 = ps_ds->i_y2;

			if( i_y1 < ps_eng->pui8_sprite_clip[ i_xidx ][ 0 ] )
			{
				i_clip_top = ps_eng->pui8_sprite_clip[ i_xidx ][ 0 ] - i_y1;
				i_y1 = ps_eng->pui8_sprite_clip[ i_xidx ][ 0 ];
			}
			else
			{
				i_clip_top = 0;
			}
			if( i_y2 > ps_eng->pui8_sprite_clip[ i_xidx ][ 1 ] )
			{
				i_y2 = ps_eng->pui8_sprite_clip[ i_xidx ][ 1 ];
			}

			if( i_y1 >= i_y2 )
			{
				goto skip_draw;
				continue;
			}

			pui8_drawbuffer = ( ( UInt8 * )ps_eng->p_drawbuffer ) + ( i_x >> 2 ) + ps_eng->rgui16_ylut[ i_y1 ];
			if( !i_flip )
			{
				i_fpu = i_u >> 16;
			}
			else
			{
				i_fpu = ps_sprite->ui8_width - ( i_u >> 16 ) - 1;
			}
			pui8_tex = pui8_sprite_data + ( mul_16_16( i_fpu, ps_sprite->ui8_height ) );
			i_v = ps_ds->i_v;
			if( i_clip_top )
			{
				i_v += mul_u16_32_ns( i_clip_top, ps_ds->i_scalev );
			}

			spritespan_draw_funcs[ i_x & 0x3 ]( pui8_drawbuffer, i_y2 - i_y1, pui8_tex, i_v, ps_ds->i_scalev );

			skip_draw:;
			i_u += ps_ds->i_scaleu;
			i_xidx++;
		}
	}
}


void draw_packing1_sprite( engine_t *ps_eng, UInt8 *pui8_sprite_data, Int16 i_width, Int16 i_height, Int16 i_x_offs, Int16 i_y_offs )
{
	Int16 i_x, i_y, i_x_frac, i_bits, i_x_frac_line, i_count, i_transparent;
	UInt16 ui16_cr;
	UInt8 ui8_pel;
	const UInt8 rgui8_tabA[ 4 ] = {0x77, 0xbb, 0xdd, 0xee};
	const UInt8 rgui8_tabP[][ 4 ] = {{ 0x0, 0x8, 0x80, 0x88 },{ 0x0, 0x4, 0x40, 0x44 },{ 0x0, 0x2, 0x20, 0x22 },{ 0x0, 0x1, 0x10, 0x11 }};

	UInt8 *pui8_drawbuffer, *pui8_drawbuffer_line;

	i_x_frac_line = i_x_offs;
	pui8_drawbuffer_line = ( ( UInt8 * )ps_eng->p_drawbuffer ) + ps_eng->rgui16_ylut[ i_y_offs ];

	i_x = 0;
	i_transparent = 0;
	i_count = 0;
	i_bits = 16;
	ui16_cr = *( pui8_sprite_data++ ) << 8;
	ui16_cr |= *( pui8_sprite_data++ );

	for( i_y = 0; i_y < i_height; i_y++ )
	{
		i_x_frac = i_x_frac_line;
		i_x = i_width;
		i_transparent = 1;

		while( i_x )
		{
			i_count = ( ui16_cr >> 8 );
			ui16_cr <<= 8;
			ui16_cr |= ( *( pui8_sprite_data++ ) << ( 16 - i_bits ) );
			i_x -= i_count;

			if( !i_transparent )
			{
				Int16 i_running_xfrac;
				pui8_drawbuffer = pui8_drawbuffer_line + ( i_x_frac >> 2 );
				i_running_xfrac = i_x_frac & 3;
				i_x_frac += i_count;
				span4pel_fastpath:;
				while( i_count )
				{
					UInt8 ui8_db;
					if( i_count >= 4 && i_running_xfrac == 0 )
					{
						do
						{
							*( pui8_drawbuffer++) = ps_eng->rgui8_bitmap_translation_tab[ ui16_cr >> 8 ];
							ui16_cr <<= 8;
							ui16_cr |= ( *( pui8_sprite_data++ ) << ( 16 - i_bits ) );
							i_count -= 4;
						} while( i_count >= 4 );
						goto span4pel_fastpath;
					}

					ui8_db = *pui8_drawbuffer;
					ui8_db = ui8_db & rgui8_tabA[ i_running_xfrac ];
					ui8_pel = ( ( ( Int32 ) ui16_cr ) << 2 ) >> 16;
					ui8_pel = rgui8_tabP[ i_running_xfrac ][ ui8_pel ];
					ui8_db |= ui8_pel;
					*pui8_drawbuffer = ui8_db;

					ui16_cr <<= 2;
					i_bits -= 2;
					if( i_bits <= 8 )
					{
						ui16_cr |= ( *( pui8_sprite_data++ ) << ( 8 - i_bits ) );
						i_bits += 8;
					}
					i_count--;
					i_running_xfrac++;
					if( i_running_xfrac & 0x4 )
					{
						pui8_drawbuffer++;
						i_running_xfrac = 0;
					}
				}
			}
			else
			{
				i_x_frac += i_count;
			}
			i_transparent = !i_transparent;
		}
		pui8_drawbuffer_line += DRAWBUFFER_SCREEN_WIDTH >> 2;
	}
}



void draw_viewmodel( engine_t *ps_eng )
{
	repository_sprite_t *ps_sprite;
	UInt8 *pui8_sprite_data;

	pui8_sprite_data = sprite_cache_get_sprite( ps_eng, ps_eng->ui8_viewmodel, &ps_sprite, 1 );

	draw_packing1_sprite( ps_eng, pui8_sprite_data, ps_sprite->ui8_width, ps_sprite->ui8_height,
		( ( REND_SCREEN_WIDTH - ps_sprite->ui8_width ) >> 1 ),
		REND_SCREEN_HEIGHT - ps_sprite->ui8_height );
}

void draw_splash( engine_t *ps_eng )
{
	repository_sprite_t *ps_sprite;
	UInt8 *pui8_sprite_data;

	pui8_sprite_data = sprite_cache_get_sprite( ps_eng, SPRITE_SCREEN_SPLASH, &ps_sprite, 1 );

	draw_packing1_sprite( ps_eng, pui8_sprite_data, ps_sprite->ui8_width, ps_sprite->ui8_height,
		( ( REND_SCREEN_WIDTH - ps_sprite->ui8_width ) >> 1 ),
		REND_SCREEN_HEIGHT - ps_sprite->ui8_height );
}

void draw_end( engine_t *ps_eng )
{
	repository_sprite_t *ps_sprite;
	UInt8 *pui8_sprite_data;

	pui8_sprite_data = sprite_cache_get_sprite( ps_eng, SPRITE_SCREEN_END, &ps_sprite, 1 );

	draw_packing1_sprite( ps_eng, pui8_sprite_data, ps_sprite->ui8_width, ps_sprite->ui8_height,
		( ( REND_SCREEN_WIDTH - ps_sprite->ui8_width ) >> 1 ),
		REND_SCREEN_HEIGHT - ps_sprite->ui8_height );
}


void draw_decorations( engine_t *ps_eng )
{
	repository_sprite_t *ps_sprite;
	UInt8 *pui8_sprite_data;

	pui8_sprite_data = sprite_cache_get_sprite( ps_eng, SPRITE_SCREEN_DECORATION, &ps_sprite, 1 );

	draw_packing1_sprite( ps_eng, pui8_sprite_data, ps_sprite->ui8_width, ps_sprite->ui8_height, 0,	0 );
}

void draw_statusbar( engine_t *ps_eng )
{
	UInt8 rgui8_buf[ 16 ];
	repository_sprite_t *ps_sprite;
	UInt8 *pui8_sprite_data;

	pui8_sprite_data = sprite_cache_get_sprite( ps_eng, SPRITE_SCREEN_STATUSBAR, &ps_sprite, 1 );

	draw_packing1_sprite( ps_eng, pui8_sprite_data, ps_sprite->ui8_width, ps_sprite->ui8_height, 0,	0 );

	if( ps_eng->s_player.ui8_weapon == 1 )
	{
		rgui8_buf[ 0 ] = ' ';
		rgui8_buf[ 1 ] = 'Z' + 2;
		rgui8_buf[ 2 ] = 0;
	}
	else
	{
		sprintf( &rgui8_buf[ 0 ], "%3d", ps_eng->s_player.ui16_ammo_rifle );
	}
	draw_string( ps_eng, 1, 7, 8, -1, &rgui8_buf[ 0 ] );

	if( ps_eng->ps_player )
	{
		sprintf( &rgui8_buf[ 0 ], "%3d", ps_eng->ps_player->i_health );
		draw_string( ps_eng, 1, 34, 8, -1, &rgui8_buf[ 0 ] );
	}
}



