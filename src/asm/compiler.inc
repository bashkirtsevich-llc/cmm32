;
; Функционал компилятора
;
; ---------------------------------------------------------------------------
  proc	Compile

	cmp	[makemapfile], 0  ; if1
	jz	Compile_if1end
	call	StartMapfile

Compile_if1end:
	; передать пустой указатель
	; пустой указатель означает, что будет подтянута основная программа
	xor	eax, eax
	push	eax
	call	Preview

	call	CompileAll

	if DEFINE_MAKE_EXE
	call	DoLink		; Формирование link
	end if

	cmp	[posts], 0	  ; if2
	jbe	Compile_if2end

	call	DoPosts 	; Обновление всех post адресов

Compile_if2end:
	push	[treestart]
	call	SeekUndefined

	cmp	[error], 0	  ; if3
	jnz	Compile_if3end

	invoke	wsprintfA, mapstr, Compile_str2, dword [error], dword [totallines]
	add	esp, 10h

	mov	eax, mapstr
	call	WRITESTR

	mov	eax, [outptr]
	sub	eax, [output]
	mov	[runfilesize], eax

	mov	eax, [postsize]
	and	eax, 1
	add	[postsize], eax

;        push    dword [stdout]
;        call    PrintMemsizes

	call	WriteEXE
	test	eax, eax	; if4
	jnz	Compile_if3end

	invoke	wsprintfA, mapstr, Compile_str3, dword [runfilesize]
	add	esp, 0Ch

	mov	eax, mapstr
	call	WRITESTR

	invoke	wsprintfA, mapstr, Compile_str4, dword [DLLcount], dword [APIcount]
	add	esp, 10h

	mov	eax, mapstr
	call	WRITESTR

Compile_if3end:
	cmp	[makemapfile], 0  ; if4
	jz	Compile_if4end
	call	FinishMapfile

Compile_if4end:
	ret
  endp
; ---------------------------------------------------------------------------
; Создание файла статистики (будет выводить текст: "начало файла")
; ---------------------------------------------------------------------------
  proc	StartMapfile

	push	StartMapfile_str1
	call	fprint

	ret
  endp
; ---------------------------------------------------------------------------
; Завершение файла статистики
; ---------------------------------------------------------------------------
  proc	FinishMapfile

	call	DisplayTree

	push	FinishMapfile_str1 ; "Component Sizes:\r\n"
	call	fprint

;        push    [mapfile]         ; handle
;        call    PrintMemsizes

	invoke	wsprintfA, mapstr, FinishMapfile_str2, [runfilesize]
	add	esp, 0Ch

	push	mapstr	 ; lpBuffer
	call	fprint

	push	FinishMapfile_str3
	call	fprint

	mov	[list], 0

	ret
  endp
; ---------------------------------------------------------------------------
; Предварительная обработка файла
; ---------------------------------------------------------------------------
  proc	Preview
  filename	  = 8

	push	ebp
	mov	ebp, esp

; загружаем файл (если имя не указано, значит загружаем главный файл)
	push	dword [ebp+filename]
	call	LoadInputfile

; проверяем, указано ли имя файла, если указано, копируем его в currentfilename (для вывода всяких сообщений в map файл)
	cmp	dword [ebp+filename], 0 ; if1
	jz	Preview_if1end

	lea	eax, [ebp+filename]
	invoke	lstrcpyA, currentfilename, eax

Preview_if1end:
	inc	[module]

	cmp	[module], MAXMDL  ; if3
	jnb	Preview_if3end

	mov	eax, [module]
	imul	eax, FILENAMESIZE
	add	eax, modules
	invoke	lstrcpyA, eax, currentfilename

	mov	eax, [module]
	mov	[currmod], eax

Preview_if3end:
	cmp	[makemapfile], 0  ; if4
	jz	Preview_if4end

	mov	ebx, [inptr]
	mov	al, [ebx]
	mov	[cha], al

	invoke	wsprintfA, mapstr, Preview_str1, currentfilename ; файл %такой-то% включен
	add	esp, 0Ch

	push	mapstr
	call	fprint

Preview_if4end:
	call	ShowSrcLine

	call	NextChar

	mov	al, [cha]
	mov	[cha2], al

	mov	eax, [inptr]
	mov	[inptr2], eax

	mov	[linenum2], 1

	call	NextTok

Preview_loop:				; Пока не кончится входной буфер
	cmp	[tok], tk_eof
	jz	Preview_loopend

	cmp	[tok], tk_directive ; if5
	jnz	Preview_if5else

	push	[number]
	push	Jmp_Directives
	call	GetDirAddr

	call	eax
	jmp	Preview_if5end

Preview_if5else:
	cmp	[tok], tk_command ; if6
	jnz	Preview_if6else

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr

	call	eax

	jmp	Preview_if5end

Preview_if6else:
	cmp	[tok], tk_id	  ; if7
	jnz	Preview_if7else

	push	tk_void
	call	GetProc

	jmp	Preview_if5end

Preview_if7else:
	cmp	[tok], tk_void	  ; if8
	jnz	Preview_if8else

	call	NextTok

	push	tk_void
	call	GetProc

	jmp	Preview_if5end

Preview_if8else:
	push	Preview_str2
	call	preerror
	call	NextTok

Preview_if5end:
	jmp	Preview_loop

Preview_loopend:

	pop	ebp
	ret	4
  endp
; ---------------------------------------------------------------------------
; Компиляция всех процедур и объявлений данных
; ---------------------------------------------------------------------------
  proc	CompileAll

	push	number
	push	CompileAll_str1 ; "main"
	push	post
	push	src
	push	type
	push	tok
	call	SearchTree

	test	eax, eax	; if1
	jz	CompileAll_if1else

	jmp	CompileAll_if1end

CompileAll_if1else:
	push	CompileAll_str3
	call	preerror

	ret

CompileAll_if1end:
	if DEFINE_MAKE_EXE
	mov	eax, dword [OptBaseOfCode]
	add	eax, dword [outptr]
	sub	eax, dword [output]
	mov	dword [OptEntryPointRVA], eax
	end if
		    
	push	dword [treeptr]
	call	CompileSrc	; Компиляция main

