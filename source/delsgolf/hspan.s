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

/*
        .text
        .even
        .globl  eng_commit_drawbuffer_line
eng_commit_drawbuffer_line:
        movm.l #0x1f8,-(%sp)
        move.l 36(%sp),%a0
        move.l 40(%sp),%a3
        move.l 44(%sp),%a2
        move.w 48(%sp),%d3
        move.w #0xf0f,%d0
        move.w #0xf0f0,%d1
        lea (%a0,%d3.w),%a4
.eng_commit_drawbuffer_line_loop:

        move.w (%a0)+,%d2
		move.w %d2,%d3
		and.w %d0,%d2
		and.w %d1,%d3
		move.w %d2,%d4
		move.w %d3,%d5
		lsr.w #4,%d2
		lsr.w #8,%d3
		lsr.w #4,%d5
		or.w %d2,%d4
		or.w %d3,%d5

        move.b %d4,(%a2)+
        move.b %d5,(%a3)+

        cmpa.l %a0,%a4
        jbne .eng_commit_drawbuffer_line_loop
        movm.l (%sp)+,#0x1cf8
        rts
*/

/*
        .text
        .even
        .globl  eng_commit_drawbuffer_wh
eng_commit_drawbuffer_wh:
        movm.l #0x1f3e,-(%sp)
        move.l 44(%sp),%a0
        move.l 48(%sp),%a1
        move.l 52(%sp),%a2
        move.w 56(%sp),%d0
        move.w 58(%sp),%d1
        move.w #0xf0f,%d2
        move.w #0xf0f0,%d3
		
.eng_commit_drawbuffer_h_loop:
		move.l %a0,%a3
		move.l %a1,%a4
		move.l %a2,%a5
		lea (%a3,%d0.w),%a6
		
.eng_commit_drawbuffer_w_loop:
        move.w (%a3)+,%d4
		move.w %d4,%d5
		and.w %d2,%d4
		and.w %d3,%d5
		move.w %d4,%d6
		move.w %d5,%d7
		lsr.w #4,%d4
		lsr.w #8,%d5
		lsr.w #4,%d7
		or.w %d4,%d6
		or.w %d5,%d7
        move.b %d6,(%a5)+
        move.b %d7,(%a4)+
        cmpa.l %a3,%a6
        jbne .eng_commit_drawbuffer_w_loop
		
		add.l #40,%a0
		add.l #30,%a1
		add.l #30,%a2
		subq.w #1,%d1
		jbne .eng_commit_drawbuffer_h_loop
		
        movm.l (%sp)+,#0x7cf8
        rts
*/

        .text
        .even
eng_commit_drawbuffer_wh_alignpel:
        move.b (%a3)+,%d6
		move.b (%a3)+,%d4
		move.b %d4,%d5
		move.b %d6,%d7
		lsr.b #4,%d5
		lsl.b #4,%d7
		and.b %d2,%d4
		and.b %d3,%d6
		or.b %d4,%d7
		or.b %d5,%d6
        move.b %d7,(%a5)+
        move.b %d6,(%a4)+
		rts

        .text
        .even
        .globl  eng_commit_drawbuffer_wh
eng_commit_drawbuffer_wh:
        movm.l #0x1f3e,-(%sp)
        move.l 44(%sp),%a0
        move.l 48(%sp),%a1
        move.l 52(%sp),%a2
		clr.l %d0
        move.w 56(%sp),%d0
        move.w 58(%sp),%d1
        move.w #0xf0f,%d2
        move.w #0xf0f0,%d3
		
		lsr.w #2,%d0
		subq.w #1,%d0
		move.l %d0,%a6
.eng_commit_drawbuffer_h_loop:
		move.l %a0,%a3
		move.l %a1,%a4
		move.l %a2,%a5
		move.w %a6,%d0
		btst #0,51(%sp)
		beq .eng_commit_drawbuffer_w_loop_fp
		jsr eng_commit_drawbuffer_wh_alignpel
		subq.w #1,%d0
