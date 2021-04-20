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
#include <string.h>
#include <memory.h>

#include "datastore.h"
#include "textures.h"
#include "sinetab.h"
#include "font4x6.h"
#include "lzo.h"
#include "lz77b.h"

int g_little_endian = 0;

texture_t rgs_textures[ ] = {
	{ "unknown", rgui8_unknown },
	{ "wall1", rgui8_wall1 },
	{ "wall2", rgui8_wall2 },
	{ "wall3", rgui8_wall3 },
	{ "wall4", rgui8_wall4 },
	{ "wall5", rgui8_wall5 },
	{ "wall6", rgui8_wall6 },
	{ "wall7", rgui8_wall7 },
	{ "wall8", rgui8_wall8 },
	/*{ "floor1", rgui8_floor1 },
	{ "floor2", rgui8_floor2 }, */
	{ "floor3", rgui8_floor3 },
	/*{ "ceil1", rgui8_ceil1 },
	{ "ceil2", rgui8_ceil2 }, */
	{ "ceil3", rgui8_ceil3 },
	{ "ele1", rgui8_ele1 },
	{ "comp1", rgui8_comp1 },
	{ "wires1", rgui8_wires1 },
	{ "crate", rgui8_crate },
	{ 0, 0 }
};

sprite_t rgs_sprites[ ] = {
	{ "SPRITE_PISTOL_IDLE", 1, 25, 28, rgui8_pistid },
	{ "SPRITE_PISTOL_FIRE0", 1, 27, 35, rgui8_pistf0 },
	{ "SPRITE_PISTOL_FIRE1", 1, 27, 41, rgui8_pistf1 },

	{ "SPRITE_RIFLE_IDLE", 1, 43, 32, rgui8_rifleid },
	{ "SPRITE_RIFLE_FIRE0", 1, 23, 32, rgui8_riflef0 },
	{ "SPRITE_RIFLE_FIRE1", 1, 23, 32, rgui8_riflef1 },

	{ "SPRITE_SCREEN_DECORATION", 1, 160, 100, rgui8_window },
	{ "SPRITE_SCREEN_STATUSBAR", 1, 64, 18, rgui8_statusb },
	{ "SPRITE_SCREEN_SPLASH", 1, 144, 72, rgui8_splash },
	{ "SPRITE_SCREEN_END", 1, 144, 72, rgui8_end },
	

	//{ "SPRITE_IMP", 0, 42, 64, rgui8_imp },
	{ "SPRITE_RIFLE", 0, 52, 20, rgui8_riflesprt },
	{ "SPRITE_RFLAMMO", 0, 24, 24, rgui8_rflammo },
	{ "SPRITE_MDPCK", 0, 24, 24, rgui8_mdpck },
	{ "SPRITE_LVLTRIG", 0, 10, 32, rgui8_lvltrig },
	{ "SPRITE_SHIP", 0, 128, 56, rgui8_ship },
	{ "SPRITE_OOO", 0, 32, 32, rgui8_ooo },

	{ "SPRITE_SG_WALK0_A0", 0, 18, 28, rgui8_sgw0a0 },
	{ "SPRITE_SG_WALK0_A1", 0, 16, 27, rgui8_sgw0a1 },
	{ "SPRITE_SG_WALK0_A2", 0, 20, 27, rgui8_sgw0a2 },
	{ "SPRITE_SG_WALK0_A3", 0, 23, 25, rgui8_sgw0a3 },
	{ "SPRITE_SG_WALK0_A4", 0, 18, 26, rgui8_sgw0a4 },

	{ "SPRITE_SG_WALK1_A0", 0, 18, 28, rgui8_sgw1a0 },
	{ "SPRITE_SG_WALK1_A1", 0, 14, 27, rgui8_sgw1a1 },
	{ "SPRITE_SG_WALK1_A2", 0, 16, 28, rgui8_sgw1a2 },
	{ "SPRITE_SG_WALK1_A3", 0, 20, 27, rgui8_sgw1a3 },
	{ "SPRITE_SG_WALK1_A4", 0, 17, 27, rgui8_sgw1a4 },

	{ "SPRITE_SG_WALK2_A0", 0, 16, 28, rgui8_sgw2a0 },
	{ "SPRITE_SG_WALK2_A1", 0, 15, 27, rgui8_sgw2a1 },
	{ "SPRITE_SG_WALK2_A2", 0, 19, 28, rgui8_sgw2a2 },
	{ "SPRITE_SG_WALK2_A3", 0, 20, 28, rgui8_sgw2a3 },
	{ "SPRITE_SG_WALK2_A4", 0, 17, 28, rgui8_sgw2a4 },

	{ "SPRITE_SG_WALK3_A0", 0, 17, 28, rgui8_sgw3a0 },
	{ "SPRITE_SG_WALK3_A1", 0, 14, 28, rgui8_sgw3a1 },
	{ "SPRITE_SG_WALK3_A2", 0, 15, 28, rgui8_sgw3a2 },
	{ "SPRITE_SG_WALK3_A3", 0, 19, 28, rgui8_sgw3a3 },
	{ "SPRITE_SG_WALK3_A4", 0, 18, 28, rgui8_sgw3a4 },

	{ "SPRITE_SG_FIRE0", 0, 13, 28, rgui8_sgf0 },
	{ "SPRITE_SG_FIRE1", 0, 13, 27, rgui8_sgf1 },

	{ "SPRITE_SG_HIT_A0", 0, 15, 27, rgui8_sgh0a0 },
	{ "SPRITE_SG_HIT_A1", 0, 17, 27, rgui8_sgh0a1 },
	{ "SPRITE_SG_HIT_A2", 0, 22, 27, rgui8_sgh0a2 },
	{ "SPRITE_SG_HIT_A3", 0, 22, 26, rgui8_sgh0a3 },
	{ "SPRITE_SG_HIT_A4", 0, 15, 26, rgui8_sgh0a4 },

	{ "SPRITE_SG_DEATH0", 0, 17, 30, rgui8_sgd0 },
	{ "SPRITE_SG_DEATH1", 0, 18, 25, rgui8_sgd1 },
	{ "SPRITE_SG_DEATH2", 0, 21, 18, rgui8_sgd2 },
	{ "SPRITE_SG_DEATH3", 0, 24, 14, rgui8_sgd3 },
	{ "SPRITE_SG_DEATH4", 0, 26, 9, rgui8_sgd4 },


	{ 0, 0 }
};

