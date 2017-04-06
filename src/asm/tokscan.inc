; ---------------------------------------------------------------------------
; Преобразование текущего символа для констукций типа: \n, \x00,  т.п.
; ---------------------------------------------------------------------------
  proc	ConvertChar
hold		= -4

	enter	4, 0
	cmp	byte [cha], '\'        ; if1
	jz	ConvertChar_if1end

	mov	al, byte [cha]

	leave
	ret

ConvertChar_if1end:
	call	NextChar	; Обработка следующего за \

	cmp	al, '0' 	; if2
	jb	ConvertChar_if2end

	cmp	al, '9'
	ja	ConvertChar_if2end

	xor	edx, edx	; Десятичная константа

	sub	al, '0'

	movzx	eax, al

	add	edx, eax

	mov	ecx, 2

ConvertChar_loop1start:
	mov	ebx, edx

	shl	edx, 1

	shl	ebx, 3

	add	edx, ebx

	call	NextChar

	movzx	eax, al

	cmp	al, '0' 	; if3
	jb	ConvertChar_ERR

	cmp	al, '9'
	ja	ConvertChar_ERR

	sub	al, '0'

	add	edx, eax

	loop	ConvertChar_loop1start

	mov	al, dl

	leave
	ret

ConvertChar_ERR:
	push	ConvertChar_str1 ; "decimal digit"
	call	expectederror

	mov	al, 0

	leave
	ret

ConvertChar_if2end:
	cmp	byte [cha], 'a'        ; if4
	jnz	ConvertChar_if4end

	mov	al, 7		; \a

	leave
	ret

ConvertChar_if4end:
	cmp	byte [cha], 'b'        ; if5
	jnz	ConvertChar_if5end

	mov	al, 8		; \b

	leave
	ret

ConvertChar_if5end:
	cmp	byte [cha], 'f'        ; if6
	jnz	ConvertChar_if6end

	mov	al, 0Ch 	; \f

	leave
	ret

ConvertChar_if6end:
	cmp	byte [cha], 'l'        ; if7
	jnz	ConvertChar_if7end

	mov	al, 10

	leave
	ret

ConvertChar_if7end:
	cmp	byte [cha], 'n'        ; if8
	jnz	ConvertChar_if8end

	mov	al, 13

	leave
	ret

ConvertChar_if8end:
	cmp	byte [cha], 'p'        ; if9
	jnz	ConvertChar_if9end

	mov	al, '_'

	leave
	ret

ConvertChar_if9end:
	cmp	byte [cha], 'r'        ; if10
	jnz	ConvertChar_if10end

	mov	al, 0Dh

	leave
	ret

ConvertChar_if10end:
	cmp	byte [cha], 't'        ; if11
	jnz	ConvertChar_if11end

	mov	al, 9

	leave
	ret

ConvertChar_if11end:
	cmp	byte [cha], 'v'        ; if12
	jnz	ConvertChar_if12end

	mov	al, 0Bh

	leave
	ret

ConvertChar_if12end:
	cmp	byte [cha], 'x'        ; if13 HEX константа
	jnz	ConvertChar_if13end

	mov	ecx, 2

	mov	dword [ebp+hold], 0

ConvertChar_loop2start:
	push	ecx
	call	NextChar
	movzx	eax, al
  
	invoke	CharUpperA, eax    
;       push    eax             ; lpsz
;       call    CharUpperA
  
	movzx	ebx, al

	cmp	al, '0' 	; if14
	jb	ConvertChar_if14start

	cmp	al, '9'
	jbe	ConvertChar_LX1

ConvertChar_if14start:
	cmp	al, 'A' 	; if15
	jnb	ConvertChar_LX1

	cmp	al, 'F'
	ja	ConvertChar_ERR1

ConvertChar_LX1:
	mov	edx, [ebp+hold]
	shl	edx, 4

	cmp	bl, 'A' 	; if16
	jb	ConvertChar_if16end

	sub	bl, 'A'-10-'0'

ConvertChar_if16end:
	sub	bl, '0'
	add	edx, ebx
	mov	dword [ebp+hold], edx

	pop	ecx
	loop	ConvertChar_loop2start

	mov	al, byte [ebp+hold]

	leave
	ret

