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

Int32 dot_16( vec2_t v1, vec2_t v2 )
{
	return mul_16_16( v1[ 0 ], v2[ 0 ] ) + mul_16_16( v1[ 1 ], v2[ 1 ] );
}

void transform_vec2( vec2_t v1, transform_t *t, vec2_t dst )
{
	vec2_t translated_v1;

	translated_v1[ 0 ] = v1[ 0 ] + t->translation[ 0 ];
	translated_v1[ 1 ] = v1[ 1 ] + t->translation[ 1 ];
	dst[ 0 ] = ( mul_16_16( t->right[ 0 ], translated_v1[ 0 ] ) + mul_16_16( t->right[ 1 ], translated_v1[ 1 ] ) ) >> 15;
	dst[ 1 ] = ( mul_16_16( t->front[ 0 ], translated_v1[ 0 ] ) + mul_16_16( t->front[ 1 ], translated_v1[ 1 ] ) ) >> 15;
}

void rotate_vec2( vec2_t v1, transform_t *t, vec2_t dst )
{
	dst[ 0 ] = ( mul_16_16( t->right[ 0 ], v1[ 0 ] ) + mul_16_16( t->right[ 1 ], v1[ 1 ] ) ) >> 15;
	dst[ 1 ] = ( mul_16_16( t->front[ 0 ], v1[ 0 ] ) + mul_16_16( t->front[ 1 ], v1[ 1 ] ) ) >> 15;
}

Int16 acos_d8( engine_t *ps_eng, Int16 i_angle )
{
	Int16 i_piv, i_step;
	Int16 *pi_costab;

	if( i_angle < 0 )
	{
		i_angle = -i_angle;
	}

	pi_costab = &ps_eng->rgi16_cosine_table[ 0x40 ];

	i_step = 16;
	i_piv = 32;
	for( i_step = 16; i_step > 0; i_step = i_step >> 1 )
	{
		if( pi_costab[ i_piv ] >= i_angle )
		{
			i_piv += i_step;
		}
		else
		{
			i_piv -= i_step;
		}
	}
	return i_piv;
}



deg8_t angle_vector( engine_t *ps_eng, vec2_t v )
{
	Int16 i_a, i_b, i_c;
	Int16 i_angle;

	i_a = v[ 0 ] >> 2;
	i_b = v[ 1 ] >> 2;
	i_c = isqrt( ( mul_16_16( v[ 0 ], v[ 0 ] ) >> 4 ) + ( mul_16_16( v[ 1 ], v[ 1 ] ) >> 4 ) );
	if( i_c < 1 )
	{
		i_c = 1;
	}
	if( i_a >= i_c || i_a <= -i_c )
	{
		i_angle = 0;
	}
	else
	{
		i_angle = div_32_u16( ( ( ( ( Int32 )i_a ) << 15 ) - 1 ), i_c );
		i_angle = acos_d8( ps_eng, i_angle );
	}
	if( i_angle > 64 )
	{
		i_angle = 64;
	}
	else if( i_angle < -64 )
	{
		i_angle = -64;
	}

	if( i_a >= 0 && i_b >= 0 )
	{
		/* q0 */
		i_angle = 128 + i_angle;
	}
	else if( i_a < 0 && i_b >= 0 )
	{
		/* q1 */
		i_angle = 192 + ( 64 - i_angle );
	}
	else if( i_a < 0 && i_b < 0 )
	{
		i_angle = i_angle;
	}
	else
	{
		i_angle = 64 + ( 64 - i_angle );
	}

	return ( deg8_t )i_angle;
}


/* by Jim Ulery */
UInt32 isqrt( UInt32 ui_val )
{
	UInt32 ui_temp, ui_g, ui_b, ui_bshift;

	ui_g = 0;
	ui_b = 0x8000;
	ui_bshift = 15;

    do {
		if ( ui_val >= ( ui_temp = ( ( ( ui_g << 1 ) + ui_b ) << ui_bshift-- ) ) )
		{
			ui_g += ui_b;
			ui_val -= ui_temp;
		}
    } while( ui_b >>= 1);

    return ui_g;
}

/* i have no idea of what i am doing here */
#define RND_INIT   55555
#define RND_MUL     2665
#define RND_ADD    32767

void rand_init( engine_t *ps_eng )
{
	ps_eng->ui16_random = RND_INIT;
}

UInt16 rand16( engine_t *ps_eng )
{
	UInt32 ui_rnd;


	ui_rnd = ( mul_u16_u16( ps_eng->ui16_random, RND_MUL ) + RND_ADD ) & 0xffff;
	ps_eng->ui16_random = ui_rnd;

	return ui_rnd;
}