map_t rgs_maps_be[ ] = {
	{ "map1", "MAP_01", "map1_be.dat" },
	{ "map2", "MAP_02", "map2_be.dat" },
	{ "map3", "MAP_03", "map3_be.dat" },
	{ "map4", "MAP_04", "map4_be.dat" },
	{ "map5", "MAP_05", "map5_be.dat" },
	{ 0, 0 }
};

map_t rgs_maps_le[ ] = {
	{ "map1", "MAP_01", "map1_le.dat" },
	{ "map2", "MAP_02", "map2_le.dat" },
	{ "map3", "MAP_03", "map3_le.dat" },
	{ "map4", "MAP_04", "map4_le.dat" },
	{ "map5", "MAP_05", "map5_le.dat" },
	{ 0, 0 }
};

unsigned int i_num_texture_data = 0;
unsigned char rgui8_texture_data[ 0x100000 ];

unsigned int i_num_ctextures = 0;
ctexture_t rgs_ctextures[ 256 ];

unsigned int i_num_csprites = 0;
csprite_t rgs_csprites[ 256 ];

unsigned int i_num_cmaps = 0;
cmap_t rgs_cmaps[ 16 ];

cbigchar_t rgs_bigchars[ 38 ];

FILE *f_sprite_defines;

void gen_bigfont()
{
	int i_idx;

	for( i_idx = 0; i_idx < 312; i_idx += 8 )
	{
		int i_x, i_y;

		for( i_y = 0; i_y < 8; i_y++ )
		{
			unsigned char ui8_charline = 0;
			for( i_x = 0; i_x < 8; i_x++ )
			{
				if( !( rgui8_bigfont[ i_x + i_idx + ( i_y * 312 ) ] & 0x4 ) )
				{
					ui8_charline |= 1 << ( 7 - i_x );
				}
			}
			rgs_bigchars[ i_idx / 8 ].rgui8_data[ i_y ] = ui8_charline;
		}
	}
}


