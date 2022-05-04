; 6805 instruction set

; .sbyte	signed byte: -128 - 127
; .ubyte	unsigned byte: 0 - 255
; .byte		signed or unsigned byte: -128 - 255

; Branch instructions

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
	.rule	cc	$28	hcc
	.rule	cc	$29	hcs
	.rule	cc	$2a	pl
	.rule	cc	$2b	mi
	.rule	cc	$2c	mc
	.rule	cc	$2d	ms
	.rule	cc	$2e	il
	.rule	cc	$2f	ih
	.rule	cc	$ad	sr

	.inst	b<cc>	<expr>
	 .byte	arg1
	 .sbyte	arg2-.+1
	.endi

; No operand instructions

	.rule	simple	$80	rti
	.rule	simple	$81	rts
	.rule	simple	$83	swi
	.rule	simple	$97	tax
	.rule	simple	$98	clc
	.rule	simple	$99	sec
	.rule	simple	$9a	cli
	.rule	simple	$9b	sei
	.rule	simple	$9c	rsp
	.rule	simple	$9d	nop
	.rule	simple	$9f	txa

	.inst	<simple>
	 .byte	arg1
	.endi

; register-memory operations

	.rule	rmi	$0	sub
	.rule	rmi	$1	cmp
	.rule	rmi	$2	sbc
	.rule	rmi	$3	cpx
	.rule	rmi	$4	and
	.rule	rmi	$5	bit
	.rule	rmi	$6	lda
	.rule	rmi	$8	eor
	.rule	rmi	$9	adc
	.rule	rmi	$a	ora
	.rule	rmi	$b	add
	.rule	rmi	$e	ldx

	.rule	rm	arg1	<rmi>
	.rule	rm	$7	sta
	.rule	rm	$c	jmp
	.rule	rm	$d	jsr
	.rule	rm	$f	stx

	.rule	<rmi>	#<expr>			; Immediate
	 .byte	arg1+$a0
	 .byte	arg2
	.endi

	.rule	<rm>	,x			; Indexed, zero offset
	 .byte	arg1+$f0
	.endi

	.rule	<rm>	<expr>,x		; Indexed
	 .if	arg2>=256
	  .byte	arg1+$d0		; 16-bit offset
	  .byte	arg2>>8
	  .byte	arg2&$ff
	 .else
	  .byte		arg1+$e0	; 8-bit offset
	  .ubyte	arg2
	 .endif
	.endi

	.rule	<rm>	<expr>			; Direct
	 .if	arg2>=256
	  .byte		arg1+$c0	; Direct
	  .byte		arg2>>8
	  .byte		arg2&$ff
	 .else
	  .byte		arg1+$b0	; Zero-page
	  .ubyte	arg2
	 .endif
	.endi

; Read-modify-write instructions

	.rule	rmw	$0	neg
	.rule	rmw	$3	com
	.rule	rmw	$4	lsr
	.rule	rmw	$6	ror
	.rule	rmw	$7	asr
	.rule	rmw	$8	asl
	.rule	rmw	$8	lsl
	.rule	rmw	$9	rol
	.rule	rmw	$a	dec
	.rule	rmw	$c	inc
	.rule	rmw	$d	tst
	.rule	rmw	$f	clr

	.rule	rrmw	$40	a
	.rule	rrmw	$50	x

	.rule	modermw	$30	<expr>		; Direct
	.rule	modermw	$60	<expr>,x	; Indexed

	.inst	<rmw><rrmw>			; Register
	 .byte	arg1+arg2
	.endi

	.inst	<rmw>	<modermw>		; Direct or indexed
	 .byte	arg1+arg2
	 .ubyte	arg3
	.endi

	.inst	<rmw>	,x			; Indexed w/ no offset
	 .byte	arg1+$70
	.endi

; Bit instructions

	.inst	bset	<expr>	<expr>
	 .byte	arg1<<1+$10	; limit arg1 0 - 7
	 .ubyte	arg2
	.endi

	.inst	bclr	<expr>	<expr>
	 .byte	arg1<<1+$11	; limit arg1 0 - 7
	 .ubyte	arg2
	.endi

	.inst	brset	<expr>	<expr>	<expr>
	 .byte	arg1<<1+$00	; limit arg1 0 - 7
	 .ubyte	arg2
	 .sbyte	arg3-.+1
	.endi

	.inst	brclr	<expr>	<expr>	<expr>
	 .byte	arg1<<1+$01	; limit arg1 0 - 7
	 .ubyte	arg2
	 .sbyte	arg3-.+1
	.endi
