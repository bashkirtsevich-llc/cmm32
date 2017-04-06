; ---------------------------------------------------------------------------
; Генерация выражения
; ---------------------------------------------------------------------------
  proc	DoExpr
  varName	  = -84h
  var		  = 8
  vtok		  = 0Ch
  vtype 	  = 10h
  mnem		  = 14h

	enter	84h, 0

	call	tok2notstopper
	test	eax, eax	; if1
	jz	DoExpr_if1else

	cmp	dword [ebp+vtok], tk_var ; if2
	jnz	DoExpr_if2else

	push	tk_dword
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression	; Expression("eax",tk_reg,tk_dword)

	jmp	DoExpr_if2end

DoExpr_if2else:
	push	dword [ebp+vtype]
	push	dword [ebp+vtok]
	push	dword [ebp+var]
	call	Expression

	jmp	DoExpr_end

DoExpr_if2end:
	jmp	DoExpr_if1end

DoExpr_if1else:
	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname	; Один операнд
	mov	[tok], eax

	cmp	[tok], tk_reg	  ; if3 ; можно и с eax сравнивать
	jnz	DoExpr_if3else

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoExpr_str2, [ebp+mnem], [ebp+var], eax
	add	esp, 14h

	jmp	DoExpr_if3end

DoExpr_if3else:
	cmp	[tok], tk_var	  ; if4
	jnz	DoExpr_if4else

	cmp	dword [ebp+vtok], tk_var ; if5
	jnz	DoExpr_if5else

	push	tk_dword
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression

	jmp	DoExpr_if5end

DoExpr_if5else:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoExpr_str2, [ebp+mnem], [ebp+var], eax
	add	esp, 14h

DoExpr_if5end:
	jmp	DoExpr_if3end

DoExpr_if4else:
	cmp	[tok], tk_number  ; if6
	jnz	DoExpr_if6else

	call	DoConstMath
	invoke	wsprintfA, mapstr, DoExpr_str3, [ebp+mnem], [ebp+var], eax
	add	esp, 14h

	jmp	DoExpr_if3end

DoExpr_if6else:
	cmp	[tok], tk_postnumber ; if7
	jnz	DoExpr_if7else

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoExpr_str4, [ebp+mnem], [ebp+var], eax
	add	esp, 14h

	jmp	DoExpr_if3end

DoExpr_if7else:
	cmp	[tok], tk_locnumber ; if8
	jnz	DoExpr_if3end

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoExpr_str5, eax
	add	esp, 0Ch

	invoke	wsprintfA, mapstr, DoExpr_str6, [ebp+mnem], [ebp+var]
	add	esp, 10h

	jmp	DoExpr_end

DoExpr_if3end:
	push	mapstr	 ; str
	call	Asm

	jmp	DoExpr_end

DoExpr_if1end:
	cmp	dword [ebp+vtype], tk_byte ; if9
	jz	DoExpr_if9start

	cmp	dword [ebp+vtype], tk_char
	jnz	DoExpr_if9else

DoExpr_if9start:
	invoke	wsprintfA, mapstr, DoExpr_str7, [ebp+mnem], [ebp+var]
	add	esp, 10h

	jmp	DoExpr_if9end

DoExpr_if9else:
	cmp	dword [ebp+vtype], tk_word ; if10
	jz	DoExpr_if10start

	cmp	dword [ebp+vtype], tk_short
	jnz	DoExpr_if10else

DoExpr_if10start:
	invoke	wsprintfA, mapstr, DoExpr_str8, [ebp+mnem], [ebp+var]
	add	esp, 10h

	jmp	DoExpr_if9end

DoExpr_if10else:
	cmp	dword [ebp+vtype], tk_dword ; if11
	jz	DoExpr_if11start

	cmp	dword [ebp+vtype], tk_int
	jnz	DoExpr_if9end

DoExpr_if11start:
	invoke	wsprintfA, mapstr, DoExpr_str9, [ebp+mnem], [ebp+var]
	add	esp, 10h

DoExpr_if9end:
	push	mapstr	 ; str
	call	Asm

DoExpr_end:
	leave
	ret	10h
  endp
; ---------------------------------------------------------------------------
; Разборка и трансляция выражений
; ---------------------------------------------------------------------------
  proc	Expression
  s		  = -44h
  dest		  = 8
  dtok		  = 0Ch
  dtype 	  = 10h

	enter	44h, 0

	push	dword [ebp+dtype]
	push	dword [ebp+dtok]
	push	dword [ebp+dest]
	call	GetInto

Expression_loopstart:
	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	cmp	[tok], tk_plus	  ; if1
	jnz	Expression_if1else

	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	lea	eax, [ebp+s]
	invoke	wsprintfA, mapstr, Expression_str1, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	Expression_loopcontinue

Expression_if1else:
	cmp	[tok], tk_minus   ; if2
	jnz	Expression_if2else

	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	lea	eax, [ebp+s]
	invoke	wsprintfA, mapstr, Expression_str2, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	Expression_loopcontinue

Expression_if2else:
	cmp	[tok], tk_or	  ; if3
	jnz	Expression_if3else

	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	lea	eax, [ebp+s]
	invoke	wsprintfA, mapstr, Expression_str3, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	Expression_loopcontinue

Expression_if3else:
	cmp	[tok], tk_xor	  ; if4
	jnz	Expression_if4else

	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	lea	eax, [ebp+s]
	invoke	wsprintfA, mapstr, Expression_str4, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	Expression_loopcontinue

Expression_if4else:
	cmp	[tok], tk_assign  ; if5
	jz	Expression_if5start

	cmp	[tok], tk_equalto
	jnz	Expression_if5else

