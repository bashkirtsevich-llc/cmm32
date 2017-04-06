;
; деревья
;
; ---------------------------------------------------------------------------
; Занесение поименованной константы в список
; ---------------------------------------------------------------------------
  proc	AddConstToTree
  ptr_		  = -8
  newptr	  = -4

  keystring	  = 8
  constvalue	  = 0Ch

	enter	8, 0

	invoke	LocalAlloc, recsize, 40h
	mov	[ebp+newptr], eax
;        push    recsize         ; recsize
;        push    40h             ; uFlags
;        call    LocalAlloc

	test	eax, eax	; if1
	jnz	AddConstToTree_if1end

	push	AddConstToTree_str1 ; "Compiler out of memory for identifier t"...
	call	preerror

;        invoke  ExitProcess, e_outofmemory
	call	RestoreState

AddConstToTree_if1end:
	mov	eax, dword [treestart]
	mov	dword [ebp+ptr_], eax

	test	eax, eax	; if2 (Пустой список?)
	jnz	AddConstToTree_loop1start

	mov	eax, [ebp+newptr]
	mov	dword [treestart], eax

	jmp	AddConstToTree_if2end

AddConstToTree_loop1start:
	; Поиск свободной ссылки
	mov	esi, dword [ebp+ptr_]

	invoke	lstrcmpA, dword [ESI+recid], dword [ebp+keystring]
;        push    [ebp+keystring] ; lpString2
;        push    dword ptr [esi+8] ; lpString1
;        call    lstrcmpA

	mov	esi, [ebp+ptr_]

	test	eax, eax	; if3
	jge	AddConstToTree_if3else

	cmp	dword [esi+left], 0 ; if4 (Нашли пустой левый - добавим)
	jnz	AddConstToTree_if4end

	mov	eax, [ebp+newptr]
	mov	dword [esi+left], eax

	jmp	AddConstToTree_if2end

AddConstToTree_if4end:
	mov	eax, [esi+left]
	mov	[ebp+ptr_], eax

	jmp	AddConstToTree_if3end

AddConstToTree_if3else:
	test	eax, eax	; if5
	jz	AddConstToTree_if5else

	cmp	dword [esi+right], 0 ; if6 (Нашли пустой правый - добавим)
	jnz	AddConstToTree_if6end

	mov	eax, [ebp+newptr]
	mov	dword [esi+right], eax

	jmp	AddConstToTree_if2end

AddConstToTree_if6end:
	mov	eax, [esi+right]
	mov	[ebp+ptr_], eax

	jmp	AddConstToTree_if3end

AddConstToTree_if5else:
	push	AddConstToTree_str2 ; "string found in tree when trying to add"...
	call	internalerror

AddConstToTree_if3end:
	jmp	AddConstToTree_loop1start

AddConstToTree_if2end:
	mov	esi, [ebp+newptr] ; Формируем новую запись в списке

	invoke	lstrlenA, [ebp+keystring]
;        push    [ebp+keystring] ; lpString
;        call    lstrlenA

	inc	eax
	invoke	LocalAlloc, 40h, eax
;        push    eax             ; uBytes
;        push    40h             ; uFlags
;        call    LocalAlloc
	mov	[esi+recid], eax

	invoke	lstrcpyA, [ebp+keystring], [esi+recid]
;        push    [ebp+keystring] ; lpString2
;        push    dword ptr [esi+recid] ; lpString1
;        call    lstrcpyA

	mov	esi, [ebp+newptr]

	mov	dword [esi+newid], 0

	mov	dword [esi+rectok], tk_number

	mov	eax, [ebp+constvalue]
	mov	dword [esi+recnumber], eax

	mov	dword [esi+recpost], 0

	mov	dword [esi+left], 0

	mov	dword [esi+right], 0

	mov	eax, [currmod]
	shl	eax, 16
	add	eax, [linenumber]
	mov	dword [esi+recmodline], eax

	leave
	ret	8
  endp

; ---------------------------------------------------------------------------
; Добавить локальную переменную в список
; ---------------------------------------------------------------------------
  proc	AddLocalvar
  newptr	  = -4
  str_		   = 8
  tk		  = 0Ch
  ltype 	  = 10h
  num		  = 14h

	enter	4, 0

	invoke	LocalAlloc, 40h, local_size
;        push    local_size      ; local_size
;        push    40h             ; uFlags
;        call    LocalAlloc
	mov	[ebp+newptr], eax

	test	eax, eax	; if1
	jnz	AddLocalvar_if1end

	push	AddLocalvar_str1 ; "Compiler out of memory for local symbol"...
	call	preerror

