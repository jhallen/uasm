; 6804 instruction set

; Memory mapped registers:
;  Accumulator is at $ff
;  X is at $80
;  Y is at $81
; Program counter is 12 bits

; .sbyte	signed byte: -128 - 127
; .ubyte	unsigned byte: 0 - 255
; .byte		signed or unsigned byte: -128 - 255

; Branch instructions

	.rule	cc	$00	bne
	.rule	cc	$20	beq
	.rule	cc	$40	bcc
	.rule	cc	$40	bhs
	.rule	cc	$60	bcs
	.rule	cc	$60	blo

	.inst	<cc>	<expr>
	 .byte	arg1+arg2+$10-.+1	; Restrict (arg2-.+1) to -16 - 15
	.endi

; Jump instructions

	.rule	jmp	$80	jsr
	.rule	jmp	$90	jmp

	.inst	<jmp>	<expr>
	 .byte	arg1+arg2>>8		; Restrict arg2 to -4096 - 4095
	 .byte	arg2&$ff
	.endi

; No operand instructions

	.rule	simple	$b2	rti
	.rule	simple	$b3	rts
	.rule	simple	$b4	coma
	.rule	simple	$b5	rola

	.inst	<simple>
	 .byte	arg1
	.endi

; register-memory operations

	.rule	rmi	$0	lda
	.rule	rmi	$2	add
	.rule	rmi	$3	sub
	.rule	rmi	$4	cmp
	.rule	rmi	$5	and

	.rule	rm	arg1	<rmi>
	.rule	rm	$1	sta
	.rule	rm	$6	inc
	.rule	rm	$7	dec

	.rule	<rmi>	#<expr>			; Immediate
	 .byte	arg1+$e8
	 .byte	arg2
	.endi

	.rule	<rm>	,x			; 0,x
	 .byte	arg1+$e0
	.endi

	.rule	<rm>	,y			; 0,y
	 .byte	arg1+$f0
	.endi

	.rule	<rm>	<expr>			; Direct
	 .byte	arg1+$f8
	 .ubyte	arg2
	.endi

; Short instructions

	.rule	short	$a8	inc
	.rule	short	$b8	dec
	.rule	short	$ac	lda
	.rule	short	$bc	sta

	.rule	<short>	<expr>
	 .byte	arg1+arg2-$80		; arg2 short be limited to $80 - $83
	.endi

; Bit instructions

	.inst	bset	<expr>	<expr>
	 .byte	arg1+$d8	; arg1 limit to 0 - 7
	 .ubyte	arg2
	.endi

	.inst	bclr	<expr>	<expr>
	 .byte	arg1+$d0	; arg1 limit to 0 - 7
	 .ubyte	arg2
	.endi

	.inst	brset	<expr>	<expr>	<expr>
	 .byte	arg1+$c8	; arg1 limit to 0 - 7
	 .ubyte	arg2
	 .sbyte	arg3-.+1
	.endi

	.inst	brclr	<expr>	<expr>	<expr>
	 .byte	arg1+$c0	; arg1 limit to 0 - 7
	 .ubyte	arg2
	 .sbyte	arg3-.+1
	.endi

; MVI instruction

	.inst	mvi	<expr>	#<expr>
	 .byte	$b0
	 .ubyte	arg1
	 .byte	arg2
	.endi

; Derived instructions

	.inst	asla
	 add	$ff
	.endi

	.inst	lsla
	 add	$ff
	.endi

	.inst	clra
	 sub	$ff
	.endi

	.inst	clrx
	 mvi	$80	#0
	.endi

	.inst	clry
	 mvi	$81	#0
	.endi

	.inst	deca
	 dec	$ff
	.endi

	.inst	decx
	 dec	$80
	.endi

	.inst	decy
	 dec	$81
	.endi

	.inst	inca
	 inc	$ff
	.endi

	.inst	incx
	 inc	$80
	.endi

	.inst	incy
	 inc	$81
	.endi

	.inst	tax
	 sta	$80
	.endi

	.inst	tay
	 sta	$81
	.endi

	.inst	txa
	 lda	$80
	.endi

	.inst	tya
	 lda	$81
	.endi

	.inst	ldx	#<expr>
	 mvi	$80	#arg1
	.endi

	.inst	ldy	#<expr>
	 mvi	$81	#arg1
	.endi
