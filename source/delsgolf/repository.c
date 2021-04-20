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

extern const UInt8 rgui8_pel_tab[ 2 ][ 4 ];

Int16 init_engine_and_data_repository( engine_t **pps_eng )
{
	Int32 i_alloc_size, i_repo_alloc_size;
	UInt16 i_ret;
	repository_t *ps_repo;
	engine_t *ps_eng;

	*pps_eng = 0;
	
	i_alloc_size = sizeof( engine_t );
	i_alloc_size += sizeof( UInt8 ) * MAX_NUM_SPRITECLIP * 2;
	i_alloc_size += sizeof( entity_t ) * MAX_NUM_ENTITIES;
	ps_eng = ( engine_t *)malloc( i_alloc_size );
	if( !ps_eng )
	{
		return 0;
	}
	memset( ps_eng, 0, sizeof( engine_t ) );
	ps_eng->pui8_sprite_clip = ( UInt8 (*)[2] )( ps_eng + 1 );
	ps_eng->rgs_entities = ( entity_t * )( ps_eng->pui8_sprite_clip + MAX_NUM_SPRITECLIP );

	*pps_eng = ps_eng;

	i_alloc_size = MEMORY_ALLOC_SIZE;
	ps_eng->p_memory = malloc( i_alloc_size );

	if( !ps_eng->p_memory )
	{
		return 0;
	}

	ps_eng->ps_texture_cache = ( texture_cache_t * )( ps_eng->p_memory );
	ps_eng->ps_sprite_cache = ( sprite_cache_t * )( ps_eng->ps_texture_cache + 1 );
	ps_eng->pui8_scratch_spr = ( UInt8 * )( ps_eng->ps_sprite_cache + 1 );
	ps_eng->p_drawbuffer = ps_eng->pui8_scratch_spr + SCRATCH_SPR_SIZE;

	i_alloc_size -= ( ( ( UInt8 * )ps_eng->p_drawbuffer ) + DRAWBUFFER_SIZE ) - ( ( UInt8 * ) ps_eng->p_memory );

	ps_repo = &ps_eng->s_repository;

#ifdef WIN32
	ps_repo->f_file = fopen( "eng_le.dat", "rb" );
	if( !ps_repo->f_file )
	{
		return 0;
	}
	ps_repo->b_file_up = TRUE;
	i_ret = fread( &ps_repo->s_header, sizeof( repository_header_t ), 1, ps_repo->f_file );
	if( i_ret <= 0 )
	{
		return 0;
	}
#else
	i_ret = FOpen( "delsrepo", &ps_repo->s_file, FM_READ, "data" );
	if( i_ret != FS_OK )
	{
		return 0;
	}
	ps_repo->b_file_up = TRUE;
	i_ret = FRead ( &ps_repo->s_header, sizeof( repository_header_t ), &ps_repo->s_file );
	if( i_ret != FS_OK )
	{
		return 0;
	}
#endif
	i_repo_alloc_size = sizeof( repository_texture_t ) * ps_repo->s_header.i_num_textures;
	i_repo_alloc_size += sizeof( repository_sprite_t ) * ps_repo->s_header.i_num_sprites;
	i_repo_alloc_size += sizeof( repository_map_t ) * ps_repo->s_header.i_num_maps;
	ps_repo->p_allocated = ( ( UInt8 *)ps_eng->p_drawbuffer ) + DRAWBUFFER_SIZE;

	ps_repo->rgs_textures = ( repository_texture_t * ) ps_repo->p_allocated;
	ps_repo->rgs_sprites = ( repository_sprite_t * ) ( ps_repo->rgs_textures + ps_repo->s_header.i_num_textures );
	ps_repo->rgs_maps = ( repository_map_t * ) ( ps_repo->rgs_sprites + ps_repo->s_header.i_num_sprites );
	i_alloc_size -= i_repo_alloc_size;

	ps_eng->ui16_size_for_map = ( UInt16 )i_alloc_size - 4;
	i_alloc_size = ( MEMORY_ALLOC_SIZE - ( i_alloc_size - 4 ) ) & ~0x3;
	ps_eng->p_memory_for_map = ( void *)( ( ( UInt8 * )( ps_eng->p_memory ) ) + i_alloc_size );
#ifdef WIN32
	fprintf( stderr, "SIZE REMAINING FOR MAP: %d bytes\n", ps_eng->ui16_size_for_map );

	fread ( &ps_eng->rgi16_cosine_table, sizeof( Int16 ) * 256, 1, ps_repo->f_file );
	fread ( &ps_eng->rgs_characters, 896, 1, ps_repo->f_file );
	fread ( &ps_eng->rgui8_big_characters, 39 * 8, 1, ps_repo->f_file );
	fread ( ps_repo->rgs_textures, sizeof( repository_texture_t ) * ps_repo->s_header.i_num_textures, 1, ps_repo->f_file );
	fread ( ps_repo->rgs_sprites, sizeof( repository_sprite_t ) * ps_repo->s_header.i_num_sprites, 1, ps_repo->f_file );
	fread ( ps_repo->rgs_maps, sizeof( repository_map_t ) * ps_repo->s_header.i_num_maps, 1, ps_repo->f_file );
#else
	FRead ( ps_eng->rgi16_cosine_table, sizeof( Int16 ) * 256, &ps_repo->s_file );
	FRead ( ps_eng->rgs_characters, 896, &ps_repo->s_file );
	FRead ( ps_eng->rgui8_big_characters, 39 * 8, &ps_repo->s_file );
	FRead ( ps_repo->rgs_textures, sizeof( repository_texture_t ) * ps_repo->s_header.i_num_textures, &ps_repo->s_file );
	FRead ( ps_repo->rgs_sprites, sizeof( repository_sprite_t ) * ps_repo->s_header.i_num_sprites, &ps_repo->s_file );
	FRead ( ps_repo->rgs_maps, sizeof( repository_map_t ) * ps_repo->s_header.i_num_maps, &ps_repo->s_file );
#endif
	return 1;
}


