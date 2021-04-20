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




void draw_string( engine_t *ps_eng, Int16 i_bigfont, Int16 i_x, Int16 i_y, Int16 i_bg, UInt8 *pui8_Int8 );

#ifdef WIN32
void eng_draw_texture_hspan( UInt8 *pui8_drawbuffer, Int16 frac_x, Int16 i_length, UInt8 *pui8_tex, Int32 i_tu, Int32 i_u_xstep, Int32 i_tv, Int32 i_v_xstep );
#else
void eng_draw_texture_hspan( uint8_t *pui8_drawbuffer asm("a0"), int16_t frac_x asm("d0"), int16_t i_length asm("d1"), uint8_t *pui8_tex asm("a1"), int32_t i_tu asm("d2"), int32_t i_u_xstep asm("d3"), int32_t i_tv asm("d4"), int32_t i_v_xstep asm("d5") );
#endif