Expression_if5start:
	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	lea	eax, [ebp+s]
	invoke	wsprintfA, mapstr, Expression_str5, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	push	Expression_str12 ; "sete al"
	call	Asm

Expression_SE:
	invoke	wsprintfA, mapstr, Expression_str6, [ebp+dest]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	jmp	Expression_loopcontinue

Expression_if5else:
	cmp	[tok], tk_notequal ; if6
	jnz	Expression_if6else

	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	lea	eax, [ebp+s]
	invoke	wsprintfA, mapstr, Expression_str5, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	push	Expression_str7 ; "setne al"
	call	Asm

	jmp	Expression_SE

	jmp	Expression_loopcontinue

Expression_if6else:			; CODE XREF: Expression+18Ej
	cmp	[tok], tk_greater ; if7
	jnz	Expression_if7else

	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	lea	eax, [ebp+s]
	invoke	wsprintfA, mapstr, Expression_str5, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	push	Expression_str8 ; "setg al"
	call	Asm

	jmp	Expression_SE

	jmp	Expression_loopcontinue

Expression_if7else:			; CODE XREF: Expression+1D8j
	cmp	[tok], tk_greaterequal ; if8
	jnz	Expression_if8else

	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	invoke	wsprintfA, mapstr, Expression_str5, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	push	Expression_str9 ; "setge al"
	call	Asm

	jmp	Expression_SE

	jmp	Expression_loopcontinue

Expression_if8else:
	cmp	[tok], tk_less	  ; if9
	jnz	Expression_if9else

	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	invoke	wsprintfA, mapstr, Expression_str5, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	push	Expression_str10 ; "setl al"
	call	Asm

	jmp	Expression_SE

	jmp	Expression_loopcontinue

Expression_if9else:
	cmp	[tok], tk_lessequal ; if10
	jnz	Expression_if10else

	push	dword [ebp+dtype]     ; ttype
	lea	eax, [ebp+s]
	push	eax		; t
	call	Term

	invoke	wsprintfA, mapstr, Expression_str5, [ebp+dest], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	push	Expression_str11 ; "setle al"
	call	Asm

	jmp	Expression_SE

	jmp	Expression_loopcontinue

Expression_if10else:
	jmp	Expression_loopend

Expression_loopcontinue:
	jmp	Expression_loopstart

Expression_loopend:
	mov	eax, [relation]

	leave
	ret	0Ch
  endp
; ---------------------------------------------------------------------------
; Чтение очередного элемента выражения
; ---------------------------------------------------------------------------
  proc	Factor
  f		  = 8
  ftype 	  = 0Ch

	push	ebp
	mov	ebp, esp

	call	NextTok

	cmp	[tok], tk_openbracket ; if1
	jnz	Factor_if1else

	call	NextTok

	call	PushEAX

	push	dword [ebp+ftype]
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression

	push	Factor_str1 ; "pop ebx; xchg eax,ebx"
	call	Asm

	invoke	wsprintfA, [ebp+f], GetImport_str1, (DoExpr_str6+6)
	add	esp, 0Ch

	jmp	Factor_end

Factor_if1else:
	cmp	[tok], tk_number  ; if2
	jnz	Factor_if2else

	call	DoConstMath

	invoke	wsprintfA, [ebp+f], Factor_str2, eax
	add	esp, 0Ch

	jmp	Factor_end

Factor_if2else:
	cmp	[tok], tk_postnumber ; if3
	jnz	Factor_if3else

	invoke	wsprintfA, [ebp+f], (DoExpr_str4+5), string
	add	esp, 0Ch

	jmp	Factor_end

Factor_if3else:
	cmp	[tok], tk_proc	  ; if4
	jnz	Factor_if4else

	call	PushEAX

	call	DoAnyProc

	push	Factor_str1 ; "pop ebx; xchg eax,ebx"
	call	Asm

	invoke	wsprintfA, [ebp+f], GetImport_str1, (DoExpr_str6+6)
	add	esp, 0Ch

	jmp	Factor_end

Factor_if4else:
if DEFINE_MAKE_EXE
	cmp	[tok], tk_API	  ; if5
	jnz	Factor_if5else

	call	PushEAX

	call	doAPI

	push	Factor_str1 ; "pop ebx; xchg eax,ebx"
	call	Asm

	invoke	wsprintfA, [ebp+f], GetImport_str1, (DoExpr_str6+6)
	add	esp, 0Ch

	jmp	Factor_end
end if
Factor_if5else:
	cmp	[tok], tk_var	  ; if6
	jz	Factor_if6start

	cmp	[tok], tk_param
	jz	Factor_if6start

	cmp	[tok], tk_local
	jz	Factor_if6start

	cmp	[tok], tk_openblock
	jz	Factor_if6start

	cmp	[tok], tk_reg
	jnz	Factor_end

Factor_if6start:
	push	dword [ebp+f]	      ; varName
	call	GetVarname

Factor_end:
	pop	ebp
	ret	8
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	GetInto
  tk		  = -4
  dest		  = 8
  dtok		  = 0Ch
  dtype 	  = 10h

	enter	4, 0

	mov	dword [ebp+tk], 0

GetInto_DOCASE:
	cmp	[tok], tk_minus   ; if1
	jnz	GetInto_if1else

GetInto_F0:				; CODE XREF: GetInto+36j
	mov	eax, [tok]
	mov	dword [ebp+tk], eax

	call	NextTok

	jmp	GetInto_DOCASE

	jmp	GetInto_if1end

GetInto_if1else:
	cmp	[tok], tk_not	  ; if2
	jnz	GetInto_if2else

	xor	[relation], 1

	jmp	GetInto_F0

	jmp	GetInto_if1end