.eng_commit_drawbuffer_w_loop_fp:
		| FIXME: oh TiEmu you dolt !
		|movep.w 0(%a3),%d4
		|movep.w 1(%a3),%d5
		|addq.l #4,%a3
        move.b (%a3)+,-(%sp)
		move.w (%sp)+,%d4
		move.b (%a3)+,-(%sp)
		move.w (%sp)+,%d5
		move.b (%a3)+,%d4
		move.b (%a3)+,%d5

		move.w %d4,%d6
		move.w %d5,%d7
		lsr.w #4,%d5
		and.w %d2,%d5
		and.w %d3,%d4
		or.w %d4,%d5
		lsl.w #4,%d6
		and.w %d3,%d6
		and.w %d2,%d7
		or.w %d7,%d6
        move.w %d6,(%a5)+
        move.w %d5,(%a4)+
		dbf %d0, .eng_commit_drawbuffer_w_loop_fp
		btst #0,51(%sp)
		beq .eng_commit_drawbuffer_w_loop_end
		jsr eng_commit_drawbuffer_wh_alignpel
.eng_commit_drawbuffer_w_loop_end:
		lea 40(%a0),%a0
		lea 30(%a1),%a1
		lea 30(%a2),%a2
		subq.w #1,%d1
		jbne .eng_commit_drawbuffer_h_loop
		
        movm.l (%sp)+,#0x7cf8
        rts

		
/*
	.even
	.globl	eng_draw_texture_hspan
eng_draw_texture_hspan:
	movm.l #0x1f30,-(%sp)
	move.l 32(%sp),%a1
	move.w 36(%sp),%d0
	move.l 40(%sp),%a2
	move.l 44(%sp),%d4
	move.l 48(%sp),%a3
	move.l 52(%sp),%d3
	move.l 56(%sp),%d6
	move.w 38(%sp),%a0
	add.w %d0,%a0
	lsl.l #5,%d3
	lsl.l #5,%d6
	moveq #0x1f,%d5
	move.l #0x3e0,%d7
	cmp.w #3,%a0
	jbls .hspan_short
	tst.w %d0
	jbeq .hspan_pel0
	cmp.w #1,%d0
	jbne .hspan_not_fpel_1
	move.b (%a1),%d2
	lsr #2,%d2
	jbra .hspan_pel1
.hspan_not_fpel_1:
	cmp.w #2,%d0
	jbne .hspan_not_fpel_2
	move.b (%a1),%d2
	lsr #1,%d2
	jbra .hspan_pel2
.hspan_not_fpel_2:
	cmp.w #3,%d0  | should not need this check for xfrac > 3 ..
	jbne .hspan_exit
	move.b (%a1),%d2
	jbra .hspan_pel3

.hspan_short:
	tst.w %d0
	jbeq .hspan_pel0_last
	cmp.w #1,%d0
	jbne .hspan_not_fpel_1_short
	move.b (%a1),%d2
	lsr #2,%d2
	jbra .hspan_pel1_last
.hspan_not_fpel_1_short:
	cmp.w #2,%d0
	jbne .hspan_exit | should not need this check for zero length xfrac 3
	move.b (%a1),%d2
	lsr #1,%d2
	jbra .hspan_pel2_last

.hspan_pel0:
	move.l %d3,%d0
	swap %d0
	and.w %d7,%d0
	move.l %d4,%d1
	swap %d1
	and.w %d5,%d1
	add.w %d0,%d1
	move.b (%a2,%d1.w),%d2
	add.b %d2,%d2
	add.l %a3,%d4
	add.l %d6,%d3
.hspan_pel1:
	move.l %d3,%d0
	swap %d0
	and.w %d7,%d0
	move.l %d4,%d1
	swap %d1
	and.w %d5,%d1
	add.w %d0,%d1
	or.b (%a2,%d1.w),%d2
	add.b %d2,%d2
	add.l %a3,%d4
	add.l %d6,%d3
.hspan_pel2:
	move.l %d3,%d0
	swap %d0
	and.w %d7,%d0
	move.l %d4,%d1
	swap %d1
	and.w %d5,%d1
	add.w %d0,%d1
	or.b (%a2,%d1.w),%d2
	add.b %d2,%d2
	add.l %a3,%d4
	add.l %d6,%d3
.hspan_pel3:
	move.l %d3,%d0
	swap %d0
	and.w %d7,%d0
	move.l %d4,%d1
	swap %d1
	and.w %d5,%d1
	add.w %d0,%d1
	or.b (%a2,%d1.w),%d2
	add.l %a3,%d4
	add.l %d6,%d3
	move.b %d2,(%a1)+

	subq.w #4,%a0
	cmp.w #3,%a0
	jbgt .hspan_pel0

.hspan_pel0_last:
	cmp.w #0,%a0
	jbeq .hspan_exit

	move.l %d3,%d0
	swap %d0
	and.w %d7,%d0
	move.l %d4,%d1
	swap %d1
	and.w %d5,%d1
	add.w %d0,%d1
	move.b (%a2,%d1.w),%d2
	add.b %d2,%d2
	add.l %a3,%d4
	add.l %d6,%d3

	cmp.w #1,%a0
	jbeq .hspan_write_last1

.hspan_pel1_last:
	move.l %d3,%d0
	swap %d0
	and.w %d7,%d0
	move.l %d4,%d1
	swap %d1
	and.w %d5,%d1
	add.w %d0,%d1
	or.b (%a2,%d1.w),%d2
	add.b %d2,%d2
	add.l %a3,%d4
	add.l %d6,%d3

	cmp.w #2,%a0
	jbeq .hspan_write_last2

.hspan_pel2_last:
	move.l %d3,%d0
	swap %d0
	and.w %d7,%d0
	move.l %d4,%d1
	swap %d1
	and.w %d5,%d1
	add.w %d0,%d1
	or.b (%a2,%d1.w),%d2
	add.b %d2,%d2
	add.l %a3,%d4
	add.l %d6,%d3

	move.b (%a1),%d5
	|andi.b #17,%d5
	or.b %d2,%d5
	move.b %d5,(%a1)
	jbra .hspan_exit

.hspan_write_last1:
	move.b (%a1),%d5
	lsl.b #2,%d2
	or.b %d2,%d5
	move.b %d5,(%a1)
	jbra .hspan_exit

.hspan_write_last2:
	move.b (%a1),%d5
	lsl.b #1,%d2
	or.b %d2,%d5
	move.b %d5,(%a1)

.hspan_exit:
	movm.l (%sp)+,#0xcf8
	rts
*/