ConvertChar_ERR1:
	pop	ecx

	push	ConvertChar_str2 ; "hexdecimal digit"
	call	expectederror

	mov	al, 0

	leave
	ret

ConvertChar_if13end:
	mov	al, byte [cha]

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Определение типа token;
; ---------------------------------------------------------------------------
  proc	TokScan
useme		= -14h
strptr		= -10h
next		= -0Ch
dirPrefix	= -8
locPrefix	= -4
tok4		= 8
type4		= 0Ch
src4		= 10h
post4		= 14h
string4 	= 18h
number4 	= 1Ch

	enter	14h, 0

	mov	dword [ebp+dirPrefix], 0
	mov	dword [ebp+locPrefix], 0

TokScan_SC_0:
	mov	eax, [ebp+string4]
	mov	dword [ebp+strptr], eax

	mov	dword [ebp+next], 1

	mov	eax, [ebp+number4]

	mov	dword [eax], 0

	mov	eax, [ebp+type4]

	mov	dword [eax], 0

	mov	eax, [ebp+src4]

	mov	dword [eax], 0

	mov	eax, [ebp+post4]

	mov	dword [eax], 0

	call	WhiteSpaces

	mov	esi, [ebp+string4]
	mov	byte [esi], 0

	mov	ecx, 17

	mov	edi, Delim1 ; "#\"'-+*/%|&!^=><@"

	mov	al, byte [cha]

	repne	scasb

	jcxz	TokScan_SC00	; Не первая группа разделителе

	sub	edi, Delim1 ; "#\"'-+*/%|&!^=><@"
	dec	edi
	shl	edi, 2
	add	edi, Jmp_Delim1

	jmp	dword [edi]

TokScan_SC00:
	mov	ecx, 14

	mov	edi, Delim2 ; ":;(){}[],.$?~"

	mov	al, [cha]

	repne	scasb

	jcxz	TokScan_SC01	; Не вторая группа разделителе

	sub	edi, Delim2 ; ":;(){}[],.$?~"
	dec	edi
	add	edi, tk_delim2

	movzx	eax, byte [edi]

	mov	ebx, [ebp+tok4]
	mov	[ebx], eax

	jmp	TokScan_ScEx

TokScan_SC01:
	cmp	dword [ebp+locPrefix], 0 ; if1
	jz	TokScan_if1end

	xchg	edi, [ebp+strptr]
	mov	al, '@'
	stosb
	xchg	edi, [ebp+strptr]

TokScan_if1end:
	cmp	byte [cha], 0	       ; if2 Конец входного буфера
	jnz	TokScan_if2else

	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_eof

	mov	dword [ebp+next], 0

	jmp	TokScan_if2end

TokScan_if2else:
	xor	eax, eax

	mov	al, [cha]
	invoke	IsCharAlphaA , eax
;        push    eax             ; ch
;        call    IsCharAlphaA

	test	eax, eax	; if3 Идентификатор начинается с буквы
	jnz	TokScan_UP ; Идентификатор начинается с буквы

	cmp	byte [cha], '_'
	jnz	TokScan_if3else

TokScan_UP:				; CODE XREF: TokScan+C9j TokScan+F9j ...
	xchg	edi, [ebp+strptr] ; Копируем идентификатор в string4

	mov	al, byte [cha]

	stosb

	xchg	edi, [ebp+strptr]

	call	NextChar

	xor	eax, eax
	mov	al, byte [cha]
	invoke	IsCharAlphaNumericA, eax
;        push    eax             ; ch
;        call    IsCharAlphaNumericA

	test	eax, eax	; loop condition
	jnz	TokScan_UP ; loop1end

	cmp	byte [cha], '_'        ; loop condition
	jz	TokScan_UP ; loop2end

	mov	eax, [ebp+strptr]
	sub	eax, [ebp+string4]

	cmp	eax, IDLENGTH	; if3 ----------------------
	jb	TokScan_if3end

	push	TokScan_str1 ; "Maximum length for an identifier exceed"...
	call	preerror

	mov	eax, [ebp+string4]
	add	eax, IDLENGTH
	dec	eax
	mov	dword [ebp+strptr], eax