CompileAll_loop:

	push	[treestart]
	call	SeekToDo 

	test	eax, eax
	jz	CompileAll_loopend

	mov	esi, [treeptr]

	invoke	wsprintfA, mapstr, CompileAll_str4, [esi+rectok], [esi+rectype], [esi+recnumber], [esi+recpost], [esi+recid]
	add	esp, 1Ch

	push	mapstr
	call	fprint
		  
	push	[treeptr]
	call	CompileSrc	; Компиляция исходников

	jmp	CompileAll_loop

CompileAll_loopend:

CompileAll_if3end:
	ret
  endp

; ---------------------------------------------------------------------------
; Обработка ссылок вперед
; ---------------------------------------------------------------------------
  proc	DoPosts

  addhold	  = -8
  i		  = -4

	enter	8, 0

	mov	dword [ebp+i], 0

DoPosts_loopstart:
	mov	eax, [posts]
	cmp	[ebp+i], eax	; loop condition
	jnb	DoPosts_loopend

	mov	ecx, [ebp+i]
	shl	ecx, 2

	mov	esi, [posttype]
	add	esi, ecx

	mov	eax, [esi]

	mov	esi, [postloc]
	add	esi, ecx

	mov	ebx, [esi]

	mov	esi, [postnum]
	add	esi, ecx

	mov	ecx, [esi]

	cmp	eax, POST_DATA	; if1
	jnz	DoPosts_if1else

	cmp	dword [ecx+recpost], 0 ; if2
	jz	DoPosts_if2else

	push	ebx
	call	GetDword

	add	eax, [ecx+recnumber]
	add	eax, [OptImageBase]
	add	eax, [OptBaseOfCode]
	add	eax, [outptr]
	sub	eax, [output]

	jmp	DoPosts_if2end

DoPosts_if2else:
	push	ebx
	call	GetDword

	add	eax, [ecx+recnumber]

DoPosts_if2end:
	push	eax
	push	ebx
	call	SetDword

	jmp	DoPosts_if1end

DoPosts_if1else:
	cmp	eax, POST_CALL	; if3
	jnz	DoPosts_if3else

	mov	eax, [ecx+recnumber]
	sub	eax, [OptImageBase]
	sub	eax, [OptBaseOfCode]
	sub	eax, ebx	; Расчет относительного смещения EAX-=addvalue;
	add	eax, [output]

	sub	eax, 4
	push	eax
	push	ebx
	call	SetDword

	jmp	DoPosts_if1end

DoPosts_if3else:
	cmp	eax, POST_LOC	; if4
	jnz	DoPosts_if4else

	mov	eax, ecx
	sub	eax, [OptImageBase]
	sub	eax, [OptBaseOfCode]
	sub	eax, ebx	; Расчет относительного смещения EAX-=addvalue;
	add	eax, [output]

	sub	eax, 4
	push	eax
	push	ebx
	call	SetDword

	jmp	DoPosts_if1end

DoPosts_if4else:
	cmp	eax, POST_API	; if5
	jnz	DoPosts_if5else

	mov	eax, [OptImageBase]
	add	eax, [ecx+recmodline]
	mov	[ebp+addhold], eax

	push	dword [ebp+addhold]
	push	ebx
	call	SetDword

	jmp	DoPosts_if1end

DoPosts_if5else:
	push	DoPosts_str1
	call	preerror

DoPosts_if1end:
	inc	dword [ebp+i]

	jmp	DoPosts_loopstart

DoPosts_loopend:
	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Компиляция одной процедуры или объявления данных
; ---------------------------------------------------------------------------
  proc	CompileSrc
  ptr_		  = 8

	push	ebp
	mov	ebp, esp

	mov	eax, [src]
	test	eax, eax	; if1
	jz	CompileSrc_if1else

	mov	[inptr], eax

	mov	[inptr2], eax

	mov	[endoffile], 0	     ; На начало буфера

	mov	eax, [modline]
	and	eax, 0FFFFh
	mov	[linenum2], eax

	mov	eax, [modline]
	shr	eax, 10h ; 16
	mov	[currmod], eax

	mov	eax, [currmod]
	imul	eax, FILENAMESIZE
	add	eax, modules

	invoke	lstrcpyA, currentfilename, eax

	call	NextChar

	mov	al, [cha]
	mov	[cha2], al

	mov	eax, [inptr]
	mov	[inptr2], eax

	cmp	[tok], tk_proc	  ; if2
	jnz	CompileSrc_if2else

	push	cpt_near	; cpt_near
	call	Proc

	call	DoPoststrings

	jmp	CompileSrc_if1end

CompileSrc_if2else:
	cmp	[tok], tk_var	  ; if3
	jnz	short CompileSrc_if3else

	push	[type]
	call	GlobalVar

	jmp	CompileSrc_if1end

CompileSrc_if3else:
;        push    4068CCh         ; aAsmBadInputFor
;        call    preerror

CompileSrc_if1end:
	jmp	CompileSrc_end

CompileSrc_if1else:
	mov	esi, [ebp+ptr_] ; post-переменная без размерности

	mov	eax, [postsize]
	mov	[esi+recnumber], eax ; recnumber

	mov	dword [esi+recpost], 1 ; recpost

	push	[type]
	call	TypeSize
	add	[postsize], eax

CompileSrc_end:
	pop	ebp
	ret	4
  endp
; ---------------------------------------------------------------------------
; Обработка процедур
; ---------------------------------------------------------------------------
  proc	Proc
  proc_type	  = 8

	push	ebp
	mov	ebp, esp

	mov	eax, [ebp+proc_type]
	mov	[current_proc_type], eax

	mov	[tok], tk_proc

	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptImageBase]
	add	eax, [OptBaseOfCode]
	mov	[number], eax

	mov	esi, [treeptr]

	mov	eax, [tok]
	mov	[esi+rectok], eax

	mov	eax, [number]
	mov	[esi+recnumber], eax

	mov	dword [esi+recpost], 0

	call	NextTok

	push	tk_openbracket
	call	expecting

	cmp	[tok], tk_closebracket ; if1
	jz	Proc_if1end

	call	DeclareParams