Int16 repo_get_texture_idx( engine_t *ps_eng, UInt8 *pui8_name )
{
	Int16 i_idx;
	Int16 i_cidx;
	repository_t *ps_repo;

	ps_repo = &ps_eng->s_repository;

	for( i_idx = 0; i_idx < ( Int16 )ps_repo->s_header.i_num_textures; i_idx++ )
	{
		i_cidx = 0;
		while( 1 )
		{
			if( pui8_name[ i_cidx ] != ps_repo->rgs_textures[ i_idx ].rgui8_name[ i_cidx ] )
			{
				break;
			}
			if( pui8_name[ i_cidx ] == 0 || i_cidx == 7 )
			{
				return i_idx;
			}
			i_cidx++;
		}
	}
	return 0;
}


void repo_get_texture( engine_t *ps_eng, Int16 i_texture_idx, UInt8 *pui8_data, Int16 i_for_vertical )
{
	Int16 i_x, i_y;
	UInt16 ui_texel;
	repository_t *ps_repo;
	repository_texture_t *ps_texture;

	ps_repo = &ps_eng->s_repository;

	if( i_texture_idx >= ( Int16 )ps_repo->s_header.i_num_textures )
	{
		return;
	}
	ps_texture = &ps_repo->rgs_textures[ i_texture_idx ];

#ifdef WIN32
	fseek( ps_repo->f_file, ps_texture->i_file_offset, SEEK_SET );
#else
	FSetPos( &ps_repo->s_file, ps_texture->i_file_offset );
#endif

	for( i_y = 0; i_y < 32; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
	#ifdef WIN32
			ui_texel = fgetc( ps_repo->f_file );
			if( ui_texel > 255 )
			{
				break;
			}
	#else
			ui_texel = FGetC( &ps_repo->s_file );
			if( ui_texel == FS_EOF )
			{
				break;
			}
	#endif
			if( i_for_vertical )
			{
				*(pui8_data++) = rgui8_pel_tab[ 1 ][ ( ui_texel >> 6 ) & 0x3 ];
				*(pui8_data++) = rgui8_pel_tab[ 1 ][ ( ui_texel >> 4 ) & 0x3 ];
				*(pui8_data++) = rgui8_pel_tab[ 1 ][ ( ui_texel >> 2 ) & 0x3 ];
				*(pui8_data++) = rgui8_pel_tab[ 1 ][ ui_texel & 0x3 ];
			}
			else
			{
				*(pui8_data++) = rgui8_pel_tab[ 0 ][ ( ui_texel >> 6 ) & 0x3 ];
				*(pui8_data++) = rgui8_pel_tab[ 0 ][ ( ui_texel >> 4 ) & 0x3 ];
				*(pui8_data++) = rgui8_pel_tab[ 0 ][ ( ui_texel >> 2 ) & 0x3 ];
				*(pui8_data++) = rgui8_pel_tab[ 0 ][ ui_texel & 0x3 ];
			}
		}
		pui8_data += 256 - 32;
	}
}


