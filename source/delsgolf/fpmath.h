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



#ifdef WIN32

#define mul_16_16( a, b ) ( (Int32)((Int16)(a)) * (Int32)((Int16)(b)) )
#define mul_u16_u16( a, b ) ( (UInt32)((UInt16)(a)) * (UInt32)((UInt16)(b)) )
#define div_16_16( a, b ) ( (Int16)((Int16)(a)) / ((Int16)(b)) )
#define div_u16_u16( a, b ) ( (UInt16)((UInt16)(a)) / (UInt32)((UInt16)(b)) )
#define div_u32_u16_u16r( a, b ) ((UInt16)( ((UInt32)(a)) / ((UInt32)((UInt16)(b))) ))

#define mul_16_u16( a, b ) ( ( (Int32)((Int16)(a)) * ((Int32)((UInt16)(b))) ) >> 16 )
#define mul_32_u32( a, b ) ( (Int32)(( ((long long)(a)) * ((long long)(b)) ) >> 16) )
#define mul_32_u16( a, b ) ( (Int32)(( ((long long)(a)) * ((long long)((UInt16)(b))) ) >> 16) )
#define mul_16_u32( a, b ) ( (Int32)(( ((Int16)(a)) * ((long long)(b)) ) >> 16) )
#define mul_16_u32_ns( a, b ) ( (Int32)( ((long long)((Int16)(a))) * ((long long)((UInt32)(b))) ) )
#define mul_u16_32_ns( a, b ) ( (Int32)( ((long long)((UInt16)(a))) * ((long long)((Int32)(b))) ) )

#define div_32_u16( n, d ) ( (Int32)( (((long long)(n))) / ( long long )((UInt16)(d)) ) )
#define div_u32_u16( n, d ) ( (Int32)( (((unsigned long long)n)) / ((UInt16)(d)) ) )


#else

#define mul_16_16( a, b )	\
({ Int32 prod; Int16 arg1=(a),arg2=(b);   \
__asm__ ( "muls.w %2,%0" : "=d" ( prod ) : "0" ( arg1 ), "d" ( arg2 ) );	\
prod; } )

#define mul_u16_u16( a, b )	\
({ UInt32 prod; UInt16 arg1=(a), arg2=(b);   \
__asm__ ( "mulu.w %2,%0;" : "=d" ( prod ) : "0" ( arg1 ), "d" ( arg2 ) );	\
prod; } )

__attribute__((__stkparm__)) Int16 mul_16_u16( Int16 sa1, UInt16 a2 );
__attribute__((__stkparm__)) Int16 mul_16_u16_r( Int16 sa1, UInt16 a2 );
__attribute__((__stkparm__)) Int32 mul_32_u32( Int32 sa1, UInt32 a2 );
__attribute__((__stkparm__)) Int32 mul_16_u32( Int16 sa1, UInt32 a2 );
Int32 mul_16_u32_ns( Int16 sa1 asm( "d1" ), UInt32 a2 asm( "d2") );
Int32 mul_32_u16( Int32 sa1 asm( "d0" ), UInt16 a2 asm( "d2" ) );
Int32 mul_u16_32_ns( UInt16 a1 asm("d1"), Int32 sa2  asm("d2") );


__attribute__((__stkparm__)) UInt32 div_u32_u16( UInt32 n, UInt16 d );
__attribute__((__stkparm__)) UInt32 div_32_u16( UInt32 n, UInt16 d );
__attribute__((__stkparm__)) UInt32 div_persp( UInt32 d );

#define div_u32_u16_u16r( a, b )	\
({ UInt16 prod; UInt16 arg1=(b); UInt32 arg2=(a);   \
__asm__ ( "divu.w %1,%0;swap %0;clr.w %0;swap %0;" : "=d" ( prod ) : "d" ( arg1 ), "0" ( arg2 ) );	\
prod; } )

#define div_16_16( a, b )	\
({ Int16 prod; Int16 arg1=(b); Int32 arg2=(a);   \
__asm__ ( "divs.w %1,%0; ext.l %0" : "=d" ( prod ) : "d" ( arg1 ), "0" ( arg2 ) );	\
prod; } )

#define div_u16_u16( a, b )	\
({ Int16 prod; UInt16 arg1=(b); Int32 arg2=(a);   \
__asm__ ( "divu.w %1,%0; and.l #0xffff,%0" : "=d" ( prod ) : "d" ( arg1 ), "0" ( arg2 ) );	\
prod; } )

#endif

Int16 acos_d8( engine_t *ps_eng, Int16 i_angle );
deg8_t angle_vector( engine_t *ps_eng, vec2_t v );
void rand_init( engine_t *ps_eng );
UInt16 rand16( engine_t *ps_eng );



