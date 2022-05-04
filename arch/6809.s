; 6809 instruction set

; need to handle pc relative offsets properly.

; .sbyte	signed byte: -128 - 127
; .ubyte	unsigned byte: 0 - 255
; .byte		signed or unsigned byte: -128 - 255

; 6809 Direct page value control

	.inst	dp	<expr>
	 .errif	arg1&255	"Invalid direct page value"
	 .set	dp,arg1
	.endi

	dp	0		; Default value is zero

; 6809 Branch instructions

	.rule	cc	$21	rn
	.rule	cc	$22	hi
	.rule	cc	$23	ls
	.rule	cc	$24	cc
	.rule	cc	$24	hs
	.rule	cc	$25	cs
	.rule	cc	$25	lo
	.rule	cc	$26	ne
	.rule	cc	$27	eq
	.rule	cc	$28	vc
	.rule	cc	$29	vs
	.rule	cc	$2a	pl
	.rule	cc	$2b	mi
	.rule	cc	$2c	ge
	.rule	cc	$2d	lt
	.rule	cc	$2e	gt
	.rule	cc	$2f	le

 ; short branches

	.inst	b<cc>	<expr>
	 .byte	arg1
	 .sbyte	arg2-.+1
	.endi

	.inst	bsr	<expr>
	 .byte	$8d
	 .sbyte	arg1-.+1
	.endi

	.inst	bra	<expr>
	 .byte	$20
	 .sbyte	arg1-.+1
	.endi

 ; long branches

	.inst	lb<cc>	<expr>
	 .byte	$10
	 .byte	arg1
	 .word	arg2-.+2
	.endi

	.inst	lbsr	<expr>
	 .byte	$17
	 .word	arg1-.+2
	.endi

	.inst	lbra	<expr>
	 .byte	$16
	 .word	arg1-.+2
	.endi

; 6809 No operand instructions

	.rule	simple	$12	nop
	.rule	simple	$13	sync
	.rule	simple	$19	daa
	.rule	simple	$1d	sex
	.rule	simple	$39	rts
	.rule	simple	$3a	abx
	.rule	simple	$3b	rti
	.rule	simple	$3c	cwai
	.rule	simple	$3d	mul
	.rule	simple	$3f	swi

	.inst	<simple>
	 .byte	arg1
	.endi

	.inst	swi3
	 .byte	$11
	 .byte	$3f
	.endi

	.inst	swi2
	 .byte	$10
	 .byte	$3f
	.endi

; 6809 andcc and orcc instructions

	.rule	ccop	$1c	andcc
	.rule	ccop	$1a	orcc

	.inst	<ccop>	#<expr>
	 .byte	arg1
	 .byte	arg2
	.endi

; 6809 push and pop instructions.  These require a register list.

 ; Register arguments

	.rule	pr	$01	cc
	.rule	pr	$02	a
	.rule	pr	$04	b
	.rule	pr	$06	d
	.rule	pr	$08	dp
	.rule	pr	$10	x
	.rule	pr	$20	y
	.rule	pr	$40	s
	.rule	pr	$40	u
	.rule	pr	$80	pc

	.rule	reglist	arg1					<pr>
	.rule	reglist	arg1|arg2				<pr>,<pr>
	.rule	reglist	arg1|arg2|arg3				<pr>,<pr>,<pr>
	.rule	reglist	arg1|arg2|arg3|arg4			<pr>,<pr>,<pr>,<pr>
	.rule	reglist	arg1|arg2|arg3|arg4|arg5		<pr>,<pr>,<pr>,<pr>,<pr>
	.rule	reglist	arg1|arg2|arg3|arg4|arg5|arg6		<pr>,<pr>,<pr>,<pr>,<pr>,<pr>
	.rule	reglist	arg1|arg2|arg3|arg4|arg5|arg6|arg7	<pr>,<pr>,<pr>,<pr>,<pr>,<pr>,<pr>
	.rule	reglist	arg1|arg2|arg3|arg4|arg5|arg6|arg7|arg8	<pr>,<pr>,<pr>,<pr>,<pr>,<pr>,<pr>,<pr>

 ; OP codes
	.rule	pshpop	$34	pshs
	.rule	pshpop	$35	puls
	.rule	pshpop	$36	pshu
	.rule	pshpop	$37	pulu

 ; instruction
	.inst	<pshpop>	<reglist>
	 .byte	arg1
	 .byte	arg2
	.endi

; 6809 TFR and EXG instructions.  These require two register arguments.

	.rule	tr	$0	d
	.rule	tr	$1	x
	.rule	tr	$2	y
	.rule	tr	$3	u
	.rule	tr	$4	s
	.rule	tr	$5	pc
	.rule	tr	$8	a
	.rule	tr	$9	b
	.rule	tr	$a	cc
	.rule	tr	$b	dp

	.rule	tfrexg	$1f	tfr
	.rule	tfrexg	$1e	exg

	.inst	<tfrexg>	<tr>,<tr>	; TFR source,dest
	 .byte	arg1
	 .byte	arg2<<4+arg3
	.endi

; 6809 accumulator-memory operations

