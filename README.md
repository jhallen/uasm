# Universal Assembler

This is an assembler that targets any machine.  The instruction set is
defined via a grammar provided in an include file.

## Pseudo-ops

	.set	label,expr		; Temporarily set label
	.equ	label,expr		; Permamently set label

	label:				; Same as '.equ label,.'

	.space	expr			; Reserve space

	.emit	expr			; Emit a single byte

	.align	expr			; Align to multiple of expr

	.sect	"name"			; Switch to named section

	.public	label, label, ....	; Make labels public

	; Do not necessarily want expressions for args, so we probably don't
	; want to eval first...
	.macro	name,arg,arg,...	; Define normal macro
	.end

	; Would like it to eval first so that foreach can process strings
	; into single chars...
	.foreach name,arg		; Define foreach macro
	.end

	name	arg,arg,arg,...		; Expand macro

	.errif	expr,string		; Print string if expr is true

	.if	expr			; Conditional assembly
	.elseif	expr
	.else
	.end

	.small	arg1>=-128 && arg1<127	; Variable size assembly
	 beq	arg1
	.bigger	arg1>=-32768 && arg1<32767
	 lbeq	arg1
	.biggest
	 bne	skip
	 jmp	arg1
	skip:
	.done

	.include "filename"		; Include a file

	.rule	name expr pattern	; Define a syntax rule

	.insn	pattern			; Define an instruction
	.end


	.rule	simple	$F0		clra
	.		$F1		rola

	.rule	mode	$F8,arg1	<expr>
	.		$FA,arg1	#<expr>

	.rule	dual	$01		lda
	.		$02		sta

	.insn	<dual>	<mode>
	 .byte	arg1|arg2
	 .word	arg3
	.end

	.insn	<simple>
	 .byte	arg1
	.end

## In a syntax rule:

	; comments are allowed (they are ignored).

	<...> refers to another rule.

	whitespace means require whitespace here.

	<> means whitespace is optional here.

	<expr> means expect an expression here, possibly by whitespace (so
	it is not necessary to surround <expr> with <>s).

	other characters are literal matches.

## expressions

	label				; Returns its 32-bit value
	.				; Current location value

	$FF80				; Hex constant
	@770				; Octal constant
	%1011				; Binary constant
	123				; Decimal constant

	( expr )

	- expr
	~ expr
	! expr

	expr << expr
	expr >> expr

	expr * expr
	expr / expr
	expr % expr
	expr & expr

	expr + expr
	expr - expr
	expr | expr
	expr ^ expr

	expr == expr
	expr != expr
	expr < expr
	expr > expr
	expr >= expr
	expr <= expr

	expr && expr

	expr || expr

	expr ? expr : expr

## Object module format

In the object module format description we use the following notation for
object module componants:

	<type:name>

	where: 'name' is replaced with a descriptive name of the componant

	       'type' indicates how the componant is stored in an object file

Object module componant types:

	'byte'		componant is a single byte

	'num'		a variable length unsigned number:

			If the number is between 0 and 125 inclusive,
			128 is added to the number and it is emitted as a
			single byte.

			If the number is between 126 and 32767 inclusive,
			the number is emitted as two bytes.  The most
			significant byte is emitted first.

			If the number is between 32768 and 2^32-1 inclusive,
			a byte equalling 255 is emitted, and then the four
			byte number is emitted, most significant byte first.

			Note that a flag value of 254 is reserved for future
			expansion.

	'zstring'	a variable length string.  the string is emitted
			as-is, and includes a terminating NUL.

	'string'	a variable length string with size prefix.  These,
			strings have the following format:

				<num:string-length> <zstring:the string>

			The string-length includes the terminating NUL of
			the the zstring.

	'expr'		is an expression emitted in reverse-polish notation. 
			See interm.c and interm.h for how expressions are
			emitted.

An object module is composed of records.  The general format of a record is
as follows:

	<byte:type-code> <num:body-size> <bytes:body>

where:
	<type-code> is a single byte record type code.  Record type codes
	            are defined in interm.h

	<body-size> gives the size of just the body in bytes.

	<body>      depends on record type and may have zero length.

Module name record.  Always first record in module.

    iMODULE <num:bodysize> <zstring:module-name>


Section list.

    iSECTS <num:bodysize> <num:no.sections> { <num:align> <num:size> <string:section-name> } ...


Symbols.  The first <no.pubs> symbols are publics.  The remaining symbols
are external references.

    iSYMS <num:bodysize> <num:no.symbols> <num:no.pubs> { <string:symbol-name> <string:source-reference> } ...


Public symbol values.  No. values is same as <no.pubs> in iSYMS record, and
in same order.

    iXDEFS <num:bodysize> { <expr:value> } ...


Data fragment to be placed at given offset of given section.

    iFRAG <num:bodysize> <num:section no.> <num:offset> <bytes:data>


Fixups for immediately previously emitted data fragment.

    iFIXUPS <num:bodysize> <num:num-fixups> { <num:data-offset> <num:type> <expr:value> <expr:msg> } ...

   A type code of 1 indicates that this is a byte fixup and the value
   of the byte is determined by the value expression.

   A type code of 2 indicates that this is a range check instruction and the
   the messages expression is printed if the value expression evaluated to
   a non-zero (true) value.

End of module.

    iEND <num:bodysize>
