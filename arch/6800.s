; 6800/6802 instruction set

; .sbyte	signed byte: -128 - 127
; .ubyte	unsigned byte: 0 - 255
; .byte		signed or unsigned byte: -128 - 255

	.foreach	.byte,a
	 .emit	a
	.end

	.foreach	.sbyte,a
	 .errif a>127 || a<-128, "out of range"
	 .emit a
	.end

	.foreach	.ubyte,a
	 .errif	a>255 || a<0, "out of range"
	 .emit a
	.end

	.foreach	.word,a
	 .emit	a>>8
	 .emit	a
	.end

; Branch instructions

	.rule	cc	$20	ra
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

	.insn	b<cc>	<expr>
	 .byte	arg1
	 .sbyte	arg2-.+1
	.end

; No operand instructions

	.rule	simple	$01	nop
	.rule	simple	$06	tap
	.rule	simple	$07	tpa
	.rule	simple	$08	inx
	.rule	simple	$09	dex
	.rule	simple	$0a	clv
	.rule	simple	$0b	sev
	.rule	simple	$0c	clc
	.rule	simple	$0d	sec
	.rule	simple	$0e	cli
	.rule	simple	$0f	sei
	.rule	simple	$10	sba
	.rule	simple	$11	cba
	.rule	simple	$16	tab
	.rule	simple	$17	tba
	.rule	simple	$19	daa
	.rule	simple	$1b	aba
	.rule	simple	$30	tsx
	.rule	simple	$31	ins
	.rule	simple	$32	pula
	.rule	simple	$33	pulb
	.rule	simple	$34	des
	.rule	simple	$35	txs
	.rule	simple	$36	psha
	.rule	simple	$37	pshb
	.rule	simple	$39	rts
	.rule	simple	$3b	rti
	.rule	simple	$3e	wai
	.rule	simple	$3f	swi

	.insn	<simple>
	 .byte	arg1
	.end

; accumulator-memory operations

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

	.insn	<rmi><r>	#<expr>
	 .byte	arg1+arg2+$00
	 .byte	arg3
	.end

	.insn	<rm><r>		<expr>
	 .if	arg3>=256
	  .byte		arg1+arg2+$30		; Extended
	  .byte		arg3>>8
	  .byte		arg3&$ff
	 .else
	  .byte		arg1+arg2+$10		; Zero-page
	  .ubyte	arg3
	 .end
	.end

	.insn	<rm><r>		<expr>,x	; Indexed
	 .byte	arg1+arg2+$20
	 .ubyte	arg3
	.end

; Read-modify-write

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

	.insn	<rmwr><racc>			; Register
	 .byte	arg1+arg2
	.end

	.insn	<rmw>	<expr>,x		; Indexed
	 .byte	arg1+$60
	 .ubyte	arg2
	.end

	.insn	<rmw>	<expr>			; Extended
	 .byte	arg1+$70
	 .byte	arg2>>8
	 .byte	arg2&$ff
	.end

; index register instructions

	.rule	indexi	$8c	cpx
	.rule	indexi	$8e	lds
	.rule	indexi	$ce	ldx

	.rule	index	arg1	<indexi>
	.rule	index	$8f	sts
	.rule	index	$cf	stx

	.insn	<indexi>	#<expr>		; Immediate
	 .byte		arg1+$00
	 .word		arg2
	.end

	.insn	<index>	<expr>,x		; Indexed
	 .byte		arg1+$20
	 .ubyte		arg2
	.end

	.insn	<index>	<expr>
	 .if	arg2>=256
	  .byte		arg1+$30		; Extended
	  .word		arg2
	 .else
	  .byte		arg1+$10		; Zero-page
	  .ubyte	arg2
	 .end
	.end

; JSR instructions

	.insn	jsr	<expr>,x
	 .byte	$ad
	 .ubyte	arg1
	.end

	.insn	jsr	<expr>
	 .byte	$bd
	 .word	arg1
	.end