void repo_get_sprite( engine_t *ps_eng, Int16 i_sprite_idx, repository_sprite_t **pps_sprite, Int16 i_raw, UInt8 *pui8_data )
{
	Int16 i_idx, i_read_bytes;
	UInt16 ui_code = 0;
	Int16 i_bits, i_transparent;
	repository_t *ps_repo;
	repository_sprite_t *ps_sprite;

	ps_repo = &ps_eng->s_repository;

	if( i_sprite_idx >= ( Int16 )ps_repo->s_header.i_num_sprites )
	{
		return;
	}
	ps_sprite = &ps_repo->rgs_sprites[ i_sprite_idx ];
	if( pps_sprite )
	{
		*pps_sprite = ps_sprite;
	}
	if( pui8_data == 0 )
	{
		return;
	}

#ifdef WIN32
	fseek( ps_repo->f_file, ps_sprite->i_file_offset, SEEK_SET );
#else
	FSetPos( &ps_repo->s_file, ps_sprite->i_file_offset );
#endif

	if( i_raw )
	{
#ifdef WIN32
		fread( pui8_data, ps_sprite->i_sprite_length, 1, ps_repo->f_file );
#else
		FRead ( pui8_data, ps_sprite->i_sprite_length, &ps_repo->s_file );
#endif
	}
	else
	{
		UInt8 *pui8_rp;
#ifdef WIN32
		assert( ps_sprite->i_sprite_length < SCRATCH_SPR_SIZE );
		fread( ps_eng->pui8_scratch_spr, ps_sprite->i_sprite_length, 1, ps_repo->f_file );
#else
		FRead ( ps_eng->pui8_scratch_spr, ps_sprite->i_sprite_length, &ps_repo->s_file );
#endif
		pui8_rp = ps_eng->pui8_scratch_spr;

		i_bits = 0;
		i_read_bytes = 0;
		for( i_idx = 0; i_idx < ps_sprite->ui8_width * ps_sprite->ui8_height; i_idx++ )
		{
			if( i_bits < 1 )
			{
				i_read_bytes++;
				ui_code |= *( pui8_rp++ ) << ( 8 - i_bits );
				i_bits += 8;
			}
			i_transparent = ui_code & 0x8000;
			if( i_transparent )
			{
				*(pui8_data++) = 0x4;
				ui_code <<= 1;
				i_bits--;
			}
			else
			{
				if( i_bits < 3 )
				{
					i_read_bytes++;
					ui_code |= *( pui8_rp++ ) << ( 8 - i_bits );
					i_bits += 8;
				}
				*(pui8_data++) = rgui8_pel_tab[ 0 ][ ui_code >> 13 ];
				ui_code <<= 3;
				i_bits-=3;
			}
		}
	}
}