Proc_if1end:
	call	NextTok

	cmp	[paramsize], 0	  ; if2
	jbe	Proc_if2end

	push	Proc_str1 ; "push ebp;mov ebp,esp"
	call	Asm

Proc_if2end:
	cmp	[tok], tk_openbrace ; if3
	jz	Proc_if3end

	call	DeclareLocals

Proc_if3end:
	call	DoBlock 	; Обработка тела процедуры { ... }

	call	LeaveProc

	call	KillLocals

	pop	ebp
	ret	4
  endp
; ---------------------------------------------------------------------------
; Возвращает размер для заданного типа данных
; ---------------------------------------------------------------------------
  proc	TypeSize
  holdstr	  = -3Ch
  vartype	  = 8

	enter	3Ch, 0

	cmp	dword [ebp+vartype], tk_void ; if1
	jnz	TypeSize_if1else

	xor	eax, eax

	jmp	TypeSize_end

TypeSize_if1else:
	cmp	dword [ebp+vartype], tk_char ; if2
	jz	TypeSize_if2start

	cmp	dword [ebp+vartype], tk_byte
	jnz	TypeSize_if2else

TypeSize_if2start:
	mov	eax, 1

	jmp	TypeSize_end

TypeSize_if2else:
	cmp	dword [ebp+vartype], tk_short ; if3
	jz	TypeSize_if3start

	cmp	dword [ebp+vartype], tk_word
	jnz	TypeSize_if3else

TypeSize_if3start:
	mov	eax, 2

	jmp	TypeSize_end

TypeSize_if3else:
	cmp	dword [ebp+vartype], tk_dword ; if4
	jz	TypeSize_if4start

	cmp	dword [ebp+vartype], tk_int
	jnz	TypeSize_if4else

TypeSize_if4start:
	mov	eax, 4

	jmp	TypeSize_end

TypeSize_if4else:
	lea	eax, [ebp+holdstr]
	invoke	wsprintfA, eax, TypeSize_str, [ebp+vartype]
	add	esp, 0Ch

	movsx	eax, byte [ebp+holdstr]
	push	eax
	call	internalerror

	or	eax, -1

TypeSize_end:
	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
; Обработка параметров при объявлении процедуры
; ---------------------------------------------------------------------------
  proc	DeclareParams
  paramtok	  = -8
  paramtype	  = -4

	enter	8, 0

DeclareParams_LL:
	cmp	[tok], tk_command ; if1
	jnz	DeclareParams_if1else

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr

	jmp	DeclareParams_if1end

DeclareParams_if1else:
	or	eax, -1

DeclareParams_if1end:
	cmp	eax, CmdShort ; if2
	jnz	DeclareParams_if2else

	mov	dword [ebp+paramtok], tk_param

	mov	dword [ebp+paramtype], tk_short

	jmp	DeclareParams_loopstart

DeclareParams_if2else:
	cmp	eax, CmdWord ; if3
	jnz	DeclareParams_if3else

	mov	dword [ebp+paramtok], tk_param

	mov	dword [ebp+paramtype], tk_word

	jmp	DeclareParams_loopstart

DeclareParams_if3else:
	cmp	eax, CmdChar ; if4
	jnz	DeclareParams_if4else

	mov	dword [ebp+paramtok], tk_param

	mov	dword [ebp+paramtype], tk_char

	jmp	DeclareParams_loopstart

DeclareParams_if4else:
	cmp	eax, CmdByte ; if5
	jnz	DeclareParams_if5else

	mov	dword [ebp+paramtok], tk_param

	mov	dword [ebp+paramtype], tk_byte

	jmp	DeclareParams_loopstart

DeclareParams_if5else:
	cmp	eax, CmdInt ; if6
	jnz	DeclareParams_if6else

	mov	dword [ebp+paramtok], tk_param

	mov	dword [ebp+paramtype], tk_int

	jmp	DeclareParams_loopstart

DeclareParams_if6else:
	cmp	eax, CmdDword ; if7
	jnz	DeclareParams_if7else

	mov	dword [ebp+paramtok], tk_param

	mov	dword [ebp+paramtype], tk_dword

	jmp	DeclareParams_loopstart

DeclareParams_if7else:
	call	datatype_expected

	call	NextTok

DeclareParams_loopstart:
	call	NextTok

	cmp	[tok], tk_id	  ; if8
	jnz	DeclareParams_if8else

	add	[paramsize], 4

	push	[paramsize]	  ; num
	push	dword [ebp+paramtype] ; ltype
	push	dword [ebp+paramtok]  ; tk
	push	string	 ; str
	call	AddLocalvar

	jmp	DeclareParams_if8end

DeclareParams_if8else:
	cmp	[tok], tk_semicolon ; if9
	jnz	DeclareParams_if9else

	call	NextTok

	jmp	DeclareParams_LL

	jmp	DeclareParams_if8end

DeclareParams_if9else:
	cmp	[tok], tk_closebracket ; if10
	jz	DeclareParams_loopend

	cmp	[tok], tk_comma   ; if11
	jz	DeclareParams_if8end

	call	idexpected

DeclareParams_if8end:
	jmp	DeclareParams_loopstart

DeclareParams_loopend:
	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Обработка локальных переменных при объявлении процедуры
; ---------------------------------------------------------------------------
  proc	DeclareLocals
  size		  = -0Ch
  loctok	  = -8
  loctype	  = -4

	enter	0Ch, 0

DeclareLocals_LL:
	cmp	[tok], tk_command ; if1
	jnz	DeclareLocals_if1end

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr

DeclareLocals_if1end:
	cmp	eax, CmdShort ; if2
	jnz	DeclareLocals_if2else

	mov	dword [ebp+loctok], tk_local

	mov	dword [ebp+loctype], tk_short

	mov	dword [ebp+size], 2

	jmp	DeclareLocals_loopstart

