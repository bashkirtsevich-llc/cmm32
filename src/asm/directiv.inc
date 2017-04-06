; ---------------------------------------------------------------------------
; Для tokens, НЕ обрабатываемых через таблицу переключателей
; ---------------------------------------------------------------------------
  proc	EMPTY
	mov	eax, string
	call	WRITESTR

	mov	eax, EMPTY_str ; "-ToDo\r\n"
	call	WRITESTR

	call	NextTok

	ret
  endp
; ---------------------------------------------------------------------------
; Возвращает адрес из Jmp_....
; ---------------------------------------------------------------------------
  proc	GetDirAddr
  table 	  = 8
  num		  = 0Ch

	push	ebp
	mov	ebp, esp

	mov	eax, [ebp+num]
	shl	eax, 2
	add	eax, [ebp+table]

	mov	eax, [eax] ; dword

	pop	ebp
	ret	8
  endp
; ---------------------------------------------------------------------------
; Директива #define
; ---------------------------------------------------------------------------
  proc	DirDefine
  holdid	  = -48h
  next		  = -4

	enter	48h, 0		; IDLENGTH

	mov	dword [ebp+next], 1

	call	NextTok

	cmp	[tok], tk_id	  ; if1
	jnz	DirDefine_if1else

	lea	eax, [ebp+holdid]
	invoke	lstrcpyA, eax, string ; Имя константы

	call	NextTok

	cmp	[tok], tk_eof	  ; if2
	jnz	DirDefine_if2else

	call	unexpectedeof

	jmp	DirDefine_if2end

DirDefine_if2else:
	cmp	[tok], tk_number  ; if3
	jnz	DirDefine_if3end

	call	DoConstLongMath

	push	eax		; constvalue
	lea	eax, [ebp+holdid]
	push	eax		; keystring
	call	AddConstToTree

	mov	dword [ebp+next], 0

	jmp	DirDefine_if2end

DirDefine_if3end:
	cmp	[tok], tk_minus   ; if4
	jnz	DirDefine_if4end

	cmp	[tok2], tk_number ; if5
	jnz	DirDefine_if5end

	call	DoConstLongMath

	push	eax		; constvalue
	lea	eax, [ebp+holdid]
	push	eax		; keystring
	call	AddConstToTree

	mov	dword [ebp+next], 0

DirDefine_if5end:
	jmp	DirDefine_if2end

DirDefine_if4end:
	cmp	[tok], tk_undefproc ; if6
	jnz	DirDefine_if6else

	mov	[tok], tk_id
	lea	eax, [ebp+holdid]
	push	eax		; keystring
	call	AddToTree

	jmp	DirDefine_if2end

DirDefine_if6else:
	lea	eax, [ebp+holdid]
	push	eax		; keystring
	call	AddToTree

DirDefine_if2end:
	jmp	DirDefine_if1end

DirDefine_if1else:
	call	idexpected

DirDefine_if1end:
	cmp	dword [ebp+next], 0   ; if7
	jz	DirDefine_end

	call	NextTok

DirDefine_end:
	leave
	ret
  endp
; ---------------------------------------------------------------------------
; #enum
; ---------------------------------------------------------------------------
  proc	DirEnum
  counter	  = -48h
  holdid	  = -44h

	enter	48h, 0

	mov	dword [ebp+counter], 0

	call	NextTok

	cmp	[tok], tk_openbrace
	jz	DirEnum_loopstart

	push	'{'
	call	expected

DirEnum_loopstart:
	call	NextTok

	cmp	[tok], tk_id	  ; if1
	jnz	DirEnum_if1end

	lea	eax, [ebp+holdid]
	invoke	lstrcpyA, eax, string

	cmp	[tok2], tk_assign ; if2
	jnz	DirEnum_if2end

	call	NextTok

	call	NextTok

	cmp	[tok], tk_number  ; if3
	jnz	DirEnum_if3else

	call	DoConstLongMath
	mov	[ebp+counter], eax

	jmp	DirEnum_if2end

DirEnum_if3else:
	call	numexpected

DirEnum_if2end:
	push	dword [ebp+counter]   ; constvalue
	lea	eax, [ebp+holdid]
	push	eax		; keystring
	call	AddConstToTree

	inc	dword [ebp+counter]

	jmp	DirEnum_loopcontinue

DirEnum_if1end:
	cmp	[tok], tk_comma
	jz	DirEnum_loopcontinue

	cmp	[tok], tk_semicolon
	jz	DirEnum_loopend

DirEnum_loopcontinue:
	jmp	DirEnum_loopstart

DirEnum_loopend:
	call	NextTok

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Директива #import
; ---------------------------------------------------------------------------
if DEFINE_MAKE_EXE
  proc	DirImport
	call	NextTok

	cmp	[tok], tk_string  ; if1
	jnz	DirImport_if1else

	push	1
	call	GetImport	; import по имени API-функций

	jmp	DirImport_if1end

DirImport_if1else:
	call	stringexpected

DirImport_if1end:
	call	NextTok

	ret
  endp
end if
; ---------------------------------------------------------------------------
; Директива #importN
; ---------------------------------------------------------------------------
if DEFINE_MAKE_EXE
  proc	DirImportN
	call	NextTok

	cmp	[tok], tk_string  ; if1
	jnz	DirImportN_if1else

	push	0
	call	GetImport	; import по имени API-функций

	jmp	DirImportN_if1end

DirImportN_if1else:
	call	stringexpected

DirImportN_if1end:
	call	NextTok

	ret
  endp
end if
; ---------------------------------------------------------------------------
; Импорт из DLL
; ---------------------------------------------------------------------------
if DEFINE_MAKE_EXE
  proc	GetImport
  dll		  = -110h
  dllpos	  = -10Ch
  base		  = -108h
  export_	   = -104h
  fptr		  = -100h
  i		  = -0FCh
  nexports	  = -0F8h
  nsect 	  = -0F4h
  delta 	  = -0F0h
  Buffer	  = -0ECh ;byte
  PathName	  = -9Ch  ;byte
  tok0		  = -24h
  type0 	  = -20h
  src0		  = -1Ch
  post0 	  = -18h
  number0	  = -14h
  ord		  = -10h
  pname1	  = -0Ch
  pname2	  = -8
  j		  = -4
  byName	  = 8

	enter	110h, 0

	mov	dword [ebp+pname1], 0
	mov	dword [ebp+ord], 0
	mov	[importFlag], 1

	cmp	[DLLcount], MAXDLLS ; if1
	jb	GetImport_if1end

	call	outofmemory2

GetImport_if1end:
	lea	eax, [ebp+number0]
	push	eax		; number4
	push	string		; string4
	lea	eax, [ebp+post0]
	push	eax		; post4
	lea	eax, [ebp+src0]
	push	eax		; src4
	lea	eax, [ebp+type0]
	push	eax		; type4
	lea	eax, [ebp+tok0]
	push	eax		; tok4
	call	SearchTree

	test	eax, eax	; if2 (DLL уже импортирован)
	jnz	GetImport_end

	lea	eax, [ebp+PathName]
	invoke	wsprintfA, eax, GetImport_str1, string
	add	esp, 0Ch

	lea	eax, [ebp+PathName]
	invoke	_lopen, eax, 0
	mov	[ebp+dll], eax

	cmp	dword [ebp+dll], -1 ; if3 (invalid_handle_value)
	jnz	GetImport_if3end

	lea	eax, [ebp+Buffer]
	invoke	GetSystemDirectoryA, eax, 80

	lea	eax, [ebp+PathName]
	lea	ebx, [ebp+Buffer]
	invoke	wsprintfA, eax, GetImport_str2, ebx, string
	add	esp, 10h

	lea	eax, [ebp+PathName]
	invoke	_lopen, eax, 0

	mov	[ebp+dll], eax

	cmp	dword [ebp+dll], -1 ; if4 (invalid_handle_value)
	jnz	GetImport_if3end

	push	string
	call	unabletoopen

	jmp	GetImport_end

