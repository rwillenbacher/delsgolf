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

__attribute__((__stkparm__)) void eng_draw_spritespan0( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
__attribute__((__stkparm__)) void eng_draw_spritespan1( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
__attribute__((__stkparm__)) void eng_draw_spritespan2( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
__attribute__((__stkparm__)) void eng_draw_spritespan3( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
#else
void eng_draw_spritespan0( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
void eng_draw_spritespan1( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
void eng_draw_spritespan2( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
void eng_draw_spritespan3( UInt8 *pui8_drawbuffer, Int16 i_height, UInt8 *pui8_tex, Int32 i_v, Int32 i_scalev );
#endif