;        invoke  ExitProcess, e_outofmemory
	call	RestoreState

AddLocalvar_if1end:
	cmp	[locallist], 0	  ; if2
	jnz	AddLocalvar_if2else

	mov	eax, [ebp+newptr]
	mov	[locallist], eax

	jmp	AddLocalvar_if2end

AddLocalvar_if2else:
	mov	eax, [locallist]
	xchg	eax, ebx

AddLocalvar_loopstart:
	mov	eax, [ebx+localnext]

	test	eax, eax
	jz	AddLocalvar_loopend

	xchg	eax, ebx

	jmp	AddLocalvar_loopstart

AddLocalvar_loopend:
	mov	eax, [ebp+newptr]
	mov	[ebx+localnext], eax

AddLocalvar_if2end:
	mov	ebx, [ebp+newptr]

	mov	eax, ebx
	add	eax, localid

	invoke	lstrcpyA, eax, [ebp+str_]
;        push    [ebp+str_]       ; lpString2
;        mov     eax, ebx
;        add     eax, localid
;        push    eax             ; lpString1
;        call    lstrcpyA

	mov	ebx, [ebp+newptr]

	mov	eax, [ebp+tk]
	mov	[ebx+localtok], eax ; localtok

	mov	eax, [ebp+ltype]
	mov	[ebx+localtype], eax ; localtype

	mov	eax, [ebp+num]
	mov	dword [ebx+localnumber], eax ; localnumber

	mov	dword [ebx+localnext], 0 ; ebx+localnext

	mov	[localptr], ebx

	leave
	retn	10h
  endp
; ---------------------------------------------------------------------------
; Добавить идентификатор в список
; ---------------------------------------------------------------------------
  proc	AddToTree
  ptr_		  = -8
  newptr	  = -4
  keystring	  = 8

	enter	8, 0

	invoke	LocalAlloc, 40h, recsize
;        push    recsize         ; recsize
;        push    40h             ; uFlags
;        call    LocalAlloc

	mov	[ebp+newptr], eax

	test	eax, eax	; if1
	jnz	AddToTree_if1end

	call	outofmemory

AddToTree_if1end:
	mov	eax, [treestart]
	mov	[ebp+ptr_], eax

	test	eax, eax	; if2
	jnz	AddToTree_loopstart

	mov	eax, [ebp+newptr]
	mov	[treestart], eax

	jmp	AddToTree_if2end

; Поиск свободной ссылки
AddToTree_loopstart:
	mov	esi, [ebp+ptr_]

	invoke	lstrcmpA, [esi+recid], [ebp+keystring]
;        push    [ebp+keystring] ; lpString2
;        push    dword ptr [esi+recid] ; lpString1
;        call    lstrcmpA

	mov	esi, [ebp+ptr_]

	test	eax, eax	; if3
	jge	AddToTree_if3else

	cmp	dword [esi+left], 0 ; if4 (Нашли пустой левый - добавим)
	jnz	AddToTree_if4end

	mov	eax, [ebp+newptr]
	mov	[esi+left], eax

	jmp	AddToTree_if2end

AddToTree_if4end:
	mov	eax, [esi+left]
	mov	[ebp+ptr_], eax

	jmp	AddToTree_if3end

AddToTree_if3else:
	test	eax, eax	; if5
	jz	AddToTree_if5else

	cmp	dword[esi+right], 0 ; if6 (Нашли пустой правый - добавим)
	jnz	AddToTree_if6end

	mov	eax, [ebp+newptr]
	mov	[esi+right], eax

	jmp	AddToTree_if2end

AddToTree_if6end:
	mov	eax, [esi+right]
	mov	[ebp+ptr_], eax

	jmp	AddToTree_if3end

AddToTree_if5else:
	push	AddConstToTree_str2 ; "string found in tree when trying to add"...
	call	internalerror

AddToTree_if3end:
	jmp	AddToTree_loopstart

AddToTree_if2end:
	mov	esi, [ebp+newptr]

	invoke	lstrlenA, [ebp+keystring]
;        push    [ebp+keystring] ; lpString
;        call    lstrlenA

	inc	eax

	invoke	LocalAlloc, 40h, eax
;        push    eax             ; uBytes
;        push    40h             ; uFlags
;        call    LocalAlloc

	mov	[esi+recid], eax

	invoke	lstrcpyA, eax, [ebp+keystring]
;        push    [ebp+keystring] ; lpString2
;        push    eax             ; lpString1
;        call    lstrcpyA

	cmp	[tok], tk_string  ; if7
	jnz	AddToTree_if7else

	mov	esi, [ebp+newptr]
	mov	eax, [number]
	inc	eax

	invoke	LocalAlloc, 40h, eax