GetInto_if2else:
	cmp	[tok], tk_openbracket ; if3
	jnz	GetInto_if3else

	call	NextTok

	push	dword [ebp+dtype]
	push	dword [ebp+dtok]
	push	dword [ebp+dest]
	call	Expression

	jmp	GetInto_if1end

GetInto_if3else:
	cmp	[tok], tk_number  ; if4
	jnz	GetInto_if4else

	cmp	dword [ebp+tk], 0     ; if5
	jz	GetInto_if5else

	call	DoConstMath
	invoke	wsprintfA, mapstr, GetInto_str1, [ebp+dest], eax
	add	esp, 10h

	jmp	GetInto_if5end

GetInto_if5else:
	call	DoConstMath
	invoke	wsprintfA, mapstr, GetInto_str2, [ebp+dest], eax
	add	esp, 10h

GetInto_if5end:
	push	mapstr	 ; str
	call	Asm

	mov	dword [ebp+tk], 0

	jmp	GetInto_if1end

GetInto_if4else:
	cmp	[tok], tk_postnumber ; if6
	jnz	GetInto_if6else

	invoke	wsprintfA, mapstr, GetInto_str3, [ebp+dest], string
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	GetInto_if1end

GetInto_if6else:
	cmp	[tok], tk_locnumber ; if7
	jnz	GetInto_if7else

	invoke	wsprintfA, mapstr, DoExpr_str5, string
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	invoke	wsprintfA, mapstr, GetInto_str4, [ebp+dest]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	jmp	GetInto_if1end

GetInto_if7else:
	cmp	[tok], tk_proc	  ; if8
	jnz	GetInto_if8else

	call	DoAnyProc

	jmp	GetInto_if1end

GetInto_if8else:
if DEFINE_MAKE_EXE
	cmp	[tok], tk_API	  ; if9
	jnz	GetInto_if9else

	call	doAPI

	jmp	GetInto_if1end
end if
GetInto_if9else:
	cmp	[tok], tk_var	  ; if10
	jz	GetInto_if10start
	cmp	[tok], tk_local
	jz	GetInto_if10start
	cmp	[tok], tk_param
	jz	GetInto_if10start
	cmp	[tok], tk_reg
	jz	GetInto_if10start
	cmp	[tok], tk_openblock
	jnz	GetInto_if10else

GetInto_if10start:
	push	dword [ebp+dtype]
	push	dword [ebp+dtok]
	push	dword [ebp+dest]
	call	GetIntoVar

	jmp	GetInto_if1end

GetInto_if10else:
	push	GetInto_str5 ; "Wrong expression member"
	call	preerror

GetInto_if1end:
	cmp	dword [ebp+tk], tk_minus ; if11
	jnz	GetInto_if11end

	invoke	wsprintfA, mapstr, GetInto_str6, [ebp+dest]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

GetInto_if11end:
	cmp	dword [ebp+tk], tk_not ; if12
	jnz	GetInto_if12end

	invoke	wsprintfA, mapstr, GetInto_str7, [ebp+dest]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

GetInto_if12end:
	leave
	ret	0Ch
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	GetIntoVar
  varName	  = -8Ch
  vtype 	  = -8
  vtok		  = -4
  dName 	  = 8
  dTok		  = 0Ch
  dType 	  = 10h

	enter	8Ch, 0

	cmp	dword [ebp+dTok], tk_reg ; if1
	jnz	GetIntoVar_if1else

	cmp	[tok], tk_reg	  ; if2
	jnz	GetIntoVar_if2else

	cmp	dword [ebp+dType], tk_dword ; if3
	jnz	GetIntoVar_if3else

	cmp	[type], tk_dword  ; if4
	jnz	GetIntoVar_if4else

	invoke	wsprintfA, mapstr, GetIntoVar_str1, [ebp+dName], string
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if3end

GetIntoVar_if4else:
	cmp	[type], tk_word   ; if5
	jz	GetIntoVar_RDW

	cmp	[type], tk_byte
	jnz	GetIntoVar_if3end

GetIntoVar_RDW:
	invoke	wsprintfA, mapstr, GetIntoVar_str2, [ebp+dName], string
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

GetIntoVar_if3end:
	jmp	GetIntoVar_if2end

GetIntoVar_if3else:
	cmp	dword [ebp+dType], tk_word ; if6
	jnz	GetIntoVar_if6else

	cmp	[type], tk_dword  ; if7
	jnz	GetIntoVar_if7else

GetIntoVar_GERR:
;        push    40826Fh         ; warning("Not same size\n");
;        call    warning

	jmp	GetIntoVar_if6end

GetIntoVar_if7else:
	cmp	[type], tk_word   ; if8
	jnz	GetIntoVar_if8else

	invoke	wsprintfA, mapstr, GetIntoVar_str1, [ebp+dName], string
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if6end

GetIntoVar_if8else:
	cmp	[type], tk_byte   ; if9
	jz	GetIntoVar_RDW

GetIntoVar_if6end:
	jmp	GetIntoVar_if2end

GetIntoVar_if6else:
	cmp	dword [ebp+dType], tk_byte ; if9
	jnz	GetIntoVar_if2end

	cmp	[type], tk_dword  ; if10
	jz	GetIntoVar_GERR

	cmp	[type], tk_word
	jz	GetIntoVar_GERR

	cmp	[type], tk_byte   ; if11
	jnz	GetIntoVar_if2end

	invoke	wsprintfA, mapstr, GetIntoVar_str1, [ebp+dName], string
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

GetIntoVar_if2end:
	jmp	GetIntoVar_if1end

GetIntoVar_if2else:
	cmp	[tok], tk_var	  ; if12
	jz	GetIntoVar_if12start

	cmp	[tok], tk_param
	jz	GetIntoVar_if12start

	cmp	[tok], tk_local
	jz	GetIntoVar_if12start

	cmp	[tok], tk_openblock
	jnz	GetIntoVar_if12else

