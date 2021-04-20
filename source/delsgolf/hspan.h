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


void eng_draw_scaled_texture_span_x_c( unsigned char *p_drawbuffer, unsigned int pel_x_f, unsigned int i_width, unsigned char *p_texture, unsigned long t_u, long i_xstep_u );

#ifdef WIN32
void eng_draw_texture_hspan( unsigned char *p_drawbuffer, unsigned int pel_x_f, unsigned int i_width, unsigned char *p_texture, unsigned long t_u, long i_xstep_u, unsigned long t_v, long i_xstep_v );
#else
__attribute__((__stkparm__)) void eng_draw_texture_hspan( unsigned char *p_drawbuffer, unsigned int pel_x_f, unsigned int i_width, unsigned char *p_texture, unsigned long t_u, long i_xstep_u, unsigned long t_v, long i_xstep_v );
#endif