TokScan_if3end:
	mov	edi, [ebp+strptr]

	mov	al, 0

	stosb

	mov	ebx, [ebp+tok4]

	mov	dword [ebx], tk_id ; DSDWORD[EBX]=tk_id;

	cmp	dword [ebp+locPrefix], 0 ; if4 -----------------------
	jnz	TokScan_FL

	push	St_Directives ; table
	push	dword [ebp+string4]   ; probe
	call	FastSearch	; Это зарезервированное слово? +if4
	jnb	TokScan_if5end ; Команда обнаружена в списке

	mov	ebx, [ebp+number4]

	mov	dword [ebx], eax      ; Запомним порядковый номер

	mov	ebx, [ebp+tok4] ; Укажем соответствующий token

	cmp	dword [ebp+dirPrefix], 0 ; if6 -----------------
	jz	TokScan_if6else

	mov	dword [ebx], tk_directive

	jmp	TokScan_if6end

TokScan_if6else:
	mov	dword [ebx], tk_command

TokScan_if6end:
	mov	dword [ebp+dirPrefix], 0

	mov	dword [ebp+next], 0

	jmp	TokScan_ScEx

TokScan_if5end:
	push	St_Mnemonics ; table
	push	dword [ebp+string4]   ; probe
	call	FastSearch	; Это мнемоника? +if7
	jnb	TokScan_if7end ; Мнемоника ассемблера

	mov	ebx, [ebp+number4]

	mov	dword [ebx], eax      ; Запомним номер мнемоники

	mov	ebx, [ebp+tok4]

	mov	dword [ebx], tk_mnemonics

	mov	dword [ebp+next], 0

	jmp	TokScan_ScEx

TokScan_if7end:
	push	St_Registers ; table
	push	dword [ebp+string4]   ; probe
	call	FastSearch	; Это имя регистра? +if8
	jnb	TokScan_FL ; Регистр

	mov	ebx, [ebp+number4]

	mov	dword [ebx], eax      ; Запомним номер региста
	shr	eax, 3

	cmp	eax, 2
	jg	TokScan_R0 ; Управляющие регистры?

	mov	ebx, [ebp+tok4]

	mov	dword [ebx], tk_reg

	mov	ebx, [ebp+type4]
	shl	eax, 1
	add	eax, tk_byte
	mov	dword [ebx], eax

	jmp	TokScan_R1

TokScan_R0:
	mov	ebx, [ebp+tok4]
	sub	eax, 3
	add	eax, tk_controlreg
	mov	[ebx], eax

TokScan_R1:
	mov	dword [ebp+next], 0

	jmp	TokScan_ScEx

TokScan_FL:
	mov	eax, [ebp+tok4]

	mov	eax, [eax]

	cmp	al, tk_id	; if8
	jnz	TokScan_if8end

	push	dword [ebp+number4]
	push	dword [ebp+string4]
	push	dword [ebp+type4]
	push	dword [ebp+tok4]
	call	SearchLocals	; Есть в списке локальных?

	mov	eax, [ebp+tok4]
	mov	eax, [eax]

	cmp	al, tk_id	; if9
	jnz	TokScan_if8end

	cmp	dword [ebp+locPrefix], 0 ; if10 (@label)
	jz	TokScan_if10end

	mov	ebx, [ebp+tok4]

	mov	dword [ebx], tk_locallabel

	cmp	dword [displaytokerrors], 0 ; if11
	jz	TokScan_if11end

	push	0
	push	0
	push	tk_locallabel
	push	string
	call	AddLocalvar

TokScan_if11end:
	mov	dword [ebp+locPrefix], 0

	jmp	TokScan_FL

TokScan_if10end:
	push	dword [ebp+number4]   ; number4
	push	dword [ebp+string4]   ; string4
	push	dword [ebp+post4]     ; post4
	push	dword [ebp+src4]      ; src4
	push	dword [ebp+type4]     ; type4
	push	dword [ebp+tok4]      ; tok4
	call	SearchTree

