; 6502 instruction set

; 6502 branch instructions

	.rule	branch	$10	bpl
	.rule	branch	$30	bmi
	.rule	branch	$50	bvc
	.rule	branch	$70	bvs
	.rule	branch	$90	bcc
	.rule	branch	$b0	bcs
	.rule	branch	$d0	bne
	.rule	branch	$f0	beq

	.insn	<branch> <expr>
	 .byte	arg1
	 .byte	arg2-.+1
	.endi

; 6502 jump instructions

	.insn	jsr	<expr>
	 .byte	$20
	 .word	arg1
	.endi

	.insn	jmp	<expr>
	 .byte	$4c
	 .word	arg1
	.endi

	.insn	jmp	(<expr>)
	 .byte	$6c
	 .word	arg1
	.endi

; 6502 No operand instructions

	.rule	simple	$18	clc
	.rule	simple	$38	sec
	.rule	simple	$58	cli
	.rule	simple	$78	sei
	.rule	simple	$98	tya
	.rule	simple	$b8	clv
	.rule	simple	$d8	cld
	.rule	simple	$f8	sed
	.rule	simple	$00	brk
	.rule	simple	$40	rti
	.rule	simple	$60	rts
	.rule	simple	$08	php
	.rule	simple	$28	plp
	.rule	simple	$48	pha
	.rule	simple	$68	pla
	.rule	simple	$88	dey
	.rule	simple	$a8	tay
	.rule	simple	$c8	iny
	.rule	simple	$e8	inx
	.rule	simple	$8a	txa
	.rule	simple	$aa	tax
	.rule	simple	$ca	dex
	.rule	simple	$ea	nop
	.rule	simple	$9a	txs
	.rule	simple	$ba	tsx

	.insn	<simple>
	 .byte	arg1
	.endi

; 6502 instructions grouped by allowable addressing mode
;      since addressing modes are so non-orthogonal, we classify
;      by each mode.

 ; Direct and zero-page
	.rule	direct	$20	bit
	.rule	direct	$80	sty
	.rule	direct	$a0	ldy
	.rule	direct	$c0	cpy
	.rule	direct	$e0	cpx
	.rule	direct	$01	ora
	.rule	direct	$21	and
	.rule	direct	$41	eor
	.rule	direct	$61	adc
	.rule	direct	$81	sta
	.rule	direct	$a1	lda
	.rule	direct	$c1	cmp
	.rule	direct	$e1	sbc
	.rule	direct	$02	asl
	.rule	direct	$22	rol
	.rule	direct	$42	lsr
	.rule	direct	$62	ror
	.rule	direct	$82	stx
	.rule	direct	$a2	ldx
	.rule	direct	$c2	dec
	.rule	direct	$e2	inc

	.insn	<direct>	<expr>
	 .if arg2<256			; Zero-page
	  .byte	arg1+%00100
	  .byte arg2
	 .else				; Extended
	  .byte	arg1+%01100
	  .word arg2
	 .endif
	.endi

 ; Accumulator inherent
	.rule	accu	$02	asl
	.rule	accu	$22	rol
	.rule	accu	$42	lsr
	.rule	accu	$62	ror

	.insn	<accu>
	 .byte	arg1+%01000
	.endi

 ; Immediate group2
	.rule	imm2	$01	ora
	.rule	imm2	$21	and
	.rule	imm2	$41	eor
	.rule	imm2	$61	adc
	.rule	imm2	$a1	lda
	.rule	imm2	$c1	cmp
	.rule	imm2	$e1	sbc

	.insn	<imm2>	#<expr>
	 .byte	arg1+%01000
	 .byte	arg2
	.endi

 ; Immediate group1
	.rule	imm1	$a0	ldy
	.rule	imm1	$c0	cpy
	.rule	imm1	$e0	cpx
	.rule	imm1	$a2	ldx

	.insn	<imm1>	#<expr>
	 .byte	arg1+%00000
	 .byte	arg2
	.endi

 ; Indirect,x
	.rule	indx	$01	ora
	.rule	indx	$21	and
	.rule	indx	$41	eor
	.rule	indx	$61	adc
	.rule	indx	$81	sta
	.rule	indx	$a1	lda
	.rule	indx	$c1	cmp
	.rule	indx	$e1	sbc

	.insn	<indx>	(<expr>,x)
	 .byte	arg1+%000000
	 .byte	arg2
	.endi

 ; Indirect,y
	.rule	indy	$01	ora
	.rule	indy	$21	and
	.rule	indy	$41	eor
	.rule	indy	$61	adc
	.rule	indy	$81	sta
	.rule	indy	$a1	lda
	.rule	indy	$c1	cmp
	.rule	indy	$e1	sbc

	.insn	<indy>	(<expr>),y
	 .byte	arg1+%10000
	 .byte	arg2
	.endi

 ; Indexed,y
	.rule	idxy	$01	ora
	.rule	idxy	$21	and
	.rule	idxy	$41	eor
	.rule	idxy	$61	adc
	.rule	idxy	$81	sta
	.rule	idxy	$a1	lda
	.rule	idxy	$c1	cmp
	.rule	idxy	$e1	sbc

	.insn	<idxy>	<expr>,y
	 .byte	arg1+%11000
	 .word	arg2
	.endi

 ; Indexed,x
	.rule	idxx	$a0	ldy
	.rule	idxx	$01	ora
	.rule	idxx	$21	and
	.rule	idxx	$41	eor
	.rule	idxx	$61	adc
	.rule	idxx	$81	sta
	.rule	idxx	$a1	lda
	.rule	idxx	$c1	cmp
	.rule	idxx	$e1	sbc
	.rule	idxx	$02	asl
	.rule	idxx	$22	rol
	.rule	idxx	$42	lsr
	.rule	idxx	$62	ror
	.rule	idxx	$c2	dec
	.rule	idxx	$e2	inc

	.insn	<idxx>	<expr>,x
	 .if	arg2<256
	  .byte	arg1+%10100
	  .byte	arg2
	 .else
	  .byte	arg1+%11100
	  .word	arg2
	 .endif
	.endi

 ; Short indexed only,x
	.rule	sidxx	$80	sty

	.insn	<sidxx> <expr>,x
	 .byte	arg1+%10100
	 .byte	arg2
	.endi

 ; Short indexed only,y
	.rule	sidxy	$82	stx

	.insn	<sidxy> <expr>,y
	 .byte	arg1+%10100
	 .byte	arg2
	.endi

 ; Indexed,y (because x is target)
	.rule	idxxy	$a2	ldx

	.insn	<idxx>	<expr>,y
	 .if	arg2<256
	  .byte	arg1+%10100
	  .byte	arg2
	 .else
	  .byte	arg1+%11100
	  .word	arg2
	 .endif
	.endi