GetImport_if3end:
	mov	dword [ebp+nsect], 0

	invoke	_llseek, [ebp+dll], 3Ch, 0

	lea	eax, [ebp+fptr]
	invoke	_lread, [ebp+dll], eax, 4

	mov	eax, [ebp+fptr]
	add	eax, 120
	invoke	_llseek, [ebp+dll], eax, 0

	lea	eax, [ebp+export_]
	invoke	_lread, [ebp+dll], eax, 4

	cmp	dword [ebp+export_], 0 ; if5
	jnz	GetImport_if5end

	invoke	wsprintfA, mapstr, GetImport_str3, string
	add	esp, 0Ch

	push	mapstr
	call	preerror

	jmp	GetImport_end

GetImport_if5end:
	mov	eax, [ebp+fptr]
	add	eax, 6
	invoke	_llseek, [ebp+dll], eax, 0

	lea	eax, [ebp+nsect]
	invoke	_lread, [ebp+dll], eax, 2

	mov	eax, [ebp+export_]
	mov	[ebp+delta], eax

	mov	dword [ebp+i], 1

GetImport_loop1start:
	mov	eax, [ebp+nsect]
	cmp	[ebp+i], eax ; loop condition
	ja	GetImport_loop1end

	mov	eax, [ebp+i]

	dec	eax

	imul	eax, 40

	add	eax, 260

	add	eax, [ebp+fptr]

	invoke	_llseek, [ebp+dll], eax, 0

	invoke	_lread,

	mov	eax, [ebp+export_]
	cmp	[ebp+base], eax ; if6
	ja	GetImport_if6end

	mov	eax, [ebp+export_]
	sub	eax, [ebp+base]

	cmp	[ebp+delta], eax ; if7
	jbe	GetImport_if6end

	mov	eax, [ebp+export_]
	sub	eax, [ebp+base]

	mov	[ebp+delta], eax
	mov	eax, [ebp+i]

	dec	eax

	imul	eax, 40

	add	eax, 268

	add	eax, [ebp+fptr]

	invoke	_llseek, [ebp+dll], eax, 0

	lea	eax, [ebp+dllpos]
	invoke	_lread, [ebp+dll], eax, 4

GetImport_if6end:
	inc	dword [ebp+i]
	jmp	GetImport_loop1start

GetImport_loop1end:
	mov	eax, [ebp+dllpos]
	add	eax, [ebp+delta]
	mov	[ebp+dllpos], eax ; filepos for export_ directory table

	mov	eax, [ebp+dllpos]
	sub	eax, [ebp+export_]
	mov	[ebp+delta], eax

	mov	eax, [ebp+dllpos]
	add	eax, 24
	invoke	_llseek, [ebp+dll], eax, 0

	lea	eax, [ebp+nexports]
	invoke	_lread, [ebp+dll], eax, 4

	mov	eax, [ebp+dllpos]
	add	eax, 32
	invoke	_llseek, [ebp+dll], eax, 0

	lea	eax, [ebp+base]
	invoke	_lread, [ebp+dll], eax, 4

	mov	eax, [ebp+dllpos]
	add	eax, 36
	invoke	_llseek, [ebp+dll], eax, 0

	lea	eax, [ebp+fptr]
	invoke	_lread, [ebp+dll], eax, 4

	mov	eax, [ebp+base]
	add	eax, [ebp+delta]
	mov	[ebp+base], eax

	mov	eax, [ebp+fptr]
	add	eax, [ebp+delta]
	mov	[ebp+fptr], eax

	mov	eax, [tok]
	mov	[ebp+tok0], eax

	mov	eax, [number]
	mov	[ebp+number0], eax

	mov	eax, [src]
	mov	[ebp+src0], eax

	mov	eax, [type]
	mov	[ebp+type0], eax

	mov	eax, post
	mov	[ebp+post0], eax

	mov	[tok], tk_DLL

	mov	eax, [ebp+nexports]
	mov	[number], eax

	mov	[src], NULL

	mov	eax, [ebp+byName]
	mov	[type], eax

	mov	[post], 0

	mov	[modline], 0

	push	string	 ; keystring
	call	AddToTree

	mov	ebx, [DLLcount]

	shl	ebx, 2

	mov	eax, [treeptr]
	mov	[DLLlist + ebx], eax ; save ptr in tree КАК ТУТ БЫТЬ? [DLLlist + ebx]??? ; DLLlist[ebx]
	; олли показывает так: MOV DWORD PTR DS:[EBX+40CD18],EAX

	mov	[tok], tk_API

	mov	eax, [treeptr]
	mov	[type], eax

	mov	dword [ebp+i], 0

GetImport_loop2start:
	mov	eax, [ebp+nexports]
	dec	eax
	cmp	[ebp+i], eax	; loop condition
	jnb	GetImport_loop2end

	mov	eax, [ebp+i]
	shl	eax, 1

	add	eax, [ebp+fptr]

	invoke	_llseek, [ebp+dll], eax, 0

	lea	eax, [ebp+ord]
	invoke	_lread, [ebp+dll], eax, 2

	mov	eax, [ebp+i]

	shl	eax, 2

	add	eax, [ebp+base]

	invoke	_llseek, [ebp+dll], eax, 0

	lea	eax, [ebp+pname1]
	invoke	_lread, [ebp+dll], eax, 8

	mov	eax, [ebp+pname1]
	add	eax, [ebp+delta]
	invoke	_llseek, [ebp+dll], eax, 0

	mov	eax, [ebp+pname2]
	sub	eax, [ebp+pname1]
	invoke	_lread, [ebp+dll], string, eax

	mov	eax, [ebp+ord]
	inc	eax
	mov	[number], eax	  ; при загрузке используется номер на 1 больше экспортируемого из DLL

	push	string	 ; keystring
	call	AddToTree

	inc	dword [ebp+i]

	jmp	GetImport_loop2start

GetImport_loop2end:
	mov	eax, [ebp+i]

	shl	eax, 1

	add	eax, [ebp+fptr]

	invoke	_llseek, [ebp+dll], eax, 0

	lea	eax, [ebp+ord]
	invoke	_lread, [ebp+dll], eax, 2

	mov	dword [ebp+j], 0

GetImport_loop3start:
	mov	eax, [ebp+pname2]
	add	eax, [ebp+delta]
	add	eax, [ebp+j]
	invoke	_llseek, [ebp+dll], eax, 0
	mov	eax, [ebp+j]

	lea	eax, [eax+84h]
	invoke	_lread, [ebp+dll], eax, 1

	mov	eax, [ebp+j]

	cmp	[string + eax], 0  ; ifN  ; CMP BYTE PTR DS:[EAX+401084],0
	jz	GetImport_loop3end

	inc	dword [ebp+j]

	jmp	GetImport_loop3start