;        push    eax             ; uBytes
;        push    40h             ; uFlags
;        call    LocalAlloc

	mov	[esi+newid], eax

	test	eax, eax	; if8
	jnz	AddToTree_if8end

	call	outofmemory

AddToTree_if8end:
	mov	ecx, [number]
	mov	edi, eax
	mov	esi, string

	rep movsb

	jmp	AddToTree_if7end

AddToTree_if7else:
	invoke	lstrlenA, string
;                push    offset string   ; lpString
;                call    lstrlenA

	test	eax, eax	; if9
	jnz	AddToTree_if9else

	mov	esi, [ebp+newptr]

	mov	dword [esi+newid], 0

	jmp	AddToTree_if7end

AddToTree_if9else:
	mov	esi, [ebp+newptr]
	invoke	lstrlenA, string
;        push    offset string   ; lpString
;        call    lstrlenA

	inc	eax

	invoke	LocalAlloc, 40h, eax
;        push    eax             ; uBytes
;        push    40h             ; uFlags
;        call    LocalAlloc

	mov	[esi+newid], eax

	test	eax, eax	; if10
	jnz	AddToTree_if10end

	call	outofmemory

AddToTree_if10end:
	invoke	lstrcpyA, eax, string
;                push    offset string   ; lpString2
;                push    eax             ; lpString1
;                call    lstrcpyA

AddToTree_if7end:
	mov	esi, dword [ebp+newptr]

	mov	eax, dword [tok]
	mov	[esi+rectok], eax ; rectok

	mov	eax, dword [number]
	mov	[esi+recnumber], eax ; recnumber

	mov	eax, dword [type]
	mov	[esi+rectype], eax ; rectype

	mov	eax, dword [src]
	mov	[esi+recsrc], eax ; recsrc

	mov	eax, dword [post]
	mov	[esi+recpost], eax ; recpost

	mov	dword [esi+left], 0 ; left

	mov	dword [esi+right], 0 ; right

	mov	eax, dword [modline]
	mov	[esi+recmodline], eax ; recmodline

	mov	eax, dword [ebp+newptr]
	mov	[treeptr], eax

	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
; Вывод всех идентификаторов
; ---------------------------------------------------------------------------
  proc	DisplayTree

	push	DisplayTree_str1
;        push    [mapfile]         ; hFile
	call	fprint

	push	DisplayTree_str2 ; "tok    type      number    post\tIDENTIF"...
;        push    [mapfile]         ; hFile
	call	fprint

	mov	[numberofids], 0

	push	[treestart]
	call	DisplayTreeAll

	invoke	wsprintfA, mapstr, DisplayTree_str3, [numberofids]
;        push    numberofids
;        push    offset DisplayTree_str3 ; "\r\n    %u Unique Global Identifiers.\r\n\r\n"...
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; lpBuffer
;        push    [mapfile]         ; hFile
	call	fprint

	push	DisplayTree_str4 ; "GLOBAL CONSTANT IDENTIFIER LIST:\r\n"
;        push    [mapfile]         ; hFile
	call	fprint

	mov	[numberofids], 0

	push	[treestart]
	call	DisplayTreeConstants

	invoke	wsprintfA, mapstr, DisplayTree_str5, [numberofids]
;        push    numberofids
;        push    offset DisplayTree_str5 ; "\r\n    %u Unique Global Constant Value I"...
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	push	mapstr	 ; lpBuffer
;        push    [mapfile]         ; hFile
	call	fprint

	ret
  endp
; ---------------------------------------------------------------------------
; Вывод всего списка идентифиакторов
; ---------------------------------------------------------------------------
  proc	DisplayTreeAll
  ptr_		  = 8

	push	ebp
	mov	ebp, esp

	cmp	dword [ebp+ptr_], 0   ; if1
	jz	DisplayTreeAll_if1end

	mov	esi, [ebp+ptr_]

	push	dword [esi+right]
	call	DisplayTreeAll

	mov	esi, [ebp+ptr_]

if DEFINE_MAKE_EXE
	mov	eax, [esi+rectok]
	sub	eax, [esi+recpost]
	cmp	eax, tk_API	; if2
	jz	DisplayTreeAll_if2end
end if