void repo_get_map( engine_t *ps_eng, Int16 i_map_idx, void *p_data )
{
	UInt8 *pui8_out;
	UInt16 i;
	repository_t *ps_repo;
	repository_map_t *ps_map;

	ps_repo = &ps_eng->s_repository;

	if( i_map_idx >= ( Int16 )ps_repo->s_header.i_num_maps )
	{
		return;
	}
	ps_map = &ps_repo->rgs_maps[ i_map_idx ];

#ifdef WIN32
	fseek( ps_repo->f_file, ps_map->i_file_offset, SEEK_SET );
#else
	FSetPos( &ps_repo->s_file, ps_map->i_file_offset );
#endif

	pui8_out = ( UInt8 * )p_data;

#if 0
	for(i = 0; i < ps_map->i_map_length; i += i_skipped)
	{
		UInt16 ui16_in;
		i_skipped = 0;

#ifdef WIN32
		ui16_in = fgetc( ps_repo->f_file );
#else
		ui16_in = FGetC( &ps_repo->s_file );
#endif

		if( ui16_in == ( MAGIC & 0xff ) )
		{
			int i_extra = 0;
#ifdef WIN32
			i_pos = fgetc( ps_repo->f_file ) << 8;
			i_pos |= fgetc( ps_repo->f_file );
#else
			i_pos = FGetC( &ps_repo->s_file ) << 8;
			i_pos |= FGetC( &ps_repo->s_file );
#endif
			if( i_pos )
			{
				i_skipped = 4 + ( i_pos & 0xf );
				if( ( i_pos & 0xf ) == 15 )
				{
#ifdef WIN32
					i_skipped += fgetc( ps_repo->f_file );
#else
					i_skipped += FGetC( &ps_repo->s_file );
#endif
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
				pui8_out[ i_outbytes + 0 ] = MAGIC & 0xff;
				i_outbytes += 1;
			}
			i_skipped = 3 + i_extra;
		}
		if( !i_skipped )
		{
			i_skipped = 1;
			pui8_out[ i_outbytes++ ] = ( UInt8 )ui16_in;
		}
	}
#else
	{
	Int16 i_outbytes = 0;
	Int16 cflags, cflagsbit;
  
	cflagsbit = 0;
	cflags = 0;
	for(i = 0; i < ps_map->i_map_length; (void)i )
	{
		UInt16 ui16_in;

#ifdef WIN32
		ui16_in = fgetc( ps_repo->f_file );
#else
		ui16_in = FGetC( &ps_repo->s_file );
#endif
		i++;

		if( cflagsbit == 0 )
		{
			cflags = ui16_in;
#ifdef WIN32
			ui16_in = fgetc( ps_repo->f_file );
#else
			ui16_in = FGetC( &ps_repo->s_file );
#endif
			i++;
			cflagsbit = 8;
		}
		if( cflags & ( 1 << ( --cflagsbit ) ) )
		{
			int i_pos, i_len;

			i_len = ui16_in >> 4;
			i_pos = ( ui16_in & 0xf ) << 8;
#ifdef WIN32
			ui16_in = fgetc( ps_repo->f_file );
#else
			ui16_in = FGetC( &ps_repo->s_file );
#endif
			i++;
			i_pos |= ui16_in; 
			i_pos++;

			i_len += 2;

			while( i_len-- )
			{
				pui8_out[ i_outbytes ] = pui8_out[ i_outbytes - i_pos ];
				i_outbytes++;
			}
		}
		else
		{
			pui8_out[ i_outbytes++ ] = ui16_in;
		}
	}
	/*return i_outbytes*/;
	}
#endif
}


void deinit_data_repository( engine_t *ps_eng )
{
	if( ps_eng )
	{
		if( ps_eng->s_repository.b_file_up )
		{
#ifdef WIN32
			fclose( ps_eng->s_repository.f_file );
#else
			FClose( &ps_eng->s_repository.s_file );
#endif
		}
		if( ps_eng->p_memory )
		{
			free( ps_eng->p_memory );
		}
		free( ps_eng );
	}
}