void export_texture( texture_t *ps_tex )
{
	int i_idx;
	unsigned char ui8_texel;

	memset( rgs_ctextures[ i_num_ctextures ].rgui8_name, 0, sizeof( rgs_ctextures[ i_num_ctextures ].rgui8_name ) );

	i_idx = 0;
	while( ps_tex->pui8_name[ i_idx ] && i_idx < 8 )
	{
		rgs_ctextures[ i_num_ctextures ].rgui8_name[ i_idx ] = ps_tex->pui8_name[ i_idx ];
		i_idx++;
	}
	rgs_ctextures[ i_num_ctextures ].i_texture_offset = i_num_texture_data;
	for( i_idx = 0; i_idx < 32 * 32; i_idx += 4 )
	{
		ui8_texel = ( ps_tex->pui8_texture[ i_idx ]<<6 ) | ( ps_tex->pui8_texture[ i_idx + 1 ]<<4 ) | ( ps_tex->pui8_texture[ i_idx + 2 ]<<2 ) | ( ps_tex->pui8_texture[ i_idx + 3 ] );
		rgui8_texture_data[ i_num_texture_data++ ] = ui8_texel;
	}

	rgs_ctextures[ i_num_ctextures ].i_texture_length = i_num_texture_data - rgs_ctextures[ i_num_ctextures ].i_texture_offset;

	i_num_ctextures++;
}

void export_sprite0( sprite_t *ps_sprite )
{
	int i_idx, i_bits, i_read_bytes;
	unsigned int ui_texel;

	fprintf( f_sprite_defines, "#define %s %d\n", ps_sprite->pui8_name, i_num_csprites );

	rgs_csprites[ i_num_csprites ].i_width = ps_sprite->i_width;
	rgs_csprites[ i_num_csprites ].i_height = ps_sprite->i_height;
	rgs_csprites[ i_num_csprites ].i_sprite_offset = i_num_texture_data;

	ui_texel = 0;
	i_bits = 0;
	for( i_idx = 0; i_idx < ps_sprite->i_width * ps_sprite->i_height; i_idx++ )
	{
		ui_texel |= ( ps_sprite->pui8_sprite[ i_idx ] & 0x4 ) >> 2;
		if( ui_texel & 1 )
		{
			ui_texel <<= 1;
			i_bits += 1;
		}
		else
		{
			ui_texel <<= 2;
			ui_texel |= ( ps_sprite->pui8_sprite[ i_idx ] & 0x3 );
			ui_texel <<= 1;
			i_bits += 3;
		}
		if( i_bits >= 8 )
		{
			i_bits -= 8;
			rgui8_texture_data[ i_num_texture_data++ ] = ui_texel >> ( i_bits + 1 );
		}
	}
	if( i_bits )
	{
		rgui8_texture_data[ i_num_texture_data++ ] = ui_texel << ( 7 - i_bits );
	}

	rgs_csprites[ i_num_csprites ].i_sprite_length = i_num_texture_data - rgs_csprites[ i_num_csprites ].i_sprite_offset;

	i_read_bytes = 0;
	i_bits = 0;
	ui_texel = 0;
	for( i_idx = 0; i_idx < ps_sprite->i_width * ps_sprite->i_height; i_idx++ )
	{
		int i_transparent, i_pel;
		if( i_bits < 1 )
		{
			ui_texel |= rgui8_texture_data[rgs_csprites[ i_num_csprites ].i_sprite_offset + i_read_bytes ] << ( 8 - i_bits );
			i_read_bytes++;
			i_bits += 8;
		}
		i_transparent = ui_texel & 0x8000;
		if( i_transparent )
		{
			i_pel = 0x4;
			ui_texel <<= 1;
			i_bits--;
		}
		else
		{
			if( i_bits < 3 )
			{
				ui_texel |= rgui8_texture_data[rgs_csprites[ i_num_csprites ].i_sprite_offset + i_read_bytes ] << ( 8 - i_bits );
				i_read_bytes++;
				i_bits += 8;
			}
			i_pel = ( ui_texel >> 13 ) & 0x7;
			ui_texel <<= 3;
			i_bits-=3;
		}
		if( ps_sprite->pui8_sprite[ i_idx ] != i_pel && ( ( ps_sprite->pui8_sprite[ i_idx ] & 0x4 ) == 0 || ( i_pel & 0x4 ) == 0 ) )
		{
			printf("sprite export failed, %d %x != %x \n", i_idx, i_pel, ps_sprite->pui8_sprite[ i_idx ] );
		}
	}


	i_num_csprites++;
}