; как эту шнягу разрулить?
	invoke	wsprintfA, mapstr, DisplayTreeAll_str1, dword [esi+rectok], dword [esi+rectype], dword [esi+recnumber], dword [esi+recpost], dword [esi+recid]
	add	esp, 1Ch

	push	mapstr	 ; lpBuffer
	call	fprint

	mov	eax, [esi+newid]

	test	eax, eax	; if3
	jz	DisplayTreeAll_if3end

	invoke	lstrcmpA, [esi+recid], eax

	test	eax, eax	; if4
	jz	DisplayTreeAll_if3end

	mov	esi, [ebp+ptr_]

	invoke	wsprintfA, mapstr, DisplayTreeAll_str2, [esi+newid]
	add	esp, 0Ch

	push	mapstr	 ; lpBuffer
	call	fprint

DisplayTreeAll_if3end:
	cmp	[list], 0	  ; if5
	jz	DisplayTreeAll_if5end

	mov	esi, [ebp+ptr_]
	mov	eax, [esi+recsrc]

	test	eax, eax	; if6
	jz	DisplayTreeAll_if5end

	mov	ebx, [esi+recmodline]
	shr	ebx, 16

	mov	eax, ebx
	imul	eax, FILENAMESIZE
	add	eax, modules	; offset modules

	mov	ebx, eax

	mov	eax, [esi+recmodline]
	and	eax, 0FFFFh

	invoke	wsprintfA, mapstr, DisplayTreeAll_str3, ebx, eax, [esi+recsrc]
	add	esp, 14h

	push	mapstr	 ; lpBuffer
	call	fprint

	mov	esi, [ebp+ptr_]

	invoke	LocalFree, [esi+recsrc]

	mov	dword [esi+recsrc], 0

DisplayTreeAll_if5end:
	inc	[numberofids]

if DEFINE_MAKE_EXE
DisplayTreeAll_if2end:
end if
	mov	esi, [ebp+ptr_]

	push	dword [esi+left]
	call	DisplayTreeAll

DisplayTreeAll_if1end:
	pop	ebp
	ret	4
  endp
; ---------------------------------------------------------------------------
; Вывод списка глобальных констант
; ---------------------------------------------------------------------------
  proc	DisplayTreeConstants
  ptr_		= 8

	push	ebp
	mov	ebp, esp
  
	cmp   dword [ebp+ptr_], 0   ; if1
	jz    DisplayTreeConstants_if1end
  
	mov	esi, [ebp+ptr_]

	push	dword [esi+right]
	call	DisplayTreeConstants

	mov	esi, [ebp+ptr_]

	mov	eax, [esi+rectok]

	cmp	eax, tk_number	; if2
	jnz	DisplayTreeConstants_if2end

	invoke	wsprintfA, mapstr, DisplayTreeConstants_str1, [esi+recnumber], [esi+recnumber], [esi+recid]
;        push    dword ptr [esi+recid] ; recnumber
;        push    dword ptr [esi+recnumber] ; recid
;        push    dword ptr [esi+recnumber] ; recid
;        push    offset DisplayTreeConstants_str1 ; "#define %10ld /* %8lX hex */ %s\r\n"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 14h

	push	mapstr	 ; lpBuffer
;        push    [mapfile]         ; hFile
	call	fprint

	inc	[numberofids]

DisplayTreeConstants_if2end:
	mov	esi, [ebp+ptr_]
	push	dword [esi+left]
	call	DisplayTreeConstants

DisplayTreeConstants_if1end:
	pop	ebp
	ret	4
  endp
; ---------------------------------------------------------------------------
; Вычисление значения беззнаковой константы
; ---------------------------------------------------------------------------
  proc	DoConstDwordMath
  value 	  = -4

	enter	4, 0

	cmp	[tok], tk_minus   ; if1
	jnz	DoConstDwordMath_if1end

	call	NextTok

	cmp	[tok], tk_number  ; if2
	jz	DoConstDwordMath_if2end

	call	numexpected

	xor	eax, eax

	leave
	ret

DoConstDwordMath_if2end:
	mov	eax, [number]
	neg	eax
	mov	[number], eax

DoConstDwordMath_if1end:
	cmp	[tok], tk_number  ; if3
	jz	DoConstDwordMath_if3end

	call	numexpected

	xor	eax, eax

	leave
	ret

DoConstDwordMath_if3end:
	mov	eax, [number]
	mov	[ebp+value], eax

DoConstDwordMath_loopstart:
	call	tok2isopperand

	test	eax, eax	; loop condition
	jz	DoConstDwordMath_loopend

	call	NextTok

	cmp	[tok2], tk_number ; if4
	jz	DoConstDwordMath_if4end

	mov	eax, [ebp+value]

	leave
	ret

DoConstDwordMath_if4end:
	mov	eax, [tok]
	sub	eax, 49 	; switch 25 cases
	cmp	eax, 24
	ja	DoConstDwordMath_case_end ; default

	jmp	[DoConstDwordMath_case+eax*4] ; switch jump ; DoConstDwordMath_case[eax*4]

