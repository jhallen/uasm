; 8080 instruction set

; Condition flags

	.rule	cc	$00	nz
	.rule	cc	$08	z
	.rule	cc	$10	nc
	.rule	cc	$18	c
	.rule	cc	$20	po
	.rule	cc	$28	pe
	.rule	cc	$30	p
	.rule	cc	$38	m

; Jump on condition

	.insn	j<cc>	<expr>
	.byte	$C2+arg1
	.word	arg2
	.endi

; Call on condition

	.insn	c<cc>	<expr>
	.byte	$C4+arg1
	.word	arg2
	.endi

; Return on condition

	.insn	r<cc>
	.byte	$C0+arg1
	.endi

; No operand

	.rule	simple	$C9	ret
	.rule	simple	$E9	pchl
	.rule	simple	$E3	xthl
	.rule	simple	$F9	sphl
	.rule	simple	$FB	ei
	.rule	simple	$F3	di
	.rule	simple	$76	hlt
	.rule	simple	$00	nop
	.rule	simple	$37	stc
	.rule	simple	$3F	cmc
	.rule	simple	$2F	cma
	.rule	simple	$1F	rar
	.rule	simple	$17	ral
	.rule	simple	$0F	rrc
	.rule	simple	$07	rlc
	.rule	simple	$27	daa
	.rule	simple	$EB	xchg

	.insn	<simple>
	.byte	arg1
	.endi

; Register pairs

	.rule	rp	$00	bc
	.rule	rp	$10	de
	.rule	rp	$20	hl
	.rule	rp	$30	sp

; Registers

	.rule	r	$00	b
	.rule	r	$01	c
	.rule	r	$02	d
	.rule	r	$03	e
	.rule	r	$04	h
	.rule	r	$05	l
	.rule	r	$07	a

; Register pair operations

	.rule	rpops	$03	inx
	.rule	rpops	$C5	push
	.rule	rpops	$C1	pop
	.rule	rpops	$09	dad
	.rule	rpops	$0A	dcx
	.rule	rpops	$02	stax
	.rule	rpops	$0A	ldax

	.insn	<rpops>	<rp>
	.byte	arg1+arg2
	.endi

; accumulator - direct memory access

	.rule	direct	$22	shld
	.rule	direct	$2A	lhld
	.rule	direct	$32	sta
	.rule	direct	$3A	lda

	.insn	<direct>	<expr>
	.byte	arg1
	.word	arg2
	.endi

; Immediate

	.rule	imm	$FE	cpi
	.rule	imm	$F6	ori
	.rule	imm	$EE	xri
	.rule	imm	$E6	ani
	.rule	imm	$DE	sbi
	.rule	imm	$D6	sui
	.rule	imm	$CE	aci
	.rule	imm	$C6	adi

	.insn	<imm>	<expr>
	.byte	arg1, arg2
	.endi

; HL indirect

	.rule	mem	$BE	cmp
	.rule	mem	$B6	ora
	.rule	mem	$AE	xra
	.rule	mem	$A6	ana
	.rule	mem	$35	dcr
	.rule	mem	$34	inr
	.rule	mem	$9E	sbb
	.rule	mem	$96	sub
	.rule	mem	$8E	adc
	.rule	mem	$86	add

	.insn	<mem>	[hl]
	.byte	arg1
	.endi

; Register source

	.rule	reg	$B8	cmp
	.rule	reg	$B0	ora
	.rule	reg	$A8	xra
	.rule	reg	$A0	ana
	.rule	reg	$98	sbb
	.rule	reg	$90	sub
	.rule	reg	$88	adc
	.rule	reg	$80	add

	.insn	<reg>	<r>
	.byte	arg1+arg2
	.endi

; Register destination

	.rule	regd	$05	dcr
	.rule	regd	$04	inr

	.insn	<regd>	<r>
	.byte	arg1+arg2<<3
	.endi

; unique format instructions

	.insn	jmp	<expr>
	.byte	$C3
	.word	arg1
	.endi

	.insn	call	<expr>
	.byte	$CD
	.word	arg1
	.endi

	.insn	lxi	<rp>,<expr>
	.byte	$01+arg1
	.word	arg2
	.endi

	.insn	mvi	[hl],<expr>
	.byte	$36, arg1
	.endi

	.insn	mvi	<r>,<expr>
	.byte	$06+arg1<<3,arg2
	.endi

	.insn	mov	[hl],<r>
	.byte	$78+arg1<<3
	.endi

	.insn	mov	<r>,[hl]
	.byte	$46+arg1<<3
	.endi

	.insn	mov	<r>,<r>
	.byte	$40+arg1<<3+arg2
	.endi

	.insn	push	psw
	.byte	$F5
	.endi

	.insn	pop	psw
	.byte	$F1
	.endi

	.insn	in	<expr>
	.byte	$DB, arg1
	.endi

	.insn	out	<expr>
	.byte	$D3, arg1
	.endi

	.insn	rst	<expr>
	.byte	$C7+arg1<<3
	.endi