/*
	.even
	.globl	eng_draw_texture_hspan
eng_draw_texture_hspan:
	movm.l #0x330,-(%sp)
	lsl.l #5,%d4
	lsl.l #5,%d5
	move.l %d5,%a3
	move.w %d3,%a2
	swap %d3
	move.w %d3,%d5
	move.w %d2,%d3
	swap %d2
	and.w #31,%d2
	move.w #992,%d7
	add.w %d0,%d1
	tst.w %d0
	jbne .hspan2_frac_ne0
	cmp.w #3,%d1
	jble .hspan2_frac0_trail0p
	jbra .hspan2_span_4p_loop
.hspan2_frac_ne0:
	move.b (%a0),%d6
	cmp.w #1,%d0
	jbne .hspan2_frac_ne1
	lsr.b #2,%d6
	and.b #34,%d6
	cmp.w #3,%d1
	jble .hspan2_frac1_trail1p
	jbra .hspan2_4p_loop_frac1
.hspan2_frac_ne1:
	cmp.w #2,%d0
	jbne .hspan2_frac_ne2
	lsr.b #1,%d6
	and.b #102,%d6
	cmp.w #3,%d1
	jble .hspan2_frac2_trail2p
	jbra .hspan2_4p_loop_frac2
.hspan2_frac_ne2:
	and.b #-18,%d6
	jbra .hspan2_4p_loop_frac3
.hspan2_span_4p_loop:
	move.l %d4,%d0
	swap %d0
	and.w %d7,%d0
	add.w %d2,%d0
	move.b (%a1,%d0.w),%d6
	add.l %a3,%d4
	add.w  %a2,%d3
	addx.w %d5,%d2
	and.w #31,%d2
	add.b %d6,%d6
.hspan2_4p_loop_frac1:
	move.l %d4,%d0
	swap %d0
	and.w %d7,%d0
	add.w %d2,%d0
	or.b (%a1,%d0.w),%d6
	add.l %a3,%d4
	add.w  %a2,%d3
	addx.w %d5,%d2
	and.w #31,%d2
	add.b %d6,%d6
.hspan2_4p_loop_frac2:
	move.l %d4,%d0
	swap %d0
	and.w %d7,%d0
	add.w %d2,%d0
	or.b (%a1,%d0.w),%d6
	add.l %a3,%d4
	add.w  %a2,%d3
	addx.w %d5,%d2
	and.w #31,%d2
	add.b %d6,%d6
.hspan2_4p_loop_frac3:
	move.l %d4,%d0
	swap %d0
	and.w %d7,%d0
	add.w %d2,%d0
	or.b (%a1,%d0.w),%d6
	add.l %a3,%d4
	add.w  %a2,%d3
	addx.w %d5,%d2
	and.w #31,%d2
	move.b %d6,(%a0)+
	subq.w #4,%d1
	cmp.w #3,%d1
	jbgt .hspan2_span_4p_loop
	tst.w %d1
	jble .hspan2_exit
.hspan2_frac0_trail0p:
	move.l %d4,%d0
	swap %d0
	and.w %d7,%d0
	add.w %d2,%d0
	move.b (%a1,%d0.w),%d6
	cmp.w #1,%d1
	jbne .hspan2_trail0p_not_last
	move.b (%a0),%d0
	and.b #119,%d0
	lsl.b #3,%d6
	or.b %d6,%d0
	jbra .hspan2_write_trail
.hspan2_trail0p_not_last:
	add.w  %a2,%d3
	addx.w %d5,%d2
	and.w #31,%d2
	add.l %a3,%d4
	add.b %d6,%d6
.hspan2_frac1_trail1p:
	move.l %d4,%d0
	swap %d0
	and.w %d7,%d0
	add.w %d2,%d0
	or.b (%a1,%d0.w),%d6
	cmp.w #2,%d1
	jbne .hspan2_trail1p_not_last
	move.b (%a0),%d0
	and.b #51,%d0
	lsl.b #2,%d6
	or.b %d6,%d0
	jbra .hspan2_write_trail
.hspan2_trail1p_not_last:
	add.w  %a2,%d3
	addx.w %d5,%d2
	and.w #31,%d2
	add.l %a3,%d4
	add.b %d6,%d6
.hspan2_frac2_trail2p:
	swap %d4
	move.w %d4,%d0
	and.w %d7,%d0
	add.w %d2,%d0
	or.b (%a1,%d0.w),%d6
	move.b (%a0),%d0
	and.b #17,%d0
	add.b %d6,%d6
	or.b %d6,%d0
.hspan2_write_trail:
	move.b %d0,(%a0)
.hspan2_exit:
	movm.l (%sp)+,#0x0cc0
	rts
*/