DeclareLocals_if2else:
	cmp	eax, CmdWord ; if3
	jnz	DeclareLocals_if3else

	mov	dword [ebp+loctok], tk_local

	mov	dword [ebp+loctype], tk_word

	mov	dword [ebp+size], 2

	jmp	DeclareLocals_loopstart

DeclareLocals_if3else:
	cmp	eax, CmdChar ; if4
	jnz	DeclareLocals_if4else

	mov	dword [ebp+loctok], tk_local

	mov	dword [ebp+loctype], tk_char

	mov	dword [ebp+size], 1

	jmp	DeclareLocals_loopstart

DeclareLocals_if4else:
	cmp	eax, CmdByte ; if5
	jnz	DeclareLocals_if5else

	mov	dword [ebp+loctok], tk_local

	mov	dword [ebp+loctype], tk_byte

	mov	dword [ebp+size], 1

	jmp	DeclareLocals_loopstart

DeclareLocals_if5else:
	cmp	eax, CmdInt ; if6
	jnz	DeclareLocals_if6else

	mov	dword [ebp+loctok], tk_local

	mov	dword [ebp+loctype], tk_int

	mov	dword [ebp+size], 4

	jmp	DeclareLocals_loopstart

DeclareLocals_if6else:
	cmp	eax, CmdDword ; if7
	jnz	DeclareLocals_if7else

	mov	dword [ebp+loctok], tk_local

	mov	dword [ebp+loctype], tk_dword

	mov	dword [ebp+size], 4

	jmp	DeclareLocals_loopstart

DeclareLocals_if7else:
	cmp	[tok], tk_eof	  ; if8
	jz	DeclareLocals_if8start

	cmp	[tok], tk_openbrace
	jnz	DeclareLocals_if8else

DeclareLocals_if8start:
	jmp	DeclareLocals_L1

	jmp	DeclareLocals_loopstart

DeclareLocals_if8else:
	call	datatype_expected

	call	NextTok

	jmp	DeclareLocals_LL

DeclareLocals_loopstart:
	call	NextTok

	cmp	[tok], tk_id	  ; if9
	jnz	DeclareLocals_if9else

	push	[localsize]	  ; num
	push	dword [ebp+loctype]   ; ltype
	push	dword [ebp+loctok]    ; tk
	push	string	 ; str
	call	AddLocalvar

	cmp	[tok2], tk_openblock ; if10
	jnz	DeclareLocals_if10else

	call	NextTok

	call	NextTok

	call	DoConstLongMath
	mul	dword [ebp+size]
	add	[localsize], eax

	mov	eax, [localsize]

	test	eax, 3		; if11
	jz	DeclareLocals_if11end

	shr	eax, 2
	inc	eax
	shl	eax, 2

	mov	[localsize], eax  ; Выравнивание на dword

DeclareLocals_if11end:
	push	tk_closeblock
	call	expecting

	jmp	DeclareLocals_if9end

DeclareLocals_if10else:
	add	[localsize], 4

DeclareLocals_if9end:
	jmp	DeclareLocals_if8end

DeclareLocals_if9else:
	cmp	[tok], tk_semicolon ; if12
	jnz	DeclareLocals_if12else

	call	NextTok

	jmp	DeclareLocals_LL

	jmp	DeclareLocals_if8end

DeclareLocals_if12else:
	cmp	[tok], tk_openbrace ; if13
	jz	DeclareLocals_L1

	cmp	[tok], tk_eof
	jz	DeclareLocals_L1

	cmp	[tok], tk_comma   ; if14
	jz	DeclareLocals_if8end

	call	idexpected

DeclareLocals_if8end:
	jmp	DeclareLocals_loopstart

DeclareLocals_L1:
	cmp	[paramsize], 0	  ; if15
	jnz	DeclareLocals_if15end

	push	DeclareLocals_str1 ; "push ebp; mov ebp,esp;"
	call	Asm

DeclareLocals_if15end:
	invoke	wsprintfA, mapstr, DeclareLocals_str2, [localsize]
	add	esp, 0Ch

	push	mapstr	 ; str_
	call	Asm

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Обработка программного блока {...}
; ---------------------------------------------------------------------------
  proc	DoBlock
	push	tk_openbrace
	call	expecting

DoBlock_loopstart:
	cmp	[tok], tk_eof	  ; if1
	jnz	DoBlock_if1end

	call	unexpectedeof

	jmp	DoBlock_end

DoBlock_if1end:
	cmp	[tok], tk_closebrace ; if2
	jnz	DoBlock_if2end

	call	NextTok

	jmp	DoBlock_end

DoBlock_if2end:
	call	DoCommand

	jmp	DoBlock_loopstart

DoBlock_end:
	ret
  endp
; ---------------------------------------------------------------------------
; Завершение тела процедуры
; ---------------------------------------------------------------------------
  proc	LeaveProc
	cmp	[localsize], 0	  ; if1
	jbe	LeaveProc_if1else

	push	LeaveProc_str1 ; "leave"
	call	Asm

	jmp	LeaveProc_end

LeaveProc_if1else:
	cmp	[paramsize], 0	  ; if2
	jbe	LeaveProc_end

	push	LeaveProc_str2 ; "pop ebp"
	call	Asm

LeaveProc_end:
	cmp	[current_proc_type], cpt_far ; if3
	jnz	LeaveProc_if3else

	cmp	[paramsize], 0	  ; if4
	jnz	LeaveProc_if4else

	mov	eax, LeaveProc_str3 ; "retf"

	jmp	LeaveProc_if4end

LeaveProc_if4else:
	invoke	wsprintfA, mapstr, LeaveProc_str4, [paramsize]
	add	esp, 0Ch

	mov	eax, mapstr

LeaveProc_if4end:
	jmp	LeaveProc_if3end

LeaveProc_if3else:
	cmp	[paramsize], 0	  ; if5
	jnz	LeaveProc_if5else

	mov	eax, LeaveProc_str5 ; "ret"

	jmp	LeaveProc_if3end

LeaveProc_if5else:
	invoke	wsprintfA, mapstr, LeaveProc_str6, [paramsize]
	add	esp, 0Ch

	mov	eax, mapstr

LeaveProc_if3end:
	push	eax		; str_
	call	Asm

	ret
  endp