TokScan_if8end:
	cmp	dword [ebp+dirPrefix], 0 ; Конструкция: #ident
	jz	TokScan_if15end ; if12

	mov	dword [ebp+dirPrefix], 0

	mov	ebx, [ebp+tok4]

	mov	eax, [ebx]

	cmp	al, tk_id	; if13 (Есть в списке?)
	jnz	TokScan_if13end

	cmp	[displaytokerrors], 0 ; if14
	jz	TokScan_if13end

	mov	eax, [ebp+post4]

	mov	dword [eax], 1 ; Добавим в список

	mov	ebx, [ebp+tok4]

	mov	dword [ebx], tk_undefproc

	push	dword [ebp+string4]   ; keystring
	call	AddToTree

TokScan_if13end:
	mov	eax, [ebp+post4] ; Идентификатор есть в списке
	mov	eax, [eax]
	mov	ebx, [ebp+tok4]

	test	eax, eax	; if15
	jz	TokScan_if15else ; (Еще не обработанный идентификатор?)

	mov	dword [ebx], tk_postnumber

	jmp	TokScan_if15end

TokScan_if15else:
	mov	eax, [ebp+tok4]

	mov	eax, [eax]

	cmp	eax, tk_param	; if16
	jnz	TokScan_if16else

	mov	dword [ebx], tk_locnumber

	jmp	TokScan_if15end

TokScan_if16else:
	cmp	eax, tk_local	; if17
	jnz	TokScan_if17else

	mov	dword [ebx], tk_locnumber

	jmp	TokScan_if15end

TokScan_if17else:
	mov	dword [ebx], tk_number

TokScan_if15end:
	mov	dword [ebp+next], 0

	jmp	TokScan_ScEx

	jmp	TokScan_if2end

TokScan_if3else:
	xor	eax, eax

	mov	al, byte [cha]
	push	eax
	call	IsNumber	; Идентификатор начинается с цифры

	test	eax, eax	; if18
	jz	TokScan_if18else

	mov	eax, [ebp+tok4]

	mov	dword [eax], tokens ; Пока неизвестный token

	cmp	[cha], '0'	  ; if19
	jnz	TokScan_if25end

	call	NextChar	; (hex - число)

	cmp	[cha], 'X'	  ; if20
	jz	TokScan_if20begin

	cmp	[cha], 'x'
	jnz	TokScan_if20else

TokScan_if20begin:
	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_number

TokScan_HEX:
	call	NextChar
	movzx	eax, al

	invoke	CharUpperA, eax
;       push    eax
;       call    CharUpperA

	cmp	al, '0' 	; if21
	jl	TokScan_EHEX

	cmp	al, '9'
	ja	TokScan_HEXAF

	sub	al, '0'

TokScan_loop3start:
	mov	ebx, [ebp+number4]

	mov	ecx, [ebx]

	shl	ecx, 4
	add	eax, ecx

	mov	[ebx], eax

	jmp	TokScan_HEX

TokScan_HEXAF:
	cmp	al, 'A' 	; if22
	jb	TokScan_EHEX

	cmp	al, 'F'
	ja	TokScan_EHEX

	sub	al, '7'

	jmp	TokScan_loop3start

TokScan_EHEX:
	jmp	TokScan_if25end

TokScan_if20else:
	cmp	byte [cha], 'B'        ; if23 (binary число)
	jz	TokScan_if23begin

	cmp	byte [cha], 'b'
	jnz	TokScan_if23else

TokScan_if23begin:
	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_number

TokScan_loop4start:
	call	NextChar

	cmp	al, '0' 	; if24
	jnz	TokScan_EBIN

	cmp	al, '1'
	jnz	TokScan_EBIN

	sub	al, '0'

	mov	ebx, [ebp+number4]

	mov	ecx, [ebx]

	shl	ecx, 1

	add	eax, ecx

	mov	dword [ebx], eax

	jmp	TokScan_loop4start

TokScan_EBIN:
	jmp	TokScan_if25end

TokScan_if23else:
	cmp	byte [cha], 'O'        ; if25 (octal число)
	jz	TokScan_if25begin

	cmp	byte [cha], 'o'
	jnz	TokScan_if25end