void export_sprite1( sprite_t *ps_sprite )
{
	int i_idx, i_idx2, i_bits, i_read_bytes, i_count_pos, i_count, i_cmode, i_y;
	unsigned int ui_texel;

	fprintf( f_sprite_defines, "#define %s %d\n", ps_sprite->pui8_name, i_num_csprites );

	rgs_csprites[ i_num_csprites ].i_width = ps_sprite->i_width;
	rgs_csprites[ i_num_csprites ].i_height = ps_sprite->i_height;
	rgs_csprites[ i_num_csprites ].i_sprite_offset = i_num_texture_data;

	ui_texel = 0;
	i_bits = 0;
	i_cmode = 1;
	for( i_y = 0; i_y < ps_sprite->i_height; i_y++ )
	{
		i_idx = 0;
		i_cmode = 1;
		while( i_idx < ps_sprite->i_width )
		{
			i_idx2 = i_idx;
			i_count = 0;
			while( i_idx2 < ps_sprite->i_width && i_count < 255 && !!i_cmode == !!(ps_sprite->pui8_sprite[ i_y * ps_sprite->i_width + i_idx2 ] & 0x4) )
			{
				i_idx2++;
				i_count++;
			}

			ui_texel <<= 8;
			ui_texel |= i_count;
			i_bits += 8;

			if( !i_cmode )
			{
				while( i_idx != i_idx2 )
				{
					ui_texel <<= 2;
					ui_texel |= ( ps_sprite->pui8_sprite[ i_y * ps_sprite->i_width + i_idx ] & 0x3 );
					i_bits += 2;
					while( i_bits >= 8 )
					{
						i_bits -= 8;
						rgui8_texture_data[ i_num_texture_data++ ] = ui_texel >> i_bits;
					}
					i_idx++;
				}
			}
			else
			{
				i_idx = i_idx2;
			}
			i_cmode = !i_cmode;
			while( i_bits >= 8 )
			{
				i_bits -= 8;
				rgui8_texture_data[ i_num_texture_data++ ] = ui_texel >> i_bits;
			}
		}
	}
	if( i_bits )
	{
		rgui8_texture_data[ i_num_texture_data++ ] = ui_texel << ( 8 - i_bits );
	}

	rgs_csprites[ i_num_csprites ].i_sprite_length = i_num_texture_data - rgs_csprites[ i_num_csprites ].i_sprite_offset;

	i_num_csprites++;
}