; ---------------------------------------------------------------------------
; Очистка списка локальных переменных
; ---------------------------------------------------------------------------
  proc	KillLocals
  ptr1		  = -8
  ptr2		  = -4

	enter	8, 0
	mov	eax, [locallist]
	mov	[ebp+ptr2], eax

KillLocals_loopstart:
	cmp	dword [ebp+ptr2], NULL ; loop condition
	jz	KillLocals_loopend

	mov	eax, [ebp+ptr2]
	mov	[ebp+ptr1], eax

	cmp	dword [eax+localtok], tk_locallabel ; if1 (Проверка на незакрытые метки)
	jnz	KillLocals_if1end

	add	eax, [localid]
	push	eax
	call	localunresolved ; localunresolved(EAX+localid);

KillLocals_if1end:
	mov	eax, [ebp+ptr2]

	mov	eax, [eax+localnext]
	mov	[ebp+ptr2], eax

	invoke	GlobalFree, [ebp+ptr1]

	jmp	KillLocals_loopstart

KillLocals_loopend:
	mov	[locallist], NULL
	mov	[paramsize], 0
	mov	[localsize], 0

	leave
	retn
endp
; ---------------------------------------------------------------------------
; Обработка одной команды внутри блока
; ---------------------------------------------------------------------------
  proc	DoCommand
DoCommand_LL:
	push	St_Sizes
	push	string	 ; lpString2
	call	FastSearch	; Это размер операнда?

	jnb	DoCommand_if1end ; if1

	add	eax, eax	; Да: byte,word или dword
	add	eax, tk_byte
	mov	[type], eax

	mov	[string+0], 0	    ; string[0]=0;

	mov	[tok], tk_var

	jmp	DoCommand_LL

DoCommand_if1end:
	cmp	[tok], tk_mnemonics ; if2
	jnz	DoCommand_if2else

	call	DoMnemonics

	call	NextTok

	jmp	DoCommand_if2end

DoCommand_if2else:
	cmp	[tok], tk_directive ; if3
	jnz	DoCommand_if3else

	push	[number]
	push	Jmp_Directives
	call	GetDirAddr

	call	eax

	jmp	DoCommand_if2end

DoCommand_if3else:
	cmp	[tok], tk_command ; if4
	jnz	DoCommand_if4else

	push	[number]
	push	Jmp_Commands
	call	GetDirAddr

	call	eax

	jmp	DoCommand_if2end

DoCommand_if4else:
	cmp	[tok], tk_id	  ; if5
	jnz	DoCommand_if5else

	push	tk_void
	call	DoId

	cmp	eax, tokens	; if6
	jz	DoCommand_if6end

	call	NextSemiNext

DoCommand_if6end:
	jmp	DoCommand_if2end

DoCommand_if5else:
	cmp	[tok], tk_undefproc ; if7
	jnz	DoCommand_if7else

	push	tk_void
	call	DoAnyUndefproc

	cmp	eax, tokens	; if8
	jz	DoCommand_if8end

	call	NextSemiNext

DoCommand_if8end:
	jmp	DoCommand_if2end

DoCommand_if7else:
	cmp	[tok], tk_proc	  ; if9
	jnz	DoCommand_if9else

	call	DoAnyProc

	call	NextSemiNext

	jmp	DoCommand_if2end

DoCommand_if9else:
if DEFINE_MAKE_EXE
	cmp	[tok], tk_API	  ; if10
	jnz	DoCommand_if10else

	call	doAPI
	cmp	eax, tokens	; if11

	jz	DoCommand_if11end

	call	NextSemiNext

DoCommand_if11end:
	jmp	DoCommand_if2end
end if
DoCommand_if10else:
	cmp	[tok], tk_var	  ; if12
	jz	DoCommand_if12start

	cmp	[tok], tk_local
	jz	DoCommand_if12start

	cmp	[tok], tk_param
	jz	DoCommand_if12start

	cmp	[tok], tk_reg
	jnz	DoCommand_if12else

DoCommand_if12start:
	push	[type]
	call	DoVar

	jmp	DoCommand_if2end

DoCommand_if12else:
	cmp	[tok], tk_openblock ; if13
	jnz	DoCommand_if13else

	push	tk_dword
	call	DoVar

	jmp	DoCommand_if2end

DoCommand_if13else:
	cmp	[tok], tk_string  ; if14
	jnz	DoCommand_if14else

	call	Macros

	call	NextSemiNext

	jmp	DoCommand_if2end

DoCommand_if14else:
	cmp	[tok], tk_locallabel ; if15
	jnz	DoCommand_if15else

	call	DoLocalPost

	jmp	DoCommand_if2end

DoCommand_if15else:
	cmp	[tok], tk_openbrace ; if16
	jnz	DoCommand_if16else

	call	DoBlock

	jmp	DoCommand_if2end

DoCommand_if16else:
	cmp	[tok], tk_comma   ; if17
	jz	DoCommand_if17start

	cmp	[tok], tk_semicolon
	jnz	DoCommand_if17else

DoCommand_if17start:
	call	NextTok

	jmp	DoCommand_if2end

DoCommand_if17else:
	cmp	[tok], tk_eof
	jnz	DoCommand_if2end

	call	unexpectedeof

DoCommand_if2end:
	ret
  endp
; ---------------------------------------------------------------------------
; Быстрый поиск по таблице
; OUTPUT: EAX=No.& CARRYFLAG - success
; ---------------------------------------------------------------------------
  proc	FastSearch
  ident 	  = -44h
  probe 	  = 8
  table 	  = 0Ch

	enter	44h, 0

	push	ecx
	push	esi
	push	edi
	push	edx

	lea	eax, [ebp+ident]
	invoke	lstrcpyA, eax, [ebp+probe]

	lea	eax, [ebp+ident]
	invoke	CharUpperA, eax

	lea	eax, [ebp+ident]
	invoke	lstrlenA, eax

	mov	edx, eax
	dec	edx

	jz	FastSearch_HET ; ident из одного символа

	xor	ebx, ebx

	mov	bl, [ebp+ident] ; Проверим первый символ в ident

	sub	bl, 'A'

	jb	FastSearch_HET ; Не буква

	cmp	bl, 'Z'-'A'

	ja	FastSearch_HET ; и не 'A'...'Z'

	xor	eax, eax

	shl	ebx, 2
	add	ebx, [ebp+table]

	mov	edi, [ebx]	; Взять адрес записи из table

	or	edi, edi

	jz	FastSearch_HET ; Нет записи для такого ident...