; operations allowed for immediate, direct-page, indexed, and extended modes.

	.rule	rmi	$0	sub
	.rule	rmi	$1	cmp
	.rule	rmi	$2	sbc
	.rule	rmi	$4	and
	.rule	rmi	$5	bit
	.rule	rmi	$6	ld
	.rule	rmi	$8	eor
	.rule	rmi	$9	adc
	.rule	rmi	$a	or
	.rule	rmi	$b	add

; operations allowable for direct-page, indexed, and extended (but
; not immediate) modes.

	.rule	rm	arg1	<rmi>
	.rule	rm	$7	st

; Accumulator selector

	.rule	r	$80	a
	.rule	r	$c0	b

; Index register

	.rule	rr	$00	x
	.rule	rr	$20	y
	.rule	rr	$40	u
	.rule	rr	$60	s

; '09 index mode byte.  Middle value of return list indicates how to handle
; offset value and how to modify index mode byte for the size of the offset:
;    0 - no offset.
;    1 - 8-bit or 16-bit (add 1) offset.
;    2 - 16-bit extended address.
;    3 - 5-bit (clear high bit), 8-bit, or 16-bit (add 1) offset.

	.rule	ndx	$80+arg1,0,0		,<rr>+
	.rule	ndx	$81+arg1,0,0		,<rr>++
	.rule	ndx	$91+arg1,0,0		[,<rr>++]
	.rule	ndx	$82+arg1,0,0		,-<rr>
	.rule	ndx	$83+arg1,0,0		,--<rr>
	.rule	ndx	$93+arg1,0,0		[,--<rr>]
	.rule	ndx	$84+arg1,0,0		,<rr>
	.rule	ndx	$94+arg1,0,0		[,<rr>]
	.rule	ndx	$85+arg1,0,0		a,<rr>
	.rule	ndx	$95+arg1,0,0		[a,<rr>]
	.rule	ndx	$86+arg1,0,0		b,<rr>
	.rule	ndx	$96+arg1,0,0		[b,<rr>]
	.rule	ndx	$88+arg2,3,arg1		<expr>,<rr>
	.rule	ndx	$98+arg2,1,arg1		[<expr>,<rr>]
	.rule	ndx	$8b+arg1,0,0		d,<rr>
	.rule	ndx	$9b+arg1,0,0		[d,<rr>]
	.rule	ndx	$8c,1,arg1		<expr>,pc
	.rule	ndx	$9c,1,arg1		[<expr>,pc]
	.rule	ndx	$9f,2,arg1		[<expr>]

	.inst	<rmi><r>	#<expr>		; Immediate
	 .byte	arg1+arg2+$00
	 .byte	arg3
	.endi

	.inst	<rm><r>		<expr>
	 .if	arg3-dp>=256 || arg3-dp<0
	  .byte		arg1+arg2+$30		; Extended
	  .byte		arg3>>8
	  .byte		arg3&$ff
	 .else
	  .byte		arg1+arg2+$10		; Direct page
	  .ubyte	arg3
	 .endif
	.endi

	.inst	<rm><r>		<ndx>		; Indexed
	 .byte	arg1+arg2+$20
	 .if arg4==0		; No offset
	  .ubyte arg3
	 .elseif arg4==1	; 8- or 16-bit offset
	  .if arg5<128 && arg5>=-128		; 8-bit
	   .ubyte arg3
	   .byte arg5
	  .else					; 16-bit
	   .ubyte arg3+1
	   .byte arg5>>8
	   .byte arg5
	  .endif
	 .elseif arg4==2	; 16-bit extended address
	  .ubyte arg3
	  .byte	arg5>>8
	  .byte	arg5
	 .else			; 5-, 8- or 16-bit offset
 	  .if arg5<16 && arg5>=-16		; 5-bit
	   .ubyte arg3&$60+arg5&$1f
	  .elseif arg5<128 && arg5>=-128	; 8-bit
	   .ubyte arg3
	   .byte arg5
	  .else					; 16-bit
	   .ubyte arg3+1
	   .byte arg5>>8
	   .byte arg5
	  .endif
	 .endif
	.endi

