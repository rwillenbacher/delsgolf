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


typedef struct
{
	void *p_sys_private;
} sys_win32_t;

#ifdef __cplusplus
extern "C" {
#endif

Bool sys_win32_init( sys_win32_t * );
Bool sys_win32_deinit( sys_win32_t * );
Bool sys_win32_commit_drawbuffer( sys_win32_t *p_win32, UInt8 *pui8_drawbuffer, Int32 i_offs_x, Int32 i_offs_y, Int32 i_width, Int32 i_height );

#define ENG_KEY_ESC   1
#define ENG_KEY_LEFT  2
#define ENG_KEY_RIGHT 4
#define ENG_KEY_UP    8
#define ENG_KEY_DOWN  16
#define ENG_KEY_ENTER 32
#define ENG_KEY_0     64
#define ENG_KEY_1     128
#define ENG_KEY_2     256
#define ENG_KEY_3	  512
#define ENG_KEY_4	  1024
#define ENG_KEY_6	  2048

Bool sys_test_key( sys_win32_t *, Int32 iKey );

#ifdef __cplusplus
}
#endif