FastSearch_TRY0:
	mov	ah, byte [edi]

	inc	edi		; Номер записи из table и на строку в table

FastSearch_TRY1:
	lea	esi, [ebp+ident]
	inc	esi

	mov	ecx, edx	; Взять ident      длину - 1

	repe cmpsb

	jnz	FastSearch_NXT ; Сравним с записью в table

	jcxz	FastSearch_YES1 ; Совпали по длине probe

FastSearch_NXT:
	dec	edi

FastSearch_LOOK:
	mov	al, byte [edi]

	inc	edi

	or	al, al

	jz	FastSearch_TRY0 ; Проверим следующую запись

	cmp	al, '/'
	jz	FastSearch_TRY1 ; Проверим ALIAS

	cmp	al, _END
	jz	FastSearch_HET	; Конец таблицы - выход

	jmp	FastSearch_LOOK

FastSearch_HET:
	clc

	jmp	FastSearch_EX ; ident не найден в table

FastSearch_YES1:
	cmp	byte [edi], 0
	jz	FastSearch_YES

	cmp	byte [edi], _END
	jz	FastSearch_YES

	cmp	byte [edi], '/'
	jz	FastSearch_YES

	jmp	FastSearch_NXT

FastSearch_YES:
	mov	al, ah
	mov	ah, 0
	stc			; ident найден в table

FastSearch_EX:
	pop	edx
	pop	edi
	pop	esi
	pop	ecx

	leave
	ret	8
  endp
; ---------------------------------------------------------------------------
; Обработка новых идентификаторов
; ---------------------------------------------------------------------------
  proc	DoId
s		= -50h
expectedreturn	= 8

	enter	50h, 0
	cmp	[tok2], tk_colon  ; if1
	jnz	DoId_if1else

	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptImageBase]
	add	eax, [OptBaseOfCode]
	mov	[number], eax

	mov	[tok], tk_proc

	mov	[post], 0

	push	string	 ; keystring
	call	AddToTree

	call	NextTok
	call	NextTok 	; пропустим идентификатор и :

	mov	eax, tokens

	jmp	DoId_end

DoId_if1else:
	cmp	[tok2], tk_openbracket ; if2 вызов процедуры
	jnz	DoId_if2else

	lea	eax, [ebp+s]
	invoke	wsprintfA, eax, DoId_str1, string
	add	esp, 0Ch

	mov	[tok], tk_undefproc

	mov	[number], 0

	mov	[post], 1

	push	string	 ; keystring
	call	AddToTree

	call	NextTok

	call	DoParams

	lea	eax, [ebp+s]
	push	eax		; str_
	call	Asm

	mov	eax, [ebp+expectedreturn]

	jmp	DoId_end

DoId_if2else:
	call	undefinederror

	call	NextTok

	mov	eax, tk_int

DoId_end:
	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
; Обработка обращения к уже описанной процедуре
; ---------------------------------------------------------------------------
  proc	DoAnyProc
  s		  = -50h

	enter	50h, 0

	lea	eax, [ebp+s]
	invoke	wsprintfA, eax, DoId_str1, string
	add	esp, 0Ch

	call	NextTok
	call	DoParams
	lea	eax, [ebp+s]
	push	eax		; str_
	call	Asm
	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Обработка ранее объвленной, но пока не известной метки
; ---------------------------------------------------------------------------
  proc	DoAnyUndefproc
  s		  = -50h

	enter	50h, 0

	cmp	[tok2], tk_colon  ; if1
	jnz	DoAnyUndefproc_if1end

	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptImageBase]
	add	eax, [OptBaseOfCode]
	mov	[number], eax

	mov	[tok], tk_proc

	mov	esi, [treeptr]

	mov	eax, [tok]
	mov	[esi+rectok], eax

	mov	eax, [number]
	mov	[esi+recnumber], eax

	mov	dword [esi+recpost], 0

	call	NextTok 	; move past id
	call	NextTok 	; move past :

	mov	eax, tokens

	jmp	DoAnyUndefproc_end

DoAnyUndefproc_if1end:
	cmp	[tok2], tk_openbracket ; if2
	jnz	DoAnyUndefproc_if2end

	lea	eax, [ebp+s]
	invoke	wsprintfA, eax, DoId_str1, string
	add	esp, 0Ch

	call	NextTok

	call	DoParams

	lea	eax, [ebp+s]
	push	eax		; str_
	call	Asm

	mov	eax, tk_dword

	jmp	DoAnyUndefproc_end

DoAnyUndefproc_if2end:
	call	undefinederror

	call	NextTok

	mov	eax, tk_int

DoAnyUndefproc_end:
	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
; Обработка обращения к API функции
; ---------------------------------------------------------------------------
  proc	doAPI
  hold		  = -48h
  s		  = -44h

	enter	48h, 0

	cmp	[tok2], tk_openbracket ; if1
	jnz	doAPI_if1end

	mov	eax, [treeptr]
	mov	[ebp+hold], eax

	lea	eax, [ebp+s]
	push	eax		; varName
	call	GetVarname

	call	NextTok

	call	DoParams

	cmp	[posts], MAXPOSTS ; if2
	jb	doAPI_if2end

	push	doAPI_str1 ; "maximum number of API procedure calls e"...
	call	preerror

	mov	eax, tokens

	leave
	ret

doAPI_if2end:
	mov	ebx, [ebp+hold]

	cmp	dword [ebx+recpost], 0 ; if3 Первый вызов API?
	jnz	doAPI_if3end

	mov	dword [ebx+recpost], 1 ; отметим вызов данной API

	inc	[APIcount]

	mov	eax, [ebx+rectype] ; Указатель на DLL, в котором находится API

	inc	dword [eax+recmodline] ; Увеличим счетчик API, вызванных из DLL