; 6809 Read-modify-write instructions

	.rule	rmwr	$0	neg
	.rule	rmwr	$3	com
	.rule	rmwr	$4	lsr
	.rule	rmwr	$6	ror
	.rule	rmwr	$7	asr
	.rule	rmwr	$8	asl
	.rule	rmwr	$8	lsl
	.rule	rmwr	$9	rol
	.rule	rmwr	$a	dec
	.rule	rmwr	$c	inc
	.rule	rmwr	$d	tst
	.rule	rmwr	$f	clr

	.rule	rmw	arg1	rmwr
	.rule	rmw	$e	jmp

	.rule	racc	$40	a
	.rule	racc	$50	b

	.inst	<rmwr><racc>			; accumulator
	 .byte	arg1+arg2
	.endi

	.inst	<rmw>	<expr>
	 .if arg2-dp>=0 && arg2-dp<256
	  .byte		arg1+$00		; Direct-page
	  .ubyte	arg2-dp
	 .else
	  .byte		arg1+$70		; Extended
	  .byte		arg2>>8
	  .byte		arg2&$ff
	 .endif
	.endi

	.inst	<rmw>	<ndx>		; Indexed
	 .byte	arg1+$60
	 .if arg3==0		; No offset
	  .ubyte arg2
	 .elseif arg3==1	; 8- or 16-bit offset
	  .if arg4<128 && arg4>=-128		; 8-bit
	   .ubyte arg2
	   .byte arg4
	  .else					; 16-bit
	   .ubyte arg2+1
	   .byte arg4>>8
	   .byte arg4
	  .endif
	 .elseif arg3==2	; 16-bit extended address
	  .ubyte arg2
	  .byte	arg4>>8
	  .byte	arg4
	 .else			; 5-, 8- or 16-bit offset
 	  .if arg4<16 && arg4>=-16		; 5-bit
	   .ubyte arg2&$60+arg4&$1f
	  .elseif arg4<128 && arg4>=-128	; 8-bit
	   .ubyte arg3
	   .byte arg4
	  .else					; 16-bit
	   .ubyte arg2+1
	   .byte arg4>>8
	   .byte arg4
	  .endif
	 .endif
	.endi

; 6809 LEA instructions

	.rule	lea	$30	leax
	.rule	lea	$31	leay
	.rule	lea	$32	leas
	.rule	lea	$33	leau

	.inst	<lea>	<ndx>		; Indexed
	 .byte	arg1
	 .if arg3==0		; No offset
	  .ubyte arg2
	 .elseif arg3==1	; 8- or 16-bit offset
	  .if arg4<128 && arg4>=-128		; 8-bit
	   .ubyte arg2
	   .byte arg4
	  .else					; 16-bit
	   .ubyte arg2+1
	   .byte arg4>>8
	   .byte arg4
	  .endif
	 .elseif arg3==2	; 16-bit extended address
	  .ubyte arg2
	  .byte	arg4>>8
	  .byte	arg4
	 .else			; 5-, 8- or 16-bit offset
 	  .if arg4<16 && arg4>=-16		; 5-bit
	   .ubyte arg2&$60+arg4&$1f
	  .elseif arg4<128 && arg4>=-128	; 8-bit
	   .ubyte arg3
	   .byte arg4
	  .else					; 16-bit
	   .ubyte arg2+1
	   .byte arg4>>8
	   .byte arg4
	  .endif
	 .endif
	.endi

; 6809 16-bit instructions

 ; Allowed for immediate mode
	.rule	indexi	$83,0	subd
	.rule	indexi	$83,$10	cmpd
	.rule	indexi	$83,$11	cmpu
	.rule	indexi	$8c,0	cmpx
	.rule	indexi	$8c,$10	cmpy
	.rule	indexi	$8c,$11	cmps
	.rule	indexi	$8e,0	ldx
	.rule	indexi	$8e,$10	ldy
	.rule	indexi	$c3,0	addd
	.rule	indexi	$cc,0	ldd
	.rule	indexi	$ce,0	ldu
	.rule	indexi	$ce,$10	lds

 ; Not allowed for immediate mode
	.rule	index	arg1	<indexi>
	.rule	index	$8d,0	jsr
	.rule	index	$8f,0	stx
	.rule	index	$8f,$10	sty
	.rule	index	$cd,0	std
	.rule	index	$cf,0	stu
	.rule	index	$cf,$10	sts

	.inst	<indexi>	#<expr>		; Immediate
	 .if arg2		; Prefix byte
	  .byte arg2
	 .endif
	 .byte		arg1+$00
	 .word		arg2
	.endi

	.inst	<index>	<ndx>		; Indexed
	 .if arg2		; Prefix byte
	  .byte arg2
	 .endif
	 .byte	arg1+$20	; OP-code
	 .if arg4==0		; No offset
	  .ubyte arg3
	 .elseif arg4==1	; 8- or 16-bit offset
	  .if arg5<128 && arg5>=-128		; 8-bit
	   .ubyte arg3
	   .byte arg5
	  .else					; 16-bit
	   .ubyte arg3+1
	   .byte arg5>>8
	   .byte arg5
	  .endif
	 .elseif arg4==2	; 16-bit extended address
	  .ubyte arg3
	  .byte	arg5>>8
	  .byte	arg5
	 .else			; 5-, 8- or 16-bit offset
 	  .if arg5<16 && arg5>=-16		; 5-bit
	   .ubyte arg3&$60+arg5&$1f
	  .elseif arg5<128 && arg5>=-128	; 8-bit
	   .ubyte arg3
	   .byte arg5
	  .else					; 16-bit
	   .ubyte arg3+1
	   .byte arg5>>8
	   .byte arg5
	  .endif
	 .endif
	.endi

	.inst	<index>	<expr>
	 .if arg2		; Prefix byte
	  .byte arg2
	 .endif
	 .if	arg3-dp>=256
	  .byte		arg1+$30		; Extended
	  .word		arg3
	 .else
	  .byte		arg1+$10		; Zero-page
	  .ubyte	arg3
	 .endif
	.endi
