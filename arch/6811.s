; 6811 instruction set

; .sbyte	signed byte: -128 - 127
; .ubyte	unsigned byte: 0 - 255
; .byte		signed or unsigned byte: -128 - 255

; 6811 Branch instructions

	.rule	cc	$20	ra
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
	.rule	cc	$8d	sr

	.inst	b<cc>	<expr>
	 .byte	arg1
	 .sbyte	arg2-.+1
	.endi

; 6811 No operand instructions

	.rule	simple	$00,0	test
	.rule	simple	$01,0	nop
	.rule	simple	$02,0	idiv
	.rule	simple	$03,0	fdiv
	.rule	simple	$04,0	lsrd
	.rule	simple	$05,0	asld
	.rule	simple	$05,0	lsld
	.rule	simple	$06,0	tap
	.rule	simple	$07,0	tpa
	.rule	simple	$08,0	inx
	.rule	simple	$08,$18	iny
	.rule	simple	$09,0	dex
	.rule	simple	$09,$18	dey
	.rule	simple	$0a,0	clv
	.rule	simple	$0b,0	sev
	.rule	simple	$0c,0	clc
	.rule	simple	$0d,0	sec
	.rule	simple	$0e,0	cli
	.rule	simple	$0f,0	sei
	.rule	simple	$10,0	sba
	.rule	simple	$11,0	cba
	.rule	simple	$16,0	tab
	.rule	simple	$17,0	tba
	.rule	simple	$19,0	daa
	.rule	simple	$1b,0	aba
	.rule	simple	$30,0	tsx
	.rule	simple	$30,$18	tsy
	.rule	simple	$31,0	ins
	.rule	simple	$32,0	pula
	.rule	simple	$33,0	pulb
	.rule	simple	$34,0	des
	.rule	simple	$35,0	txs
	.rule	simple	$35,$18	tys
	.rule	simple	$36,0	psha
	.rule	simple	$37,0	pshb
	.rule	simple	$38,0	pulx
	.rule	simple	$38,$18	puly
	.rule	simple	$39,0	rts
	.rule	simple	$3a,0	abx
	.rule	simple	$3a,$18	aby
	.rule	simple	$3b,0	rti
	.rule	simple	$3c,0	pshx
	.rule	simple	$3c,$18	pshy
	.rule	simple	$3d,0	mul
	.rule	simple	$3e,0	wai
	.rule	simple	$3f,0	swi
	.rule	simple	$cf,0	stop
	.rule	simple	$8f,0	xgdx
	.rule	simple	$8f,$18	xgdy

	.inst	<simple>
	 .if arg2
	  .byte arg2
	 .endif
	 .byte	arg1
	.endi

; 6811 accumulator-memory operations

	.rule	rmi	$0	sub
	.rule	rmi	$1	cmp
	.rule	rmi	$2	sbc
	.rule	rmi	$4	and
	.rule	rmi	$5	bit
	.rule	rmi	$6	lda
	.rule	rmi	$8	eor
	.rule	rmi	$9	adc
	.rule	rmi	$a	ora
	.rule	rmi	$b	add

	.rule	rm	arg1	<rmi>
	.rule	rm	$7	sta

	.rule	r	$80	a
	.rule	r	$c0	b

	.inst	<rmi><r>	#<expr>		; Immediate
	 .byte	arg1+arg2+$00
	 .byte	arg3
	.endi

	.inst	<rm><r>		<expr>
	 .if	arg3>=256
	  .byte		arg1+arg2+$30		; Extended
	  .byte		arg3>>8
	  .byte		arg3&$ff
	 .else
	  .byte		arg1+arg2+$10		; Zero-page
	  .ubyte	arg3
	 .endif
	.endi

	.inst	<rm><r>		<expr>,x	; Indexed,x
	 .byte	arg1+arg2+$20
	 .ubyte	arg3
	.endi

	.inst	<rm><r>		<expr>,y	; Indexed,y
	 .byte	$18
	 .byte	arg1+arg2+$20
	 .ubyte	arg3
	.endi