GetIntoVar_if12start:
	mov	eax, [type]
	mov	[ebp+vtype], eax

	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname
	mov	[ebp+vtok], eax

	cmp	dword [ebp+vtype], tk_dword ; if13
	jz	GetIntoVar_if13start

	cmp	dword [ebp+vtype], tk_int
	jz	GetIntoVar_if13start

	mov	eax, [ebp+vtype]
	cmp	dword [ebp+dType], eax
	jnz	GetIntoVar_if13else

GetIntoVar_if13start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str1, [ebp+dName], eax
	add	esp, 10h

	jmp	GetIntoVar_if13end

GetIntoVar_if13else:
	cmp	dword [ebp+vtype], tk_word ; if14
	jz	GetIntoVar_if14start

	cmp	dword [ebp+vtype], tk_byte
	jnz	GetIntoVar_if14else

GetIntoVar_if14start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str2, [ebp+dName], eax
	add	esp, 10h

	jmp	GetIntoVar_if13end

GetIntoVar_if14else:
	cmp	dword [ebp+vtype], tk_short; if15
	jz	GetIntoVar_if15start

	cmp	dword [ebp+vtype], tk_char
	jnz	GetIntoVar_if13end

GetIntoVar_if15start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str3, [ebp+dName], eax
	add	esp, 10h

GetIntoVar_if13end:
	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if1end

GetIntoVar_if12else:
	cmp	[tok], tk_number  ; if16
	jnz	GetIntoVar_if16else

	call	DoConstMath

	invoke	wsprintfA, mapstr, GetIntoVar_str4, [ebp+dName], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if1end

GetIntoVar_if16else:
	cmp	[tok], tk_postnumber ; if17
	jnz	GetIntoVar_if17else

	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetInto_str3, [ebp+dName], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if1end

GetIntoVar_if17else:
	cmp	[tok], tk_locnumber ; if18
	jnz	GetIntoVar_if1end

	mov	eax, [type]
	mov	[ebp+vtype], eax

	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname
	mov	[ebp+vtok], eax

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str5, [ebp+dName], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

GetIntoVar_if1end:
	jmp	GetIntoVar_end

GetIntoVar_if1else:
	cmp	dword [ebp+dTok], tk_var ; if19
	jnz	GetIntoVar_end

	cmp	[tok], tk_reg	  ; if20
	jnz	GetIntoVar_if20else

	cmp	[type], tk_dword  ; if21
	jnz	GetIntoVar_if21else

	invoke	wsprintfA, mapstr, GetIntoVar_str6, [ebp+dName], string
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if21end

GetIntoVar_if21else:
	cmp	[type], tk_word   ; if22
	jnz	GetIntoVar_if22else

	invoke	wsprintfA, mapstr, GetIntoVar_str7, [ebp+dName], string
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if21end

GetIntoVar_if22else:
	cmp	[type], tk_byte   ; if23
	jnz	GetIntoVar_if21end

	invoke	wsprintfA, mapstr, GetIntoVar_str8, [ebp+dName], string
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

GetIntoVar_if21end:
	jmp	GetIntoVar_end

GetIntoVar_if20else:
	cmp	[tok], tk_var	  ; if24
	jnz	GetIntoVar_if24else

	mov	eax, [type]
	mov	[ebp+vtype], eax

	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname
	mov	[ebp+vtok], eax

	cmp	dword [ebp+dType], tk_byte ; if25
	jz	GetIntoVar_if25start

	cmp	dword [ebp+dType], tk_char
	jnz	GetIntoVar_if25else

GetIntoVar_if25start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str9, eax
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	invoke	wsprintfA, mapstr, GetIntoVar_str10, [ebp+dName]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if25end

GetIntoVar_if25else:
	cmp	dword [ebp+dType], tk_word ; if26
	jz	GetIntoVar_if26start

	cmp	dword [ebp+dType], tk_short
	jnz	GetIntoVar_if26else

GetIntoVar_if26start:
	cmp	dword [ebp+vtype], tk_byte ; if27
	jnz	GetIntoVar_if27else

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str11, eax
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if26end

GetIntoVar_if27else:
	cmp	dword [ebp+vtype], tk_char ; if28
	jnz	GetIntoVar_if28else

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str12, eax
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if26end

GetIntoVar_if28else:
	cmp	dword [ebp+vtype], tk_word ; if29
	jz	GetIntoVar_if29start

	cmp	dword [ebp+vtype], tk_short
	jnz	GetIntoVar_if29else

GetIntoVar_if29start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str13, eax
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if26end

GetIntoVar_if29else:
	cmp	dword [ebp+vtype], tk_dword ; if30
	jz	GetIntoVar_if30start

	cmp	dword [ebp+vtype], tk_int
	jnz	GetIntoVar_if26end

GetIntoVar_if30start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str14, eax
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

GetIntoVar_if26end:
	invoke	wsprintfA, mapstr, GetIntoVar_str15, [ebp+dName]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if25end

GetIntoVar_if26else:
	cmp	dword [ebp+dType], tk_dword ; if31
	jz	GetIntoVar_if31start

	cmp	dword [ebp+dType], tk_int
	jnz	GetIntoVar_if25end

GetIntoVar_if31start:
	cmp	dword [ebp+vtype], tk_byte ; if32
	jz	GetIntoVar_if32start

	cmp	dword [ebp+vtype], tk_word
	jnz	GetIntoVar_if32else

GetIntoVar_if32start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str16, eax
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if32end

GetIntoVar_if32else:
	cmp	dword [ebp+vtype], tk_char ; if33
	jz	GetIntoVar_if33start

	cmp	dword [ebp+vtype], tk_short
	jnz	GetIntoVar_if33else