doAPI_if3end:
	mov	eax, 15FFh
	call	OUTWORD 	; call [dword]

	push	POST_API
	push	dword [ebp+hold]
	call	SetPost

	xor	eax, eax
	call	OUTDWORD

	cmp	[list], 0	  ; if4
	jz	doAPI_if4end

	push	doAPI_str2 ; "\t//\tcall "
	call	fprint

	lea	eax, [ebp+s]
	push	eax		; lpBuffer
	call	fprint

	push	doAPI_str2 ; \n
	call	fprint

doAPI_if4end:
	mov	eax, tk_int

	leave
	ret

doAPI_if1end:
	call	undefinederror

	call	NextTok

	mov	eax, tokens

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Обработка <Var> ...
; ---------------------------------------------------------------------------
  proc	DoVar
  next		  = -110h
  vtok		  = -10Ch
  varName	  = -108h
  varName2	  = -84h
  vartype	  = 8

	enter	110h, 0

	mov	dword [ebp+next], 1

	lea	eax, [ebp+varName]
	push	eax		; varName
	call	GetVarname
	mov	[ebp+vtok], eax

	call	NextTok

	cmp	[tok], tk_assign  ; if1
	jnz	DoVar_if1else

	call	NextTok

	call	tok2notstopper
	test	eax, eax	; if2
	jz	DoVar_if2else

	push	DoVar_str1 ; "mov"
	push	dword [ebp+vartype]
	push	dword [ebp+vtok]
	lea	eax, [ebp+varName]
	push	eax
	call	DoExpr

	mov	dword [ebp+next], 0

	jmp	DoVar_if2end

DoVar_if2else:
	push	dword [ebp+vartype]
	push	dword [ebp+vtok]
	lea	eax, [ebp+varName]
	push	eax
	call	GetIntoVar

DoVar_if2end:
	jmp	DoVar_if1end

DoVar_if1else:
	cmp	[tok], tk_minusminus ; if3 Var--;
	jnz	DoVar_if3else

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str2, eax
	add	esp, 0Ch

	push	mapstr	 ; str_
	call	Asm

	jmp	DoVar_if1end

DoVar_if3else:
	cmp	[tok], tk_plusplus ; if4 Var++;
	jnz	DoVar_if4else

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str3, eax
	add	esp, 0Ch

	push	mapstr	 ; str_
	call	Asm

	jmp	DoVar_if1end

DoVar_if4else:
	cmp	[tok], tk_plusequals ; if5 Var+=Expr;
	jnz	DoVar_if5else

	call	NextTok

	push	DoVar_str4 ; "add"
	push	dword [ebp+vartype]
	push	tk_var
	lea	eax, [ebp+varName]
	push	eax
	call	DoExpr

	mov	dword [ebp+next], 1

	jmp	DoVar_if1end

DoVar_if5else:
	cmp	[tok], tk_minusequals ; if6 Var-=Expr;
	jnz	DoVar_if6else

	call	NextTok

	push	DoVar_str5 ; "sub"
	push	dword [ebp+vartype]
	push	tk_var
	lea	eax, [ebp+varName]
	push	eax
	call	DoExpr

	mov	dword [ebp+next], 1

	jmp	DoVar_if1end

DoVar_if6else:
	cmp	[tok], tk_andequals ; if7 Var&=Expr;
	jnz	DoVar_if7else

	call	NextTok

	push	DoVar_str6 ; "and"
	push	dword [ebp+vartype]
	push	tk_var
	lea	eax, [ebp+varName]
	push	eax
	call	DoExpr

	mov	dword [ebp+next], 1

	jmp	DoVar_if1end

DoVar_if7else:
	cmp	[tok], tk_xorequals ; if8 Var^=Expr;
	jnz	DoVar_if8else

	call	NextTok

	push	DoVar_str7 ; "xor"
	push	dword [ebp+vartype]
	push	tk_var
	lea	eax, [ebp+varName]
	push	eax
	call	DoExpr

	mov	dword [ebp+next], 1

	jmp	DoVar_if1end

DoVar_if8else:
	cmp	[tok], tk_orequals ; if9 Var|=Expr;
	jnz	DoVar_if9else

	call	NextTok

	push	DoVar_str8 ; "or"
	push	dword [ebp+vartype]
	push	tk_var
	lea	eax, [ebp+varName]
	push	eax
	call	DoExpr

	mov	dword [ebp+next], 1

	jmp	DoVar_if1end

DoVar_if9else:
	cmp	[tok], tk_swap	  ; if10 Var><Var;
	jnz	DoVar_if10else

	call	NextTok

	lea	eax, [ebp+varName2]
	push	eax		; varName
	call	GetVarname

	cmp	[tok], tk_reg	  ; if11
	jnz	DoVar_if11else

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str9, string, eax
	add	esp, 10h

	push	mapstr	 ; str_
	call	Asm

	jmp	DoVar_if10end

DoVar_if11else:
	cmp	[tok], tk_var	  ; if12
	jnz	DoVar_if12else

	call	tok2notstopper
	test	eax, eax
	jnz	DoVar_if12else

	cmp	dword [ebp+vartype], tk_dword ; if13
	jz	DoVar_if13start

	cmp	dword [ebp+vartype], tk_int
	jnz	DoVar_if13else

DoVar_if13start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str10, eax
	add	esp, 0Ch

	push	mapstr	; str_
	call	Asm

	lea	eax, [ebp+varName2]
	invoke	wsprintfA, mapstr, DoVar_str10, eax
	add	esp, 0Ch

	push	mapstr	 ; str_
	call	Asm

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str10, eax
	add	esp, 0Ch

	jmp	DoVar_if13end

DoVar_if13else:
	cmp	dword [ebp+vartype], tk_word ; if14
	jz	DoVar_if14start

	cmp	dword [ebp+vartype], tk_short
	jnz	DoVar_if14else