GetImport_loop3end:
	mov	eax, [ebp+ord]
	inc	eax
	mov	[number], eax

	push	string	 ; keystring
	call	AddToTree

	mov	eax, [ebp+tok0]
	mov	[tok], eax

	mov	eax, [ebp+number0]
	mov	[number], eax

	mov	eax, [ebp+src0]
	mov	[src], eax

	mov	eax, [ebp+type0]
	mov	[type], eax

	mov	eax, [ebp+post0]
	mov	[post], eax

	push	dword [ebp+dll]       ; hFile
	call	_lclose

	inc	[DLLcount]

	mov	[importFlag], 0

GetImport_end:
	leave
	ret	4
  endp
end if
; ---------------------------------------------------------------------------
; Директива #include
; ---------------------------------------------------------------------------
  proc	DirInclude
  s		  = -7D0h
  s2		  = -3E8h

	enter	7D0h, 0

	call	NextTok

	cmp	[tok], tk_string  ; if1
	jnz	DirInclude_if1else

	mov	al, byte [cha2]

	push	eax
	push	[linenum2]
	push	[inptr2]
	push	[number]
	push	[tok2]
	push	[tok]
	push	[input]
	push	[inptr]
	push	[endoffile]
	push	[displaytokerrors]
	push	[currmod]

	lea	eax, [ebp+s]
	invoke	lstrcpyA, eax, string

	lea	eax, [ebp+s2]
	invoke	lstrcpyA, eax, string2

	lea	eax, [ebp+s]
	push	eax
	call	Preview

	lea	eax, [ebp+s]
	invoke	lstrcpyA, string, eax

	lea	eax, [ebp+s2]
	invoke	lstrcpyA, string2, eax

	pop	[currmod]
	pop	[displaytokerrors]
	pop	[endoffile]
	pop	[inptr]
	pop	[input]
	pop	[tok]
	pop	[tok2]
	pop	[number]
	pop	[inptr2]
	pop	[linenum2]
	pop	eax

	mov	byte [cha2], al
	call	NextTok

	jmp	DirInclude_if1end

DirInclude_if1else:
	call	stringexpected

DirInclude_if1end:
	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Директива list
; ---------------------------------------------------------------------------
  proc	DirList
;        cmp     [mapfile], 0      ; if1
	jnz	DirList_if1end

	mov	[makemapfile], 1

	call	StartMapfile

DirList_if1end:
	xor	[list], 1	  ; Переключение вывода листинга
	call	NextTok

	ret
  endp
; ---------------------------------------------------------------------------
; Директива map
; ---------------------------------------------------------------------------
  proc	DirMap
	mov	[makemapfile], 1

	call	StartMapfile

	call	NextTok

	ret
  endp
; ---------------------------------------------------------------------------
; Обработка глобальной переменной или процедуры с типом
; ---------------------------------------------------------------------------
  proc	GetProc
  src0		  = -50h
  beg		  = -4Ch
  count 	  = -48h
  var_name	  = -44h
  vartype	  = 8

	enter	50h, 0

	lea	eax, [ebp+var_name]
	invoke	lstrcpyA, eax, string ; Имя процедуры

	mov	eax, [inptr2]
	mov	[ebp+beg], eax	; отметим начало описания

	mov	dword [ebp+count], 0

	xor	eax, eax	; ищем начало блока процедуры

	mov	eax, [currmod]
	shl	eax, 16
	add	eax, [linenum2]
	mov	[modline], eax

GetProc_loop1start:
	xchg	esi, [inptr2]

	lodsb

	xchg	esi, [inptr2]

	mov	[cha2], al

	test	al, al		; if1
	jnz	GetProc_if1end

	call	unexpectedeof

	jmp	GetProc_end

GetProc_if1end:
	cmp	al, 13		; if2
	jnz	GetProc_if2end

	inc	[linenum2]	  ; Обнаружен конец строки

	inc	[totallines]

	jmp	GetProc_loop1continue

GetProc_if2end: 			; CODE XREF: GetProc+57j
	cmp	al, '{' 	; if3 (список инициализации)
	jnz	GetProc_loop1continue

	inc	dword [ebp+count]

	jmp	GetProc_loop2start

GetProc_loop1continue:
	jmp	GetProc_loop1start

GetProc_loop2start:
	xchg	esi, [inptr2]
	lodsb
	xchg	esi, [inptr2]
	mov	[cha2], al

	test	al, al		; if4
	jnz	GetProc_if4end

	call	unexpectedeof

	jmp	GetProc_loop2end

GetProc_if4end:
	cmp	al, 13		; if5
	jnz	GetProc_if5else

	inc	[linenum2]	  ; Обнаружен конец строки
	inc	[totallines]

	jmp	GetProc_if5end

GetProc_if5else:
	cmp	al, '}' 	; if6 (блок закрыт)
	jnz	GetProc_if6else

	dec	dword [ebp+count]

	cmp	dword [ebp+count], 0  ; if7 (конец процедуры)
	jnz	GetProc_if7end

	xchg	esi, [inptr2]
	lodsb
	xchg	esi, [inptr2]

	mov	[cha2], al	  ; замыкающая }
	mov	eax, [inptr2]

	sub	eax, [ebp+beg]
	inc	eax		; +2
	inc	eax

	invoke	LocalAlloc, 40h, eax ; копируем исх.текст
	mov	[ebp+src0], eax

	mov	eax, [ebp+src0]

	mov	byte [eax], '('

;============ замениьт на invoke
	mov	eax, [inptr2]
	sub	eax, [ebp+beg]
	push	eax		; iMaxLength
	push	dword [ebp+beg]       ; lpString2
	mov	eax, [ebp+src0]
	inc	eax
	push	eax		; lpString1
	call	[lstrcpynA]
;============

	mov	[tok], tk_proc

	mov	eax, [ebp+vartype]
	mov	[type], eax

	mov	eax, [ebp+src0]
	mov	[src], eax

	mov	[number], 0

	mov	[post], 1

	lea	eax, [ebp+var_name]
	push	eax		; keystring
	call	AddToTree

	jmp	GetProc_loop2end

GetProc_if7end:
	jmp	GetProc_if5end

GetProc_if6else:
	cmp	al, '{' 	; if8 (список инициализации)
	jnz	GetProc_if5end

	inc	dword [ebp+count]

GetProc_if5end:
	jmp	GetProc_loop2start

GetProc_loop2end:
	call	NextTok

GetProc_end:
	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
; Обработка глобальной переменной или процедуры с типом
; ---------------------------------------------------------------------------
  proc	GetVar
  src0		  = -58h
  beg		  = -54h
  end_		  = -50h
  count 	  = -4Ch
  size		  = -48h
  var_name	  = -44h
  vartype	  = 8

	enter	58h, 0

	mov	eax, [inptr]
	mov	[ebp+beg], eax

	mov	[modline], 0 ; отметим начало описания

	call	NextTok

	cmp	[tok2], tk_openbracket ; if1 (Объявление функции: type FunctionName(...))
	jnz	GetVar_loopstart

	push	dword [ebp+vartype]
	call	GetProc

	jmp	GetVar_end

GetVar_loopstart:
	cmp	[tok], tk_semicolon ; if2 (Объявление переменной Конец определения переменной)
	jnz	GetVar_if2end

	mov	[tok], tk_var

	mov	eax, [ebp+vartype]
	mov	[type], eax

	mov	eax, [ebp+src0]
	mov	[src], eax

	mov	[number], 0

	mov	[post], 1

	lea	eax, [ebp+var_name]
	push	eax
	call	AddToTree

	jmp	GetVar_loopend