GetIntoVar_if33start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA,
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_if32end

GetIntoVar_if33else:
	cmp	dword [ebp+vtype], tk_dword ; if34
	jz	GetIntoVar_if34start

	cmp	dword [ebp+vtype], tk_int
	jnz	GetIntoVar_if32end

GetIntoVar_if34start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetIntoVar_str18, eax
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

GetIntoVar_if32end:
	invoke	wsprintfA, mapstr, GetIntoVar_str19, [ebp+dName]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

GetIntoVar_if25end:
	jmp	GetIntoVar_end

GetIntoVar_if24else:
	cmp	[tok], tk_number  ; if35
	jnz	GetIntoVar_if35else

	call	DoConstMath

	invoke	wsprintfA,mapstr, GetIntoVar_str4, [ebp+dName], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_end

GetIntoVar_if35else:
	cmp	[tok], tk_postnumber ; if36
	jnz	GetIntoVar_if36else

	mov	eax, [type]
	mov	dword [ebp+vtype], eax

	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname
	mov	dword [ebp+vtok], eax

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, GetInto_str3, [ebp+dName], eax
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	jmp	GetIntoVar_end

GetIntoVar_if36else:
	cmp	[tok], tk_locnumber ; if37
	jnz	GetIntoVar_end

	mov	eax, [type]
	mov	dword [ebp+vtype], eax

	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname
	mov	dword [ebp+vtok], eax

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoExpr_str5, eax
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	invoke	wsprintfA, mapstr, GetInto_str4, [ebp+dName]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

GetIntoVar_end:
	leave
	ret	0Ch
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	GetVarname
  vtok		  = -4
  varName	  = 8

	enter	4, 0

	cmp	[tok], tk_openblock ; if1
	jz	GetVarname_G0

	invoke	lstrcpyA, [ebp+varName], string
;        push    offset string   ; lpString2
;        push    [ebp+varName]   ; lpString1
;        call    lstrcpyA

	mov	eax, [tok]
	mov	[ebp+vtok], eax

	cmp	dword [ebp+vtok], tk_local ; if2
	jnz	GetVarname_if2else

	mov	dword [ebp+vtok], tk_var

	jmp	GetVarname_if2end

GetVarname_if2else:
	cmp	dword [ebp+vtok], tk_param ; if3
	jnz	GetVarname_if2end

	mov	dword [ebp+vtok], tk_var

GetVarname_if2end:
	cmp	[tok2], tk_openblock ; if4
	jnz	GetVarname_if4end

	call	NextTok

GetVarname_G0:
	mov	dword [ebp+vtok], tk_var

	invoke	lstrcatA, [ebp+varName], GetVarname_str1
;        push    offset GetVarname_str1 ; "["
;        push    [ebp+varName]   ; lpString1
;        call    lstrcatA

GetVarname_loopstart:
	call	NextTok

	cmp	[tok], tk_reg	  ; if5
	jnz	GetVarname_if5else

	invoke	lstrcatA, [ebp+varName], string
;        push    offset string   ; lpString2
;        push    [ebp+varName]   ; lpString1
;        call    lstrcatA

	jmp	GetVarname_if5end

GetVarname_if5else:
	cmp	[tok], tk_plus	  ; if6
	jnz	GetVarname_if6else

	invoke	lstrcatA, [ebp+varName], GetVarname_str2
;        push    offset GetVarname_str2 ; "+"
;        push    [ebp+varName]   ; lpString1
;        call    lstrcatA

	jmp	GetVarname_if5end

GetVarname_if6else:
	cmp	[tok], tk_mult	  ; if7
	jnz	GetVarname_if7else

	invoke	lstrcatA, [ebp+varName], GetVarname_str3
;        push    offset GetVarname_str3 ; "*"
;        push    [ebp+varName]   ; lpString1
;        call    lstrcatA

	jmp	GetVarname_if5end

GetVarname_if7else:
	cmp	[tok], tk_number  ; if8
	jnz	GetVarname_if8else

	call	DoConstMath
	invoke	wsprintfA, mapstr, (CmdCycle_str1+6), eax ; %d
;        push    eax
;        push    (offset CmdCycle_str1+6)
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	invoke	lstrcatA, [ebp+varName], mapstr
;        push    offset mapstr   ; lpString2
;        push    [ebp+varName]   ; lpString1
;        call    lstrcatA

	jmp	GetVarname_if5end

GetVarname_if8else:
	cmp	[tok], tk_postnumber ; if9
	jnz	GetVarname_if9else

	invoke	lstrcatA, [ebp+varName], GetVarname_str4
;        push    offset GetVarname_str4 ; "#"
;        push    [ebp+varName]   ; lpString1
;        call    lstrcatA

	invoke	lstrcatA, [ebp+varName], string
;        push    offset string   ; lpString2
;        push    [ebp+varName]   ; lpString1
;        call    lstrcatA

	jmp	GetVarname_if5end

GetVarname_if9else:
	cmp	[tok], tk_closeblock ; if10
	jnz	GetVarname_if10else

	invoke	lstrcatA, [ebp+varName], GetVarname_str5
;        push    offset GetVarname_str5 ; "]"
;        push    [ebp+varName]   ; lpString1
;        call    lstrcatA

	jmp	GetVarname_if4end

	jmp	GetVarname_if5end

GetVarname_if10else:
	push	GetVarname_str6 ; "Illegal index expression in []"
	call	preerror

GetVarname_if5end:
	jmp	GetVarname_loopstart

GetVarname_if4end:
	mov	eax, [ebp+vtok]

	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	Term
  t		  = 8
  ttype 	  = 0Ch

	push	ebp
	mov	ebp, esp