DoConstDwordMath_tk_minus:
	mov	eax, [number2]	  ; jumptable 00403293 case 49
	sub	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_plus:
	mov	eax, [number2]	  ; jumptable 00403293 case 50
	add	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_xor:
	mov	eax, [number2]	  ; jumptable 00403293 case 69
	xor	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_and:
	mov	eax, [number2]	  ; jumptable 00403293 case 68
	and	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_or:
	mov	eax, [number2]	  ; jumptable 00403293 case 67
	or	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_mod:
	mov	eax, [ebp+value] ; jumptable 00403293 case 55
	xor	edx, edx
	div	[number2]
	xchg	eax, edx
	mov	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_div:
	mov	eax, [ebp+value] ; jumptable 00403293 case 54
	xor	edx, edx
	div	[number2]
	mov	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_mult:
	mov	eax, [ebp+value] ; jumptable 00403293 case 53
	mul	[number2]
	mov	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_rr:
	mov	cl, byte [number2] ; jumptable 00403293 case 59
	shr	dword [ebp+value], cl
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_ll:
	mov	cl, byte [number2] ; jumptable 00403293 case 60
	shl	dword [ebp+value], cl
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_xorminus:
	mov	eax, [number2]	  ; jumptable 00403293 case 73
	neg	eax
	xor	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_andminus:
	mov	eax, [number2]	  ; jumptable 00403293 case 72
	neg	eax
	and	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_orminus:
	mov	eax, [number2]	  ; jumptable 00403293 case 71
	neg	eax
	or	[ebp+value], eax
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_rrminus:
	mov	al, byte [number2] ; jumptable 00403293 case 61
	neg	al
	mov	cl, al
	shr	dword [ebp+value], cl
	jmp	DoConstDwordMath_case_end ; default

DoConstDwordMath_tk_llminus:
	mov	al, byte [number2] ; jumptable 00403293 case 62
	neg	al
	mov	cl, al
	shl	dword [ebp+value], cl

DoConstDwordMath_case_end:		; default
	call	NextTok
					; jumptable 00403293 cases 51,52,56-58,63-66,70
	jmp	DoConstDwordMath_loopstart

DoConstDwordMath_loopend:
	mov	eax, [ebp+value]

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Вычисление значения знаковой константы
; ---------------------------------------------------------------------------
  proc	DoConstMath
  value 	  = -4

	enter	4, 0

	cmp	[tok], tk_minus   ; if1
	jnz	DoConstMath_if1end

	call	NextTok

	cmp	[tok], tk_number  ; if2
	jz	DoConstMath_if2end

	call	numexpected

	xor	eax, eax

	leave
	ret

DoConstMath_if2end:
	mov	eax, [number]
	neg	eax
	mov	[number], eax

DoConstMath_if1end:
	cmp	[tok], tk_number  ; if3
	jz	DoConstMath_if3end

	call	numexpected

	xor	eax, eax

	leave
	ret

DoConstMath_if3end:
	mov	eax, [number]
	mov	[ebp+value], eax

DoConstMath_loop1:
	call	tok2isopperand

	test	eax, eax
	jz	DoConstMath_loop1end

	call	NextTok

	cmp	[tok2], tk_number ; if4
	jz	DoConstMath_if4end

	mov	eax, [ebp+value]

	leave
	retn

DoConstMath_if4end:
	mov	eax, [tok]
	sub	eax, 31h	; switch 25 cases

	cmp	eax, 18h
	ja	DoConstMath_case_end ; default
					; jumptable 004033E8 cases 51,52,56-58,63-66,70
	jmp	[DoConstMath_case + eax*4] ; switch jump

DoConstMath_tk_minus:
	mov	eax, [number2]	  ; jumptable 004033E8 case 49
	sub	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_plus:
	mov	eax, [number2]	  ; jumptable 004033E8 case 50
	add	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_xor:
	mov	eax, [number2]	  ; jumptable 004033E8 case 69
	xor	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_and:
	mov	eax, [number2]	  ; jumptable 004033E8 case 68
	and	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_or:
	mov	eax, [number2]	  ; jumptable 004033E8 case 67
	or	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_mod:
	mov	eax, [ebp+value] ; jumptable 004033E8 case 55
	cdq
	idiv	[number2]
	xchg	eax, edx
	mov	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_div:
	mov	eax, [ebp+value] ; jumptable 004033E8 case 54
	cdq
	idiv	[number2]
	mov	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_mult:
	mov	eax, [ebp+value] ; jumptable 004033E8 case 53
	imul	[number2]
	mov	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_rr:
	mov	cl, byte [number2] ; почему так? ; jumptable 004033E8 case 59
	sar	dword [ebp+value], cl
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_ll:
	mov	cl, byte [number2] ; разобраться ; jumptable 004033E8 case 60
	shl	dword [ebp+value], cl
	jmp	DoConstMath_case_end ; default


