; 8086 Instruction set

; need mov, jmp and call instructions

; Register fields

 ; 16-bit reg-field values

	.rule	reg16	%000	ax
	.rule	reg16	%001	cx
	.rule	reg16	%010	dx
	.rule	reg16	%011	bx
	.rule	reg16	%100	sp
	.rule	reg16	%101	bp
	.rule	reg16	%110	si
	.rule	reg16	%111	di

 ; 8-bit reg-field values

	.rule	reg8	%000	al
	.rule	reg8	%001	cl
	.rule	reg8	%010	dl
	.rule	reg8	%011	bl
	.rule	reg8	%100	ah
	.rule	reg8	%101	ch
	.rule	reg8	%110	dh
	.rule	reg8	%111	bh

 ; Segment register field values

	.rule	seg	%00	es
	.rule	seg	%01	cs
	.rule	seg	%10	ss
	.rule	seg	%11	ds

; 8086 address mode byte

	.rule	mode	%00000000,0,0		[bx+si]
	.rule	mode	%00000001,0,0		[bx+di]
	.rule	mode	%00000010,0,0		[bp+si]
	.rule	mode	%00000011,0,0		[bp+di]
	.rule	mode	%00000100,0,0		[si]
	.rule	mode	%00000101,0,0		[di]
	.rule	mode	%01000110,1,0		[bp]
	.rule	mode	%00000110,2,arg1	<expr>
	.rule	mode	%00000111,0,0		[bx]
	.rule	mode	%01000000,1,arg1	[bx+si+<expr>]
	.rule	mode	%01000001,1,arg1	[bx+di+<expr>]
	.rule	mode	%01000010,1,arg1	[bp+si+<expr>]
	.rule	mode	%01000011,1,arg1	[bp+di+<expr>]
	.rule	mode	%01000100,1,arg1	[si+<expr>]
	.rule	mode	%01000101,1,arg1	[di+<expr>]
	.rule	mode	%01000110,1,arg1	[bp+<expr>]
	.rule	mode	%01000111,1,arg1	[bx+<expr>]
	.rule	mode	%11000000+arg1,0,0	<reg16>
	.rule	mode	%11000000+arg1,0,0	<reg8>

; Macro to generate addressing mode stuff

	.macro	reg,mode,type,offset
	 .if type==1		; If there is an offset
	  .if offset<256		; Small offset
	   .byte mode|reg
	   .byte offset
	  .else
	   .byte mode^%11000000|reg	; Large offset
	   .word offset
	  .endif
	 .elseif arg3==2	; If there is a direct address
	  .byte	mode|reg
	  .word	offset
	 .else			; If there is no offset
	  .byte	mode|reg
	 .endif
	.endm

; 8086 no operand instructions

	.rule	simple	%00110111		aaa
	.rule	simple	%00111111		aas
	.rule	simple	%10011000		cbw
	.rule	simple	%11111000		clc
	.rule	simple	%11111100		cld
	.rule	simple	%11111010		cli
	.rule	simple	%11110101		cmc
	.rule	simple	%10100110		cmpsb
	.rule	simple	%10100111		cmpsw
	.rule	simple	%10011001		cwd
	.rule	simple	%00100111		daa
	.rule	simple	%00101111		das
	.rule	simple	%11110100		hlt
	.rule	simple	%11001100		int3
	.rule	simple	%11001110		into
	.rule	simple	%11001111		iret
	.rule	simple	%10011111		lahf
	.rule	simple	%11110000		lock
	.rule	simple	%10101100		lodsb
	.rule	simple	%10101101		lodsw
	.rule	simple	%10100100		movsb
	.rule	simple	%10100101		movsw
	.rule	simple	%10010000		nop
	.rule	simple	%10011100		popf
	.rule	simple	%10011101		pushf
	.rule	simple	%10011110		sahf
	.rule	simple	%10101110		scasb
	.rule	simple	%10101111		scasw
	.rule	simple	%11111001		stc
	.rule	simple	%11111101		std
	.rule	simple	%11111011		sti
	.rule	simple	%10101010		stosb
	.rule	simple	%10101011		stosw
	.rule	simple	%10011011		wait
	.rule	simple	%11010111		xlat
	.rule	simple	%11000011		retf
	.rule	simple	%11001011		retn
	.rule	simple	%11110011		rep
	.rule	simple	%11110011		repe
	.rule	simple	%11110011		repz
	.rule	simple	%11110010		repne
	.rule	simple	%11110010		repnz

	.insn	<simple>
	 .byte	arg1
	.endi

	.insn	aad
	 .byte	%11010101
	 .byte	%00001010
	.endi

	.insn	aam
	 .byte	%11010100
	 .byte	%00001010
	.endi