Term_loopstart:
	push	dword [ebp+ttype]     ; ftype
	push	dword [ebp+t]	      ; f
	call	Factor

	cmp	[tok], tk_mult	  ; if1
	jnz	Term_if1else

	push	dword [ebp+ttype]     ; ftype
	push	dword [ebp+t]	      ; f
	call	Factor

	cmp	[tok], tk_number  ; if2
	jnz	Term_if2else

	call	DoConstMath
	invoke	wsprintfA, mapstr, Term_str1, eax
;        push    eax
;        push    offset Term_str1 ; "mov ebx,%d"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	invoke	wsprintfA, mapstr, Term_str2, (DoExpr_str6+6)
;        push    (offset DoExpr_str6+6)
;        push    offset Term_str2 ; "mul %s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	jmp	Term_if2end

Term_if2else:
	invoke	wsprintfA, mapstr, Term_str2, [ebp+t]
;        push    [ebp+t]
;        push    offset Term_str2 ; "mul %s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

Term_if2end:
	push	mapstr	 ; str
	call	Asm

	jmp	Term_if1end

Term_if1else:
	cmp	[tok], tk_div	  ; if3
	jnz	Term_if3else

	push	dword [ebp+ttype]     ; ftype
	push	dword [ebp+t]	      ; f
	call	Factor

	cmp	[tok], tk_number  ; if4
	jnz	Term_if4else

	call	DoConstMath
	invoke	wsprintfA, mapstr, Term_str1, eax
;        push    eax
;        push    offset Term_str1 ; "mov ebx,%d"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	invoke	wsprintfA, mapstr, Term_str3, (DoExpr_str6+6)
;        push    (offset DoExpr_str6+6)
;        push    offset Term_str3 ; "div %s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	jmp	Term_if3end

Term_if4else:
	invoke	wsprintfA, mapstr, Term_str3, [ebp+t]
;        push    [ebp+t]
;        push    offset Term_str3 ; "div %s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

Term_if3end:
	push	mapstr	 ; str
	call	Asm

	jmp	Term_if1end

Term_if3else:
	cmp	[tok], tk_mod	  ; if5
	jnz	Term_if5else

	push	dword [ebp+ttype]     ; ftype
	push	dword [ebp+t]	      ; f
	call	Factor

	cmp	[tok], tk_number  ; if6
	jnz	Term_if6else

	call	DoConstMath
	invoke	wsprintfA, mapstr, Term_str1, eax
;        push    eax
;        push    offset Term_str1 ; "mov ebx,%d"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	invoke	wsprintfA, mapstr, Term_str3, (DoExpr_str6+6)
;        push    (offset DoExpr_str6+6)
;        push    offset Term_str3 ; "div %s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	jmp	Term_if5end

Term_if6else:
	invoke	wsprintfA, mapstr, Term_str3, [ebp+t]
;        push    [ebp+t]
;        push    offset Term_str3 ; "div %s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

Term_if5end:
	push	mapstr	 ; str
	call	Asm

	push	Term_str4 ; "xchg eax,edx"
	call	Asm

	jmp	Term_if1end

Term_if5else:
	cmp	[tok], tk_and	  ; if7
	jnz	Term_if7else

	push	dword [ebp+ttype]     ; ftype
	push	dword [ebp+t]	      ; f
	call	Factor

	cmp	[tok], tk_number  ; if8
	jnz	Term_if8else

	call	DoConstMath
	invoke	wsprintfA, mapstr, Term_str5, eax
;        push    eax
;        push    offset Term_str5 ; "and ebx,%d"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	jmp	Term_if8end

Term_if8else:
	invoke	wsprintfA, mapstr, Term_str6, [ebp+t]
;        push    [ebp+t]
;        push    offset Term_str6 ; "and eax,%s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

Term_if8end:
	push	mapstr	 ; str
	call	Asm

	jmp	Term_if1end

Term_if7else:
	cmp	[tok], tk_not	  ; if9
	jnz	Term_if9else

	push	Term_str7 ; "not eax"
	call	Asm

	jmp	Term_if1end

Term_if9else:
	jmp	Term_loopend

Term_if1end:
	jmp	Term_loopstart

Term_loopend:
	pop	ebp
	ret	8
  endp
; ---------------------------------------------------------------------------
; Сохранение EAX в стеке
; ---------------------------------------------------------------------------
  proc	PushEAX
	push	PushEAX_str1
	call	Asm

	ret
  endp
; ---------------------------------------------------------------------------
; Обработка строки-параметра: proc("string")
; ---------------------------------------------------------------------------
  proc	AddPoststring
  returnvalue	  = -4

	enter	4, 0

	cmp	[posts], MAXPOSTS ; if1
	jb	AddPoststring_if1end

	push	AddPoststring_str1 ; "cannot add post string, post queue full"...
	call	preerror

;        invoke  ExitProcess, -1 ; нельзя же так
	call	RestoreState

AddPoststring_if1end:
	mov	ebx, [posts]
	shl	ebx, 2
	add	ebx, [posttype]

	mov	dword [ebx], POST_STR

	mov	ebx, [posts]
	shl	ebx, 2
	add	ebx, [postloc]

	mov	eax, [outptr]
	mov	[ebx], eax

	inc	[posts]

	mov	eax, MAXDATA-1
	sub	eax, [poststrptr]
	mov	[ebp+returnvalue], eax

	mov	esi, string ; pointer

	xchg	ebx, [poststrptr]