| void eng_draw_texture_hspan_n2( uint8_t *pui8_drawbuffer asm("a0"), int16_t frac_x asm("d0"), int16_t i_length asm("d1"), uint8_t *pui8_tex asm("a1"), int32_t i_tu asm("d2"), int32_t i_u_xstep asm("d3"), int32_t i_tv asm("d4"), int32_t i_v_xstep asm("d5") );

	.even
	.globl	eng_draw_texture_hspan
eng_draw_texture_hspan:
	movm.l #0x330,-(%sp)
	move.q #31,%d7
	move.w %d3,%a2		| ufracstep
	swap %d3
	and %d7,%d3
	move.w %d5,%a3		| vfracstep
	swap %d5
	and %d7,%d5
	asl.w #8,%d5
	add.w %d5,%d3		| tstep
	move.l %d4,%d7		| d4 = vfrac
	swap %d7
	asl.w #8,%d7
	move.w %d2,%d5		| ufrac
	swap %d2
	add.w %d7,%d2		| tpos


	clr.w %d7
	add.w %d0,%d1

	lsl.w #2,%d0
	lea hspan3_xfrac_jmptab,%a4
	move.l (%a4,%d0.w),%a4
	move.w #0x1f1f,%d0
	and.w %d0,%d2
	jmp (%a4)