DoVar_if14start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str11, eax
	add	esp, 0Ch

	push	mapstr	 ; str_
	call	Asm

	lea	eax, [ebp+varName2]
	invoke	wsprintfA, mapstr, DoVar_str11, eax
	add	esp, 0Ch

	push	mapstr	 ; str_
	call	Asm

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str11, eax
	add	esp, 0Ch

	jmp	DoVar_if13end

DoVar_if14else:
	cmp	dword [ebp+vartype], tk_byte ; if15
	jz	DoVar_if15start

	cmp	dword [ebp+vartype], tk_char
	jnz	DoVar_if13end

DoVar_if15start:
	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str12, eax
	add	esp, 0Ch

	push	mapstr	 ; str_
	call	Asm

	lea	eax, [ebp+varName2]
	invoke	wsprintfA, mapstr, DoVar_str12, eax
	add	esp, 0Ch

	push	mapstr	 ; str_
	call	Asm

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str12
	add	esp, 0Ch

DoVar_if13end:
	push	mapstr	 ; str_
	call	Asm

	jmp	DoVar_if10end

DoVar_if12else:
	call	swaperror

DoVar_if10end:
	jmp	DoVar_if1end

DoVar_if10else:
	cmp	[tok], tk_llequals ; if16 Var<<=Expr;
	jnz	DoVar_if16else

	call	NextTok

	cmp	[tok], tk_number  ; if17
	jnz	DoVar_if17else

	call	DoConstMath

	lea	ebx, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str17, ebx, eax
	add	esp, 10h

	jmp	DoVar_if17end

DoVar_if17else:
	push	tk_byte
	push	tk_reg
	push	DoVar_str13 ; "cl"
	call	Expression

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str14, eax
	add	esp, 0Ch

	mov	dword [ebp+next], 0

DoVar_if17end:
	push	mapstr	 ; str_
	call	Asm

	jmp	DoVar_if1end

DoVar_if16else:
	cmp	[tok], tk_rrequals ; if18 Var>>=Expr;
	jnz	DoVar_if18else

	call	NextTok

	cmp	[tok], tk_number  ; if19
	jnz	DoVar_if19else

	call	DoConstMath

	lea	ebx, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str15, ebx, eax
	add	esp, 10h

	jmp	DoVar_if19end

DoVar_if19else:
	push	tk_byte
	push	tk_reg
	push	DoVar_str13 ; "cl"
	call	Expression

	lea	eax, [ebp+varName]
	invoke	wsprintfA, mapstr, DoVar_str16, eax
	add	esp, 0Ch

	mov	dword [ebp+next], 0

DoVar_if19end:
	push	mapstr	 ; str_
	call	Asm

	jmp	DoVar_if1end

DoVar_if18else:
	call	operatorexpected

DoVar_if1end:
	cmp	dword [ebp+next], 0	; if20
	jz	DoVar_if20else

	call	NextSemiNext

	jmp	DoVar_end

DoVar_if20else:
	call	SemiNext

DoVar_end:
	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
; Обработка макроса ((strlen+strlen+1)*1)
; ---------------------------------------------------------------------------
  proc	Macros
  holdcha	  = -7D4h
  s		  = -7D0h
  s2		  = -3E8h

	enter	7D4h, 0

	cmp	[makemapfile], 0  ; if1
	jz	Macros_if1end

	push	string	 ; lpBuffer
	call	fprint

Macros_if1end:
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

	lea	eax, [ebp+s2]
	invoke	lstrcpyA, eax, string2

	lea	eax, [ebp+s]
	mov	[input], eax

	mov	eax, [input] ; лишнее
	mov	[inptr], eax

	mov	eax, [input] ; лишнее
	mov	[inptr2], eax

	mov	[endoffile], 0 ; На начале файла

	call	NextChar

	mov	al, [cha]
	mov	[cha2], al

	mov	eax, [inptr]
	mov	[inptr2], eax

	mov	[linenum2], 1

	call	NextTok

Macros_loopstart:
	cmp	[tok], tk_eof	  ; if1
	jz	Macros_loopend

	call	DoCommand
	jmp	Macros_loopstart

Macros_loopend:
	lea	eax, [ebp+s]
	invoke	lstrcpyA, string, eax

	lea	eax, [ebp+s2]
	invoke	lstrcpyA, string2, eax

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
; Обработка параметров при вызове процедуры
; ---------------------------------------------------------------------------
  proc	DoParams
	cmp	[tok], tk_openbracket ; if1
	jnz	DoParams_if1else

	dec	[inptr2]

	call	DoParam

	call	NextTok

	jmp	DoParams_end

DoParams_if1else:
	push	tk_openbracket
	call	expecting

DoParams_end:
	ret
  endp
; ---------------------------------------------------------------------------
; Запись байта в CODE
; ---------------------------------------------------------------------------
  proc	OP
	xchg	edi, [outptr]
	stosb
	xchg	edi, [outptr]
	ret
  endp
; ---------------------------------------------------------------------------
; Запись слова в CODE
; ---------------------------------------------------------------------------
  proc	OUTWORD
	xchg	edi, [outptr]
	stosw
	xchg	edi, [outptr]
	ret
  endp
; ---------------------------------------------------------------------------
; Запись двойного слова в CODE
; ---------------------------------------------------------------------------
  proc	OUTDWORD
	xchg	edi, [outptr]
	stosd
	xchg	edi, [outptr]
	ret
  endp
; ---------------------------------------------------------------------------
; Пропуск пробелов и табуляций
; ---------------------------------------------------------------------------
  proc	WhiteSpaces
	mov	al, byte [cha]

	cmp	al, 32
	jz	WhiteSpaces_NXT

	cmp	al, 10
	jz	WhiteSpaces_NXT

	cmp	al, 9
	jnz	WhiteSpaces_loopend

WhiteSpaces_NXT:
	call	NextChar

	jmp	WhiteSpaces

WhiteSpaces_loopend:
	cmp	al, 13
	jnz	WhiteSpaces_end

	inc	[linenumber]	  ; Обнаружен конец строки

	inc	[totallines]

	call	ShowSrcLine

	jmp	WhiteSpaces_NXT

WhiteSpaces_end:
	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------