; 6811 Read-modify-write instructions

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

	.inst	<rmwr><racc>			; Register
	 .byte	arg1+arg2
	.endi

	.inst	<rmw>	<expr>,x		; Indexed
	 .byte	arg1+$60
	 .ubyte	arg2
	.endi

	.inst	<rmw>	<expr>			; Extended
	 .byte	arg1+$70
	 .byte	arg2>>8
	 .byte	arg2&$ff
	.endi

; 6811 16-bit instructions

; instructions which can use immediate, indexed, zero-page, and extended modes.
;            Prefix code:   -  ,x  ,y
	.rule	indexi	$83,$00,$00,$18	subd
	.rule	indexi	$83,$1A,$1A,$CD	cpd
	.rule	indexi	$8c,$00,$00,$CD	cpx
	.rule	indexi	$8c,$18,$1A,$18	cpy
	.rule	indexi	$8e,$00,$00,$18	lds
	.rule	indexi	$c3,$00,$00,$18	addd
	.rule	indexi	$cc,$00,$00,$18	ldd
	.rule	indexi	$ce,$00,$00,$CD	ldx
	.rule	indexi	$ce,$18,$1A,$18	ldy

; instructions which can use indexed, zero-page and extended (but not
; immediate) modes.
	.rule	index	arg1	<indexi>
	.rule	index	$8d,$00,$00,$18	jsr
	.rule	index	$8f,$00,$00,$18	sts
	.rule	index	$cd,$00,$00,$18	std
	.rule	index	$cf,$00,$00,$CD	stx
	.rule	index	$cf,$18,$1A,$18	sty

	.inst	<indexi>	#<expr>		; Immediate
	 .if arg2
	  .byte	arg2
	 .endif
	 .byte		arg1+$00
	 .word		arg5
	.endi

	.inst	<index>		<expr>,x	; Indexed,x
	 .if arg3
	  .byte	arg3
	 .endif
	 .byte		arg1+$20
	 .ubyte		arg5
	.endi

	.inst	<index>		<expr>,y	; Indexed,y
	 .if arg4
	  .byte arg4
	 .endif
	 .byte		arg1+$20
	 .ubyte		arg5
	.endi

	.inst	<index>		<expr>
	 .if arg2
	  .byte arg2
	 .endif
	 .if	arg5>=256
	  .byte		arg1+$30	; Extended
	  .word		arg5
	 .else
	  .byte		arg1+$10	; Zero-page
	  .ubyte	arg5
	 .endif
	.endi

; 6811 bit instructions

	.rule	bit	$14	bset
	.rule	bit	$15	bclr

	.rule	brbit	$12	brset
	.rule	brbit	$13	brclr

	.inst	<bit>	<expr> #<expr>		; Direct
	 .byte	arg1
	 .byte	arg2
	 .byte	arg3
	.endi

	.inst	<bit>	<expr>,x #<expr>	; ,X
	 .byte	arg1+8
	 .byte	arg2
	 .byte	arg3
	.endi

	.inst	<bit>	<expr>,y #<expr>	; ,Y
	 .byte	$18
	 .byte	arg1+8
	 .byte	arg2
	 .byte	arg3
	.endi

	.inst	<brbit>	<expr> #<expr> <expr>	; Direct
	 .byte	arg1
	 .byte	arg2
	 .byte	arg3
	 .byte	arg4-.+1
	.endi

	.inst	<brbit>	<expr>,x #<expr> <expr>	; ,X
	 .byte	arg1+12
	 .byte	arg2
	 .byte	arg3
	 .byte	arg4-.+1
	.endi

	.inst	<brbit>	<expr>,y #<expr> <expr>	; ,Y
	 .byte	$18
	 .byte	arg1+12
	 .byte	arg2
	 .byte	arg3
	 .byte	arg4-.+1
	.endi