hspan3_xfrac_jmptab:
	.long .hspan3_xfrac0
	.long .hspan3_xfrac1
	.long .hspan3_xfrac2
	.long .hspan3_xfrac3


.hspan3_xfrac3:
	move.b (%a0),%d6
	and.b #-18,%d6
	jbra .hspan3_4p_loop_frac3
.hspan3_xfrac1:
	move.b (%a0),%d6
	lsr.b #2,%d6
	and.b #34,%d6
	cmp.w #3,%d1
	jbgt .hspan3_4p_loop_frac1
	subq.w #1,%d1
	jbra .hspan3_frac1_trail1p
.hspan3_xfrac2:
	move.b (%a0),%d6
	lsr.b #1,%d6
	and.b #102,%d6
	cmp.w #3,%d1
	jbgt .hspan3_4p_loop_frac2
	jbra .hspan3_frac2_trail2p
.hspan3_xfrac0:
	cmp.w #3,%d1
	jble .hspan3_frac0_trail0p
.hspan3_span_4p_loop:
	move.b (%a1,%d2.w),%d6
	add.w %a3,%d4
	scs.b %d7
	addx.w %d7,%d2
	add.w %a2,%d5
	addx.w %d3,%d2
	and.w %d0,%d2
	add.b %d6,%d6
.hspan3_4p_loop_frac1:
	or.b (%a1,%d2.w),%d6
	add.w %a3,%d4
	scs.b %d7
	addx.w %d7,%d2
	add.w %a2,%d5
	addx.w %d3,%d2
	and.w %d0,%d2
	add.b %d6,%d6
.hspan3_4p_loop_frac2:
	or.b (%a1,%d2.w),%d6
	add.w %a3,%d4
	scs.b %d7
	addx.w %d7,%d2
	add.w %a2,%d5
	addx.w %d3,%d2
	and.w %d0,%d2
	add.b %d6,%d6
.hspan3_4p_loop_frac3:
	or.b (%a1,%d2.w),%d6
	move.b %d6,(%a0)+
	subq.w #4,%d1
	jble .hspan3_exit
	add.w %a3,%d4
	scs.b %d7
	addx.w %d7,%d2
	add.w %a2,%d5
	addx.w %d3,%d2
	and.w %d0,%d2
	cmp.w #3,%d1
	jbgt .hspan3_span_4p_loop
.hspan3_frac0_trail0p:
	move.b (%a1,%d2.w),%d6
	subq.w #1,%d1
	jbeq .hspan3_trail0p_last
	add.w %a3,%d4
	scs.b %d7
	addx.w %d7,%d2
	add.w %a2,%d5
	addx.w %d3,%d2
	and.w %d0,%d2
	add.b %d6,%d6
.hspan3_frac1_trail1p:
	or.b (%a1,%d2.w),%d6
	subq.w #1,%d1
	jbeq .hspan3_trail1p_last
	add.w %a3,%d4
	scs.b %d7
	addx.w %d7,%d2
	add.w %a2,%d5
	addx.w %d3,%d2
	and.w %d0,%d2
	add.b %d6,%d6
.hspan3_frac2_trail2p:
	or.b (%a1,%d2.w),%d6
	move.b (%a0),%d0
	and.b #17,%d0
	add.b %d6,%d6
	or.b %d6,%d0
	jbra .hspan3_write_trail
.hspan3_trail0p_last:
	move.b (%a0),%d0
	and.b #119,%d0
	lsl.b #3,%d6
	or.b %d6,%d0
	jbra .hspan3_write_trail
.hspan3_trail1p_last:
	move.b (%a0),%d0
	and.b #51,%d0
	lsl.b #2,%d6
	or.b %d6,%d0
.hspan3_write_trail:
	move.b %d0,(%a0)
.hspan3_exit:
	movm.l (%sp)+,#0x0cc0
	rts