GetVar_if2end:
	cmp	[tok], tk_comma   ; if3 (список переменных)
	jnz	GetVar_if3else

	mov	[tok], tk_var

	mov	eax, [ebp+vartype]
	mov	[type], eax

	mov	eax, [ebp+src0]
	mov	[src], eax

	mov	[number], 0

	mov	[post], 1

	lea	eax, [ebp+var_name]
	push	eax		; keystring
	call	AddToTree

	call	NextTok

	jmp	GetVar_if3end

GetVar_if3else:
	cmp	[tok], tk_id	  ; if4
	jnz	GetVar_if4else

	mov	dword [ebp+src0], NULL

	mov	eax, [inptr2]
	mov	dword [ebp+beg], eax

	mov	dword [ebp+size], 0

	lea	eax, [ebp+var_name]
	invoke	lstrcpyA, eax, string ; Имя переменной

	mov	[number], 0

	mov	[tok], tk_var

	mov	eax, [ebp+vartype]
	mov	[type], eax

	mov	[post], 1

	call	NextTok

	jmp	GetVar_if3end

GetVar_if4else:
	cmp	[tok], tk_assign  ; if5
	jz	GetVar_if5start

	cmp	[tok], tk_openblock
	jnz	GetVar_if3end

GetVar_if5start:
	dec	[inptr2]

	mov	dword [ebp+count], 0

	xor	eax, eax

GetVar_loop2start:
	xchg	esi, [inptr2]

	lodsb

	xchg	esi, [inptr2]

	mov	byte [cha2], al

	test	al, al		; if6
	jnz	GetVar_if6end

	call	unexpectedeof

	jmp	GetVar_loop2end

GetVar_if6end:
	cmp	al, '"' 	; if7
	jnz	GetVar_if7else

	xchg	esi, [inptr2]

GetVar_loop3start:
	lodsb
	cmp	al, '"' 	; loop condition
	jnz	GetVar_loop3start

	xchg	esi, [inptr2]

	mov	byte [cha2], al

	jmp	GetVar_if12end

GetVar_if7else:
	cmp	al, ',' 	; if8
	jz	GetVar_if8start

	cmp	al, ';'
	jnz	GetVar_if8else

GetVar_if8start:
	cmp	dword [ebp+count], 0  ; if9
	jnz	GetVar_if9end

	mov	eax, [inptr2]
	mov	[ebp+end_], eax

	mov	eax, [ebp+end_]
	sub	eax, [ebp+beg]
	inc	eax
	inc	eax
	invoke	LocalAlloc, 40h, eax
;        push    eax             ; uBytes
;        push    40h             ; uFlags
;        call    LocalAlloc

	mov	[ebp+src0], eax

	cmp	dword [ebp+size], 0   ; if10
	jz	GetVar_if10else

	mov	eax, [ebp+src0]

	mov	byte [eax], '['

;       зменить на invoke lstrcpynA, eax, ebx
	mov	eax, [ebp+end_]
	sub	eax, [ebp+beg]
	push	eax		; iMaxLength
	push	dword [ebp+beg]       ; lpString2
	mov	eax, [ebp+src0]
	inc	eax
	push	eax		; lpString1
	call	[lstrcpynA]
;       -------------------------------------
	jmp	GetVar_if10end

GetVar_if10else:
	mov	eax, [ebp+end_]
	sub	eax, [ebp+beg]

	invoke	lstrcpynA, [ebp+src0], [ebp+beg], eax

GetVar_if10end:
	mov	eax, [currmod]
	shl	eax, 16
	add	eax, [linenumber]
	mov	[modline], eax

	jmp	GetVar_loop2end

GetVar_if9end:
	jmp	GetVar_if12end

GetVar_if8else:
	cmp	al, '}' 	; if11 (список закончен)
	jnz	GetVar_if11else

	dec	dword [ebp+count]

	jmp	GetVar_if12end

GetVar_if11else:
	cmp	al, '{' 	; if12 (список инициализации)
	jnz	GetVar_if12end

	inc	dword [ebp+count]

GetVar_if12end:
	cmp	al, ']' 	; if13 (размерность)
	jnz	GetVar_loop2continue

	inc	dword [ebp+size]

GetVar_loop2continue:
	jmp	GetVar_loop2start

GetVar_loop2end:
	call	NextTok

GetVar_if3end:
	jmp	GetVar_loopstart

GetVar_loopend:
	call	NextTok

GetVar_end:
	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
; Объявление типа данных
; ---------------------------------------------------------------------------
  proc	CmdByte
	push	tk_byte
	call	GetVar

	ret
  endp
;
  proc	CmdChar
	push	tk_char
	call	GetVar

	ret
  endp
;
  proc	CmdWord
	push	tk_word
	call	GetVar

	ret
  endp
;
  proc	CmdShort
	push	tk_short
	call	GetVar

	ret
  endp
;
  proc	CmdDword
	push	tk_dword
	call	GetVar

	ret
  endp
;
  proc	CmdInt
	push	tk_int
	call	GetVar

	ret
  endp
