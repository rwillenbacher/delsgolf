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

void generate_texture_mapping_table( engine_t *ps_eng )
{
	Int16 i_idx;

	i_idx = 0;
	while( i_idx < ps_eng->s_map.i_num_textures )
	{
		ps_eng->rgui8_texture_mapping_table[ i_idx ] = repo_get_texture_idx( ps_eng, &ps_eng->s_map.rgpui8_textures[ i_idx ][ 0 ] );
		i_idx++;
	}
	ps_eng->ps_texture_cache->i_num_cached_textures = 0;
	ps_eng->ps_texture_cache->i_lru_counter = 0;
	for( i_idx = 0; i_idx < TEXTURE_CACHE_SIZE; i_idx++ )
	{
		ps_eng->ps_texture_cache->rgs_textures[ i_idx ].i_offset = ( ( i_idx / 8 ) * 256 * 32 ) + ( ( i_idx & 7 ) * 32 );
	}
}

UInt8 *texture_get( engine_t *ps_eng, Int16 i_map_idx, Int16 i_for_vertical )
{
	Int16 i_idx, i_lowidx;
	Int16 i_low, i_internal_map_idx;
	cached_texture_t *ps_ctex;


	i_low = 0;
	i_lowidx = 0;
	i_internal_map_idx = i_map_idx | ( i_for_vertical << 8 );
	for( i_idx = 0; i_idx < ps_eng->ps_texture_cache->i_num_cached_textures; i_idx++ )
	{
		Int16 i_tlow;
		ps_ctex = &ps_eng->ps_texture_cache->rgs_textures[ i_idx ];
		if( ps_ctex->i_idx == i_internal_map_idx )
		{
			ps_ctex->i_lru = ps_eng->ps_texture_cache->i_lru_counter++;
			return &ps_eng->ps_texture_cache->rgui8_texture_data[ ps_ctex->i_offset ];
		}
		i_tlow = ps_eng->ps_texture_cache->i_lru_counter - ps_eng->ps_texture_cache->rgs_textures[ i_idx ].i_lru;
		if( i_tlow > i_low )
		{
			i_low = i_tlow;
			i_lowidx = i_idx;
		}
	}

	if( i_idx >= TEXTURE_CACHE_SIZE )
	{

	}
	else
	{
		i_lowidx = ps_eng->ps_texture_cache->i_num_cached_textures++;
	}

	ps_ctex = &ps_eng->ps_texture_cache->rgs_textures[ i_lowidx ];
	ps_ctex->i_idx = i_internal_map_idx;
	ps_ctex->i_lru = ps_eng->ps_texture_cache->i_lru_counter++;
	repo_get_texture( ps_eng, ps_eng->rgui8_texture_mapping_table[ i_map_idx ], &ps_eng->ps_texture_cache->rgui8_texture_data[ ps_ctex->i_offset ], i_for_vertical );

	return &ps_eng->ps_texture_cache->rgui8_texture_data[ ps_ctex->i_offset ];
}