; 8086 branch instructions

	.rule	branch	%01110111		ja
	.rule	branch	%01110111		jnbe
	.rule	branch	%01110011		jae
	.rule	branch	%01110011		jnb
	.rule	branch	%01110010		jb
	.rule	branch	%01110010		jnae
	.rule	branch	%01110110		jbe
	.rule	branch	%01110110		jna
	.rule	branch	%01110010		jc
	.rule	branch	%11100011		jcxz
	.rule	branch	%01110100		je
	.rule	branch	%01110100		jz
	.rule	branch	%01111111		jg
	.rule	branch	%01111111		jnle
	.rule	branch	%01111101		jge
	.rule	branch	%01111101		jnl
	.rule	branch	%01111100		jl
	.rule	branch	%01111100		jnge
	.rule	branch	%01111110		jle
	.rule	branch	%01111110		jng
	.rule	branch	%01110011		jnc
	.rule	branch	%01110101		jne
	.rule	branch	%01110101		jnz
	.rule	branch	%01110001		jno
	.rule	branch	%01111001		jns
	.rule	branch	%01111011		jnp
	.rule	branch	%01111011		jpo
	.rule	branch	%01110000		jo
	.rule	branch	%01111010		jp
	.rule	branch	%01111010		jpe
	.rule	branch	%01111000		js
	.rule	branch	%11100010		loop
	.rule	branch	%11100001		loope
	.rule	branch	%11100001		loopz
	.rule	branch	%11100000		loopnz
	.rule	branch	%11100000		loopne

	.insn	<branch>	<expr>
	 .byte	arg1
	 .byte	arg2-.+1
	.endi

; 8086 Accumulator/immediate instructions

	.rule	acci	%00010100	adc
	.rule	acci	%00000100	add
	.rule	acci	%00100100	and
	.rule	acci	%00111100	cmp
	.rule	acci	%00001100	or
	.rule	acci	%00011100	sbb
	.rule	acci	%00101100	sub
	.rule	acci	%00110100	xor
	.rule	acci	%10101000	test

	.insn	<acci>	al,#<expr>
	 .byte	arg1
	 .byte	arg2
	.endi

	.insn	<acci>	ax,#<expr>
	 .byte	arg1+1
	 .word	arg2
	.endi

; 8086 reversible reg,memory instructions

	.rule	regm	%00010000	adc
	.rule	regm	%00000000	add
	.rule	regm	%00100000	and
	.rule	regm	%00111000	cmp
	.rule	regm	%00001000	or
	.rule	regm	%00011000	sbb
	.rule	regm	%00101000	sub
	.rule	regm	%00110000	xor

; 8086 reg,memory instructions

	.rule	regm	%10000100	test

; 8086 memory,immediate instructions

	.rule	memi	%10000000,%010	adc
	.rule	memi	%10000000,%000	add
	.rule	memi	%10000000,%100	and
	.rule	memi	%10000000,%111	cmp
	.rule	memi	%10000000,%001	or
	.rule	memi	%10000000,%110	xor
	.rule	memi	%11110110,%000	test

; 8086 memory,immediate instructions which have sign extension

	.rule	memi	%10000000,%011	sbb
	.rule	memi	%10000000,%101	sub

; 8086 RMW memory instructions

	.rule	rmwm	%11111110,%001	dec
	.rule	rmwm	%11110110,%110	div
	.rule	rmwm	%11110110,%111	idiv
	.rule	rmwm	%11110110,%101	imul
	.rule	rmwm	%11111110,%000	inc
	.rule	rmwm	%11110110,%100	mul
	.rule	rmwm	%11110110,%011	neg
	.rule	rmwm	%11110110,%010	not
	.rule	rmwm	%11111111,%110	push
	.rule	rmwm	%10001111,%000	pop

; 8086 RMW register instructions

	.rule	rmwr	%01001000	dec
	.rule	rmwr	%01000000	inc
	.rule	rmwr	%01010000	push
	.rule	rmwr	%01011000	pop

; 8086 I/O direct instructions

	.rule	iod	%11100100	in
	.rule	iod	%11100110	out

; 8086 I/O reg instructions

	.rule	ior	%11101100	in
	.rule	ior	%11101110	out

; 8086 shift & rotate instructions

	.rule	rot	%11010000,%000	rol
	.rule	rot	%11010000,%001	ror
	.rule	sal	%11010000,%100	sal
	.rule	shl	%11010000,%100	shl
	.rule	sar	%11010000,%111	sar
	.rule	shr	%11010000,%101	shr
	.rule	rcr	%11010000,%011	rcr
	.rule	rcl	%11010000,%010	rcl

; 8086 accumulator,reg instructions

	.insn	xchg	ax,<mode>
	 .byte	%10010000
	 .mode	0,arg2,arg3,arg4
	.endi

; 8086 memory,reg instructions

	.rule	xchgm	%10000110	xchg

; Return instructions with stack adjustments

	.rule	ret	%11000010	retf
	.rule	ret	%11001010	retn

	.insn	<ret>	<expr>
	 .byte	arg1
	 .byte	arg2
	.endi

; push/pop seg regs

	.rule	psr	%00000110	push
	.rule	psr	%00000111	pop

	.insn	<psr>	<seg>
	 .byte	arg1+arg2<<3
	.endi

; 8086 load reg and segreg instructions

	.rule	lsr	%11000100	les
	.rule	lsr	%10001101	lea
	.rule	lsr	%11000101	lds

; 8086 interrupt instruction

	.insn	int	<expr>
	 .byte	%11001101
	 .byte	arg1
	.endi
