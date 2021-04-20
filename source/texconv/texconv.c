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

typedef unsigned char UInt8;
typedef int Int32;

#include <png.h>



UInt8 *read_png( UInt8 *pui8_filename, Int32 *pi_width, Int32 *pi_height )
{
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep* row_pointers;
	FILE *fp;
	UInt8* pRgb;
	int rowbytes;
	Int32 i;

	char header[ 8 ];

	if( strlen( pui8_filename ) < 1 )
	{
		return 0;
	}

	fp = fopen( pui8_filename, "rb" );
	if( ! fp )
	{
		return 0;
	}
	fread( header, 1, 8, fp );
	if( png_sig_cmp( ( png_bytep )header, 0, 8 ) )
	{
		return 0;
	}

	// initialize stuff 
	png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if( ! png_ptr )
	{
		return 0;
	}

	info_ptr = png_create_info_struct( png_ptr );
	if( ! info_ptr )
	{
		return 0;
	}

	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		return 0;
	}

	png_init_io( png_ptr, fp );
	png_set_sig_bytes( png_ptr, 8 );

	png_read_info( png_ptr, info_ptr );

	*pi_width = png_get_image_width( png_ptr, info_ptr );
	*pi_height = png_get_image_height( png_ptr, info_ptr );
	color_type = png_get_color_type( png_ptr, info_ptr );
	bit_depth = png_get_bit_depth( png_ptr, info_ptr );
	rowbytes = png_get_rowbytes( png_ptr, info_ptr );

	// palette to RGB
	if( color_type == PNG_COLOR_TYPE_PALETTE )
	{
		png_set_palette_to_rgb( png_ptr );
		color_type = PNG_COLOR_TYPE_RGB;
		rowbytes *= 3;
	}

	// grayscale to RGB
	if( color_type == PNG_COLOR_TYPE_GRAY )
	{
		png_set_gray_to_rgb( png_ptr );
		color_type = PNG_COLOR_TYPE_RGB;
		rowbytes *= 3;
	}

	// grayscale to RGB
	if( color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
	{
		png_set_gray_to_rgb( png_ptr );
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		rowbytes *= 2;
	}

	// add alpha channel
	if( color_type == PNG_COLOR_TYPE_RGB )
	{
		png_set_add_alpha( png_ptr, 0xff, PNG_FILLER_AFTER );
		rowbytes += *pi_width;
	}

	png_set_interlace_handling( png_ptr );
	png_read_update_info( png_ptr, info_ptr );

	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		return 0;
	}

	row_pointers = (png_bytep*) malloc( *pi_height * sizeof( png_bytep ) );
	pRgb = malloc( *pi_width * *pi_height * sizeof( png_bytep ) );
	memset( pRgb, 0, *pi_width * *pi_height * sizeof( png_bytep ) );

	for( i = 0; i < *pi_height; i++ )
	{
		row_pointers[i] = ( png_byte* )( &pRgb[ i * rowbytes ] );
	}

	png_read_image( png_ptr, row_pointers );

	fclose( fp );
	free( row_pointers );

	return pRgb;
}



int main( int i_argc, char *pi8_argv[] )
{
	Int32 i_idx;
	UInt8 *pui8_rgba;
	Int32 i_width, i_height;

	pui8_rgba = read_png( pi8_argv[ 1 ], &i_width, &i_height );

	{
		int i_flip = 0;
		if( i_argc > 2 )
		{
			i_flip = 1;
		}

#define IDX ( ( i_flip ? ( ( i_idx % i_height ) * i_width ) + ( i_idx / i_height ) : i_idx ) * 4 )

		pi8_argv[ 1 ][ strstr( pi8_argv[ 1 ], ".png" ) - pi8_argv[ 1 ] ] = 0;
		printf("unsigned char rgui8_%s[ %d * %d ] = { \n", pi8_argv[ 1 ], i_width, i_height );
		for( i_idx = 0; i_idx < i_width * i_height; i_idx++ )
		{
			if( pui8_rgba[ IDX ] < 43 )
			{
				pui8_rgba[ IDX ] = 0;
			}
			else if( pui8_rgba[ IDX ] < 128 )
			{
				pui8_rgba[ IDX ] = 1;
			}
			else if( pui8_rgba[ IDX ] < 212 )
			{
				pui8_rgba[ IDX ] = 2;
			}
			else
			{
				pui8_rgba[ IDX ] = 3;
			}
			
			printf("0x%x,", ( ( pui8_rgba[ IDX + 3 ] > 128 ? 0 : 1 ) << 2 ) | ( ( !( pui8_rgba[ IDX ] & 1) ) << 1 ) | ( !( ( pui8_rgba[ IDX ] >> 1 ) & 1 ) ) );
			if( ( i_idx % ( i_width ) ) == ( i_width -1 ) )
			{
				printf("\n");
			}
		}
		printf("};\n");
		return 0;
	}
}