DoConstMath_tk_xorminus:
	mov	eax, [number2]	  ; jumptable 004033E8 case 73
	neg	eax
	xor	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_andminus:
	mov	eax, [number2]	  ; jumptable 004033E8 case 72
	neg	eax
	and	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_orminus:
	mov	eax, [number2]	  ; jumptable 004033E8 case 71
	neg	eax
	or	[ebp+value], eax
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_rrminus:
	mov	al, byte [number2] ; jumptable 004033E8 case 61
	neg	al
	mov	cl, al
	sar	dword [ebp+value], cl
	jmp	DoConstMath_case_end ; default

DoConstMath_tk_llminus:
	mov	al, byte [number2] ; jumptable 004033E8 case 62
	neg	al
	mov	cl, al
	shl	dword [ebp+value], cl

DoConstMath_case_end:
	call	NextTok 	; default

	jmp	DoConstMath_loop1

DoConstMath_loop1end:
	mov	eax, [ebp+value]

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Вычисление значения знаковой константы
; ---------------------------------------------------------------------------
  proc	DoConstLongMath
  value 	  = -4

	enter	4, 0

	call	DoConstMath
	mov	[ebp+value], eax

	call	NextTok

	mov	eax, [ebp+value]

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Следующий token - операция?
; ---------------------------------------------------------------------------
  proc	tok2isopperand
	mov	eax, [tok2]

	cmp	eax, tk_plus
	jz	tok2isopperand_ifend

	cmp	eax, tk_minus
	jz	tok2isopperand_ifend

	cmp	eax, tk_mult
	jz	tok2isopperand_ifend

	cmp	eax, tk_div
	jz	tok2isopperand_ifend

	cmp	eax, tk_mod
	jz	tok2isopperand_ifend

	cmp	eax, tk_rr
	jz	tok2isopperand_ifend

	cmp	eax, tk_ll
	jz	tok2isopperand_ifend

	cmp	eax, tk_or
	jz	tok2isopperand_ifend

	cmp	eax, tk_and
	jz	tok2isopperand_ifend

	cmp	eax, tk_xor
	jz	tok2isopperand_ifend

	cmp	eax, tk_divminus
	jz	tok2isopperand_ifend

	cmp	eax, tk_modminus
	jz	tok2isopperand_ifend

	cmp	eax, tk_multminus
	jz	tok2isopperand_ifend

	cmp	eax, tk_xorminus
	jz	tok2isopperand_ifend

	cmp	eax, tk_orminus
	jz	tok2isopperand_ifend

	cmp	eax, tk_andminus
	jz	tok2isopperand_ifend

	cmp	eax, tk_llminus
	jz	tok2isopperand_ifend

	cmp	eax, tk_rrminus
	jnz	tok2isopperand_end

tok2isopperand_ifend:
	mov	eax, 1

	ret

tok2isopperand_end:
	xor	eax, eax
	ret
  endp
; ---------------------------------------------------------------------------
; Следующий token закрывает выражение?
; ---------------------------------------------------------------------------
  proc	tok2notstopper
	mov	eax, [tok2]

	cmp	eax, tk_semicolon
	jz	tok2notstopper_ifend

	cmp	eax, tk_comma
	jz	tok2notstopper_ifend

	cmp	eax, tk_closebracket
	jz	tok2notstopper_ifend

	cmp	eax, tk_openblock
	jnz	tok2notstopper_ifelse

tok2notstopper_ifend:
	xor	eax, eax
	jmp	tok2notstopper_end

tok2notstopper_ifelse:
	mov	eax, 1

tok2notstopper_end:
	ret
  endp
; ---------------------------------------------------------------------------
; Поиск в списке локальных переменных
; ---------------------------------------------------------------------------
  proc	SearchLocals
  tok4		  = 8
  type4 	  = 0Ch
  string4	  = 10h
  number4	  = 14h

	push	ebp
	mov	ebp, esp

	cmp	[locallist], 0	  ; if1
	jz	SearchLocals_if1end

	mov	eax, [locallist]
	mov	[localptr], eax

SearchLocals_S00:
	mov	esi, eax	; localptr;

	add	eax, localid
	invoke	lstrcmpA, [ebp+string4], eax