;
  proc	CmdBreak
	invoke	wsprintfA, mapstr, CmdCycle_str1, [endlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	call	NextSemiNext

	ret
  endp
; ---------------------------------------------------------------------------
; case(Cond) ...
; ---------------------------------------------------------------------------
  proc	CmdCase
  loclabel	  = -4

	enter	4, 0

	call	NextTok

	push	tk_openbracket
	call	expecting

	mov	eax, [label_]
	mov	[ebp+loclabel], eax

	inc	[label_]

	mov	[relation], 0

	cmp	[tok], tk_command ; if1
	jnz	CmdCase_if1else

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr

	cmp	eax, CmdCarryFlag ; if2
	jnz	CmdCase_if2else

	invoke	wsprintfA, mapstr, CmdCase_str1, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdCase_if2end

CmdCase_if2else:
	; именно сверка с указателями
	cmp	eax, CmdCarryFlag ; if3
	jnz	CmdCase_if3else

	invoke	wsprintfA, mapstr, CmdCase_str2, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdCase_if2end

CmdCase_if3else:
	cmp	eax, CmdCarryFlag ; if4
	jnz	CmdCase_if4else

	invoke	wsprintfA, mapstr, CmdCase_str3, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdCase_if2end

CmdCase_if4else:
	cmp	eax, CmdCarryFlag ; if5
	jnz	CmdCase_if5else

	invoke	wsprintfA, mapstr, CmdCase_str4, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdCase_if2end

CmdCase_if5else:
	cmp	eax, CmdCarryFlag ; if6
	jnz	CmdCase_if6else

	invoke	wsprintfA, mapstr, CmdCase_str5, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdCase_if2end

CmdCase_if6else:
	cmp	eax, CmdCarryFlag ; if7
	jnz	CmdCase_if2end

	invoke	wsprintfA, mapstr, CmdCase_str6, [ebp+loclabel]
	add	esp, 0Ch

CmdCase_if2end:
	call	NextTok

	jmp	CmdCase_if1end

CmdCase_if1else:
	push	tk_dword
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression

	test	eax, eax	; if8
	jz	CmdCase_if8else

	invoke	wsprintfA, [ebp+loclabel], CmdCase_str8, mapstr
	add	esp, 0Ch

	jmp	CmdCase_if1end

CmdCase_if8else:
	invoke	wsprintfA, mapstr, CmdCase_str9, [ebp+loclabel]
	add	esp, 0Ch

CmdCase_if1end:
	push	mapstr	 ; str
	call	Asm

	push	tk_closebracket
	call	expecting

	call	DoCommand

	invoke	wsprintfA, mapstr, CmdCycle_str1, [endlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	invoke	wsprintfA, mapstr, CmdCase_str10, [ebp+loclabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	leave
	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	CmdContinue
	invoke	wsprintfA, mapstr, CmdContinue_str1, [startlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	call	NextSemiNext

	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	CmdCycle
  varName	  = -84h

	enter	84h, 0

	call	NextTok

	push	tk_openbracket
	call	expecting

	push	[startlabel]
	push	[endlabel]

	mov	eax, [label_]
	mov	[startlabel], eax

	inc	[label_]

	mov	eax, [label_]
	mov	[endlabel], eax

	inc	[label_]

	mov	[relation], 0

	invoke	wsprintfA, mapstr, CmdCase_str10, [startlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname

	call	NextTok

	push	tk_closebracket
	call	expecting

	call	DoCommand

	cmp	byte [ebp+varName + 0], 0 ; if1 (Счетчик цикла отсутствует - бесконечный цикл)
	jnz	CmdCycle_if1else

	invoke	wsprintfA, mapstr, CmdCycle_str1, [startlabel]
	add	esp, 0Ch

	jmp	CmdCycle_if1end

CmdCycle_if1else:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, CmdCycle_str2, eax, [startlabel]
	add	esp, 10h

CmdCycle_if1end:
	push	mapstr	 ; str
	call	Asm

	invoke	wsprintfA, mapstr, CmdCase_str10, [endlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	pop	[endlabel]
	pop	[startlabel]

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Флаги условий в if
; ---------------------------------------------------------------------------
  proc	CmdCarryFlag

	CmdNotCarryFlag:
	CmdZeroFlag:
	CmdNotZeroFlag:
	CmdOverflow:
	CmdNotOverflow:

	ret
  endp
; ---------------------------------------------------------------------------
; else
; ---------------------------------------------------------------------------
  proc	CmdElse
	ret
  endp
; ---------------------------------------------------------------------------
; Объявление нумерованных констант
; ---------------------------------------------------------------------------
  proc	CmdEnum
  counter	  = -48h
  holdid	  = -44h

	enter	48h, 0

	mov	dword [ebp+counter], 0

	call	NextTok

	push	tk_openbrace
	call	expecting

CmdEnum_loopstart:
	cmp	[tok], tk_eof	  ; if1
	jnz	CmdEnum_if1else

	call	unexpectedeof

	jmp	CmdEnum_if1end

CmdEnum_if1else:
	cmp	[tok], tk_comma   ; if2
	jnz	CmdEnum_if2else

	call	NextTok

	jmp	CmdEnum_if1end

CmdEnum_if2else:
	cmp	[tok], tk_closebrace ; if3
	jz	CmdEnum_loopend

	cmp	[tok], tk_id	  ; if4
	jnz	CmdEnum_if4else

	lea	eax, [ebp+holdid]
	invoke	lstrcpyA, eax, string

	cmp	[tok2], tk_assign ; if5
	jnz	CmdEnum_if5end

	call	NextTok

	call	NextTok

	cmp	[tok], tk_number  ; if6
	jnz	CmdEnum_if6else

	call	DoConstLongMath
	mov	[ebp+counter], eax

	jmp	CmdEnum_if5end

CmdEnum_if6else:
	call	numexpected

CmdEnum_if5end:
	push	dword [ebp+counter]   ; constvalue
	lea	eax, [ebp+holdid]
	push	eax		; keystring
	call	AddConstToTree

	inc	dword [ebp+counter]

	call	NextTok

	jmp	CmdEnum_if1end

CmdEnum_if4else:
	call	idexpected

	call	NextTok

CmdEnum_if1end:
	jmp	CmdEnum_loopstart

CmdEnum_loopend:
	push	tk_closebrace
	call	expecting

	call	SemiNext

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; while(Cond) ...
; ---------------------------------------------------------------------------
  proc	CmdWhile
	call	NextTok

	push	tk_openbracket
	call	expecting

	push	[startlabel]
	push	[endlabel]

	mov	eax, [label_]
	mov	[startlabel], eax

	inc	[label_]

	mov	eax, [label_]
	mov	[endlabel], eax

	inc	[label_]

	mov	[relation], 0

	invoke	wsprintfA, mapstr, CmdCase_str10, [startlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	cmp	[tok], tk_command ; if1
	jnz	CmdWhile_if1else

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr


	cmp	eax, CmdCarryFlag ; if2
	jnz	CmdWhile_if2else

	invoke	wsprintfA, mapstr, CmdCase_str1, [endlabel]
	add	esp, 0Ch

	jmp	CmdWhile_if2end

CmdWhile_if2else:
	cmp	eax, CmdNotCarryFlag ; if3
	jnz	CmdWhile_if3else

	invoke	wsprintfA, mapstr, CmdCase_str2, [endlabel]
	add	esp, 0Ch

	jmp	CmdWhile_if2end

CmdWhile_if3else:
	cmp	eax, CmdZeroFlag ; if4
	jnz	CmdWhile_if4else

	invoke	wsprintfA, mapstr, CmdCase_str3, [endlabel]
	add	esp, 0Ch

	jmp	CmdWhile_if2end

CmdWhile_if4else:
	cmp	eax, CmdNotZeroFlag ; if5
	jnz	CmdWhile_if5else

	invoke	wsprintfA, mapstr, CmdCase_str4, [endlabel]
	add	esp, 0Ch

	jmp	CmdWhile_if2end

CmdWhile_if5else:
	cmp	eax, CmdOverflow ; if6
	jnz	CmdWhile_if6else

	invoke	wsprintfA, mapstr, CmdCase_str5, [endlabel]
	add	esp, 0Ch

	jmp	CmdWhile_if2end

CmdWhile_if6else:
	cmp	eax, CmdNotOverflow
	jnz	CmdWhile_if2end

	invoke	wsprintfA, mapstr, CmdCase_str6, endlabel
	add	esp, 0Ch

CmdWhile_if2end:
	call	NextTok

	jmp	CmdWhile_if1end

CmdWhile_if1else:
	push	tk_dword
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression

	test	eax, eax	; if7
	jz	CmdWhile_if7else

	invoke	wsprintfA, mapstr, CmdCase_str8, [endlabel]
	add	esp, 0Ch

	jmp	CmdWhile_if1end

CmdWhile_if7else:
	invoke	wsprintfA, mapstr, CmdCase_str9, [endlabel]
	add	esp, 0Ch

CmdWhile_if1end:
	push	mapstr	 ; str
	call	Asm

	push	tk_closebracket
	call	expecting

	call	DoCommand

	invoke	wsprintfA, mapstr, CmdCycle_str1, [startlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	invoke	wsprintfA, mapstr, CmdCase_str10, [endlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	pop	[endlabel]
	pop	[startlabel]

	ret
  endp
; ---------------------------------------------------------------------------
; default
; ---------------------------------------------------------------------------
  proc	CmdDefault
	call	NextTok

	call	DoCommand

	ret
  endp
; ---------------------------------------------------------------------------
; byte
; ---------------------------------------------------------------------------
  proc	CmdDb
	call	NextTok

CmdDb_loopstart:
	cmp	[tok], tk_number  ; if1
	jnz	CmdDb_if1else

	call	DoConstLongMath
	call	OP

	jmp	CmdDb_if1end

CmdDb_if1else:
	cmp	[tok], tk_string  ; if2
	jnz	CmdDb_if2else

	mov	ecx, [number]

	mov	edx, string

CmdDb_loop2start:
	mov	al, byte [edx]
	call	OP

	inc	edx

	loop	CmdDb_loop2start

	call	NextTok
	jmp	CmdDb_if1end

CmdDb_if2else:
	cmp	[tok], tk_comma   ; if3
	jnz	CmdDb_if3else

	call	NextTok

	jmp	CmdDb_if1end

CmdDb_if3else:
	cmp	[tok], tk_semicolon ; if4
	jz	CmdDb_if4else

	call	numexpected

	call	NextTok

CmdDb_if1end:
	jmp	CmdDb_loopstart

CmdDb_if4else:
		ret
  endp
; ---------------------------------------------------------------------------
; ключевое слово dword
; ---------------------------------------------------------------------------
  proc	CmdDd
	call	NextTok

CmdDd_loopstart:
	cmp	[tok], tk_number  ; if1
	jnz	CmdDd_if1else

	call	DoConstDwordMath
	call	OUTDWORD

	jmp	CmdDd_if1end

CmdDd_if1else:
	cmp	[tok], tk_comma   ; if2
	jnz	CmdDd_if2else

	call	NextTok

	jmp	CmdDd_if1end

CmdDd_if2else:
	cmp	[tok], tk_semicolon ; if3
	jz	CmdDd_if3else

	call	numexpected

	call	NextTok

CmdDd_if1end:
	jmp	CmdDd_loopstart

CmdDd_if3else:
	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	CmdDw
	call	NextTok

CmdDw_loopstart:
	cmp	[tok], tk_number  ; if1
	jnz	CmdDw_if1else

	call	DoConstDwordMath
	call	OUTWORD

	jmp	CmdDw_if1end

CmdDw_if1else:
	cmp	[tok], tk_comma   ; if2
	jnz	CmdDw_if2else

	call	NextTok

	jmp	CmdDw_if1end

CmdDw_if2else:
	cmp	[tok], tk_semicolon ; if3
	jz	CmdDw_if3else

	call	numexpected

	call	NextTok

CmdDw_if1end:
	jmp	CmdDw_loopstart

CmdDw_if3else:
	ret
  endp
; ---------------------------------------------------------------------------
; do ... while(Cond)
; ---------------------------------------------------------------------------
  proc	CmdDo
	call	NextTok

	push	[startlabel]
	push	[endlabel]

	mov	eax, [label_]
	mov	[startlabel], eax

	inc	[label_]

	mov	eax, [label_]
	mov	[endlabel], eax

	inc	[label_]

	mov	[relation], 0

	invoke	wsprintfA, mapstr, CmdCase_str10, [startlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	call	DoCommand

	cmp	[tok], tk_command ; if1
	jnz	CmdDo_if1else

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr

	cmp	eax, CmdWhile ; if2
	jnz	CmdDo_if1else

	call	NextTok

	push	tk_openbracket
	call	expecting

	cmp	[tok], tk_command ; if2
	jnz	CmdDo_if2else

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr

	cmp	eax, CmdCarryFlag ; if3
	jnz	CmdDo_if3else

	invoke	wsprintfA, mapstr, CmdCase_str2, [startlabel]
	add	esp, 0Ch

	jmp	CmdDo_if3end

CmdDo_if3else:
	cmp	eax, CmdNotCarryFlag ; if4
	jnz	CmdDo_if4else

	invoke	wsprintfA, mapstr, CmdCase_str1, [startlabel]
	add	esp, 0Ch

	jmp	CmdDo_if3end

CmdDo_if4else:
	cmp	eax, CmdZeroFlag ; if5
	jnz	CmdDo_if5else

	invoke	wsprintfA, mapstr, CmdCase_str4, [startlabel]
	add	esp, 0Ch

	jmp	CmdDo_if3end

CmdDo_if5else:
	cmp	eax, CmdNotZeroFlag ; if6
	jnz	CmdDo_if6else

	invoke	wsprintfA, mapstr, CmdCase_str3, [startlabel]
	add	esp, 0Ch

	jmp	CmdDo_if3end

CmdDo_if6else:
	cmp	eax, CmdOverflow ; if7
	jnz	CmdDo_if7else

	invoke	wsprintfA, mapstr, CmdCase_str6, [startlabel]
	add	esp, 0Ch

	jmp	CmdDo_if3end

CmdDo_if7else:
	cmp	eax, CmdNotOverflow ; if8
	jnz	CmdDo_if3end

	invoke	wsprintfA, mapstr, CmdCase_str5, [startlabel]
	add	esp, 0Ch

CmdDo_if3end:
	call	NextTok

	jmp	CmdDo_if2end

CmdDo_if2else:
	push	tk_dword
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression

	test	eax, eax	; if9
	jz	CmdDo_if9else

	invoke	wsprintfA, mapstr, CmdCase_str9, [startlabel]
	add	esp, 0Ch

	jmp	CmdDo_if2end

CmdDo_if9else:
	invoke	wsprintfA, mapstr, CmdCase_str8, [startlabel]
	add	esp, 0Ch

CmdDo_if2end:
	push	mapstr	 ; str
	call	Asm

	push	tk_closebracket
	call	expecting

	jmp	CmdDo_if1end

CmdDo_if1else:
	push	CmdDo_str1 ; "'while' expected following 'do'"
	call	preerror

CmdDo_if1end:
	invoke	wsprintfA, mapstr, CmdCase_str10, [endlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	pop	[endlabel]
	pop	[startlabel]

	ret
  endp
; ---------------------------------------------------------------------------
; docase ...
; ---------------------------------------------------------------------------
  proc	CmdDoCase
	call	NextTok

	push	[startlabel]
	push	[endlabel]

	mov	eax, [label_]
	mov	[startlabel], eax

	inc	[label_]

	mov	eax, [label_]
	mov	[endlabel], eax

	inc	[label_]

	invoke	wsprintfA, mapstr, CmdCase_str10, [startlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	call	DoCommand

	invoke	wsprintfA, mapstr, CmdCase_str10, [endlabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	pop	[endlabel]
	pop	[startlabel]

	ret
  endp
; ---------------------------------------------------------------------------
; if(Cond) ...
; ---------------------------------------------------------------------------
  proc	CmdIf
  loclabel	  = -4

	enter	4, 0

	call	NextTok

	push	tk_openbracket	; tk_openbracket
	call	expecting

	mov	eax, [label_]
	mov	[ebp+loclabel], eax

	inc	[label_]

	mov	[relation], 0 ; типа флаг отрицания в выражении

	cmp	[tok], tk_command ; if1
	jnz	CmdIf_if1else

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr

	cmp	eax, CmdCarryFlag ; if2
	jnz	CmdIf_if2else

	invoke	wsprintfA, mapstr, CmdCase_str1, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdIf_if2end

CmdIf_if2else:
	cmp	eax, CmdNotCarryFlag ; if3
	jnz	CmdIf_if3else

	invoke	wsprintfA, mapstr, CmdCase_str2, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdIf_if2end

CmdIf_if3else:
	cmp	eax, CmdZeroFlag ; if4
	jnz	CmdIf_if4else

	invoke	wsprintfA, mapstr, CmdCase_str3, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdIf_if2end

CmdIf_if4else:
	cmp	eax, CmdNotZeroFlag ; if5
	jnz	CmdIf_if5else

	invoke	wsprintfA, mapstr, CmdCase_str4, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdIf_if2end

CmdIf_if5else:
	cmp	eax, CmdOverflow ; if6
	jnz	CmdIf_if6else

	invoke	wsprintfA, mapstr, CmdCase_str5, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdIf_if2end

CmdIf_if6else:
	cmp	eax, CmdNotOverflow ; if7
	jnz	CmdIf_if2end

	invoke	wsprintfA, mapstr, CmdCase_str6, [ebp+loclabel]
	add	esp, 0Ch

CmdIf_if2end:
	call	NextTok

	jmp	CmdIf_if1end

CmdIf_if1else:
	push	tk_dword
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression

	test	eax, eax	; if8
	jz	CmdIf_if8else

	invoke	wsprintfA, mapstr, CmdCase_str8, [ebp+loclabel]
	add	esp, 0Ch

	jmp	CmdIf_if1end

CmdIf_if8else:
	invoke	wsprintfA, mapstr, CmdCase_str9, [ebp+loclabel]
	add	esp, 0Ch

CmdIf_if1end:
	push	mapstr	 ; str
	call	Asm

	push	tk_closebracket ; tk_closebracket
	call	expecting

	call	DoCommand

	cmp	[tok], tk_command ; if9
	jnz	CmdIf_if9end

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr

	cmp	eax, CmdElse ; if10
	jnz	CmdIf_if9end

	invoke	wsprintfA, mapstr, CmdIf_str1, [label_], [ebp+loclabel]
	add	esp, 10h

	push	mapstr	 ; str
	call	Asm

	mov	eax, [label_]
	mov	[ebp+loclabel], eax

	inc	[label_]

	call	NextTok

	call	DoCommand

CmdIf_if9end:
	invoke	wsprintfA, mapstr, CmdCase_str10, [ebp+loclabel]
	add	esp, 0Ch

	push	mapstr	 ; str
	call	Asm

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; return(Expr)
; ---------------------------------------------------------------------------
  proc	CmdReturn
	call	NextTok

	cmp	[tok], tk_openbracket ; if1
	jnz	CmdReturn_if1end

	call	NextTok

	cmp	[tok], tk_closebracket ; if2
	jz	CmdReturn_if2end

	cmp	[returntype], tk_void ; if3
	jz	CmdReturn_if2end

	push	[returntype]
	push	tk_reg
	push	CmdCase_str7 ; "eax"
	call	Expression

CmdReturn_if2end:
	push	tk_closebracket
	call	expecting

CmdReturn_if1end:
	call	LeaveProc

	call	SemiNext

	ret
  endp
; ---------------------------------------------------------------------------
; Объявление глобальной переменной
; ---------------------------------------------------------------------------
  proc	GlobalVar
  size		  = -14h
  elements	  = -10h
  ptr_		  = -0Ch
  i		  = -8
  count 	  = -4
  vartype	  = 8

	enter	14h, 0

	mov	eax, [treeptr]
	mov	dword [ebp+ptr_], eax

	mov	dword [ebp+elements], 1

	push	dword [ebp+vartype]
	call	TypeSize
	mov	dword [ebp+size], eax

	call	NextTok

GlobalVar_loop1start:
	cmp	[tok], tk_eof	  ; if1
	jz	GlobalVar_G04

	cmp	[tok], tk_semicolon ; if2
	jnz	GlobalVar_if2else

	call	NextTok

	jmp	GlobalVar_if1end

GlobalVar_if2else:
	cmp	[tok], tk_assign  ; if3
	jnz	GlobalVar_if3else

	call	NextTok

	jmp	GlobalVar_if1end

GlobalVar_if3else:
	cmp	[tok], tk_plus	  ; if4
	jnz	GlobalVar_if4else

	call	NextTok

	jmp	GlobalVar_if1end

GlobalVar_if4else:
	cmp	[tok], tk_openblock ; if5
	jnz	GlobalVar_if5else

	call	NextTok

	call	DoConstLongMath
	mov	[ebp+elements], eax

	push	tk_closeblock
	call	expecting

	jmp	GlobalVar_if1end

GlobalVar_if5else:
	cmp	[tok], tk_number  ; if6
	jnz	GlobalVar_if6else

	mov	esi, [ebp+ptr_]  ; Откорректируем ранее сделанную запись

	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptImageBase]
	add	eax, [OptBaseOfCode]
	mov	[esi+recnumber], eax

	mov	dword [esi+recpost], 0 ; dword

GlobalVar_G01:
	cmp	dword [ebp+vartype], tk_byte ; if7
	jnz	GlobalVar_if7else

	call	DoConstDwordMath
	mov	[ebp+i], eax

	jmp	GlobalVar_if7end

GlobalVar_if7else:
	cmp	dword [ebp+vartype], tk_word ; if8
	jnz	GlobalVar_if8else

	call	DoConstDwordMath
	mov	dword [ebp+i], eax

	jmp	GlobalVar_if7end

GlobalVar_if8else:
	cmp	dword [ebp+vartype], tk_dword ; if9
	jnz	GlobalVar_if9else

	call	DoConstDwordMath
	mov	[ebp+i], eax

	jmp	GlobalVar_if7end

GlobalVar_if9else:
	call	DoConstLongMath
	mov	[ebp+i], eax

GlobalVar_if7end:
	mov	eax, [ebp+elements]
	mov	[ebp+count], eax

GlobalVar_loop2start:
	cmp	dword [ebp+size], 1   ; if10
	jnz	GlobalVar_if10else

	mov	al, byte [ebp+i]
	call	OP

	jmp	GlobalVar_if10end

GlobalVar_if10else:
	cmp	dword [ebp+size], 2   ; if11
	jnz	GlobalVar_if11else

	mov	eax, [ebp+i]
	call	OUTWORD

	jmp	GlobalVar_if10end

GlobalVar_if11else:
	cmp	dword [ebp+size], 4   ; if12
	jnz	GlobalVar_if10end

	mov	eax, [ebp+i]
	call	OUTDWORD

GlobalVar_if10end:
	dec	dword [ebp+count] ; decrease loop var
	jnz	GlobalVar_loop2start ; loop condition

	jmp	GlobalVar_if1end

GlobalVar_if6else:
	cmp	[tok], tk_minus   ; if13
	jnz	GlobalVar_if13else

	call	NextTok

	mov	eax, [number]
	neg	eax
	mov	[number], eax

	jmp	GlobalVar_G01

	jmp	GlobalVar_if1end

GlobalVar_if13else:
	cmp	[tok], tk_string  ; if14
	jnz	GlobalVar_if14else

	mov	esi, [ebp+ptr_]  ; Откорректируем ранее сделанную запись
	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptImageBase]
	add	eax, [OptBaseOfCode]
	mov	[esi+recnumber], eax

	mov	dword [esi+recpost], 0 ; dword

	mov	dword [ebp+count], 0

GlobalVar_loop3start:
	mov	eax, [number]
	mov	[ebp+i], eax

	mov	edx, string

GlobalVar_loop4start:
	mov	al, byte [edx]

	call	OP

	inc	edx
	inc	dword [ebp+count]

	dec	dword [ebp+i]
	jnz	GlobalVar_loop4start ; loop condition

	call	NextTok
	cmp	[tok], tk_string  ; loop condition
	jz	GlobalVar_loop3start

	mov	al, 0
	call	OP

	mov	eax, [ebp+elements]
	mov	[ebp+i], eax

	mov	eax, [ebp+count]
	sub	[ebp+i], eax

	cmp	dword [ebp+i], 0      ; if15
	jle	GlobalVar_if15end

GlobalVar_loop5start:
	mov	al, 0

	call	OP

	dec	dword [ebp+i]
	jnz	GlobalVar_loop5start ; loop condition

GlobalVar_if15end:
	jmp	GlobalVar_if1end

GlobalVar_if14else:
	cmp	[tok], tk_from	  ; if16
	jnz	GlobalVar_if16else

	call	NextTok

	mov	eax, aCountDofrom1 ; "count = DoFrom(1);\r\n"
	call	WRITESTR

	mov	eax, [ebp+size]
	imul	dword [ebp+elements]
	mov	[ebp+i], eax

	mov	eax, [ebp+count]
	sub	[ebp+i], eax

GlobalVar_loop6start:
	mov	al, 0

	call	OP

	dec	dword [ebp+i]
	jnz	GlobalVar_loop6start ; loop condition

	call	NextTok

	jmp	GlobalVar_if1end

GlobalVar_if16else:
	cmp	[tok], tk_extract ; if17
	jnz	GlobalVar_if17else

	call	NextTok

	mov	eax, aCountDoextract ; "count = DoExtract(1);\r\n"
	call	WRITESTR

	mov	eax, [ebp+size]
	imul	dword [ebp+elements]
	mov	[ebp+i], eax

	mov	eax, [ebp+count]
	sub	[ebp+i], eax

GlobalVar_loop7start:
	mov	al, 0
	call	OP

	dec	dword [ebp+i]
	jnz	GlobalVar_loop7start

	jmp	GlobalVar_if1end

GlobalVar_if17else:
	cmp	[tok], tk_openbrace ; if18
	jnz	GlobalVar_G04

	mov	esi, [ebp+ptr_]  ; Откорректируем ранее сделанную запись

	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptImageBase]
	add	eax, [OptBaseOfCode]
	mov	[esi+recnumber], eax

	mov	dword [esi+recpost], 0 ; dword

	mov	dword [ebp+count], 0

	call	NextTok

GlobalVar_loop8start:
	cmp	[tok], tk_closebrace ; if19
	jz	GlobalVar_loop8end ; break

	cmp	[tok], tk_comma   ; if20
	jnz	GlobalVar_if20else

	call	NextTok

	jmp	GlobalVar_if20end

GlobalVar_if20else:
	cmp	[tok], tk_plus	  ; if21
	jnz	GlobalVar_if21else

	call	NextTok

	jmp	GlobalVar_if20end

GlobalVar_if21else:
	cmp	[tok], tk_string  ; if22
	jnz	GlobalVar_if22else

	mov	eax, [number]
	mov	[ebp+i], eax

	mov	edx, string

GlobalVar_loop9start:
	movzx	eax, byte [edx]
	call	OP

	inc	edx
	inc	dword [ebp+count]

	dec	dword [ebp+i]
	jnz	GlobalVar_loop9start ; loop condition

	cmp	[tok2], tk_plus   ; if23
	jz	GlobalVar_if23end

	mov	al, 0
	call	OP

	inc	dword [ebp+count]

GlobalVar_if23end:
	call	NextTok

	jmp	GlobalVar_if20end

GlobalVar_if22else:
	cmp	[tok], tk_postnumber ; if24
	jnz	GlobalVar_if24else

	push	POST_DATA
	push	[treeptr]
	call	SetPost

	mov	eax, [number]
	call	OUTDWORD

	call	NextTok

	jmp	GlobalVar_if20end

GlobalVar_if24else:
	cmp	[tok], tk_number  ; if25
	jnz	GlobalVar_if25else

GlobalVar_G02:
	cmp	dword [ebp+vartype], tk_byte ; if26
	jnz	GlobalVar_if26else

	call	DoConstDwordMath
	call	OP

	jmp	GlobalVar_if26end

GlobalVar_if26else:
	cmp	dword [ebp+vartype], tk_word ; if27
	jnz	GlobalVar_if27else

	call	DoConstDwordMath
	call	OUTWORD

	jmp	GlobalVar_if26end

GlobalVar_if27else:
	cmp	dword [ebp+vartype], tk_char ; if28
	jnz	GlobalVar_if28else

	call	DoConstLongMath
	call	OP

	jmp	GlobalVar_if26end

GlobalVar_if28else:
	cmp	dword [ebp+vartype], tk_short ; if29
	jnz	GlobalVar_if29else

	call	DoConstLongMath
	call	OUTWORD

	jmp	GlobalVar_if26end

GlobalVar_if29else:
	cmp	dword [ebp+vartype], tk_dword ; if30
	jnz	GlobalVar_if30else

	call	DoConstDwordMath
	call	OUTDWORD

	jmp	GlobalVar_if26end

GlobalVar_if30else:
	cmp	dword [ebp+vartype], tk_int ; if31
	jnz	GlobalVar_if26end

	call	DoConstLongMath
	call	OUTDWORD

GlobalVar_if26end:
	inc	dword [ebp+count]

	jmp	GlobalVar_if20end

GlobalVar_if25else:
	cmp	[tok], tk_minus   ; if32
	jnz	GlobalVar_if32else

	call	NextTok

	mov	eax, [number]
	neg	eax
	mov	[number], eax

	jmp	GlobalVar_G02

	jmp	GlobalVar_if20end

GlobalVar_if32else:
	call	numexpected

	call	NextTok

GlobalVar_if20end:
	jmp	GlobalVar_loop8start

GlobalVar_loop8end:
	mov	eax, [ebp+elements]
	sub	eax, [ebp+count]
	mov	[ebp+count], eax

	cmp	dword [ebp+count], 0  ; if33
	jle	GlobalVar_if33end

GlobalVar_loop10start:
	cmp	dword [ebp+size], 1   ; if34
	jnz	GlobalVar_if34else

	mov	al, 0
	call	OP

	jmp	GlobalVar_if34end

GlobalVar_if34else:
	cmp	dword [ebp+size], 2   ; if35
	jnz	GlobalVar_if35else

	xor	eax, eax
	call	OUTWORD

	jmp	GlobalVar_if34end

GlobalVar_if35else:
	cmp	dword [ebp+size], 4   ; if36
	jnz	GlobalVar_if34end

	xor	eax, eax
	call	OUTDWORD

GlobalVar_if34end:
	dec	dword [ebp+count]
	jnz	GlobalVar_loop10start ; loop condition

GlobalVar_if33end:
	jmp	GlobalVar_if1end

GlobalVar_G04:
	mov	esi, [ebp+ptr_]

	mov	eax, [postsize]
	mov	[esi+recnumber], eax

	mov	dword [esi+recpost], 1 ; dword

	mov	eax, [ebp+elements]
	mul	dword [ebp+size]
	add	eax, [postsize]
	mov	[postsize], eax

	jmp	GlobalVar_loop1end

GlobalVar_if1end:
	jmp	GlobalVar_loop1start

GlobalVar_loop1end:
	leave
	ret	4
  endp