AddPoststring_loopstart:
	lodsb

	mov	edi, [output]
	add	edi, ebx

	mov	byte [edi], al

	dec	ebx

	test	al, al		; loop condition
	jnz	AddPoststring_loopstart

	xchg	ebx, [poststrptr]
	mov	eax, [ebp+returnvalue]

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Обработка post-строк и ссылок на них
; ---------------------------------------------------------------------------
  proc	DoPoststrings
  addvalue	  = -0Ch
  addhold	  = -8
  i		  = -4

	enter	0Ch, 0

	cmp	[poststrptr], MAXDATA-1 ; if1
	jnz	DoPoststrings_if1end

	leave
	ret

DoPoststrings_if1end:
	mov	eax, [OptImageBase]
	add	eax, [OptBaseOfCode]
	add	eax, [outptr]
	sub	eax, [output]
	mov	[ebp+addvalue], eax

	xchg	edi, [outptr]

	mov	ebx, MAXDATA-1

DoPoststrings_D0:
	mov	esi, [output]
	add	esi, ebx

	mov	al, byte [esi]

	stosb

	dec	ebx

	cmp	ebx, [poststrptr]
	ja	DoPoststrings_D0

	xchg	edi, [outptr]

	mov	dword [ebp+i], 0

DoPoststrings_loopstart:
	mov	eax, [posts]
	cmp	dword [ebp+i], eax    ; if2 это условие выхода из цикла
	jnb	DoPoststrings_loopend

	mov	ebx, [ebp+i]
	shl	ebx, 2
	add	ebx, [posttype]

	cmp	dword [ebx], POST_STR
	jnz	DoPoststrings_if2end

	mov	ebx, [ebp+i]
	shl	ebx, 2
	add	ebx, [postloc]

	push	dword [ebx]
	call	GetDword
	add	eax, [ebp+addvalue]
	mov	dword [ebp+addhold], eax

	push	dword [ebp+addhold]
	push	dword [ebx]
	call	SetDword

	dec	[posts]

	mov	ebx, [ebp+i]
	shl	ebx, 2
	add	ebx, [postloc]

	mov	ecx, [posts]
	shl	ecx, 2
	add	ecx, [postloc]

	mov	eax, [ecx]
	mov	[ebx], eax

	mov	ebx, [ebp+i]
	shl	ebx, 2
	add	ebx, [posttype]

	mov	ecx, [posts]
	shl	ecx, 2
	add	ecx, [posttype]

	mov	eax, [ecx]
	mov	[ebx], eax

	mov	ebx, [ebp+i]
	shl	ebx, 2
	add	ebx, [postnum]

	mov	ecx, [posts]
	shl	ecx, 2
	add	ecx, [postnum]

	mov	eax, [ecx]
	mov	[ebx], eax

	dec	dword [ebp+i]

DoPoststrings_if2end:
	inc	dword [ebp+i]
	jmp	DoPoststrings_loopstart

DoPoststrings_loopend:
	mov	[poststrptr], MAXDATA-1

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Чтение одного параметра при вызове процедуры
; ---------------------------------------------------------------------------
  proc	GetParam
  count 	  = -4 ; счетчик скобок в параметре
  p		  = 8

	enter	4, 0

	mov	dword [ebp+count], 0

GetParam_loopstart:
	xor	eax, eax

	xchg	esi, [inptr2]

	lodsb

	xchg	esi, [inptr2]

	mov	byte [cha2], al

	xchg	edi, [ebp+p]

	stosb

	xchg	edi, [ebp+p]

	test	al, al		; if1
	jz	GetParam_loopend

	cmp	al, ')' 	; if2
	jnz	GetParam_if2else

	cmp	dword [ebp+count], 0  ; if3
	jnz	GetParam_if3end

	xchg	edi, [ebp+p]

	mov	al, 0

	dec	edi

	stosb

	xchg	edi, [ebp+p]

	jmp	GetParam_loopend

GetParam_if3end:
	dec	dword [ebp+count]

	jmp	GetParam_if1end

GetParam_if2else:
	cmp	al, ',' 	; if4
	jnz	GetParam_if4else

	cmp	dword [ebp+count], 0  ; if5
	jnz	GetParam_if5end

	xchg	edi, [ebp+p]

	dec	edi

	mov	al, 0

	stosb

	xchg	edi, [ebp+p]

	call	DoParam

	jmp	GetParam_loopend

GetParam_if5end:
	jmp	GetParam_if1end

GetParam_if4else:
	cmp	al, '(' 	; if6
	jnz	GetParam_if1end

	inc	dword [ebp+count]

GetParam_if1end:
	jmp	GetParam_loopstart

GetParam_loopend:
	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
; Обработка одного параметра при вызове процедуры
; ---------------------------------------------------------------------------
  proc	DoParam
  vtok		  = -958h
  p		  = -954h ; место под копию параметра
  holdcha	  = -858h
  s		  = -854h
  s2		  = -46Ch
  varName	  = -84h

	enter	958h, 0

	lea	eax, [ebp+p]
	push	eax
	call	GetParam

	mov	al, [cha2]
	mov	[ebp+holdcha], al

	push	[linenum2]
	push	[inptr2]
	push	[number]
	push	[tok2]
	push	[tok]
	push	[input]
	push	[inptr]
	push	[currmod]
	push	[linenumber]
	push	[endoffile]
	push	[displaytokerrors]

	lea	eax, [ebp+s]
	invoke	lstrcpyA, eax, string
;        push    offset string   ; lpString2
;        lea     eax, [ebp+s]
;        push    eax             ; lpString1
;        call    lstrcpyA

	lea	eax, [ebp+s2]
	invoke	lstrcpyA, eax, string2
;        push    offset string2  ; lpString2
;        lea     eax, [ebp+s2]
;        push    eax             ; lpString1
;        call    lstrcpyA

	lea	eax, [ebp+p]
	mov	[input], eax

	mov	eax, [input]
	mov	[inptr], eax

	mov	eax, [input]
	mov	[inptr2], eax

	mov	[endoffile], 0 ; На начале файла

	call	NextChar

	mov	al, byte [cha]
	mov	byte [cha2], al

	mov	eax, [inptr]
	mov	[inptr2], eax

	mov	[linenum2], 1

	call	NextTok