TokScan_if25begin:
	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_number

TokScan_loop5start1:
	call	NextChar

	cmp	al, '0' 	; if26
	jb	TokScan_if25end

	cmp	al, '7'
	ja	TokScan_if25end

	sub	al, '0'

	mov	ebx, dword [ebp+number4]

	mov	ecx, [ebx]
	shl	ecx, 3
	add	eax, ecx

	mov	[ebx], eax

	jmp	TokScan_loop5start1

TokScan_if25end:
	mov	eax, dword [ebp+tok4]

	mov	eax, [eax]

	cmp	eax, tk_number	; if27 (decimal число)
	jz	TokScan_if27end

TokScan_loop6start:
	xor	eax, eax

	mov	al, byte [cha]

	cmp	al, '0' 	; if28
	jb	TokScan_loop6end

	cmp	al, '9'
	ja	TokScan_loop6end

	sub	al, '0'

	mov	ebx, dword [ebp+number4]

	mov	edx, [ebx]

	mov	ecx, edx
	shl	edx, 1
	shl	ecx, 3
	add	edx, ecx
	add	eax, edx
	mov	dword [ebx], eax

	call	NextChar

	jmp	TokScan_loop6start

TokScan_loop6end:
	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_number

TokScan_if27end:
	mov	dword [ebp+next], 0

	jmp	TokScan_if2end

TokScan_if18else:
	cmp	[displaytokerrors], 0 ; if29
	jz	TokScan_if29end

	push	TokScan_str2 ; "tokenizer: bad character value"
	call	preerror

TokScan_if29end:
	call	NextChar

	push	dword [ebp+number4]   ; int
	push	dword [ebp+string4]   ; string4
	push	dword [ebp+post4]     ; int
	push	dword [ebp+src4]      ; int
	push	dword [ebp+type4]     ; int
	push	dword [ebp+tok4]      ; int
	call	TokScan

	mov	dword [ebp+next], 0

TokScan_if2end:
	jmp	TokScan_ScEx

TokScan_Jmp_Number:
	call	NextChar	; #directive || #identifier

	mov	dword [ebp+dirPrefix], 1

	jmp	TokScan_SC_0

TokScan_Jmp_Local:
	call	NextChar	; @LocalLabel

	mov	dword [ebp+locPrefix], 1

	jmp	TokScan_SC01	; if1

TokScan_Jmp_String:
	call	NextChar	; Строка символов в "" (loop7start)

	cmp	byte [cha], '"'        ; if30 (Закрывающая кавычка)
	jz	TokScan_loop7end ; break

	mov	eax, [ebp+strptr]
	sub	eax, [ebp+string4]

	cmp	eax, STRLEN-1	; if31
	jnb	TokScan_if31else

	call	ConvertChar

	mov	edi, dword [ebp+strptr]
	stosb

	cmp	al, 13		; if32
	jnz	TokScan_if32end

	cmp	byte [cha], 'n'
	jnz	TokScan_if32end

	mov	al, 10
	stosb			; Добавим char 10 для \n

TokScan_if32end:
	mov	dword [ebp+strptr], edi

	jmp	TokScan_loop7cond

TokScan_if31else:
	cmp	[displaytokerrors], 0 ; if33
	jz	TokScan_loop8start

	push	TokScan_str3 ; "Maximum String Length Exceeded"
	call	preerror

TokScan_loop8start:
	cmp	byte [cha], '"'
	jz	TokScan_loop8end

	cmp	[endoffile], 0	  ; if34
	jnz	TokScan_loop8end

	call	NextChar

	jmp	TokScan_loop8start

TokScan_loop8end:
	jmp	TokScan_loop7end

TokScan_loop7cond:
	cmp	byte [cha], 0	       ; loop7 condition
	jnz	TokScan_Jmp_String     ; jz

TokScan_loop7end:
	mov	edi, dword [ebp+strptr]

	mov	byte [edi], 0

	sub	edi, [ebp+string4]
	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_string

	mov	eax, [ebp+number4] ; Запомним длину строк
	mov	[eax], edi

	cmp	byte [cha], '"'        ; if35
	jz	TokScan_if35end1

	push	'"'
	call	expected

