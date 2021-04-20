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




	.even
        .globl  mul_32_u32
mul_32_u32:
        movm.l #0x1f00,-(%sp)
        move.l 24(%sp),%d1
        move.l 28(%sp),%d2
        move.l %d1,%d7
        jbge .mul_32_u32_positive
        neg.l %d1
.mul_32_u32_positive:
| low x low
        move.w %d1,%d3
        mulu.w %d2,%d3
| high x low
        move.l %d1,%d4
        swap.w %d4
        mulu.w %d2,%d4

| low x high
        swap.w %d2
        move.w %d1,%d5
        mulu.w %d2,%d5
| high x high
	swap.w %d1
	mulu.w %d2,%d1

| high x low += ( low x low ) >> 16
	clr.w %d3
	swap.w %d3
	add.l %d3,%d4
| accum = ( ( high x high ) << 16 ) + ( low x high )
	swap.w %d1
	clr.w %d1
| low x high += high x low
	add.l %d4,%d5
	addx.l %d5, %d1


        tst.l %d7
        jbge .mul_32_u32_negative
        neg.l %d1
.mul_32_u32_negative:
        move.l %d1,%d0
        movm.l (%sp)+,#0xf8
        rts


	.even
        .globl  mul_16_u32
mul_16_u32:
        move.l %d3,-(%sp)
        movq #0,%d1
        move.w 8(%sp),%d1
        move.l 10(%sp),%d2
        move.w %d1,%d0
        jbge .mul_16_u32_positive
        neg.w %d1
.mul_16_u32_positive:
| low x low
        move.w %d2,%d3
        mulu.w %d1,%d3
| low x high
        swap.w %d2
        mulu.w %d1,%d2

| accum = ( low x high ) + ( ( low x low ) >> 16 )
	clr.w %d3
	swap.w %d3
	add.l %d3,%d2

        tst.w %d0
        jbge .mul_16_u32_negative
        neg.l %d2
.mul_16_u32_negative:
        move.l %d2,%d0
        move.l (%sp)+,%d3
        rts

	.even
        .globl  mul_16_u32_ns
mul_16_u32_ns:
        move.l %d3,-(%sp)
        move.w %d1,%d3
        jbge .mul_16_u32_ns_positive
        neg.w %d1
.mul_16_u32_ns_positive:
| low x low
        move.w %d2,%d0
        mulu.w %d1,%d0
| low x high
        swap.w %d2
        mulu.w %d1,%d2

| accum = ( low x high ) + ( ( low x low ) >> 16 )
	swap.w %d2
	clr.w %d2
	add.l %d2,%d0

        tst.w %d3
        jbge .mul_16_u32_ns_negative
        neg.l %d0
.mul_16_u32_ns_negative:
        move.l (%sp)+,%d3
        rts

	.even
        .globl  mul_16_u16
mul_16_u16:
        move.w 4(%sp),%d1
        move.w %d1,%d0
        jbge .mul_16_u16_positive
        neg.w %d0
.mul_16_u16_positive:
| low x low
        mulu.w 6(%sp),%d0
        clr.w %d0
        swap.w %d0

        tst.w %d1
        jbge .mul_16_u16_negative
        neg.w %d0
.mul_16_u16_negative:
        rts

	.even
        .globl  mul_16_u16_r
mul_16_u16_r:
        move.w 4(%sp),%d1
        move.w %d1,%d0
        jbge .mul_16_u16_r_positive
        neg.w %d0
.mul_16_u16_r_positive:
| low x low
        mulu.w 6(%sp),%d0
        add.l #0x8000, %d0
        clr.w %d0
        swap.w %d0

        tst.w %d1
        jbge .mul_16_u16_r_negative
        neg.w %d0
.mul_16_u16_r_negative:
        rts


	.even
	.globl  mul_32_u16