void export_map( map_t *ps_map )
{
	int i_idx, i_len, i_lend;
	FILE *f;
	unsigned char rgui8_map[ 21000 ], rgui8_mapdec[ 21000 ];

	memset( rgs_cmaps[ i_num_cmaps ].rgui8_name, 0, sizeof( rgs_cmaps[ i_num_cmaps ].rgui8_name ) );

	i_idx = 0;
	while( ps_map->pui8_name[ i_idx ] && i_idx < 8 )
	{
		rgs_cmaps[ i_num_cmaps ].rgui8_name[ i_idx ] = ps_map->pui8_name[ i_idx ];
		i_idx++;
	}

	f = fopen( ps_map->pui8_fname, "rb");

	if( !f )
	{
		printf("error opening map %s %s, skipping\n", ps_map->pui8_name, ps_map->pui8_fname );
		return;
	}

	fseek( f, 0, SEEK_END );
	i_idx = ftell( f );
	fseek( f, 0, SEEK_SET );
	fread( &rgui8_map[ 0 ], i_idx, 1, f );
	fclose( f );

/*
	i_len = datastore_lzo_enc( &rgui8_map[ 0 ], i_idx, &rgui8_texture_data[ i_num_texture_data ] );
	fprintf( stderr, "compressed map %s from %d to %d\n", ps_map->pui8_fname, i_idx, i_len );
	i_lend = datastore_lzo_dec( &rgui8_texture_data[ i_num_texture_data ], i_len, &rgui8_mapdec[ 0 ], i_idx );

	if( memcmp( rgui8_map, rgui8_mapdec, i_idx ) != 0 )
	{
		fprintf( stderr, "decompression failure!\n");
		exit( 1 );
	}
*/

	i_len = datastore_lz77_enc( &rgui8_map[ 0 ], i_idx, &rgui8_texture_data[ i_num_texture_data ] );
	fprintf( stderr, "compressed map %s from %d to %d\n", ps_map->pui8_fname, i_idx, i_len );
	i_lend = datastore_lz77_dec( &rgui8_texture_data[ i_num_texture_data ], i_len, &rgui8_mapdec[ 0 ] );
	fprintf( stderr, "decomp %d %d\n", i_len, i_lend );
	if( memcmp( rgui8_map, rgui8_mapdec, i_idx ) != 0 )
	{
		fprintf( stderr, "decompression failure!\n");
		exit( 1 );
	}
	if( i_idx != i_lend )
	{
		fprintf( stderr, "decomp length failure\n");
		exit( 1 );
	}


	rgs_cmaps[ i_num_cmaps ].i_map_offset = i_num_texture_data;
	rgs_cmaps[ i_num_cmaps ].i_map_length = i_len;

	fprintf( f_sprite_defines, "#define %s %d\n", ps_map->pui8_defname, i_num_cmaps );


	i_num_texture_data += i_len;
	i_num_cmaps++;
}


FILE *f_outf;

void write_int( int i_int )
{
	unsigned char i_c;

	if( g_little_endian )
	{
		i_c = i_int & 0xff;
		fwrite( &i_c, 1, 1, f_outf );

		i_c = ( i_int >> 8 ) & 0xff;
		fwrite( &i_c, 1, 1, f_outf );
	}
	else
	{
		i_c = ( i_int >> 8 ) & 0xff;
		fwrite( &i_c, 1, 1, f_outf );

		i_c = i_int & 0xff;
		fwrite( &i_c, 1, 1, f_outf );
	}
}

void write_str( unsigned char *str, int i_length )
{
	fwrite( str, 1, i_length, f_outf );
}