TokScan_if35end1:
	jmp	TokScan_ScEx

TokScan_Jmp_Const:
	call	NextChar	; Символьная константа: 'AbCD'

	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_number

	mov	eax, [ebp+number4]

	mov	dword [eax], 0

TokScan_loop9:
	cmp	byte [cha], 27h        ; '
	jz	TokScan_loop9end

	cmp	[endoffile], 0
	jnz	TokScan_loop9end

	call	ConvertChar

	movzx	eax, al
	mov	ebx, dword [ebp+number4]
	mov	ecx, [ebx]
	shl	ecx, 8
	add	eax, ecx

	mov	dword [ebx], eax

	call	NextChar

	jmp	TokScan_loop9

TokScan_loop9end:
	cmp	byte [cha], 27h        ; if36 '
	jz	TokScan_if36else1

	cmp	[displaytokerrors], 0 ; if37
	jz	TokScan_if37end1

	push	27h
	call	expected

TokScan_if37end1:
	jmp	TokScan_if36end1

TokScan_if36else1:
	call	NextChar

TokScan_if36end1:
	mov	dword [ebp+next], 0

	jmp	TokScan_ScEx

TokScan_Jmp_Minus:
	call	NextChar	; Jmp_Minus

	mov	ebx, [ebp+tok4]

	cmp	byte [cha], '='        ; if38
	jnz	TokScan_if38else

	mov	dword [ebx], tk_minusequals ; -=

	jmp	TokScan_if38end1

TokScan_if38else:
	cmp	byte [cha], '-'        ; if39
	jnz	TokScan_if39else

	mov	dword [ebx], tk_minusminus ; --

	jmp	TokScan_if38end1

TokScan_if39else:
	mov	dword [ebx], tk_minus

	mov	dword [ebp+next], 0

TokScan_if38end1:
	jmp	TokScan_ScEx

TokScan_Jmp_Plus:
	call	NextChar	; Jmp_Plus

	mov	ebx, [ebp+tok4]

	cmp	byte [cha], '='        ; if40
	jnz	TokScan_if40else1

	mov	dword [ebx], tk_plusequals ; +=

	jmp	TokScan_if40end

TokScan_if40else1:
	cmp	byte [cha], '+'        ; if41
	jnz	TokScan_if41else

	mov	dword [ebx], tk_plusplus     ; ++

	jmp	TokScan_if40end

TokScan_if41else:
	call	WhiteSpaces

	mov	ebx, [ebp+tok4] ; возможно наличие пробелов

	cmp	byte [cha], '-'        ; if42 (оптимизация + -)
	jnz	TokScan_if42else

	mov	dword [ebx], tk_minus

	jmp	TokScan_if40end

TokScan_if42else:
	mov	dword [ebx], tk_plus

	mov	dword [ebp+next], 0

TokScan_if40end:
	jmp	TokScan_ScEx

TokScan_Jmp_Mul:
	call	NextChar	; Jmp_Mul

	call	WhiteSpaces

	mov	ebx, dword [ebp+tok4]

	cmp	byte [cha], '-'        ; if43 (*-)
	jnz	TokScan_if43else

	mov	dword [ebx], tk_multminus

	jmp	TokScan_if43end

TokScan_if43else:
	mov	dword [ebx], tk_mult

	mov	dword [ebp+next], 0		  ; *

TokScan_if43end:
	jmp	TokScan_ScEx

TokScan_Jmp_Div:
	call	NextChar	; Jmp_Div

	cmp	byte [cha], '*'        ; if44 (Комментарий)
	jnz	TokScan_if44else

	call	NextChar

	mov	dword [ebp+useme], 1	 ; В useme - счетчик вложенных комментариев

TokScan_loop10start:
	cmp	dword [ebp+useme], 0
	jbe	TokScan_loop10end

	call	WhiteSpaces

	cmp	byte [cha], '*'        ; if45
	jnz	TokScan_if45else

	call	NextChar

	cmp	byte [cha], '/'        ; if46 (Закрытие комментария)
	jnz	TokScan_if46end

	cmp	dword [ebp+useme], 0  ; if47
	jbe	TokScan_if47end

	dec	dword [ebp+useme]     ; Уменишим счетчик