mul_32_u16:
	move.l %d3,-(%sp)
	move.l %d0,%d1
	jbge .mul_32_u16_positive
	neg.l %d0
	| low x low
	move.w %d0,%d3
	mulu.w %d2,%d3
	| low x high
	swap.w %d0
	mulu.w %d2,%d0
	| accum = ( low x high ) + ( ( low x low ) >> 16 )
	clr.w %d3
	swap.w %d3
	add.l %d3,%d0
	neg.l %d0
	move.l (%sp)+,%d3
	rts

	.mul_32_u16_positive:
	| low x low
	move.w %d0,%d3
	mulu.w %d2,%d3
	| low x high
	swap.w %d0
	mulu.w %d2,%d0
	| accum = ( low x high ) + ( ( low x low ) >> 16 )
	clr.w %d3
	swap.w %d3
	add.l %d3,%d0
	move.l (%sp)+,%d3
	rts


	.even
	.globl  mul_u16_32_ns
mul_u16_32_ns:
|	move.l %d2,-(%sp)
|	move.w 8(%sp),%d1
|	move.l 10(%sp),%d2
	jbge .mul_u16_32_ns_positive
	neg.l %d2
	move.w %d2,%d0
	mulu.w %d1,%d0
	swap.w %d2
	mulu.w %d1,%d2
	swap.w %d2
	clr.w %d2
	add.l %d2,%d0
	neg.l %d0
|	move.l (%sp)+,%d2
	rts

.mul_u16_32_ns_positive:
	move.w %d2,%d0
	mulu.w %d1,%d0
	swap.w %d2
	mulu.w %d1,%d2
	swap.w %d2
	clr.w %d2
	add.l %d2,%d0
|	move.l (%sp)+,%d2
	rts


	.even
        .globl div_u32_u16
div_u32_u16:
        move.l %d3,-(%sp)
        move.l 8(%sp),%d0
        move.w 12(%sp),%d2
| high / low
        move.l %d0,%d1
        clr.w %d1
        swap.w %d1
        divu.w %d2,%d1
        move.w %d1,%d3
        clr.w %d1
	andi.l #0xffff,%d0
| low / low
        add.l %d0,%d1
        divu.w %d2,%d1
        swap.w %d3
        move.w %d1,%d3
        move.l %d3,%d0
        move.l (%sp)+,%d3
        rts

	.even
        .globl div_32_u16
div_32_u16:
        move.l %d3,-(%sp)
        move.l %d4,-(%sp)
        move.l 12(%sp),%d0
        move.w 16(%sp),%d2
| high / low
        move.l %d0,%d1
        jbge .div_32_u16_positive
        neg.l %d1
.div_32_u16_positive:
        clr.w %d1
        swap.w %d1
        divu.w %d2,%d1
        move.w %d1,%d3
        clr.w %d1
        move.l %d0,%d4
	andi.l #0xffff,%d4
        add.l %d4,%d1
        divu.w %d2,%d1
        swap.w %d3
        move.w %d1,%d3
        tst.l %d0
        jbge .div_32_u16_negative
        neg.l %d3
.div_32_u16_negative:
        move.l %d3,%d0
        move.l (%sp)+,%d4
        move.l (%sp)+,%d3
        rts

/*
	.even
	.globl	div_persp
div_persp:
	move.l %d3,-(%sp)
	move.l %d4,-(%sp)
	move.l 12(%sp),%d0
	cmp.l #0x10000, %d0
	jbge .divisor_gt_16_enter

	move.l #0xffff, %d4
	move.l %d4, %d2
	divu.w %d0,%d4

	move.w %d4,%d3
	swap.w %d3
	| low / low
	clr.w %d4
	add.l %d4,%d2
	divu.w %d0,%d2
	move.w %d2,%d3
	move.l %d3,%d0
	jbra .div_persp_end

.divisor_gt_16_enter:
	move.l #0xffffffff, %d4
	move.l %d4,%d2
	move.l #0x10000, %d3
.divisor_gt_16_loop:
	lsr.l #1, %d4
	lsr.l #1, %d0
	cmp.l %d3, %d0
	jbge .divisor_gt_16_loop

	divu.w %d0,%d4
	andi.l #0xffff,%d4

	| if quotient * high is more than 16bit its too large
	move.w %d4,%d0
	mulu.w %d4,%d2
	swap.w %d2
	tst.w %d2
	jbeq .div_persp_end
	subq #1, %d0
.div_persp_end:
	move.l (%sp)+,%d4
	move.l (%sp)+,%d3
	rts
*/