DoParam_loopstart:
	cmp	[tok], tk_eof	  ; if1
	jz	DoParam_if1start

	cmp	[tok], tk_closebracket
	jnz	DoParam_if1end

DoParam_if1start:
	jmp	DoParam_loopend

DoParam_if1end:
	cmp	[tok], tk_comma   ; if2
	jnz	DoParam_if2start

	call	NextTok

	jmp	DoParam_loopcontinue

DoParam_if2start:
	cmp	[tok], tk_string  ; if3
	jnz	DoParam_if3else

	mov	al, 68h
	call	OP

	call	AddPoststring
	call	OUTDWORD

	cmp	[list], 0	  ; if4
	jz	DoParam_if4end

	invoke	wsprintfA, mapstr, aPushS, string
;        push    offset string
;        push    offset aPushS   ; "\t//\tpush #\"%s\"\r\n"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; lpBuffer
;        push    [mapfile]         ; hFile
	call	fprint

DoParam_if4end:
	call	NextTok

	jmp	DoParam_loopcontinue

DoParam_if3else:
	call	tok2isopperand
	test	eax, eax	; if5
	jz	DoParam_if5else

	push	tk_dword
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression

	call	PushEAX

	jmp	DoParam_loopcontinue

DoParam_if5else:
	cmp	[tok], tk_number  ; if6
	jnz	DoParam_if6else

	call	DoConstMath
	invoke	wsprintfA, mapstr, aPushX, eax
;        push    eax
;        push    offset aPushX   ; "push %#x"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	call	NextTok

	jmp	DoParam_loopcontinue

DoParam_if6else:
	cmp	[tok], tk_postnumber ; if7
	jnz	DoParam_if7else

	invoke	wsprintfA, mapstr, aPushS_0, string
;        push    offset string
;        push    offset aPushS_0 ; "push #%s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	call	NextTok

	jmp	DoParam_loopcontinue

DoParam_if7else:
	cmp	[tok], tk_reg	  ; if8
	jnz	DoParam_if8else

	cmp	[type], tk_dword  ; if9
	jnz	DoParam_if9else

	invoke	wsprintfA, mapstr, aPushS_1, string
;        push    offset string
;        push    offset aPushS_1 ; "push %s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	call	NextTok

	jmp	DoParam_if9end

DoParam_if9else:
	cmp	[type], tk_word   ; if10
	jnz	DoParam_if10else

	invoke	wsprintfA, mapstr, aMovsxESSPushES, string, string, string
;        push    offset string
;        push    offset string
;        push    offset string
;        push    offset aMovsxESSPushES ; "movsx e%s,%s;push e%s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 14h

	push	mapstr	 ; str
	call	Asm

	call	NextTok

	jmp	DoParam_if9end

DoParam_if10else:			; CODE XREF: DoParam+21Fj
	cmp	[tok], tk_byte	  ; if11
	jnz	DoParam_if9end

	xor	eax, eax
	mov	al, byte [string + 0]
	invoke	wsprintfA, mapstr, aMovsxECxSPushE, eax, string, eax
;        xor     eax, eax
;        mov     al, string[0]
;        push    eax
;        push    offset string
;        xor     eax, eax
;        mov     al, string[0]
;        push    eax
;        push    offset aMovsxECxSPushE ; "movsx e%cx,%s;push e%cx"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 14h

	push	mapstr	 ; str
	call	Asm

	call	NextTok

DoParam_if9end:
	jmp	DoParam_loopcontinue

DoParam_if8else:
	cmp	[tok], tk_var	  ; if12
	jz	DoParam_if12start

	cmp	[tok], tk_local
	jz	DoParam_if12start

	cmp	[tok], tk_param
	jnz	DoParam_if12else

DoParam_if12start:
	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname
	mov	[ebp+vtok], eax

	cmp	[type], tk_dword  ; if13
	jnz	DoParam_if13else

DoParam_D0:
	lea	eax, [ebp+varName]

	invoke	wsprintfA, mapstr, aPushS_1, eax
;        push    eax
;        push    offset aPushS_1 ; "push %s"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	call	NextTok

	jmp	DoParam_if13end

DoParam_if13else:
	jmp	DoParam_if12else

DoParam_if13end:
	cmp	[type], tk_int	  ; if14
	jnz	DoParam_if14end

	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname

	mov	[ebp+vtok], eax

	jmp	DoParam_D0

DoParam_if14end:
	jmp	DoParam_loopcontinue

DoParam_if12else:
	push	tk_dword
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression

	call	PushEAX

DoParam_loopcontinue:
	jmp	DoParam_loopstart

DoParam_loopend:
	lea	eax, [ebp+s]
	invoke	lstrcpyA, string, eax
;        lea     eax, [ebp+s]
;        push    eax             ; lpString2
;        push    offset string   ; lpString1
;        call    lstrcpyA

	lea	eax, [ebp+s2]
	invoke	lstrcpyA, string2, eax
;        lea     eax, [ebp+s2]
;        push    eax             ; lpString2
;        push    offset string2  ; lpString1
;        call    lstrcpyA

	pop	[displaytokerrors]
	pop	[endoffile]
	pop	[linenumber]
	pop	[currmod]
	pop	[inptr]
	pop	[input]
	pop	[tok]
	pop	[tok2]
	pop	[number]
	pop	[inptr2]
	pop	[linenum2]

	mov	al, byte [ebp+holdcha]
	mov	byte [cha2], al

	leave
	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