void main( int argc, char *argv[] )
{
	int i_idx, i_endian;
	unsigned int i_texture_data_offset;
	unsigned char rgui8_fname[ 0x200 ];

	if( argc < 2 )
	{
		fprintf( stderr, "usage: %s <destination>\n", argv[ 0 ] );
		exit( 1 );
	}
/*
	if( strcmp( argv[ 1 ], "lzo" ) == 0 )
	{
		int i_len, i_clen, i_dlen;
		unsigned char rgui8_tmp[ 0x10000 ], rgui8_tmp2[ 0x10000 ], rgui8_tmp3[ 0x10000 ];
		FILE *f;
		i_idx = 0;
		while( rgs_maps_be[ i_idx ].pui8_name )
		{
			f = fopen( rgs_maps_le[ i_idx ].pui8_fname, "rb" );
			fseek( f, 0, SEEK_END );
			i_len = ftell( f );
			fseek( f, 0, SEEK_SET );
			fread( rgui8_tmp, i_len, 1, f );
			fclose( f );

			i_clen = datastore_lzo_enc( rgui8_tmp, i_len, rgui8_tmp2 );
			i_dlen = datastore_lzo_dec( rgui8_tmp2, i_clen, rgui8_tmp3, 0x10000 );

			fprintf( stderr, "%s: %d -> %d -> %d\n", rgs_maps_be[ i_idx ].pui8_fname, i_len, i_clen, i_dlen );

			i_idx++;
		}
		exit( 0 );
	}
*/
	for( i_endian = 0; i_endian < 2; i_endian++ )
	{
		i_num_cmaps = 0;
		i_num_texture_data = 0;
		i_num_ctextures = 0;
		i_num_csprites = 0;

		f_sprite_defines = fopen( "spritedef.h", "wt");

		i_idx = 0;
		while( rgs_textures[ i_idx ].pui8_name )
		{
			export_texture( &rgs_textures[ i_idx ] );
			i_idx++;
		}

		i_idx = 0;
		while( rgs_sprites[ i_idx ].pui8_name )
		{
			if( rgs_sprites[ i_idx ].i_coding == 0 )
			{
				export_sprite0( &rgs_sprites[ i_idx ] );
			}
			else
			{
				export_sprite1( &rgs_sprites[ i_idx ] );
			}
			i_idx++;
		}



		if( i_endian == 0 )
		{
			i_idx = 0;
			while( rgs_maps_be[ i_idx ].pui8_name )
			{
				export_map( &rgs_maps_be[ i_idx ] );
				i_idx++;
			}
		}
		else
		{
			i_idx = 0;
			while( rgs_maps_le[ i_idx ].pui8_name )
			{
				export_map( &rgs_maps_le[ i_idx ] );
				i_idx++;
			}
		}
		printf( "%d textures\n%d sprites\n%d maps\n%d texture data\n", i_num_ctextures, i_num_csprites, i_num_cmaps, i_num_texture_data );

		fclose( f_sprite_defines );


		if( i_endian == 0 )
		{
			sprintf( rgui8_fname, "%s_be.dat", argv[ 1 ]);
			g_little_endian = 0;
		}
		else
		{
			sprintf( rgui8_fname, "%s_le.dat", argv[ 1 ]);
			g_little_endian = 1;
		}
		f_outf = fopen( rgui8_fname, "wb" );

		/* data */
		write_int( i_num_ctextures );
		write_int( i_num_csprites );
		write_int( i_num_cmaps );
		write_int( i_num_texture_data );

		i_texture_data_offset = 8 + i_num_ctextures * 12 + i_num_csprites * 6 + i_num_cmaps * 4 + (256 * 2 ) + 896 + ( 39 * 8 );

		for( i_idx = 0; i_idx < 256; i_idx++ )
		{
			write_int( rgi16_sinetab[ i_idx ] );
		}

		write_str( rgui8_font4x6, 896 );

		gen_bigfont();
		write_str( &rgs_bigchars, 39 * 8 );

		for( i_idx = 0; i_idx < i_num_ctextures; i_idx++ )
		{
			write_str( &rgs_ctextures[ i_idx ].rgui8_name[ 0 ], 8 );
			write_int( rgs_ctextures[ i_idx ].i_texture_offset + i_texture_data_offset );
			write_int( rgs_ctextures[ i_idx ].i_texture_length );
		}
		for( i_idx = 0; i_idx < i_num_csprites; i_idx++ )
		{
			write_str( &rgs_csprites[ i_idx ].i_width, 1 );
			write_str( &rgs_csprites[ i_idx ].i_height, 1 );
			write_int( rgs_csprites[ i_idx ].i_sprite_offset + i_texture_data_offset );
			write_int( rgs_csprites[ i_idx ].i_sprite_length );
		}
		for( i_idx = 0; i_idx < i_num_cmaps; i_idx++ )
		{
			write_int( rgs_cmaps[ i_idx ].i_map_offset + i_texture_data_offset );
			write_int( rgs_cmaps[ i_idx ].i_map_length );
		}

		write_str( &rgui8_texture_data[ 0 ], i_num_texture_data );

		fclose( f_outf );
	}

	exit( 0 );
}


