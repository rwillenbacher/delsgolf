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
	.globl	eng_draw_vspan0
	.globl	eng_draw_vspan1
	.globl	eng_draw_vspan2
	.globl	eng_draw_vspan3


/*
	.even
eng_draw_vspan0:
	movm.l #0x1f40,-(%sp)

	moveq #0x1f,%d6
	moveq #-120,%d7

	move.l %d2,%a2
	swap %d2
	move.l %d1,%d3
	swap %d3

	move.w %d0,%d5
	and.w #1,%d0
	lsr.w #1,%d5
	subq.w #1,%d5
	jbmi .vspan_0_enter_single
.vspan_0_loop_double:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,40(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 80(%a0),%a0
	dbf %d5, .vspan_0_loop_double

.vspan_0_enter_single:
	subq.w #1, %d0
	jbmi .vspan_0_loop_single_end
.vspan_0_loop_single:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 40(%a0),%a0
	dbf %d0, .vspan_0_loop_single
.vspan_0_loop_single_end:
	movm.l (%sp)+,#0x2f8
	rts


	.even
eng_draw_vspan1:
	movm.l #0x1f40,-(%sp)

	moveq #0x1f,%d6
	moveq #0x44,%d7

	move.l %d2,%a2
	swap %d2
	move.l %d1,%d3
	swap %d3

	move.w %d0,%d5
	and.w #1,%d0
	lsr.w #1,%d5
	subq.w #1,%d5
	jbmi .vspan_1_enter_single
.vspan_1_loop_double:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,40(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 80(%a0),%a0
	dbf %d5, .vspan_1_loop_double

.vspan_1_enter_single:
	subq.w #1, %d0
	jbmi .vspan_1_loop_single_end
.vspan_1_loop_single:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 40(%a0),%a0
	dbf %d0, .vspan_1_loop_single
.vspan_1_loop_single_end:
	movm.l (%sp)+,#0x2f8
	rts


	.even
eng_draw_vspan2:
	movm.l #0x1f40,-(%sp)

	moveq #0x1f,%d6
	moveq #0x22,%d7

	move.l %d2,%a2
	swap %d2
	move.l %d1,%d3
	swap %d3

	move.w %d0,%d5
	and.w #1,%d0
	lsr.w #1,%d5
	subq.w #1,%d5
	jbmi .vspan_2_enter_single
.vspan_2_loop_double:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,40(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 80(%a0),%a0
	dbf %d5, .vspan_2_loop_double

.vspan_2_enter_single:
	subq.w #1, %d0
	jbmi .vspan_2_loop_single_end
.vspan_2_loop_single:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 40(%a0),%a0
	dbf %d0, .vspan_2_loop_single
.vspan_2_loop_single_end:
	movm.l (%sp)+,#0x2f8
	rts


	.even
eng_draw_vspan3:
	movm.l #0x1f40,-(%sp)

	moveq #0x1f,%d6
	moveq #0x11,%d7

	move.l %d2,%a2
	swap %d2
	move.l %d1,%d3
	swap %d3

	move.w %d0,%d5
	and.w #1,%d0
	lsr.w #1,%d5
	subq.w #1,%d5
	jbmi .vspan_3_enter_single
.vspan_3_loop_double:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,40(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 80(%a0),%a0
	dbf %d5, .vspan_3_loop_double

.vspan_3_enter_single:
	subq.w #1, %d0
	jbmi .vspan_3_loop_single_end
.vspan_3_loop_single:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 40(%a0),%a0
	dbf %d0, .vspan_3_loop_single
.vspan_3_loop_single_end:
	movm.l (%sp)+,#0x2f8
	rts
*/

/*
	.even
	.globl  eng_draw_vspanX
eng_draw_vspanX:
	movm.l #0x0f20,-(%sp)

	moveq #0x1f,%d6
	moveq #-120,%d7
	lsr.b %d3,%d7

	move.l %d2,%a2
	swap %d2
	move.l %d1,%d3
	swap %d3

	move.w %d0,%d5
	and.w #3,%d0
	lsr.w #2,%d5
	subq.w #1,%d5
	jbmi .vspan_X_enter_single
.vspan_X_loop_quad:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3

	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,40(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3

	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,80(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3

	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,120(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3

	lea 160(%a0),%a0
	dbf %d5, .vspan_X_loop_quad

.vspan_X_enter_single:
	subq.w #1, %d0
	jbmi .vspan_X_loop_single_end
.vspan_X_loop_single:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 40(%a0),%a0
	dbf %d0, .vspan_X_loop_single
.vspan_X_loop_single_end:
	movm.l (%sp)+,#0x4f0
	rts
*/


| void eng_draw_wall_segment_vlines( render_vline_gen_struct_t *ps_gstructv asm( "%a0" ), render_vline_struct_t *ps_vlines asm( "%a1" ), render_texture_params_t *ps_texture_paramsv asm( "%a2" ) );

	.globl eng_draw_wall_segment_vlines
	.even
eng_draw_wall_segment_vlines:
	movm.l #0xffff,-(%sp)
	sub.w #16,%sp

	move.l %a0,%a4
	move.l %a2,%a6

/*  i_x = ps_gstruct->i16_x_start;
	ps_vlines = ps_vlines + i_x; */

	move.w 6(%a4),%d0
	move.w %d0,%d7
	add.w %d0,%d0
	add.w %d0,%d0
	lea (%a1,%d0.w),%a5

/*	i_v_offset = ( ( Int32 )ps_gstruct->i_v_offset ) << 12;*/
	move.w 4(%a4),%d0
	swap %d0
	clr.w %d0
	lsr.l #4,%d0
	move.l %d0,(%sp)			| i_v_offset

