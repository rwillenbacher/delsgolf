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



	.text
	.even
	.globl	eng_draw_spritespan0
eng_draw_spritespan0:
	movm.l #0x1c20,-(%sp)
	move.l 20(%sp),%a0
	move.w 24(%sp),%d4
	move.l 26(%sp),%a2
	move.l 30(%sp),%d2
	move.l 34(%sp),%d3
	tst.w %d4
	jble .eng_draw_spritespan0_exit
	moveq #119,%d5
.eng_draw_spritespan0_loop:
	move.l %d2,%d0
	swap %d0
	add.l %d3,%d2
	move.b (%a2,%d0.w),%d1
	btst #2,%d1
	jbne .eng_draw_spritespan0_transparent
	move.b (%a0),%d0
	and.b %d5,%d0
	lsl.w #3,%d1
	or.b %d1,%d0
	move.b %d0,(%a0)
.eng_draw_spritespan0_transparent:
	lea (40,%a0),%a0
	subq.w #1,%d4
	jbne .eng_draw_spritespan0_loop

.eng_draw_spritespan0_exit:
	movm.l (%sp)+,#0x438
	rts


	.even
	.globl	eng_draw_spritespan1
eng_draw_spritespan1:
	movm.l #0x1c20,-(%sp)
	move.l 20(%sp),%a0
	move.w 24(%sp),%d4
	move.l 26(%sp),%a2
	move.l 30(%sp),%d2
	move.l 34(%sp),%d3
	tst.w %d4
	jble .eng_draw_spritespan1_exit
	moveq #0xffffffbb,%d5
.eng_draw_spritespan1_loop:
	move.l %d2,%d0
	swap %d0
	add.l %d3,%d2
	move.b (%a2,%d0.w),%d0
	btst #2,%d0
	jbne .eng_draw_spritespan1_transparent
	move.b (%a0),%d1
	and.b %d5,%d1
	add.b %d0,%d0
	add.b %d0,%d0
	or.b %d0,%d1
	move.b %d1,(%a0)
.eng_draw_spritespan1_transparent:
	lea (40,%a0),%a0
	subq.w #1,%d4
	jbne .eng_draw_spritespan1_loop
.eng_draw_spritespan1_exit:
	movm.l (%sp)+,#0x438
	rts


	.even
	.globl	eng_draw_spritespan2
eng_draw_spritespan2:
	movm.l #0x1c20,-(%sp)
	move.l 20(%sp),%a0
	move.w 24(%sp),%d4
	move.l 26(%sp),%a2
	move.l 30(%sp),%d2
	move.l 34(%sp),%d3
	tst.w %d4
	jble .eng_draw_spritespan2_exit
	moveq #0xffffffdd,%d5
.eng_draw_spritespan2_loop:
	move.l %d2,%d0
	swap %d0
	add.l %d3,%d2
	move.b (%a2,%d0.w),%d0
	btst #2,%d0
	jbne .eng_draw_spritespan2_transparent
	move.b (%a0),%d1
	and.b %d5,%d1
	add.w %d0,%d0
	or.b %d0,%d1
	move.b %d1,(%a0)
.eng_draw_spritespan2_transparent:
	lea (40,%a0),%a0
	subq.w #1,%d4
	jbne .eng_draw_spritespan2_loop
.eng_draw_spritespan2_exit:
	movm.l (%sp)+,#0x438
	rts



	.even
	.globl	eng_draw_spritespan3
eng_draw_spritespan3:
	movm.l #0x1c20,-(%sp)
	move.l 20(%sp),%a0
	move.w 24(%sp),%d4
	move.l 26(%sp),%a2
	move.l 30(%sp),%d2
	move.l 34(%sp),%d3
	tst.w %d4
	jble .eng_draw_spritespan3_exit
	moveq #0xffffffee,%d5
.eng_draw_spritespan3_loop:
	move.l %d2,%d0
	swap %d0
	move.b (%a2,%d0.w),%d1
	add.l %d3,%d2
	btst #2,%d1
	jbne .eng_draw_spritespan3_transparent
	move.b (%a0),%d0
	and.b %d5,%d0
	or.b %d1,%d0
	move.b %d0,(%a0)
.eng_draw_spritespan3_transparent:
	lea (40,%a0),%a0
	subq.w #1,%d4
	jbne .eng_draw_spritespan3_loop
.eng_draw_spritespan3_exit:
	movm.l (%sp)+,#0x438
	rts
