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


#ifndef WIN32
UInt32 eng_draw_vspan0( UInt8 *pui8_drawbuffer asm("%a0"), UInt16 i_height asm("%d0"), UInt8 *pui8_tex asm("%a1"), UInt32 i_v asm("%d1"), UInt32 i_y_stepv asm("%d2") );
UInt32 eng_draw_vspan1( UInt8 *pui8_drawbuffer asm("%a0"), UInt16 i_height asm("%d0"), UInt8 *pui8_tex asm("%a1"), UInt32 i_v asm("%d1"), UInt32 i_y_stepv asm("%d2") );
UInt32 eng_draw_vspan2( UInt8 *pui8_drawbuffer asm("%a0"), UInt16 i_height asm("%d0"), UInt8 *pui8_tex asm("%a1"), UInt32 i_v asm("%d1"), UInt32 i_y_stepv asm("%d2") );
UInt32 eng_draw_vspan3( UInt8 *pui8_drawbuffer asm("%a0"), UInt16 i_height asm("%d0"), UInt8 *pui8_tex asm("%a1"), UInt32 i_v asm("%d1"), UInt32 i_y_stepv asm("%d2") );
UInt32 eng_draw_vspanX( UInt8 *pui8_drawbuffer asm("%a0"), UInt16 i_height asm("%d0"), UInt8 *pui8_tex asm("%a1"), UInt32 i_v asm("%d1"), UInt32 i_y_stepv asm("%d2"), Int16 i_sr asm("%d3") );
#else
UInt32 eng_draw_vspan0( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv );
UInt32 eng_draw_vspan1( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv );
UInt32 eng_draw_vspan2( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv );
UInt32 eng_draw_vspan3( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv );
UInt32 eng_draw_vspanX( UInt8 *pui8_drawbuffer, UInt16 i_height, UInt8 *pui8_tex, UInt32 i_v, UInt32 i_y_stepv, Int16 i_sr );
#endif