/* 	i_rzi = ( mul_u16_32_ns( i_x, ps_texture_params->i_zx ) ) + ps_texture_params->i_zbase; */
	move.l 14(%a6),%d1
	jbge .eng_draw_wall_segment_vlines_i_zx_pos
	neg.l %d1
	move.w %d1,%d3
	mulu.w %d7,%d3
	swap.w %d1
	mulu.w %d7,%d1
	swap.w %d1
	clr.w %d1
	add.l %d3,%d1
	neg.l %d1
	jbra .eng_draw_wall_segment_vlines_i_zx_pos_done
.eng_draw_wall_segment_vlines_i_zx_pos:
	move.w %d1,%d3
	mulu.w %d7,%d3
	swap.w %d1
	mulu.w %d7,%d1
	swap.w %d1
	clr.w %d1
	add.l %d3,%d1
.eng_draw_wall_segment_vlines_i_zx_pos_done:
	add.l 10(%a6),%d1
	move.l %d1,4(%sp)			| i_rzi

/* i_ru = mul_16_16( i_x, ps_texture_params->i_ux ) + ps_texture_params->i_ubase; */
	move.w 24(%a6),%d1
	muls.w %d7,%d1
	add.w 22(%a6),%d1
	move.w %d1,8(%sp)			| i_ru

.eng_draw_wall_segment_vlines_xloop:
/*
		i_py = ps_vlines->i_py;
		i_pheight = ps_vlines->i_pheight;
		ps_vlines++;
*/
	move.w (%a5)+,%d5
	move.w (%a5)+,%d0

/*
	i_zi = i_rzi;
	i_rzi += ps_texture_params->i_zx;
*/
	move.l 4(%sp),%d6
	move.l %d6,%d2
	add.l 14(%a6),%d6
	move.l %d6,4(%sp)

/*
	i_u = i_ru;
	i_ru += ps_texture_params->i_ux;
*/
	move.w 8(%sp),%d6
	move.w %d6,%d3
	add.w 24(%a6),%d6
	move.w %d6,8(%sp)
/*
	if( i_pheight > i_py )
	{
		i_pheight = ( i_pheight - i_py );
*/
	sub.w %d5,%d0
	jle .eng_draw_wall_segment_vlines_xloop_tail

/*
	if( i_zi < 0x1000 )
	{
		i_zi = 0x1000;
	}
*/
	cmp.l #0x1000,%d2
	jge .eng_draw_wall_segment_vlines_xloop_rzi_ok
	move.l #0x1000,%d2
.eng_draw_wall_segment_vlines_xloop_rzi_ok:

/*	i_z = div_u32_u16_u16r( 0xff>>8, i_zi >> 8 );*/
	asr.l #8,%d2
	move.l #0x7ffff,%d4
	divu.w %d2,%d4


/*	i_u = ( ( UInt16 )( ( mul_16_16( i_z, i_u ) >> 16 ) ) ) + ( UInt16 )( ps_texture_params->i_ut );*/
	muls.w %d4,%d3
	clr.w %d3
	swap %d3
	add.w 26(%a6),%d3
	and.w #0x3e,%d3
	asl.w #7,%d3		| tex offset


	/* pui8_tex = ps_gstruct->pui8_tex + ( ( ( i_pu & 0x3e ) << 7 ) ); */
	lea 10(%a4),%a3
	move.l (%a3)+,%a1	| tex
	add.l %d3,%a1		| tex done
	move.l (%a3)+,%a0	| db
	move.l (%a3)+,%a2	| ylut
	move.l (%a3),%a3	| ritab

/* 			i_y_stepv = mul_u16_u16( i_z, ( INVERSE_VSCALE_SCALE >> 4 ) ); */
	moveq #56,%d2
	muls.w %d4,%d2

/*
			i_v = ps_gstruct->pi16_ritab[ i_py ];
			i_v = ( mul_16_16( i_v, i_z ) );
			i_v += i_v_offset;
*/

	add.w %d5,%d5 | i_py * 2
	move.w (%a3,%d5.w),%d1
	muls.w %d4,%d1
	add.l (%sp),%d1

	move.w (%a2,%d5.w),%d4
	ext.l %d4
	add.l %d4,%a0
	move.w %d7,%d3
	and.w #3,%d3
	move.w %d7,%d4
	lsr.w #2,%d4
	add.l %d4,%a0

|	jbsr eng_draw_vspanX
	move.w %d7,-(%sp)

	moveq #0x1f,%d6
	moveq #-120,%d7
	lsr.b %d3,%d7

	move.l %d2,%a2
	swap %d2
	move.l %d1,%d3
	swap %d3

	move.w %d0,%d5
	and.w #3,%d0
	lsr.w #2,%d5
	subq.w #1,%d5
	jbmi .vspan_X_enter_single
.vspan_X_loop_quad:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3

	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,40(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3

	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,80(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3

	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,120(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3

	lea 160(%a0),%a0
	dbf %d5, .vspan_X_loop_quad

.vspan_X_enter_single:
	subq.w #1, %d0
	jbmi .vspan_X_loop_single_end
.vspan_X_loop_single:
	and.w %d6,%d3
	move.b (%a1,%d3.w),%d4
	and.w %d7,%d4
	or.b %d4,(%a0)
	add.w %a2,%d1
	addx.w %d2,%d3
	lea 40(%a0),%a0
	dbf %d0, .vspan_X_loop_single
.vspan_X_loop_single_end:
	move.w (%sp)+,%d7


.eng_draw_wall_segment_vlines_xloop_tail:
	addq.w #1,%d7
	cmp.w 8(%a4),%d7
	jbne .eng_draw_wall_segment_vlines_xloop

	add.w #16,%sp
	movm.l (%sp)+,#0xffff
	rts