;        mov     eax, esi
;        add     eax, localid
;        push    eax             ; lpString2
;        push    [ebp+string4]   ; lpString1
;        call    lstrcmpA

	mov	esi, [localptr]

	test	eax, eax	; if2 (Переменная найдена)
	jnz	SearchLocals_if2else

	mov	ebx, [ebp+number4]

	mov	eax, [esi+localnumber]
	mov	[ebx], eax

	mov	ebx, [ebp+type4]

	mov	eax, [esi+localtype]
	mov	[ebx], eax

	mov	ebx, [ebp+tok4]

	mov	eax, [esi+localtok]
	mov	[ebx], eax

	cmp	eax, tk_local	; if3
	jnz	SearchLocals_if3else

	mov	ebx, [ebp+number4]
	mov	eax, [localsize]
	sub	[ebx], eax

	jmp	SearchLocals_if3end

SearchLocals_if3else:
	cmp	eax, tk_param	; if4
	jnz	SearchLocals_if4else

	mov	ebx, [ebp+number4]

	mov	eax, [ebx]
	add	eax, 4

	mov	[ebx], eax

	cmp	[current_proc_type], cpt_far ; if5
	jnz	SearchLocals_if5end

	add	dword [ebx], 4 ; move over seg on stack

SearchLocals_if5end:
	jmp	SearchLocals_if3end

SearchLocals_if4else:
	cmp	eax, tk_locallabel ; if6
	jz	SearchLocals_if3end

	cmp	eax, tk_number
	jz	SearchLocals_if3end

	push	SearchLocals_str1 ; "Bad *tok4 value in SearchLocals"
	call	internalerror

SearchLocals_if3end:
	jmp	SearchLocals_if1end

SearchLocals_if2else:
	cmp	dword [esi+localnext], 0 ; if7
	jz	SearchLocals_if1end

	mov	eax, [esi+localnext]
	mov	[localptr], eax

	jmp	SearchLocals_S00

SearchLocals_if1end:
	pop	ebp
	ret	10h
  endp
; ---------------------------------------------------------------------------
; Поиск в списке глобальных идентификаторов
; ---------------------------------------------------------------------------
  proc	SearchTree
  ptr_		  = -8
  cmpresult	  = -4
  tok4		  = 8
  type4 	  = 0Ch
  src4		  = 10h
  post4 	  = 14h
  string4	  = 18h
  number4	  = 1Ch

	enter	8, 0

	mov	dword [ebp+cmpresult], 123

	mov	eax, [treestart]
	mov	dword [ebp+ptr_], eax

SearchTree_loopstart:
	mov	esi, eax	; Поиск свободной ссылки

	test	esi, esi	; if1
	jnz	SearchTree_if1end

	mov	[treeptr], 0

	xor	eax, eax
	jmp	SearchTree_end	; Not found

SearchTree_if1end:
	invoke	lstrcmpA, [esi+recid], [ebp+string4]
;        push    [ebp+string4]   ; lpString2
;        push    dword ptr [esi+recid] ; lpString1
;        call    lstrcmpA
	mov	[ebp+cmpresult], eax

	mov	esi, [ebp+ptr_]

	cmp	dword [ebp+cmpresult], 0 ; if2
	jge	SearchTree_if2else

	mov	eax, [esi+left]
	mov	[ebp+ptr_], eax

	jmp	SearchTree_if2end

SearchTree_if2else:
	cmp	dword [ebp+cmpresult], 0 ; if3
	jle	SearchTree_if3else

	mov	eax, [esi+right]
	mov	[ebp+ptr_], eax

	jmp	SearchTree_if2end

SearchTree_if3else:
	jmp	SearchTree_loopend

SearchTree_if2end:
	jmp	SearchTree_loopstart

SearchTree_loopend:
	mov	ebx, [ebp+number4]

	mov	eax, [esi+recnumber]
	mov	[ebx], eax

	mov	ebx, [ebp+type4]

	mov	eax, [esi+rectype]
	mov	[ebx], eax

	mov	ebx, [ebp+src4]

	mov	eax, [esi+recsrc]
	mov	[ebx], eax

	mov	ebx, [ebp+post4]

	mov	eax, [esi+recpost]
	mov	[ebx], eax

	mov	ebx, [ebp+tok4]

	mov	eax, dword [esi+rectok]
	mov	dword [ebx], eax

	cmp	eax, tk_string	; if4
	jnz	SearchTree_if4else

	mov	ebx, dword [ebp+number4]
	mov	ecx, dword [ebx]
	mov	edi, dword [ebp+string4]
	mov	esi, dword [esi+newid] ; newid

	rep	movsb

	jmp	SearchTree_if4end

