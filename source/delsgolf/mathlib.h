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



typedef UInt8 deg8_t;
typedef Int16 frac16_t;
typedef Int16 fixed4_t;
typedef Int32 fixed16_t;

typedef fixed4_t vec2_t[2];
typedef fixed4_t vec3_t[3];
typedef frac16_t normal_t[3];

typedef struct {
	normal_t front;
	normal_t right;
	vec3_t translation;
}transform_t;

#define ONE_F4  (1<<4)
#define ONE_F16 (1<<16)


#define sin_d8( ps_eng, r ) ( ps_eng->rgi16_cosine_table[(r) & 0xff] )
#define cos_d8( ps_eng, r ) ( ps_eng->rgi16_cosine_table[(((int)(r))+0x40) & 0xff] )

#define load_vector( v, x, y, z ) do { v[0] = x; v[1] = y; v[2] = z; } while( 0 )
#define load_vector2( v, x, y ) do { v[0] = x; v[1] = y; } while( 0 )

#define load_vector2v( v, vs ) do { v[ 0 ] = vs[ 0 ]; v[ 1 ] = vs[ 1 ]; } while( 0 )

Int32 dot_16( vec2_t v1, vec2_t v2 );

void transform_vec2( vec2_t v1, transform_t *t, vec2_t dst );
void rotate_vec2( vec2_t v1, transform_t *t, vec2_t dst );



UInt32 isqrt( UInt32 i_ui );