TokScan_if47end:
	call	NextChar

TokScan_if46end:
	jmp	TokScan_if37end

TokScan_if45else:
	cmp	byte [cha], '/'        ; if48
	jnz	TokScan_if48else

	call	NextChar

	cmp	byte [cha], '*'        ; if49 (Вложенный комментарий)
	jnz	TokScan_if49end

	inc	dword [ebp+useme]

	call	NextChar

TokScan_if49end:
	jmp	TokScan_if37end

TokScan_if48else:
	call	NextChar	; Не ограничители комментария

TokScan_if37end:
	jmp	TokScan_loop10start
; ---------------------------------------------------------------------------

TokScan_loop10end:			; CODE XREF: TokScan+602j
	cmp	[endoffile], 0	  ; if50
	jz	TokScan_if50else

	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_eof

	cmp	dword [ebp+useme], 0  ; if51
	jbe	TokScan_if51end

	cmp	[displaytokerrors], 0
	jz	TokScan_if51end

	call	unexpectedeof

TokScan_if51end:
	jmp	TokScan_if44end

TokScan_if50else:
	push	dword [ebp+number4]   ; int
	push	dword [ebp+string4]   ; string4
	push	dword [ebp+post4]     ; int
	push	dword [ebp+src4]      ; int
	push	dword [ebp+type4]     ; int
	push	dword [ebp+tok4]      ; int
	call	TokScan

TokScan_if44end:
	jmp	TokScan_if35end