SearchTree_if4else:
	cmp	dword [esi+newid], 0 ; if5
	jz	SearchTree_if4end

	invoke	lstrcpyA, [ebp+string4], [esi+newid] ; что делает этот код? каково его предназначение?
;       invoke  lstrcpyA, [esi+newid], [ebp+string4]
;        push    dword ptr [esi+newid] ; newid
;        push    [ebp+string4]   ; lpString1
;        call    lstrcpyA

SearchTree_if4end:
	mov	esi, [ebp+ptr_]
	invoke	lstrcmpA, [ESI+recid], [ebp+string4]
;        push    [ebp+string4]   ; string4
;        push    dword ptr [esi+recid] ; recid
;        call    lstrcmpA

	test	eax, eax	; if6 (Проверим: менялось ли имя идентификатора)
	jz	SearchTree_if6end

	push	dword [ebp+number4]   ; number4
	push	dword [ebp+string4]   ; string4
	push	dword [ebp+post4]     ; post4
	push	dword [ebp+src4]      ; src4
	push	dword [ebp+type4]     ; type4
	push	dword [ebp+tok4]      ; tok4
	call	SearchTree	; Да - повторим поиск

SearchTree_if6end:
	mov	eax, [ebp+ptr_]
	mov	dword [treeptr], eax

	xor	eax, eax
	mov	eax, 1

SearchTree_end:
	leave
	ret	18h
  endp
; ---------------------------------------------------------------------------
; Поиск неоткомпилированных еще ссылок
; ---------------------------------------------------------------------------
  proc	SeekToDo
  ptr_		  = 8

	push	ebp
	mov	ebp, esp

	cmp	dword [ebp+ptr_], 0   ; if1
	jz	SeekToDo_if1end

	mov	esi, dword [ebp+ptr_]

	push	dword [esi+right]
	call	SeekToDo

	test	eax, eax	; if2
	jz	SeekToDo_if2end

	xor	eax, eax
	mov	eax, 1

	jmp	SeekToDo_end

SeekToDo_if2end:
	mov	esi, dword [ebp+ptr_]

	mov	eax, dword [esi+recpost]

	cmp	eax, 1		; if3
	jbe	SeekToDo_if3end

	mov	eax, [ebp+ptr_]
	mov	[treeptr], eax

	mov	esi, [ebp+ptr_]

	mov	eax, [esi+recnumber]
	mov	[number], eax

	mov	eax, [esi+rectype]
	mov	[type], eax

	mov	eax, [esi+recmodline]
	mov	[modline], eax

	mov	eax, [esi+recsrc]
	mov	[src], eax

	mov	eax, [esi+recpost]
	mov	[post], eax

	mov	eax, [esi+rectok]
	mov	[tok], eax

	xor	eax, eax
	mov	eax, 1

	jmp	SeekToDo_end

SeekToDo_if3end:
	mov	esi, [ebp+ptr_]

	push	dword [esi+left]
	call	SeekToDo

	test	eax, eax
	jz	SeekToDo_if1end

	xor	eax, eax
	mov	eax, 1

	jmp	SeekToDo_end

SeekToDo_if1end:
	xor	eax, eax

SeekToDo_end:
	pop	ebp
	ret	4
  endp
; ---------------------------------------------------------------------------
; Поиск незакрытых ссылок
; ---------------------------------------------------------------------------
  proc	SeekUndefined
  ptr_		  = 8

	push	ebp
	mov	ebp, esp

	cmp	dword [ebp+ptr_], 0   ; if1
	jz	SeekUndefined_if1end

	mov	esi, [ebp+ptr_]

	push	dword [esi+right]
	call	SeekUndefined

	mov	esi, [ebp+ptr_]

	mov	eax, [esi+rectok]

	cmp	eax, tk_undefproc ; if2
	jnz	SeekUndefined_if2end

	invoke	wsprintfA, mapstr, SeekUndefined_str1, dword [esi+recid]
;        push    dword ptr [esi+recid]
;        push    offset SeekUndefined_str1 ; "'%s' undefined\r\n"
;        push    offset mapstr   ; LPSTR
;        call    wsprintfA
	add	esp, 0Ch

	cmp	[makemapfile], 0  ; if3
	jz	SeekUndefined_if3end

	push	mapstr
;        push    [mapfile]
	call	fprint

SeekUndefined_if3end:
	mov	eax, mapstr
	call	WRITESTR

SeekUndefined_if2end:
	mov	esi, [ebp+ptr_]
	push	dword [esi+left]
	call	SeekUndefined

SeekUndefined_if1end:
	pop	ebp
	ret	4
  endp
