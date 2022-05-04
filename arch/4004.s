; Intel 4004 instruction set

; Registers

	.rule	r	$0	r0
	.rule	r	$1	r1
	.rule	r	$2	r2
	.rule	r	$3	r3
	.rule	r	$4	r4
	.rule	r	$5	r5
	.rule	r	$6	r6
	.rule	r	$7	r7
	.rule	r	$8	r8
	.rule	r	$9	r9
	.rule	r	$A	ra
	.rule	r	$B	rb
	.rule	r	$C	rc
	.rule	r	$D	rd
	.rule	r	$E	re
	.rule	r	$F	rf

; Register pairs

	.rule	rp	$0	r0r1
	.rule	rp	$2	r2r3
	.rule	rp	$4	r4r5
	.rule	rp	$6	r6r7
	.rule	rp	$8	r8r9
	.rule	rp	$A	rarb
	.rule	rp	$C	rcrd
	.rule	rp	$E	rerf

; Zero operand instructions

	.rule	simple	$00	nop	; No operation
	.rule	simple	$e0	wrm	; Write accu to ram
	.rule	simple	$e1	wmp	; Write accu to ram output port
	.rule	simple	$e2	wrr	; Write accu to rom i/o lines
	.rule	simple	$e3	wpm	; Write accu to selected half-byte
	.rule	simple	$e4	wr0	; Write ram status char0 to accu
	.rule	simple	$e5	wr1	; Write ram status char1 to accu
	.rule	simple	$e6	wr2	; Write ram status char2 to accu
	.rule	simple	$e7	wr3	; Write ram status char3 to accu
	.rule	simple	$e8	sbm	; Subtract ram from accu
	.rule	simple	$e9	rdm	; Move ram to accu
	.rule	simple	$ea	rdr	; Move rom I/O lines to accu
	.rule	simple	$eb	adm	; Add ram to accu w/ carry
	.rule	simple	$ec	ad0	; Read ram status char0 from accu
	.rule	simple	$ed	ad1	; Read ram status char1 from accu
	.rule	simple	$ee	ad2	; Read ram status char2 from accu
	.rule	simple	$ef	ad3	; Read ram status char3 from accu

	.rule	simple	$f0	clb	; Clear both (accu and carry)
	.rule	simple	$f1	clc	; Clear carry
	.rule	simple	$f2	iac	; Increment accu
	.rule	simple	$f3	cmc	; Complement carry
	.rule	simple	$f4	cma	; Complement accu
	.rule	simple	$f5	ral	; Rotate accu left
	.rule	simple	$f6	rar	; Rotate accu right
	.rule	simple	$f7	tcc	; Transfer carry to accu then clear
	.rule	simple	$f8	dac	; Decrement accu
	.rule	simple	$f9	tcs	; Transfer carry, subtract then clear
	.rule	simple	$fa	stc	; Set carry
	.rule	simple	$fb	daa	; Decimal adjust accu
	.rule	simple	$fc	kbp	; Keyboard process
					; (find first 1).
	.rule	simple	$fd	dcl	; Designate command line

	.inst	<simple>
	 .byte	arg1
	.endi

; Long (4K) jumps

	.rule	long	$40	jun	; Jump unconditional
	.rule	long	$50	jms	; Jump to subroutine

	.inst	<long>	<expr>
	 .byte	arg1+arg2>>8
	 .byte	arg2 & $FF
	.endi

; Register

	.rule	reg	$60	inc	; Increment register
	.rule	reg	$80	add	; Add register to accu with carry
	.rule	reg	$90	sub	; Subtract reg from accu with borrow
	.rule	reg	$A0	ld	; Load accu from reg
	.rule	reg	$B0	xch	; Exchange reg with accu

	.inst	<reg>	<r>
	 .byte	arg1+arg2
	.endi

; Register pair

	.rule	regp	$21	src	; Send register control (sets
					; address for $EX op codes).
	.rule	regp	$30	fin	; Fetch indirect (AC=[RP]).
	.rule	regp	$31	jin	; Jump indirect (PC=RP).

	.inst	<regp>	<rp>
	 .byte	arg1+arg2
	.endi

; Immediate

	.inst	fim	<rp>,<expr>	; Fetch immediate data to register pair
	 .byte	$20+arg1
	 .byte	arg2
	.endi

; Increment and jump on zero

	.inst	isz	<r>,<expr>
	 .byte	$70+arg1
	 .byte	arg2
	.endi

; Jump on condition

	.inst	jcm	<c>,<expr>
	 .byte	$10+arg1
	 .byte	arg2
	.endi

; Short immediate

	.rule	shorti	$c0	bbl	; Return and load accu immediate
	.rule	shorti	$d0	ldm	; Load accu immediate

	.inst	<shorti>	<expr>
	 .byte	arg1+arg2
	.endi