.even
.globl	div_persp_combine
div_persp_combine:
	|fixme: dont need to store all dx
	move.l %d3,-(%sp)
	move.l %d4,-(%sp)
|	move.l 12(%sp),%d0
|	move.l 16(%sp),%a0
|	move.l 20(%sp),%a1
	cmp.l #0x10000, %d0
	jbge .div_persp_combine_divisor_gt_16_enter

	move.l #0xffff, %d4
	move.l %d4, %d2
	divu.w %d0,%d4

	move.w %d4,%d3
	swap.w %d3
	| low / low
	clr.w %d4
	add.l %d4,%d2
	divu.w %d0,%d2
	move.w %d2,%d3
	move.l %d3,%d0

	| full 32xu32 for i_ystep_v
	move.l (%a0),%d1
	jbge .L40
	neg.l %d1
.L40:
	| low x low
	move.w %d1,%d2
	mulu.w %d0,%d2
	| high x low
	move.l %d1,%d3
	swap.w %d3
	mulu.w %d0,%d3

	| low x high
	swap.w %d0
	move.w %d1,%d4
	mulu.w %d0,%d4
	| high x high
	swap.w %d1
	mulu.w %d0,%d1
	| restore quotient
	swap %d0

	| high x low += ( low x low ) >> 16
	clr.w %d2
	swap.w %d2
	add.l %d2,%d3

	swap.w %d1
	clr.w %d1
	| low x high += high x low
	add.l %d3,%d4
	| accum = ( ( high x high ) << 16 ) + ( low x high ) + carry
	addx.l %d4, %d1

	tst.l (%a0)
	jbge .L41
	neg.l %d1
.L41:
	move.l %d1,(%a0)

	| full 16xu32 for i_persp_u
	move.l (%a1),%d1
	jbge .L42
	neg.l %d1
.L42:
	| low x low
	move.w %d0,%d2
	mulu.w %d1,%d2
	| low x high
	move.l %d0,%d4
	swap.w %d4
	mulu.w %d1,%d4


	| accum = ( low x high ) + ( ( low x low ) >> 16 )
	clr.w %d2
	swap.w %d2
	add.l %d2,%d4

	tst.l (%a1)
	jbge .L43
	neg.l %d4
.L43:
	move.l %d4,(%a1)

	jbra .div_persp_combine_end

	.div_persp_combine_divisor_gt_16_enter:
	move.l #0xffffffff, %d4
	move.l %d4,%d2
	move.l #0x10000, %d3
	.div_persp_combine_divisor_gt_16_loop:
	lsr.l #1, %d4
	lsr.l #1, %d0
	cmp.l %d3, %d0
	jbge .div_persp_combine_divisor_gt_16_loop

	divu.w %d0,%d4
	andi.l #0xffff,%d4

	| if quotient * high is more than 16bit its too large
	move.w %d4,%d0
	mulu.w %d4,%d2
	swap.w %d2
	tst.w %d2
	jbeq .div_persp_combine_quotient_ok
	subq #1, %d0
.div_persp_combine_quotient_ok:
	| 32xu16 for ystep_v

	move.l (%a0),%d1
	jbge .L44
	neg.l %d1
.L44:
	| low x low
	move.w %d1,%d2
	mulu.w %d0,%d2
	| low x high
	swap.w %d1
	mulu.w %d0,%d1

	| accum = ( low x high ) + ( ( low x low ) >> 16 )
	clr.w %d2
	swap.w %d2
	add.l %d2,%d1

	tst.l (%a0)
	jbge .L45
	neg.l %d1
.L45:
	move.l %d1,(%a0)

	| 16xu16 for u_persp
	move.l (%a1),%d1
	jbge .L46
	neg.l %d1
	mulu.w %d0,%d1
	neg.l %d1
	jbra .L47
.L46:
	mulu.w %d0,%d1
.L47:
	clr.w %d1
	swap %d1
	move.l %d1,(%a1)

.div_persp_combine_end:
	move.l (%sp)+,%d4
	move.l (%sp)+,%d3
	rts