TokScan_if44else:
	cmp	byte [cha], '/'        ; if52 (Комментарий до конца строки //)
	jnz	TokScan_if52else

TokScan_loop11start:			; CODE XREF: TokScan+6B5j
	call	NextChar

	cmp	[endoffile], 0
	jnz	TokScan_loop11end

	cmp	byte [cha], 10	       ; loop condition
	jnz	TokScan_loop11start

TokScan_loop11end:
	cmp	[endoffile], 0	  ; if53
	jz	TokScan_if53else

	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_eof

	jmp	TokScan_if53end

TokScan_if53else:
	call	WhiteSpaces

	push	dword [ebp+number4]   ; int
	push	dword [ebp+string4]   ; string4
	push	dword [ebp+post4]     ; int
	push	dword [ebp+src4]      ; int
	push	dword [ebp+type4]     ; int
	push	dword [ebp+tok4]      ; int
	call	TokScan

TokScan_if53end:
	jmp	TokScan_if35end

TokScan_if52else:
	call	WhiteSpaces

	cmp	byte [cha], '-'        ; if54
	jnz	TokScan_if54else

	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_divminus

	call	NextChar

	jmp	TokScan_if35end

TokScan_if54else:
	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_div

TokScan_if35end:
	mov	dword [ebp+next], 0

	jmp	TokScan_ScEx

TokScan_Jmp_Mod:
	call	NextChar

	call	WhiteSpaces

	cmp	byte [cha], '-'        ; if55
	jnz	TokScan_if55else

	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_modminus

	jmp	TokScan_if55end

TokScan_if55else:
	mov	eax, [ebp+tok4]

	mov	dword [eax], tk_mod

	mov	dword [ebp+next], 0

TokScan_if55end:
	jmp	TokScan_ScEx

TokScan_Jmp_Or:
	call	NextChar

	cmp	byte [cha], '='        ; if56
	jnz	TokScan_if56else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_orequals

	jmp	TokScan_if56end

TokScan_if56else:
	cmp	byte [cha], '|'        ; if57
	jnz	TokScan_if57else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_oror

	jmp	TokScan_if56end

TokScan_if57else:
	call	WhiteSpaces

	cmp	byte [cha], '-'        ; if58
	jnz	TokScan_if58else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_orminus

	jmp	TokScan_if56end

TokScan_if58else:
	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_or

	mov	dword [ebp+next], 0

TokScan_if56end:
	jmp	TokScan_ScEx

TokScan_Jmp_And:
	call	NextChar

	cmp	byte [cha], '='        ; if59
	jnz	TokScan_if59else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_andequals

	jmp	TokScan_if59end

TokScan_if59else:
	cmp	byte [cha], '&'        ; if60
	jnz	TokScan_if60else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_andand

	jmp	TokScan_if59end

TokScan_if60else:
	call	WhiteSpaces

	cmp	byte [cha], '-'        ; if61
	jnz	TokScan_if61else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_andminus

	jmp	TokScan_if59end

TokScan_if61else:
	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_and

	mov	dword [ebp+next], 0

TokScan_if59end:
	jmp	TokScan_ScEx

TokScan_Jmp_Not:
	call	NextChar

	cmp	byte [cha], '='        ; if62
	jnz	TokScan_if62else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_notequal

	jmp	TokScan_if62end

TokScan_if62else:
	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_not

	mov	dword [ebp+next], 0

TokScan_if62end:
	jmp	TokScan_ScEx

TokScan_Jmp_Xor:
	call	NextChar

	cmp	byte [cha], '='        ; if63
	jnz	TokScan_if63else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_xorequals

	jmp	TokScan_if63end

TokScan_if63else:
	call	WhiteSpaces

	cmp	byte [cha], '-'        ; if64
	jnz	TokScan_if64else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_xorminus

	jmp	TokScan_if63end

TokScan_if64else:
	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_xor

	mov	dword [ebp+next], 0

TokScan_if63end:
	jmp	TokScan_ScEx

TokScan_Jmp_Equal:
	call	NextChar

	cmp	byte [cha], '='        ; if65
	jnz	TokScan_if65else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_equalto

	jmp	TokScan_if65end

TokScan_if65else:
	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_assign

	mov	dword [ebp+next], 0

TokScan_if65end:
	jmp	TokScan_ScEx

TokScan_Jmp_Great:
	call	NextChar

	cmp	byte [cha], '>'        ; if66
	jnz	TokScan_if66else

	call	NextChar

	cmp	byte [cha], '='        ; if67
	jnz	TokScan_if67else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_rrequals

	jmp	TokScan_if67end

TokScan_if67else:
	call	WhiteSpaces

	cmp	byte [cha], '-'        ; if68
	jnz	TokScan_if68else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_rrminus

	jmp	TokScan_if67end

TokScan_if68else:
	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_rr

	mov	dword [ebp+next], 0

TokScan_if67end:
	jmp	TokScan_if66end

TokScan_if66else:
	cmp	byte [cha], '<'        ; if69
	jnz	TokScan_if69else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_swap

	jmp	TokScan_if66end

TokScan_if69else:
	cmp	byte [cha], '='        ; if70
	jnz	TokScan_if70else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_greaterequal

	jmp	TokScan_if66end

TokScan_if70else:
	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_greater

	mov	dword [ebp+next], 0

TokScan_if66end:
	jmp	TokScan_ScEx

TokScan_Jmp_Less:
	call	NextChar

	cmp	byte [cha], '<'        ; if71
	jnz	TokScan_if71else

	call	NextChar

	cmp	byte [cha], '='        ; if72
	jnz	TokScan_if72else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_llequals

	jmp	TokScan_if72end

TokScan_if72else:
	call	WhiteSpaces

	cmp	byte [cha], '-'        ; if73
	jnz	TokScan_if73else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_llminus

	jmp	TokScan_if72end

TokScan_if73else:
	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_ll

	mov	dword [ebp+next], 0

TokScan_if72end:
	jmp	TokScan_ScEx

TokScan_if71else:
	cmp	byte [cha], '>'        ; if74
	jnz	TokScan_if74else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_notequal

	jmp	TokScan_ScEx

TokScan_if74else:
	cmp	byte [cha], '='        ; if75
	jnz	TokScan_if75else

	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_lessequal

	jmp	TokScan_ScEx

TokScan_if75else:
	mov	eax, dword [ebp+tok4]

	mov	dword [eax], tk_less

	mov	dword [ebp+next], 0

TokScan_ScEx:
	cmp	dword [ebp+next], 0   ; if76
	jz	TokScan_if76end

	call	NextChar

TokScan_if76end:
	leave
	ret	18h
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------