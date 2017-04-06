; ---------------------------------------------------------------------------
; Кодогенератор
; ---------------------------------------------------------------------------
; ---------------------------------------------------------------------------
; Ассемблирование строки
; ---------------------------------------------------------------------------
  proc	Asm
  holdcha	  = -8D4h
  source	  = -8D0h
  s		  = -7D0h
  s2		  = -3E8h
  str_		  = 8

	enter	8D4h, 0

	lea	eax, [ebp+source]
	invoke	lstrcpyA, eax, [ebp+str_]

	cmp	[list], 0	  ; if1
	jz	Asm_if1end

	push	Asm_str1 ; "\t//\t"
	call	fprint

	push	dword [ebp+str_]
	call	fprint

	push	Asm_str3 ; "\n"
	call	fprint

Asm_if1end:
	mov	al, [cha2]	  ; запомнить позицию разбора
	mov	byte [ebp+holdcha], al

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
	push	[type]

	lea	eax, [ebp+s]
	invoke	lstrcpyA, eax, string

	lea	eax, [ebp+s2]
	invoke	lstrcpyA, eax, string2

	lea	eax, [ebp+source]
	mov	[input], eax	  ; разбираем новую строку

	mov	eax, [input]
	mov	[inptr], eax

	mov	eax, [input]
	mov	[inptr2], eax

	mov	[endoffile], 0	     ; На начале файла

	call	NextChar

	mov	al, [cha]
	mov	byte [cha2], al

	mov	eax, [inptr]
	mov	[inptr2], eax

	mov	[linenum2], 1

Asm_loopstart:
	call	NextTok

	cmp	[tok], tk_mnemonics ; if2
	jnz	Asm_if2else

	call	DoMnemonics

	jmp	Asm_if2end

Asm_if2else:
	cmp	[tok], tk_eof	  ; if3
	jz	Asm_loopend

	cmp	[tok], tk_locallabel ; if4
	jnz	Asm_if4else

	call	DoLocalPost

	jmp	Asm_if2end

Asm_if4else:
	cmp	[tok], tk_semicolon ; if5
	jz	Asm_if2end

	push	Asm_str2 ; "ASM: Bad input format\r\n"
	call	preerror

Asm_if2end:
	jmp	Asm_loopstart

Asm_loopend:
	lea	eax, [ebp+s]
	invoke	lstrcpyA, string, eax

	lea	eax, [ebp+s2]
	invoke	lstrcpyA, string2, eax

	pop	[type]
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

	mov	al, [ebp+holdcha]
	mov	[cha2], al

	leave
	ret	4
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	DoLocalPost
  i		  = -4

	enter	4, 0

	mov	[tok], tk_number

	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptImageBase]
	add	eax, [OptBaseOfCode]
	mov	[number], eax

	mov	esi, [localptr]

	mov	eax, [tok]
	mov	[esi+localtok], eax

	mov	eax, [number]
	mov	[esi+localnumber], eax

	mov	dword [ebp+i], 0

DoLocalPost_loopstart:
	mov	eax, [posts]
	cmp	dword [ebp+i], eax    ; loop condition
	jnb	DoLocalPost_loopend

	mov	ecx, [ebp+i]
	shl	ecx, 2
	add	ecx, [postnum]

	mov	eax, [ecx]
	cmp	[localptr], eax   ; if1
	jnz	DoLocalPost_if1end

	mov	eax, [number]
	mov	[ecx], eax

	mov	ebx, [ebp+i]
	shl	ebx, 2
	add	ebx, [posttype]

	mov	dword [ebx], POST_LOC

DoLocalPost_if1end:
	inc	dword [ebp+i]

	jmp	DoLocalPost_loopstart

DoLocalPost_loopend:
	call	NextTok

	call	NextTok

	leave
	ret
  endp
; ---------------------------------------------------------------------------
; Установка ссылки на пока необъявленный идентификатор
; ---------------------------------------------------------------------------
  proc	SetPost
  ref		  = 8
  ptype 	  = 0Ch

	push	ebp
	mov	ebp, esp

	cmp	[posts], MAXPOSTS ; if1
	jb	SetPost_if1end

	call	maxwordpostserror

SetPost_if1end:
	mov	ebx, [posts]
	shl	ebx, 2
	add	ebx, [postloc]

	mov	eax, [outptr]
	mov	[ebx], eax

	mov	ebx, [posts]
	shl	ebx, 2
	add	ebx, [postnum]

	mov	eax, [ebp+ref]
	mov	[ebx], eax

	mov	ebx, [posts]
	shl	ebx, 2
	add	ebx, [posttype]

	mov	eax, [ebp+ptype]
	mov	[ebx], eax

	inc	[posts]
	pop	ebp
	ret	8
  endp
; ---------------------------------------------------------------------------
; Пропуск до следующей записи в шаблоне инструкции
; ---------------------------------------------------------------------------
  proc	SCDEND
	lodsb

	test	al, al		; if1
	jz	SCDEND_end

	cmp	al, _END	 ; if2 (_END)
	jnz	SCDEND

	call	illegaloperand

	pop	eax		; Выход из MapOperands

SCDEND_end:
	ret
  endp
; ---------------------------------------------------------------------------
; Получим значение масштабного коэффициента
; ---------------------------------------------------------------------------
proc  GETSCALING
	call	NextTok

	cmp	[tok], tk_number
	jnz	GETSCALING_if1else ; if1

GETSCALING_DoScale:
	mov	eax, [number]
	mov	dword [EA_SCALE], eax
	mov	byte [EA_SIBFLAG], 1   ; Отметим наличие Sib в 32-битном EA
	jmp	GETSCALING_end

GETSCALING_if1else:
	push	GETSCALING_str1 ; "ASM: Illegal scaling value\r\n"
	call	preerror

GETSCALING_end:
	ret
  endp
; ---------------------------------------------------------------------------
; Поиск в шаблоне подходящей к операнду записи (для однооперандной инструкции)
; ---------------------------------------------------------------------------
  proc	GETOL1
	cmp	byte [esi], 0 ; if1
	jz	GETOL1_if1end	; Инструкция JMP - не проверяем совпадение размеров

GETOL1_G4:
	lodsb

	mov	ah, al

	and	ah, NOLBITS	; Исключим число операндов

	jz	GETOL1_if1end	; if2 (Это все-таки JMP...)

	mov	dl, al
	and	dl, OPBITS	; Маска для типа операнда

	cmp	dl, SOO        ; if3
	jz	GETOL1_if1end	; В поле размера операнда содержится информация о типе операнда?

	and	al, OLBITS	; Выделим количество операндов

	cmp	al, OL1 	; if4
	jz	GETOL1_G2 ; Шаблон для инструкции с одним операндом?

GETOL1_G3:
	lodsb			; Просмотрим следующую запись шаблона

	test	al, al		; if5(Конец записи?)
	jz	GETOL1_G4

	cmp	al, _END	; loop condition
	jnz	GETOL1_G3 ; Это конец шаблона?

	call	illegaloperand	; Нет подходящей записи для такой мнемоники

	pop	eax

	retn			; Выход из MapOperands

GETOL1_G2:
	mov	al, ah
	and	al, TRANSBITS	; Выделим размер операнда

	cmp	al, _D
	jbe	GETOL1_G5

	cmp	al, _OW
	jnz	GETOL1_J0

	mov	byte [OPERANDSIZE], _W

	jmp	GETOL1_G40

GETOL1_J0:
	cmp	al, _OD
	jnz	GETOL1_J1

	mov	byte [OPERANDSIZE], _D

	jmp	GETOL1_G40

GETOL1_J1:
	cmp	al, NB
	jb	GETOL1_G90	; Операнд м.б. размера WB,DW OR BW.

	sub	al, NB		; Пересчитаем для фиксированных размеров: NB->B,OW->W, AF->D

GETOL1_G90:
	cmp	al, byte [OPERANDSIZE] ; Размер операнда и размер из шаблона совпали?
	jnz	GETOL1_G3 ; Нет - смотрим следующую запись в шаблоне

	jmp	GETOL1_G40 ; Размеры совпали - продолжим

GETOL1_G5:
	cmp	al, byte [OPERANDSIZE] ; Размер операнда и размер из шаблона совпали?
	ja	GETOL1_G3 ; Нет - смотрим следующую запись в шаблоне

	xor	ebx, ebx	; WBIT/RANDFLAG=0

	mov	dl, al

	mov	al, byte [OPERANDSIZE]

	test	dl, dl		; if6 (сверяется с константой _B, она равна 0, поэтому тут test влепил компилятор)
	jnz	GETOL1_if6end

	test	al, al
	jz	GETOL1_if6end

	inc	bl

GETOL1_if6end:
	cmp	al, _W		; В шаблоне word?
	jnz	GETOL1_G30

	ja	GETOL1_G30

	inc	bh		; Операнд обязательно д.б. word - требуется префикс RAND

GETOL1_G30:
	mov	byte [WBIT], bl
	mov	byte [RANDFLAG], bh    ; Запомним информацию о префиксе и W-бите

GETOL1_G40:
	and	ah, NTRANSBITS	; Оставим SOO биты

GETOL1_if1end:
	mov	al, ah		; Возвратим SOO поле
	ret
  endp
; ---------------------------------------------------------------------------
; Разборка шаблона для двухоперандной инструкции
; ---------------------------------------------------------------------------
  proc	GETOL2
GETOL2_G7:

	lodsb			; Получим байт из шаблона

	mov	ah, al

	and	al, OLBITS	; Выделим число операндов

	cmp	al, OL2 	; Число операндов = 2?
	jz	GETOL2_G8 ; Да - начнем проверку

GETOL2_G9:
	lodsb			; Поиск следующей записи в шаблоне

	or	al, al		; Конец записи?

	jz	GETOL2_G7    ; Да - проверим новую запись (прыжок на G7)

	cmp	al, _END	; Конец шаблона?
	jnz	GETOL2_G9 ; Смотри дальше иначе - ошибка

	call	toomuchoperands

	pop	eax

	ret			; Выход из MapOperands

GETOL2_G8:
	and	ah, NOLBITS	; Исключим число операндов

	mov	al, ah

	and	al, TRANSBITS	; Выделим размер операнда

	cmp	al, _D
	jbe	GETOL2_G100 ; Операнд м.б. размера: byte, word, dword

	cmp	al, NB
	jb	GETOL2_J0 ; Операнд м.б. размера WB,DW OR BW.

	sub	al, NB		; Пересчитаем для фиксированных размеров: NB->B,OW->W, AF->D

GETOL2_G95:
	cmp	al, [OPERANDSIZE] ; Размер операнда и размер из шаблона совпали?
	jnz	GETOL2_G9 ; Размеры не совпали - ищем следующую запись

	jmp	GETOL2_G11	; Размеры совпали - продолжим

GETOL2_J0:
	cmp	byte [OPDESC + 0], CO	   ; OPDESC[0],CO
	jnz	GETOL2_J1

	cmp	al, WB
	jnz	GETOL2_J1

	mov	byte [OPCONSTSIZE + 0], _W ; OPCONSTSIZE[0]=_W;
	mov	byte [OPCONSTSIZE + 1], _B ; OPCONSTSIZE[1]=_B;

	jmp	GETOL2_G11

GETOL2_J1:
	cmp	al, _DW
	jnz	GETOL2_J2

	mov	byte [RANDFLAG], 0

	mov	byte [OPCONSTSIZE + 0], _D ; OPCONSTSIZE[0]=_D

	jmp	GETOL2_G11

GETOL2_J2:
	cmp	al, BW
	jnz	GETOL2_G95

	mov	byte [OPCONSTSIZE + 0], _B ; [0]

	mov	byte [OPCONSTSIZE + 1], _W ; [1]

	jmp	GETOL2_G11

GETOL2_G100:
	cmp	byte [OPERANDSIZE], _D
	ja	GETOL2_G9	; Размер операнда > dword - на следующую запись

	cmp	byte [OPERANDSIZE], al
	jb	GETOL2_G9	; Размеры не совпали - ищем следующую запись

	xor	ebx, ebx

	mov	dl, al

	mov	al, byte [OPERANDSIZE]

	cmp	dl, _B		; Размер в шаблоне = byte?
	jnz	GETOL2_G50

	cmp	al, _B		; Размер операнда = byte?
	jz	GETOL2_G50

	inc	bl		; W-бит=TRUE

GETOL2_G50:				; CODE XREF: GETOL2+A1j GETOL2+A5j
	cmp	al, _W		; В шаблоне word?
	jnz	GETOL2_G60

	ja	GETOL2_G60

	inc	bh		; Операнд обязательно д.б. word - требуется префикс RAND

GETOL2_G60:
	mov	byte [WBIT], bl

	mov	byte [RANDFLAG], bh

GETOL2_G11:
	and	ah, NTRANSBITS
	mov	al, AH ; Возвратим SOO поле
	retn
  endp
; ---------------------------------------------------------------------------
; Разборка шаблона для трехоперандной инструкции
; ---------------------------------------------------------------------------
  proc	GETOL3
GETOL3_G12:
	lodsb

	mov	ah, al
	and	al, OLBITS

	cmp	al, OL3
	jz	GETOL3_G13

GETOL3_G14:
	lodsb			; TRY NEXT ENTRY.

	or	al, al

	jz	GETOL3_G12

	cmp	al, _END
	jnz	GETOL3_G14

	call	toomuchoperands

	pop	eax
	ret		       ; Выход из MapOperands

GETOL3_G13:
	and	ah, NOLBITS
	cmp	[OPERANDSIZE], _D ; DWORD ?

	jz	GETOL3_G15
	cmp	[OPERANDSIZE], _W ; WORD ?

	jz	GETOL3_G16

	push	GETOL3_str1 ; "ASM: This instruction required a WORD/D"...
	call	preerror

	ret

GETOL3_G16:
	mov	byte [RANDFLAG], 1

GETOL3_G15:
	mov	al, ah
	and	al, 0E0h

	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	CREATE_EA
	and	al, 7
	mov	byte [EA_M], al

	mov	byte [EA_X], 3

	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	CREATE_R
	and	al, 7
	mov	byte [EA_R], al

	ret
  endp
; ---------------------------------------------------------------------------
; Генеррация ModRM и Sib (Количество регистров в EA)
; ---------------------------------------------------------------------------
  proc	GETMODRMBYTE
	mov	dl, byte [EALENGTH]

	or	dl, dl		; Нет регистров в EA?

	jz	GETMODRMBYTE_C11

	test	byte [EADESC + 0], _W+_D*8
	jz	GETMODRMBYTE_E1 ; 8-битные регистры нельзя применать в адресе

	test	byte [EADESC + 0], _W*8   ; 16-битный регистр?
	jnz	GETMODRMBYTE_E4 ; 16-битная адресация не разрешена

	call	GETEADISPX

	cmp	dh, 2

	jnz	GETMODRMBYTE_X00

	mov	eax, [opDescInd]
	mov	byte [OPCONSTSIZE + eax], _D ; Обязательно 32-битный disp

GETMODRMBYTE_X00:
	dec	dl		; 1 регистр?

	jnz	GETMODRMBYTE_N1 ; нет...

	mov	al, byte [EADESC + 0]
	and	al, 7

	cmp	byte [EA_SIBFLAG], 1   ; Флаг наличия Sib в 32-битном EA
	jnz	GETMODRMBYTE_L2 ; Нет Sib

	mov	byte [EA_R2], 5        ; Фиксируем базовый регистр

	mov	byte [EA_M], 4

	mov	byte [EA_I], al

	mov	edx, [opDescInd]

	cmp	byte [OPCONSTFLAG + edx], 1 ; Используем disp?
	jz	GETMODRMBYTE_L1

	xor	eax, eax

	mov	byte [OPCONSTFLAG + edx], 1

	shl	edx, 2
	mov	dword [OPCONST + edx], eax ; disp=0 в EA

GETMODRMBYTE_L1:
	mov	edx, [opDescInd]

	mov	byte [OPCONSTSIZE + edx], _D

	mov	byte [EA_X], 0

	ret

GETMODRMBYTE_L2:
	mov	byte [EA_M], al

	ret

GETMODRMBYTE_N1:
	mov	byte [EA_M], 4	       ; 2 REGISTERS USED.

	mov	byte [EA_SIBFLAG], 1

	mov	al, byte [EADESC + 1]	 ; EADESC[1]
	shr	al, 3

	cmp	al, _W
	jz	GETMODRMBYTE_E5 ; ERROR: INDEX REGISTER ISN'T OF SIZE DWORD

	mov	al, byte [EADESC + 0]

	mov	ah, byte [EADESC + 1]

	and	eax, 707h

	cmp	ah, 5		; CAN'T USE BP AS INDEX.
	jz	GETMODRMBYTE_E6

	mov	byte [EA_R2], al
	mov	byte [EA_I], ah
	ret

GETMODRMBYTE_E1:			;
	push	GETMODRMBYTE_str1 ; "ASM: You can't use byte registers in ad"...
	call	preerror

	ret

GETMODRMBYTE_E4:
	push	GETMODRMBYTE_str2 ; "ASM: 16-bit addressing mode not allowed"...
	call	preerror

	ret

GETMODRMBYTE_E5:
	push	GETMODRMBYTE_str3 ; "ASM: You must use a 32-bit registers fo"...
	call	preerror

	ret
; ---------------------------------------------------------------------------

GETMODRMBYTE_E6:
	push	GETMODRMBYTE_str4 ; "ASM: You can't use EBP as an index\r\n"
	call	preerror

	ret

GETMODRMBYTE_C11:
	mov	byte [EA_X], 0

	mov	byte [EA_M], 5

	mov	ecx, [opDescInd]

	mov	al, byte [OPCONSTSIZE + ecx]

	test	al, al		; if al == _B
	jnz	GETMODRMBYTE_if1else

	mov	byte [OPCONSTSIZE + ecx], _D
	jmp	GETMODRMBYTE_end

GETMODRMBYTE_if1else:
	cmp	al, _W
	jnz	GETMODRMBYTE_end

	mov	byte [ADDRFLAG], 1

GETMODRMBYTE_end:
	ret
  endp
; ---------------------------------------------------------------------------
; CREATE X NIBBLE OF DISPLACEMENT SIZE.
; ---------------------------------------------------------------------------
  proc GETEADISPX
	mov	dh, 0

	push	ecx

	mov	ecx, [opDescInd]

	cmp	byte [OPCONSTFLAG + ecx], 1 ; if1
	jnz	GETEADISPX_if1end

	mov	al, byte [OPCONSTSIZE + ecx]

	mov	dh, 2		; (D)WORD DISPLACEMENT

	test	al, al		; if al == _B
	jnz	GETEADISPX_if1end

	dec	dh		; SBYTE DISPLACEMENT

GETEADISPX_if1end:
	mov	byte [EA_X], dh

	pop	ecx

	ret
  endp
; ---------------------------------------------------------------------------
; Инициализация буфера ассемблера
; ---------------------------------------------------------------------------
  proc	INIT_LINE
	mov	ecx, opDescInd
	sub	ecx, OPERANDSIZE ; pointer

	mov	al, 0

	mov	edi, OPERANDSIZE ; ptr

	rep stosb

	mov	al, 255

	mov	byte [OPERANDSIZE], al
	mov	byte [SEGREGISTER], al

	ret
  endp
; ---------------------------------------------------------------------------
; Запись переопределения сегмента
; ---------------------------------------------------------------------------
  proc	WRITEOVERRIDE
	movzx	ebx, byte [OVERRIDE]

	test	bl, bl		; if1
	jz	WRITEOVERRIDE_if1end

	mov	al, byte [OVERRIDETAB + ebx]
	sub	al, regES

	call	OP

WRITEOVERRIDE_if1end:
	ret
  endp
; ---------------------------------------------------------------------------
; Запись префикса размерности операнда
; ---------------------------------------------------------------------------
  proc	WRITERAND
	push	eax

	cmp	byte [RANDFLAG], 1	  ; if1
	jnz	WRITERAND_if1end

	mov	al, 66h

	call	OP

WRITERAND_if1end:
	pop	eax

	ret
  endp
; ---------------------------------------------------------------------------
; Запись константы: CL=TYPE; EDI указатель на значение
; ---------------------------------------------------------------------------
  proc	WRITECONST
	test	cl, cl		; if1 ; IF(CL==_B)CL=1;
	jnz	WRITECONST_if1else

	mov	cl, 1

	jmp	WRITECONST_loopstart

WRITECONST_if1else:
	cmp	cl, _W		 ; if2
	jnz	WRITECONST_if2else

	mov	cl, 2

	jmp	WRITECONST_loopstart

WRITECONST_if2else:
	cmp	cl, _D		 ; if3
	jnz	WRITECONST_if3else

	mov	cl, 4

	jmp	WRITECONST_loopstart

WRITECONST_if3else:
	inc	cl

WRITECONST_loopstart:
	mov	al, byte [edi]

	inc	edi

	call	OP

	loop	WRITECONST_loopstart

	ret
  endp
; ---------------------------------------------------------------------------
; Обработка Override
; ---------------------------------------------------------------------------
  proc	GETOVERRIDE
	cmp	[tok], tk_seg	  ; if1
	jnz	GETOVERRIDE_end
	cmp	[tok2], tk_colon
	jnz	GETOVERRIDE_end

	cmp	byte [OVERRIDE], 0     ; if2
	jnz	GETOVERRIDE_if2else

	mov	al, byte [number]
	mov	byte [OVERRIDE], al

	stc

	jmp	GETOVERRIDE_if2end

GETOVERRIDE_if2else:
	push	aAsmDoubleSegme ; "ASM: Double segment override"
	call	preerror

GETOVERRIDE_if2end:
	call	NextTok

	call	NextTok

GETOVERRIDE_end:
	ret
   endp
; ---------------------------------------------------------------------------
; Вычисление размера операнда: _B,_W,_D,WB,_DW & RAND-FLAG. AL=SIZE
; ---------------------------------------------------------------------------
  proc	DEF_OPSIZE
	mov	ah, byte [OPERANDSIZE]

	cmp	ah, 255 	; if1
	jnz	DEF_OPSIZE_if1end

	mov	byte [OPERANDSIZE], al ; Один операнд

	ret

DEF_OPSIZE_if1end:			; CODE XREF: DEF_OPSIZE+9j
	cmp	ah, al		; if2 (Размеры совпадают)
	jnz	DEF_OPSIZE_if2end

	ret

DEF_OPSIZE_if2end:
	cmp	ax, 100h	; if3 RW,RB ?
	jnz	DEF_OPSIZE_if3end

	mov	byte [RANDFLAG], 1     ; OPERANDSIZE=WB;

	ret

DEF_OPSIZE_if3end:
	cmp	ax, 200h	; if4 RD,RB ?
	jnz	DEF_OPSIZE_if4end

	cmp	[ACTUALMNEMDESC], PCOMMANDS3_ ; if5 comparation with offset
	jnz	DEF_OPSIZE_if5end

	ret

DEF_OPSIZE_if5end:
	mov	byte [OPERANDSIZE], WB
	ret

DEF_OPSIZE_if4end:
	cmp	ax, 201h	; if6 RD,RW
	jnz	DEF_OPSIZE_if6end

	mov	byte [RANDFLAG], 1

	mov	byte [OPERANDSIZE], _DW

DEF_OPSIZE_if6end:
	ret
  endp
; ---------------------------------------------------------------------------
; Запись префикса адресации
; ---------------------------------------------------------------------------
  proc	WRITEADDR
	push	eax

	cmp	byte [ADDRFLAG], 1     ; if1
	jnz	WRITEADDR_if1end

	mov	al, 67h

	call	OP

WRITEADDR_if1end:
	pop	eax

	ret
  endp
; ---------------------------------------------------------------------------
; Определение размерности константы
; ---------------------------------------------------------------------------
  proc	DefConstSize
	mov	ebx, dword [opDescInd]

	mov	dl, _D

	cmp	byte [OPPOST + ebx], 0	; if1
	jnz	DefConstSize_W2

	mov	ebx, dword [opDescInd]
	shl	ebx, 2

	mov	eax, dword [OPCONST + ebx]

	mov	dl, _B		; byte

	cmp	eax, -128	; if2
	jl	DefConstSize_if2end

	cmp	eax, 0FFh
	jle	DefConstSize_W2

DefConstSize_if2end:
	inc	dl		; _W - word

	cmp	eax, -32768	; if3
	jl	DefConstSize_if3end

	cmp	eax, 0FFFFh
	jle	DefConstSize_W2

DefConstSize_if3end:
	inc	dl		; _D - dword

DefConstSize_W2:
	mov	ebx, dword [opDescInd]

	mov	byte [OPCONSTSIZE + ebx], dl

	mov	byte [OPCONSTFLAG + ebx], 1

	ret
  endp
; ---------------------------------------------------------------------------
; Обработка мнемоники ассемблера
; ---------------------------------------------------------------------------
  proc	DoMnemonics
	mov	dword [opDescInd], 0
	mov	dword [EADescInd], 0

	call	INIT_LINE	; Очистка буферов

	cmp	[number], 24	  ; if1
	jnb	DoMnemonics_if1else

	cmp	[number], 8	  ; if2
	jnb	DoMnemonics_if2else

	mov	ebx, PCOMMANDS1 ; #PCOMMANDS1

	jmp	DoMnemonics_if2end

DoMnemonics_if2else:
	cmp	[number], 12	  ; if3
	jnb	DoMnemonics_if3else

	sub	[number], 8

	mov	ebx, PCOMMANDS2

	jmp	DoMnemonics_if2end

DoMnemonics_if3else:
	cmp	[number], 20	  ; if4
	jnb	DoMnemonics_if4else

	sub	[number], 12

	mov	ebx, PCOMMANDS3

	jmp	DoMnemonics_if2end

DoMnemonics_if4else:
	sub	[number], 20

	mov	ebx, PCOMMANDS4

DoMnemonics_if2end:
	movzx	eax, byte [ebx]
	add	[number], eax

	mov	al, byte [number]
	mov	byte [PFLAG], al

	inc	ebx

	jmp	DoMnemonics_if1end

DoMnemonics_if1else:
	sub	[number], 24

	mov	ebx, [number]
	shl	ebx, 2

	mov	ebx, [TAB_MNEMONICS + ebx]

	cmp	ebx, T_DAA ; if5 Проверим на наличие инструкции без операндов
	jb	DoMnemonics_if1end

	cmp	ebx, T_NOT
	jnb	DoMnemonics_if1end

	mov	[ACTUALMNEMDESC], ebx

	cmp	[tok2], tk_semicolon ; if6
	jnz	DoMnemonics_if6end

	call	NextTok

DoMnemonics_if6end:
	mov	esi, [ACTUALMNEMDESC]

	jmp	CreateCode	; Генерация кода для инструкции без операндов

DoMnemonics_if1end:
	mov	[ACTUALMNEMDESC], ebx ; Запомним указатель на текущий шаблон мнемоники

DoMnemonics_loop1start:
	mov	eax, [tok]	  ; Цикл анализа операндов
	mov	[prevTok], eax

	call	NextTok 	; Следующий операнд

	push	St_Sizes ; table
	push	string	 ; probe
	call	FastSearch	; Это размер операнда?

	jnb	DoMnemonics_if7end ; if7

	mov	[OPERANDSIZE], al ; Запомним _B,_W,_D

	jmp	DoMnemonics_loop1continue

DoMnemonics_if7end:
	call	GETOVERRIDE	; Обработка конструкции SEG:

DoMnemonics_ContLine:
	cmp	[tok], tk_eof	  ; Точка для продолжения обработки текущего
	jz	DoMnemonics_if8start ; if8

	cmp	[tok], tk_semicolon
	jnz	DoMnemonics_if8else

DoMnemonics_if8start:
	mov	ebx, [opDescInd]

	cmp	byte [OPDESC + ebx], E	; if9 Обработка в EA?
	jnz	DoMnemonics_if9else

	call	DefConstSize

	call	GETMODRMBYTE	; EOL - GENERATE MODRM OPCODE BYTE.

DoMnemonics_if9else:			; CODE XREF: DoMnemonics+117j
	cmp	[prevTok], tk_mnemonics ; if10 Были операнды
	jz	DoMnemonics_if10end

	inc	[OPLENGTH]

	cmp	[OPERANDSIZE], 255 ; if11
	jnz	DoMnemonics_if10end

	mov	[OPERANDSIZE], _D ; Принудительно установим dword

DoMnemonics_if10end:
	jmp	MapOperands

	jmp	DoMnemonics_loop1continue

DoMnemonics_if8else:
	cmp	[tok], tk_comma   ; if12
	jnz	DoMnemonics_if12else

	cmp	[opDescInd], 3	  ; if13
	jnz	DoMnemonics_if13end

	call	toomuchoperands

	jmp	DoMnemonics_loop1end

DoMnemonics_if13end:
	mov	ebx, [opDescInd]

	cmp	byte [OPDESC + ebx], E	; if14 Обработка в EA?
	jnz	DoMnemonics_if14end

	call	DefConstSize

	call	GETMODRMBYTE	; EOL - GENERATE MODRM OPCODE BYTE.

DoMnemonics_if14end:
	inc	[opDescInd]

	inc	[OPLENGTH]

	jmp	DoMnemonics_loop1continue

DoMnemonics_if12else:
	cmp	[tok], tk_openblock ; if15
	jnz	DoMnemonics_if15else

	mov	ebx, [opDescInd]

	mov	byte [OPDESC + ebx], E	; Отметим, что работаем с EA операндом

	jmp	DoMnemonics_loop1continue

DoMnemonics_if15else:
	cmp	[tok], tk_closeblock ; if16
	jnz	DoMnemonics_if16else

	call	DefConstSize

	call	GETMODRMBYTE	; EOL - GENERATE MODRM OPCODE BYTE.

	jmp	DoMnemonics_loop1continue

DoMnemonics_if16else:
	cmp	[tok], tk_minus   ; if17
	jnz	DoMnemonics_if17else

	cmp	[tok2], tk_number ; if18
	jnz	DoMnemonics_if18end

	call	NextTok

	mov	eax, [number]
	neg	eax
	mov	[number], eax

	jmp	DoMnemonics_ContLine ; Продолжим docase без выборки след.token

DoMnemonics_if18end:
	jmp	DoMnemonics_loop1continue

DoMnemonics_if17else:
	cmp	[tok], tk_plus	  ; if19
	jz	DoMnemonics_loop1continue

	cmp	[tok], tk_mult	  ; if20
	jnz	DoMnemonics_if20else

	call	GETSCALING	; Указан масшаб в Sib

	jmp	DoMnemonics_loop1continue

DoMnemonics_if20else:
	cmp	[tok], tk_reg	  ; if21
	jnz	DoMnemonics_if21else

DoMnemonics_G0:
	mov	ebx, [opDescInd]

	cmp	byte [OPDESC + ebx], E	; if22 Обработка в EA?
	jnz	DoMnemonics_if22else

	cmp	[type], tk_byte   ; if23
	jnz	DoMnemonics_if23else

	push	DoMnemonics_str1 ; "ASM: No byte register in address\r\n"
	call	preerror

	ret

DoMnemonics_if23else:
	cmp	[EALENGTH], 2	  ; if24 Количество регистров в EA < 2 ?
	jnb	DoMnemonics_if24else

	inc	[EALENGTH]	  ; Отметим, что есть еще один регистр в EA

	mov	ebx, [EADescInd]

	mov	al, byte [number]

	mov	byte [EADESC + ebx], al ; Запомним  регистра

	inc	[EADescInd]

	jmp	DoMnemonics_if23end

DoMnemonics_if24else:	; Слишком много регистров в EA
	push	DoMnemonics_str2 ; "ASM: too much registers in combination\r"...
	call	preerror

	ret

DoMnemonics_if23end:
	jmp	DoMnemonics_if22end

DoMnemonics_if22else:
	mov	al, byte [number]
	mov	byte [OPDATA + ebx], al ;  регистра

	mov	byte [OPDESC + ebx], R	; const

	mov	ah, byte [number]
	and	ah, 7

	mov	byte [EA_R], ah

	cmp	[opDescInd], 2	  ; if25
	jz	DoMnemonics_if22end

	shr	al, 3

	call	DEF_OPSIZE

DoMnemonics_if22end:
	jmp	DoMnemonics_loop1continue

DoMnemonics_if21else:
	cmp	[tok], tk_number  ; if26 Обработка константы
	jnz	DoMnemonics_if26else

	cmp	[tok2], tk_mult   ; if27
	jnz	DoMnemonics_NUM ; выход из условия, а там метка num

	call	GETSCALING_DoScale

	call	NextTok

	jmp	DoMnemonics_loop1continue

DoMnemonics_NUM:
	mov	ebx, [opDescInd]
	shl	ebx, 2

	mov	eax, [number]
	add	dword [OPCONST + ebx], eax ; Запомним константу

	call	DefConstSize	; Определим размерность константы

	cmp	byte [OPDESC + ebx], E	; if28 Константа в EA?
	jz	DoMnemonics_if28end

	mov	byte [OPDESC + ebx], CO ; const

DoMnemonics_if28end:
	jmp	DoMnemonics_loop1continue

DoMnemonics_if26else:
	cmp	[tok], tk_postnumber ; if29
	jnz	DoMnemonics_if29else

	mov	ebx, [opDescInd]

	mov	[OPPOST + ebx], POST_DATA

	shl	ebx, 2

	mov	eax, [treeptr]
	mov	dword [OPPOSTREF + ebx], eax

	mov	esi, [treeptr]

	inc	dword [esi+recpost]

	jmp	DoMnemonics_NUM

	jmp	DoMnemonics_loop1continue

DoMnemonics_if29else:
	cmp	[tok], tk_proc	  ; if30
	jnz	DoMnemonics_if30else

	cmp	[post], 0	  ; if31
	jz	DoMnemonics_if31end

	mov	ebx, [opDescInd]

	mov	byte [OPPOST + ebx], POST_CALL

	shl	ebx, 2

	mov	eax, [treeptr]
	mov	dword [OPPOSTREF + ebx], eax

	mov	esi, [treeptr]

	inc	dword [esi+recpost]

DoMnemonics_if31end:
	jmp	DoMnemonics_NUM

	jmp	DoMnemonics_loop1continue

DoMnemonics_if30else:
	cmp	[tok], tk_locallabel ; if32
	jnz	DoMnemonics_if32else

	mov	ebx, [opDescInd]
	shl	ebx, 2

	mov	eax, [localptr]
	mov	dword [OPPOSTREF + ebx], eax

DoMnemonics_I2:
	mov	ebx, [opDescInd]

	cmp	[ACTUALMNEMDESC], T_JCXZ ; сравнение с адресом
	jb	DoMnemonics_I1
	cmp	[ACTUALMNEMDESC], T_CALLFAR ; offset T_CALLFAR
	ja	DoMnemonics_I1
	mov	byte [OPPOST + ebx], POST_CALL
	jmp	DoMnemonics_NUM

DoMnemonics_I1:
	mov	byte [OPPOST + ebx], POST_DATA

	mov	al, _D

	call	DEF_OPSIZE

	jmp	DoMnemonics_PARSE_EA1

	jmp	DoMnemonics_loop1continue

DoMnemonics_if32else:
	cmp	[tok], tk_undefproc ; if33
	jnz	DoMnemonics_if33else

DoMnemonics_I0:
	mov	ebx, [opDescInd]
	shl	ebx, 2

	mov	eax, [treeptr]
	mov	dword [OPPOSTREF + ebx], eax

	jmp	DoMnemonics_I2

	jmp	DoMnemonics_loop1continue

DoMnemonics_if33else:
	cmp	[tok], tk_id	  ; if34
	jnz	DoMnemonics_if34else

	mov	[tok], tk_undefproc

	mov	[post], 1

	mov	[number], 0

	push	string	 ; keystring
	call	AddToTree

	jmp	DoMnemonics_I0

	jmp	DoMnemonics_loop1continue

DoMnemonics_if34else:
	cmp	[tok], tk_var	  ; if35 Инициализация EA с константой: EA+disp
	jnz	DoMnemonics_if35else

	mov	al, byte [type]
	sub	al, tk_byte
	shr	al, 1		; AL=размер адресуемого операнда (_B,_W,_D)

	call	DEF_OPSIZE

	mov	ebx, [opDescInd]

	cmp	[post], 0	  ; if36
	jz	DoMnemonics_PARSE_EA1

	shl	ebx, 2

	mov	eax, [treeptr]
	mov	dword [OPPOSTREF + ebx], eax

	mov	ebx, [opDescInd]
	mov	byte [OPPOST + ebx], POST_DATA

	mov	esi, [treeptr]
	inc	dword [esi+recpost]

DoMnemonics_PARSE_EA1:
	mov	byte [OPDESC + ebx], E

	mov	byte [OPCONSTFLAG + ebx], 1 ; Отметим, что работаем с EA операндом

	shl	ebx, 2

	mov	eax, [number]
	add	dword [OPCONST + ebx], eax ; Запомним адрес

	jmp	DoMnemonics_loop1continue

DoMnemonics_if35else:
	cmp	[tok], tk_seg	  ; if36
	jnz	DoMnemonics_if36else

	mov	ebx, [opDescInd]

	mov	al, byte [number]
	sub	al, regES
	mov	byte [OPDATA + ebx], al ; сегментного регистра

	mov	[SEGREGISTER], al

	shl	al, 3		; Создать код для XSM поля
	mov	byte [EA_S], al

	mov	byte [OPDESC + ebx], _SR

	mov	al, _W

	call	DEF_OPSIZE

	jmp	DoMnemonics_loop1continue

DoMnemonics_if36else:
	cmp	[tok], tk_param   ; if37
	jz	DoMnemonics_PARSE_PAR

	cmp	[tok], tk_local
	jnz	DoMnemonics_if37else

DoMnemonics_PARSE_PAR:
	mov	al, byte [type]
	sub	al, tk_byte
	shr	al, 1

	call	DEF_OPSIZE

	mov	ebx, [opDescInd]

	mov	byte [OPDESC + ebx], E

	shl	ebx, 2		; Отметим, что работаем с EA операндом

	mov	eax, [number]
	add	dword [OPCONST + ebx], eax ; Запомним адрес

	mov	byte [OPCONSTFLAG + ebx], 1

	mov	[number], regEBP

	jmp	DoMnemonics_G0

	jmp	DoMnemonics_loop1continue

DoMnemonics_if37else:
	cmp	[tok], tk_controlreg ; if38
	jnz	DoMnemonics_if38else

	mov	ebx, [opDescInd]

	mov	byte [OPDESC + ebx], SYSR

	mov	al, byte [number]
	mov	[SYSRNUM], al

	cmp	al, 4		; if39
	jnz	DoMnemonics_if39else

	mov	[SYSRTYPE], _CR4

	jmp	DoMnemonics_if39end

DoMnemonics_if39else:
	mov	[SYSRTYPE], _CR

	mov	[SYSRCODE], 0

DoMnemonics_if39end:
	jmp	DoMnemonics_loop1continue

DoMnemonics_if38else:
	cmp	[tok], tk_debugreg ; if40
	jnz	DoMnemonics_if40else

	mov	ebx, [opDescInd]

	mov	byte [OPDESC + ebx], SYSR

	mov	al, byte [number]
	mov	[SYSRNUM], al

	mov	[SYSRTYPE], _DR

	mov	[SYSRCODE], 1

	jmp	DoMnemonics_loop1continue

DoMnemonics_if40else:
	cmp	[tok], tk_testreg ; if41
	jnz	DoMnemonics_if41else

	mov	ebx, [opDescInd]
	mov	byte [OPDESC + ebx], SYSR

	mov	al, byte [number]
	mov	[SYSRNUM], al

	mov	[SYSRTYPE], _TR

	mov	[SYSRCODE], 4

	jmp	DoMnemonics_loop1continue

DoMnemonics_if41else:
	push	DoMnemonics_str3 ; "ASM: Syntax error\r\n"
	call	preerror

DoMnemonics_loop1continue:
	jmp	DoMnemonics_loop1start

DoMnemonics_loop1end:
	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	CreateScale
	cmp	byte [ADDRFLAG], 0	  ; if1
	jz	CreateScale_if1end

	ret

CreateScale_if1end:
	cmp	byte [EA_SIBFLAG], 0   ; if2 Флаг наличия Sib в 32-битном EA
	jz	CreateScale_if2end

	cmp	dword [EA_SCALE], 0	  ; if3 Отсутствует
	jnz	CreateScale_if3else

	mov	byte [EA_SCALING], 0

	jmp	CreateScale_if2end

CreateScale_if3else:
	cmp	dword [EA_SCALE], 1	  ; if4
	jnz	CreateScale_if4else

	mov	byte [EA_SCALING], 0

	jmp	CreateScale_if2end

CreateScale_if4else:
	cmp	dword [EA_SCALE], 2	  ; if5
	jnz	CreateScale_if5else

	mov	byte [EA_SCALING], 40h

	jmp	CreateScale_if2end

CreateScale_if5else:
	cmp	dword [EA_SCALE], 4	  ; if6
	jnz	CreateScale_if6else

	mov	byte [EA_SCALING], 80h

	jmp	CreateScale_if2end

CreateScale_if6else:
	cmp	dword [EA_SCALE], 8	  ; if7
	jnz	CreateScale_if7else

	mov	byte [EA_SCALING], 0C0h

	jmp	CreateScale_if2end

CreateScale_if7else:
	mov	byte [EA_SCALING], 0

	cmp	dword [EA_SCALE], 255	; if8
	jbe	CreateScale_if8else

	mov	al, 69h
	call	OP

	jmp	CreateScale_if8end

CreateScale_if8else:
	mov	al, 6Bh

	call	OP

CreateScale_if8end:
	mov	al, byte [EA_I]
	shl	al, 3
	or	al, byte [EA_I]
	or	al, 0C0h
	call	OP

	cmp	dword [EA_SCALE], 255	; if9
	jbe	CreateScale_if9else

	mov	eax, dword [EA_SCALE]

	call	OUTDWORD

	jmp	CreateScale_if2end

CreateScale_if9else:
	mov	al, byte [EA_SCALE]
	call	OP

CreateScale_if2end:
	ret
  endp
; ---------------------------------------------------------------------------
; Генерация кода. ESI=ptr на запись о описание мнемоники (запись типа T_...)
; ---------------------------------------------------------------------------
  proc	CreateCode
	call	WRITEOVERRIDE

	call	CreateScale

	cmp	byte [ADDRFLAG], 1	  ; if1   ADDR: PREFIX ?
	jnz	CreateCode_if1end

	mov	al, 67h
	call	OP

CreateCode_if1end:
	cmp	byte [RANDFLAG], 1	  ; if2
	jnz	CreateCode_if2end

	mov	al, 66h
	call	OP

CreateCode_if2end:
	mov	edi, esi

	cmp	[ACTUALMNEMDESC], T_TEST ; if3
	jnz	CreateCode_if3end

	mov	byte [DBIT], 0	  ; DON'T ADD ANYTHING IF TESTING

CreateCode_if3end:
	shl	byte [DBIT], 1	  ; SHIFT D-BIT TO THE RIGHT POSITION.

CreateCode_NEXT_DESC_BYTE:	  ; Обработка байта из дескриптора мнемоники
	xor	ebx, ebx

	mov	bl, byte [edi]

	inc	edi

	cmp	bl, X7M 	; X7M
	ja	CreateCode_NC3

	cmp	bl, X0M 	; X0M
	jb	CreateCode_N24

	mov	ah, bl
	sub	ah, X0M 	; X0M

	mov	al, byte [EA_X]
	shl	al, 3
	or	al, ah
	shl	al, 3
	or	al, byte [EA_M]

	call	OP

	jmp	CreateCode_NEXT_DESC_BYTE

CreateCode_N24:
	shl	ebx, 2

	add	ebx, Dsc_Jump

	jmp	dword [ebx]

CreateCode_NC3:
	cmp	bl, _END	; _END
	jnz	CreateCode_E42

	jmp	CreateConstants ; Конец шаблона для мнемоники

CreateCode_E42:
	push	CreateCode_str1 ; "Descriptor damaged\r\n"
	call	preerror

	ret

; OpCode - 1 байт
CreateCode_Dsc_O:
	mov	al, byte [edi]
	add	al, byte [WBIT]
	add	al, byte [DBIT]

	inc	edi

	call	OP

	jmp	CreateCode_NEXT_DESC_BYTE

; OpCode - 1 слово
CreateCode_Dsc_OW:
	mov	al, byte [edi]

	inc	edi

	call	OP

	mov	al, byte [edi]
	add	al, byte [WBIT]
	add	al, byte [DBIT]

	inc	edi

	call	OP

	jmp	CreateCode_NEXT_DESC_BYTE

; OpCode - 1 байт и следующий байт, заданный 8-ричной строкой
CreateCode_Dsc_OS:
	mov	al, byte [edi]

	inc	edi

	call	OP

; OpCode - 8-ричная строка с кодом
CreateCode_Dsc_S:
	mov	cl, 3

	xor	eax, eax

CreateCode_loop1statrt:
	mov	al, [edi]

	inc	edi

	cmp	al, 'X' 	; if4
	jnz	CreateCode_if4else

	mov	al, byte [EA_X]

	jmp	CreateCode_if4end

CreateCode_if4else:
	cmp	al, 'R' 	; if5
	jnz	CreateCode_if5else

	mov	al, byte [EA_R]

	jmp	CreateCode_if4end

CreateCode_if5else:
	cmp	al, 'M' 	; if6
	jnz	CreateCode_if6else

	mov	al, byte [EA_M]

	jmp	CreateCode_if4end

CreateCode_if6else:
	cmp	al, 'S' 	; if7
	jnz	CreateCode_if7else

	mov	al, byte [SEGREGISTER]

	jmp	CreateCode_if4end

CreateCode_if7else:
	cmp	al, 'N' 	; if8
	jnz	CreateCode_if8else

	mov	al, byte [SYSRNUM]

	jmp	CreateCode_if4end

CreateCode_if8else:
	cmp	al, 'P' 	; if9
	jnz	CreateCode_if9else

	mov	al, byte [PFLAG]

	jmp	CreateCode_if4end

CreateCode_if9else:
	sub	al, '0'

CreateCode_if4end:
	shl	ah, 3
	or	ah, al

	dec	cl
	jnz	CreateCode_loop1statrt ; loop condition

	mov	al, ah
	add	al, byte [DBIT]
	add	al, byte [WBIT]

CreateCode_N15:
	call	OP

	jmp	CreateCode_NEXT_DESC_BYTE

; OpCode - ModRM байт
CreateCode_Dsc_XRM:
	mov	al, byte [EA_X]
	shl	al, 3
	or	al, byte [EA_R]
	shl	al, 3
	or	al, byte [EA_M]        ; Получить регистр из описания мнемоники

	jmp	CreateCode_N15

; OpCode - ModRM с P-флагом (арифметические инструкции)
CreateCode_Dsc_XPM:
	mov	al, byte [EA_X]
	shl	al, 3
	or	al, byte [PFLAG]
	shl	al, 3
	or	al, byte [EA_M]

	jmp	CreateCode_N15
; OpCode - ModRM с сегментным регистром
CreateCode_Dsc_XSM:
	mov	al, byte [EA_X]
	shl	al, 3
	or	al, byte [SEGREGISTER]
	shl	al, 3
	or	al, byte [EA_M]

	jmp	CreateCode_N15

	ret
  endp
; ---------------------------------------------------------------------------
; Разбор закончен -> на генерацию кода по шаблону
; ---------------------------------------------------------------------------
  proc	MapOperands
	mov	[opDescInd], 0

	mov	esi, [ACTUALMNEMDESC] ; Указатель на информацию по генерации

	mov	al, byte [OPLENGTH]    ; Количество операндов

	mov	ecx, OPDESC ; Указатель на информацию об операндах
	mov	ecx, dword [ecx] ; нуликовый элемент 

	cmp	esi, T_MOV ; if1 Инструкция MOV?
	jz	MapOperands_if1end

	test	al, al		; if2
	jz	MapOperands_loop1start

	cmp	al, 1		; if3
	jz	MapOperands_ONEOP ; Инструкция с одним операндом
	cmp	al, 2		; if4
	jz	MapOperands_TWOOPS ; Инструкция с двумя операндами
	cmp	al, 3		; if5
	jz	MapOperands_THREEOPS ; Инструкция с тремя операндами

	call	toomuchoperands

	ret

; Инструкция без операндов
MapOperands_loop1start:
	lodsb

	test	al, al		; if6
	jz	CreateCode	; Генерация кода

	cmp	al, _END
	jnz	MapOperands_loop1start ; loop condition

	push	MapOperands_str1 ; "ASM: Operand required\r\n"
	call	preerror

	ret
; Генерация MOV инструкции
MapOperands_if1end:
	push	eax
	push	ecx

	call	WRITEOVERRIDE

	call	CreateScale

	pop	ecx
	pop	eax

	cmp	al, 2		; if7 2 OPERANDS IN INSTRUCTION?
	jz	MapOperands_L1

	push	MapOperands_str2 ; "ASM: Two operands required\r\n"
	call	preerror

	ret

MapOperands_L2:
	push	MapOperands_str3 ; "ASM: Not same size\r\n"
	call	preerror

	ret

MapOperands_L1:
	mov	bl, 0

	mov	al, byte [OPERANDSIZE]

	cmp	al, _D		; if8
	jz	MapOperands_if8end

	cmp	al, _W
	ja	MapOperands_L2
	jnz	MapOperands_N4

	mov	byte [RANDFLAG], 1

MapOperands_if8end:
	mov	bl, 1

MapOperands_N4:
	mov	byte [WBIT], bl      ; STORE W-BIT

	mov	dl, 0	     ; CLEAR D-BIT

	call	WRITEADDR

	xor	ebx, ebx

	mov	bl, cl

	shl	ebx, 2
	add	ebx, Jmp_Mov

	jmp	dword [ebx]

MapOperands_Mov_ERR:
	push	MapOperands_str4 ; "ASM: a constant can't be used as a dest"...
	call	preerror

	ret

MapOperands_Mov_R:
	xor	ebx, ebx

	mov	bl, ch

	shl	ebx, 2
	add	ebx, Jmp_Mov_R

	jmp	dword [ebx]

MapOperands_Mov_E:
	xor	ebx, ebx

	mov	bl, ch

	shl	ebx, 2
	add	ebx, Jmp_Mov_E

	jmp	dword [ebx]

MapOperands_Mov_R2R:
	call	WRITERAND

	mov	al, byte [OPDATA + 1]	; OPDATA[1]
	mov	ah, byte [OPDATA + 0]	; OPDATA[0]

	mov	dl, byte [DBIT]

	push	eax

	mov	al, 210o
	add	al, byte [WBIT]

	shl	dl, 1

	add	al, dl		; D-BIT

	call	OP

	pop	eax

	and	al, 7
	shl	al, 3
	and	ah, 7
	or	al, ah
	or	al, 300o

	call	OP

	jmp	CreateConstants

MapOperands_Mov_R2E:
	mov	al, byte [OPDATA + 0]
	and	al, 7		; AL/AX/EAX ?

	or	al, al
	jnz	MapOperands_N1

	cmp	byte [EA_M], 6	  ; AR,[DW] POSSIBLE?
	jnz	MapOperands_N1 ; NO, ONLY AR,[EA]

	cmp	byte [ADDRFLAG], 0     ; 32BIT-EA ?
	jz	MapOperands_N1 ; NO, TRY ANOTHER...

	call	WRITERAND

	mov	al, 240o
	add	al, byte [WBIT]        ; INSTRUCTION FOUND.

	call	OP

	jmp	CreateConstants

MapOperands_Mov_E2R:
	mov	al, byte [OPDATA + 1]	 ; OPDATA[1]
	and	al, 7		; [DW],AR POSSIBLE?

	or	al, al
	jnz	MapOperands_Y1

	cmp	byte [ADDRFLAG], 0	  ; 32BIT EA ?
	jnz	MapOperands_Y1 ; YES, RAVE ON...

	cmp	byte [EA_M], 6
	jnz	MapOperands_Y1

	call	WRITERAND

	mov	al, 242o
	add	al, byte [WBIT]   ; INSTRUCTION FOUND.

	call	OP

	jmp	CreateConstants

MapOperands_N1:
	mov	dl, 2		; SET D-BIT

MapOperands_Y1:
	add	dl, 210o

	call	WRITERAND

	mov	al, dl

	mov	dl, 0

	add	al, byte [WBIT]

	call	OP

	mov	al, byte [EA_X]
	shl	al, 3
	or	al, byte [EA_R]
	shl	al, 3
	or	al, byte [EA_M]

	call	OP

	jmp	CreateConstants

MapOperands_E1:
	push	MapOperands_str3 ; "ASM: Not same size\r\n"
	call	preerror
	ret

MapOperands_Mov_E2C:
	mov	al, byte [OPERANDSIZE]
	mov	byte [OPCONSTSIZE + 1], al ; OPCONSTSIZE[1]

	cmp	al, _D
	ja	MapOperands_E1
	jz	MapOperands_X1

	cmp	al, _W
	jnz	MapOperands_X1

	mov	al, 66h

	call	OP

MapOperands_X1:
	mov	al, 306o
	add	al, byte [WBIT]

	call	OP

	mov	al, byte [EA_X]
	shl	al, 6
	or	al, byte [EA_M]

	call	OP

	jmp	CreateConstants

MapOperands_Mov_R2C:
	mov	al, byte [OPERANDSIZE]
	mov	byte [OPCONSTSIZE + 1], al ; OPCONSTSIZE[1]

	cmp	byte [OPERANDSIZE], _B
	jnz	MapOperands_N2

	mov	al, byte [OPDATA + 0]
	and	al, 7
	or	al, 260o

	call	OP

	jmp	CreateConstants

MapOperands_N2:
	cmp	byte [OPERANDSIZE], _D ; BYTE, WORD OR DWORD?
	ja	MapOperands_E1	; Not same size

	cmp	byte [OPERANDSIZE], _W ; if6
	jnz	MapOperands_if6end

	mov	al, 66h

	call	OP

MapOperands_if6end:
	mov	al, byte [OPDATA + 0]
	and	al, 7
	or	al, 270o

	call	OP

	jmp	CreateConstants

MapOperands_E21:
	push	MapOperands_str5 ; "ASM: Word required\r\n"
	call	preerror
	ret			 ; SEGMENT REGISTER IS ALWAYS WORD.

MapOperands_Mov_S:
	mov	al, 0

	cmp	cx, _SR*256+E	; mov EA,segreg
	jz	MapOperands_O1

	cmp	cx, _SR*256+R	; mov segreg,reg
	jz	MapOperands_O2

	cmp	cx, R*256+_SR	; mov reg,segreg
	jz	MapOperands_O3

	cmp	cx, E*256+_SR	; mov segreg,EA
	jnz	MapOperands_N12

	mov	al, 2		; SET D-BIT

MapOperands_O1: 			; CODE XREF: CODE:00407AC5j
	cmp	byte [OPERANDSIZE], _W
	jnz	MapOperands_E21

	add	al, 214o

	call	OP

	mov	al, byte [EA_X]
	shl	al, 6
	or	al, byte [EA_S]
	or	al, byte [EA_M]

	call	OP

	jmp	CreateConstants

MapOperands_O2:
	cmp	byte [OPERANDSIZE], _W
	jnz	MapOperands_E21

	mov	al, 214o

	call	OP

	mov	al, byte [EA_S]
	or	al, 300o
	or	al, byte [EA_R]        ; CREATE XSM BYTE

	call	OP

	stc

	ret

MapOperands_O3:
	cmp	byte [OPERANDSIZE], _W
	jnz	MapOperands_E21

	mov	al, 216o

	call	OP

	mov	al, byte [EA_S]
	or	al, 300o
	or	al, byte [EA_R]

	call	OP

	stc
	ret

MapOperands_E31:
	push	MapOperands_str6 ; "ASM: CR1 only readable\r\n"
	call	preerror

	ret

MapOperands_E32:
	push	MapOperands_str7 ; "ASM: SysR must be dword\r\n"
	call	preerror

	ret

Mov_SYSR:
MapOperands_N12:
	mov	ah, 40o

	cmp	cx, SYSR*256+R
	jz	MapOperands_O11

	cmp	cx, R*256+SYSR
	jnz	MapOperands_N22 ; ERROR: ILLEGAL OPERANDS

	mov	ah, 42o

	cmp	byte [SYSRTYPE], _CR   ; CR1 REGISTER USED?
	jnz	MapOperands_O11

	cmp	byte [SYSRNUM], 1
	jz	MapOperands_E31 ; YES, ONLY READ FROM IT.

MapOperands_O11:
	add	ah, byte [SYSRCODE]

	cmp	byte [OPERANDSIZE], _D ; SYSTEM REGISTERS ARE ALWAYS DWORD.
	jnz	MapOperands_E32

	mov	al, 17o

	call	OP

	cmp	byte [SYSRTYPE], _CR4  ; EXCEPTION: CR4
	jz	MapOperands_N22

	mov	al, ah

	call	OP

	mov	al, byte [SYSRNUM]
	shl	al, 3
	or	al, 300o
	or	al, byte [EA_R]        ; CREATE 3NR OPCODE

	call	OP

	stc

	ret

MapOperands_N22:
	cmp	cx, SYSR*256+R
	jnz	MapOperands_N32

	mov	al, 22h

	call	OP

	jmp	MapOperands_L11

MapOperands_N32:
	mov	al, 20h

	call	OP

MapOperands_L11:
	mov	al, 340o
	or	al, byte [EA_R]

	call	OP

	stc

	ret

; Инструкция с одним операндом
MapOperands_ONEOP:
	movzx	ebx, cl
	shl	ebx, 2
	add	ebx, Jmp_Op1

	jmp	dword [ebx]

Op1ERR:
	push	MapOperands_str10 ; "ASM: only use system registers within M"...
	call	preerror

	ret

MapOperands_L31:
	call	SCDEND		; RX

MapOperands_Op1R:
	call	GETOL1		; GET FIRST GENERATION INFO.

	cmp	al, SOR 	; SINGLE OPERAND/REGISTER ?
	jnz	MapOperands_X23

	jmp	CreateCode

MapOperands_X23:
	cmp	al, SOE 	; CONVERT REGISTER INTO EFFECTIVE ADDRESS?
	jnz	MapOperands_L31

	mov	byte [EA_X], 3	       ; метка C2

	mov	al, byte [EA_R]
	mov	byte [EA_M], al

	jmp	CreateCode

MapOperands_L41:
	call	SCDEND

MapOperands_Op1E:
	call	GETOL1

	cmp	al, SOE 	; SINGLE OPERAND/EFFECTIVE ADDRESS ?
	jnz	MapOperands_X24

	jmp	CreateCode

MapOperands_X24:
	cmp	al, SOM 	; SINGLE OPERAND/MEMORY POINTER ?
	jnz	MapOperands_L41

	cmp	byte [EA_X], 0
	jnz	MapOperands_L41

	cmp	byte [EA_M], 6	       ; [WORD CONSTANT]?
	jnz	MapOperands_L41

	jmp	CreateCode

MapOperands_L51:
	call	SCDEND

MapOperands_Op1C:
	call	GETOL1

	or	al, al		; JUMP INSTRUCTION?

	jnz	MapOperands_N13

	mov	ecx, dword [OPCONST + 0]    ; Здесь обработка call&jmp
	sub	ecx, [OptImageBase]
	sub	ecx, [OptBaseOfCode]
	sub	ecx, [outptr]
	add	ecx, [output]	; Расчет относительного смещения

	lodsb			; Получим или near

	cmp	al, _JB 	; short?
	jnz	MapOperands_N14

	dec	ecx		; -2
	dec	ecx

	cmp	ecx, 0FFFFFF80h
	jl	MapOperands_L10

	cmp	ecx, 7Fh
	jg	MapOperands_L10

	mov	byte [OPCONSTSIZE + 0], 0  ; OPCONSTSIZE[0]

MapOperands_H1:
	mov	dword [OPCONST + 0], ecx    ; OPCONST[0]

	jmp	CreateCode

MapOperands_N14:
	xor	eax, eax

	lodsb			; Поправка на размер call или jmp

	sub	ecx, eax
	mov	byte [OPCONSTSIZE + 0], _D  ; OPCONSTSIZE[0]

	jmp	MapOperands_H1

MapOperands_L10:
	cmp	[ACTUALMNEMDESC], T_JCXZ
	jb	MapOperands_L51

	cmp	[ACTUALMNEMDESC], T_LOOP
	ja	MapOperands_L51

	push	MapOperands_str8 ; "ASM: Jump range too long\r\n"
	call	preerror

	ret

MapOperands_N13:
	cmp	al, SO3 	; CONSTANT VALUE 3 ?
	jnz	MapOperands_N23

	cmp	dword [OPCONST + 0], 3	    ; OPCONST[0]
	jnz	MapOperands_L51

	mov	byte [OPCONSTFLAG + 0], 0  ; OPCONSTFLAG[0] YES, AVOID CONSTANT GENERATION.
	jmp	CreateCode

MapOperands_N23:
	cmp	al, SOC 	; SINGLE OPERAND/CONSTANT?
	jnz	MapOperands_X25

	mov	al, byte [OPERANDSIZE]
	mov	byte [OPCONSTSIZE + 0], al

	jmp	CreateCode

MapOperands_X25:
	cmp	al, SOO_CC	; SINGLE OPERAND/SIGNED BYTE ?
	jnz	MapOperands_L51

	cmp	byte [OPPOST + 0], 0	   ; OPPOST[0]
	jz	MapOperands_if9end

	jmp	MapOperands_L51

MapOperands_if9end:
	cmp	dword [OPCONST + 0], 7Fh
	jg	MapOperands_L51

	cmp	dword [OPCONST + 0], 0FFFFFF80h
	jl	MapOperands_L51

	mov	byte [OPCONSTSIZE + 0], _B ; OPCONSTSIZE[0]

	jmp	CreateCode

MapOperands_L61:
	call	SCDEND

MapOperands_Op1S:
	call	GETOL1

	cmp	al, SOS 	; SINGLE OPERAND/SEGMENT REGISTER?
	jnz	MapOperands_L61

	jmp	CreateCode

MapOperands_L71:
	call	SCDEND

MapOperands_Op1AF:
	call	GETOL1

	cmp	al, SOO_AF
	jnz	MapOperands_L71

	jmp	CreateCode

MapOperands_TWOOPS:
	movzx	ebx, cl 	; Инструкция с двумя операндами
	shl	ebx, 2
	add	ebx, Jmp_Op2

	jmp	dword [ebx] ; Переход по типу первого операнда

MapOperands_Op2ERRS:
	push	MapOperands_str9 ; "ASM: segment register can only be used "...
	call	preerror

	ret

MapOperands_Op2ERRSYS:
	push	MapOperands_str10 ; "ASM: only use system registers within M"...
	call	preerror

	ret

MapOperands_Op2ERRAF:
	push	MapOperands_str11 ; "Absolute FAR addresses can only be used"...
	call	preerror

	ret

MapOperands_Op2R:
	xor	ebx, ebx	; Первый операнд в 2-операндной инструкции - регистр
	mov	bl, ch
	shl	ebx, 2
	add	ebx, Jmp_Op2R

	jmp	dword [ebx] ; Переход по типу второго операнда

MapOperands_Op2E:
	xor	ebx, ebx	; Первый операнд в 2-операндной инструкции - EA
	mov	bl, ch
	shl	ebx, 2
	add	ebx, Jmp_Op2E 
; второй элемент массива равен нулю, потенциально может возникнуть AV
	jmp	dword [ebx] ; Переход по типу второго операнда

MapOperands_L81:
	call	SCDEND		; mnem EA,RX

MapOperands_OpE2R:
	call	GETOL2		; Обработка записи из шаблона

	cmp	al, DER 	; EA & R + D-BIT ?
	jz	MapOperands_C21

	cmp	al, ERO 	; 'ERO' ORDER ?
	jz	MapOperands_C21

	cmp	al, EAO 	; EA, ?
	jnz	MapOperands_L81 ; На пропуск записи в шаблоне

	cmp	byte [OPDATA + 1], regCL   ; OPDATA[1] CL REGISTER USED??
	jnz	MapOperands_L81 ; На пропуск записи в шаблоне

	cmp	byte [esi+1], regCL ; CL IN GENERATION INFO?
	jnz	MapOperands_L81 ; На пропуск записи в шаблоне

	inc	esi
	inc	esi

MapOperands_C21:
	jmp	CreateCode

MapOperands_L91:
	call	SCDEND

MapOperands_OpR2E:
	call	GETOL2

	cmp	al, DER 	; DER ?
	jnz	MapOperands_N43

	mov	byte [DBIT], 1	  ; (DIRECTION BIT)

	jmp	CreateCode

MapOperands_N43:
	cmp	al, REO 	; REO ?
	jnz	MapOperands_L91

	jmp	CreateCode

MapOperands_W2:
	inc	esi

	jmp	MapOperands_W1

MapOperands_LA1:
	call	SCDEND

MapOperands_OpR2R:
	cmp	byte [esi], _B+OL2+EAO ; байт+2оп+EAold?
	jz	MapOperands_W2 ; EAO FOUND, R+R COMBINATION NOT PERMITTED.

	call	GETOL2		; Обработка записи шаблона

	cmp	al, DER 	; EA,reg или reg,EA с D/W-битом?
	jnz	MapOperands_N53

MapOperands_LB2:
	mov	al, byte [OPDATA + 0]	   ; Преобразуем регистр в EA OPDATA[0]

	call	CREATE_EA

	mov	al, byte [OPDATA + 1]	 ; OPDATA[1] Второй операнд

	call	CREATE_R

	jmp	CreateCode

MapOperands_N53:
	cmp	al, ERO
	jz	MapOperands_LB2

	cmp	al, REO
	jnz	MapOperands_N63

	mov	al, byte [OPDATA + 1]	 ; RX,EP

	call	CREATE_EA

	mov	al, byte [OPDATA + 0]

	call	CREATE_R

	jmp	CreateCode

MapOperands_N63:
	cmp	al, EAO

	jnz	MapOperands_LA1 ; На пропуск записи в шаблоне

MapOperands_W1:
	mov	ecx, 2		; COMPARE 2 OPERANDS.

	mov	[opDescInd], 0

MapOperands_LX2:
	lodsb			; поле из записи шаблона

	cmp	al, 255 	; 1ST OPERAND OK.
	jz	MapOperands_L022

	cmp	al, AR		; AL/AX/EAX?
	jz	MapOperands_OL2X_AR

	cmp	al, regDX	  ; DX ?
	jz	MapOperands_OL2X_DX

	cmp	al, regCL
	jz	MapOperands_OL2X_CL

	jmp	MapOperands_LA1 ; На пропуск записи в шаблоне

MapOperands_OL2X_AR:
	mov	ebx, [opDescInd]

	mov	ah, byte [OPDATA + ebx]
	and	ah, 7		; Операнд из мнемоники

	jnz	MapOperands_LA1 ; На пропуск записи в шаблоне

MapOperands_L022:
	inc	[opDescInd]

	loopw	MapOperands_LX2

	jmp	MapOperands_L23

MapOperands_OL2X_DX:
	mov	ebx, [opDescInd]

	cmp	byte [OPDATA + ebx], 12o
	jnz	MapOperands_LA1 ; На пропуск записи в шаблоне

	inc	[opDescInd]

	loopw	MapOperands_LX2

	jmp	MapOperands_L23

MapOperands_OL2X_CL:
	mov	ebx, [opDescInd]

	cmp	byte [OPDATA + ebx], regCL ; CL
	jnz	MapOperands_LC1

	inc	[opDescInd]

	loopw	MapOperands_LX2

	test	byte [OPDATA + 0], 8+16    ; 1ST REGISTER WORD/DWORD?
	jz	MapOperands_L23

	mov	byte [WBIT], 1	       ; YES, SET W-BIT.

MapOperands_L23:
	mov	al, byte [OPDATA + 0]

	call	CREATE_EA

	jmp	CreateCode

MapOperands_LB1:
	call	SCDEND

MapOperands_OpE2C:
	call	GETOL2

	mov	[opDescInd], 1

	cmp	al, ECO
	jnz	MapOperands_N73

	mov	al, byte [OPERANDSIZE]
	mov	byte [OPCONSTSIZE + 1], al

	jmp	CreateCode

MapOperands_N73:
	cmp	al, EAO
	jnz	MapOperands_N83

	cmp	byte [OPERANDSIZE], _B
	jnz	MapOperands_N83

	cmp	byte [esi], _1
	jnz	MapOperands_N83

	cmp	dword [OPCONST + 4], 1
	jnz	MapOperands_N83

	mov	byte [OPCONSTFLAG + 1], 0

	inc	esi

	jmp	CreateCode

MapOperands_N83:
	cmp	al, ECCO	; EA/SIGNED BYTE ?
	jnz	MapOperands_LB1

	cmp	dword [OPCONST + 4], 0FFFFFF80h
	jl	MapOperands_LB1

	cmp	dword [OPCONST + 4], 7Fh
	jg	MapOperands_LB1

	mov	byte [OPERANDSIZE], _B ; OMIT A SINGLE BYTE ON GENERATION.

	jmp	CreateCode

MapOperands_LC1:
	call	SCDEND

MapOperands_OpR2C:
	call	GETOL2

	mov	[opDescInd], 1

	cmp	al, RCO
	jnz	MapOperands_Q1

MapOperands_A0:
	mov	al, byte [OPERANDSIZE]
	mov	byte [OPCONSTSIZE + 1], al

	jmp	CreateCode	; reg,const

MapOperands_Q1:
	cmp	al, ECO
	jnz	MapOperands_L110

MapOperands_A1:
	mov	al, byte [EA_R]
	call	CREATE_EA

	jmp	MapOperands_A0

MapOperands_L110:
	cmp	al, EAO
	jz	MapOperands_N93

	cmp	al, ECCO
	jnz	MapOperands_LC1 ; SIGNED BYTE CONST ?

	cmp	dword [OPCONST + 4], 0FFFFFF80h
	jl	MapOperands_LC1

	cmp	dword [OPCONST + 4], 7Fh
	jg	MapOperands_LC1

	mov	byte [OPERANDSIZE], _B

	mov	byte [OPCONSTSIZE + 1], _B

	jmp	MapOperands_A1 ; CONVERT REGISTER TO EFFECTIVE ADDRESS AND GENERATE OPCODE.

MapOperands_N93:
	mov	ecx, 2		; COMPARE 2 OPERAND.

MapOperands_B2:
	lodsb

	cmp	al, 255
	jz	MapOperands_L122

	cmp	al, AR
	jz	MapOperands_OL2_AR ; AL/AX/EAX?

	cmp	al, CO
	jz	MapOperands_OL2_CO ; CONSTANT?

	cmp	al, regDX
	jz	MapOperands_OL2_DX ; DX?

	cmp	al, _1
	jz	MapOperands_OL2_1 ; CONSTANT VALUE 1?

	jmp	MapOperands_LC1

MapOperands_OL2_AR:
	mov	ah, byte [OPDATA]
	and	ah, 7

	jnz	MapOperands_LC1

MapOperands_L122:
	loopw	MapOperands_B2

	jmp	CreateCode

MapOperands_OL2_CO:
	cmp	byte [OPDESC + 1], CO
	jnz	MapOperands_LC1

	mov	al, byte [OPERANDSIZE]
	mov	byte [OPCONSTSIZE + 1], al

	jmp	MapOperands_L122

MapOperands_OL2_DX:
	cmp	byte [OPDATA + 0], 12o
	jz	MapOperands_L122

	jmp	MapOperands_LC1

MapOperands_OL2_1:
	cmp	byte [OPCONSTSIZE + 1], _B
	jnz	MapOperands_LC1

	cmp	dword [OPCONST + 4], 1
	jnz	MapOperands_LC1

	mov	byte [OPCONSTFLAG + 1], 0
	jmp	MapOperands_A1

MapOperands_LD1:
	call	SCDEND

MapOperands_Op2C:
	call	GETOL2		; Первый операнд в 2-операндной инструкции - константа

	cmp	al, EAO
	jnz	MapOperands_LD1

	mov	ecx, 2		   ; COMPARE 2 OPERANDS.

	mov	[opDescInd], 0

MapOperands_B12:
	lodsb

	cmp	al, 255
	jz	MapOperands_L222

	cmp	al, AR		; AL/AX/EAX
	jz	MapOperands_XOL2_AR

	cmp	al, CO
	jz	MapOperands_XOL2_CO

	cmp	al, regDX
	jz	MapOperands_XOL2_DX

	cmp	al, _1
	jz	MapOperands_XOL2_1

	jmp	MapOperands_LD1

MapOperands_XOL2_AR:
	mov	ebx, [opDescInd]

	mov	ah, byte [OPDATA + ebx]
	and	ah, 7

	jnz	MapOperands_N21

MapOperands_L222:
	inc	[opDescInd]

	loopw	MapOperands_B12

	jmp	CreateCode

MapOperands_N21:
	jmp	MapOperands_LD1

MapOperands_XOL2_CO:
	mov	ebx, [opDescInd]

	cmp	byte [OPDESC + ebx], CO
	jnz	MapOperands_LD1

	inc	[opDescInd]

	loopw	MapOperands_B12

	jmp	CreateCode

MapOperands_XOL2_DX:
	mov	ebx, [opDescInd]

	cmp	byte [OPDATA + ebx], 12o
	jnz	MapOperands_LD1

	inc	[opDescInd]

	loopw	MapOperands_B12

	jmp	CreateCode

MapOperands_XOL2_1:
	mov	edx, [opDescInd]

	cmp	byte [OPCONSTSIZE + edx], _B
	jnz	MapOperands_LD1

	shl	edx, 2

	cmp	dword [OPCONST + edx], 1
	jnz	MapOperands_LD1

	mov	edx, [opDescInd]

	mov	byte [OPCONSTFLAG + edx], 0

	mov	al, byte [EA_R]

	call	CREATE_EA

	jmp	CreateCode

MapOperands_LE1:
	call	SCDEND

MapOperands_THREEOPS:
	call	GETOL3

	cmp	al, ERO
	jnz	MapOperands_N42

	cmp	cx, R*256+E
	jz	MapOperands_O21

	cmp	cx, R*256+R
	jnz	MapOperands_LE1

	mov	al, byte [OPDATA + 0]	   ; OPDATA[0]

	call	CREATE_EA

	mov	al, byte [OPDATA + 1]	 ; OPDATA[1]

	call	CREATE_R

	jmp	MapOperands_O21

MapOperands_N42:
	cmp	al, REO
	jnz	MapOperands_N52 ; ERROR: INVALID OPERANDS.

	cmp	cx, E*256+R
	jz	MapOperands_O21

	cmp	cx, R*256+R
	jnz	MapOperands_LE1

	mov	al, byte [OPDATA + 1]	 ; OPDATA[1]

	call	CREATE_EA

	mov	al, byte [OPDATA + 0]	   ; OPDATA[0]

	call	CREATE_R

MapOperands_O21:
	mov	bl, ah
	and	bl, 7

	cmp	byte [OPCONSTFLAG + 2], 1
	jnz	MapOperands_NA3

	cmp	bl, CC3
	jnz	MapOperands_N52

	cmp	dword [OPCONST + 8], 0FFFFFF80h
	jl	MapOperands_LE1

	cmp	dword [OPCONST + 8], 7Fh
	jg	MapOperands_LE1

	mov	byte [OPCONSTSIZE + 2], _B

	jmp	CreateCode

MapOperands_N52:
	cmp	bl, CB3
	jnz	MapOperands_N62

	cmp	dword [OPCONST + 8], 0FFh
	ja	MapOperands_LE1

	mov	byte [OPCONSTSIZE + 2], _B

	jmp	CreateCode

MapOperands_N62:
	cmp	bl, CW3
	jnz	MapOperands_NA3

	cmp	dword [OPCONST + 8], 0FFFFFFFFh
	ja	MapOperands_LE1

	cmp	byte [RANDFLAG], 1
	jnz	MapOperands_NA2

	mov	byte [OPCONSTSIZE + 2], _W

	jmp	CreateCode

MapOperands_NA2:
	mov	byte [OPCONSTSIZE + 2], 2

MapOperands_NA_2:
	jmp	CreateCode

MapOperands_NA3:
	cmp	bl, CL3
	jnz	MapOperands_LE1

	cmp	byte [OPDESC + 2], R
	jnz	MapOperands_LE1

	cmp	byte [OPDATA + 2], regCL
	jz	MapOperands_NA_2

	call	illegaloperand

	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	CreateConstants
	cmp	byte [EA_SIBFLAG], 1   ; Флаг наличия Sib в 32-битном EA
	jnz	CreateConstants_L3 ; Sib отсутствует

	cmp	byte [ADDRFLAG], 1
	jz	CreateConstants_L3 ; NO, NORMAL XRM

	mov	al, byte [EA_I]
	shl	al, 3
	or	al, byte [EA_R2]
	or	al, byte [EA_SCALING]
	call	OP

CreateConstants_L3:
	cmp	byte [OPCONSTFLAG], 1
	jnz	CreateConstants_N1

	cmp	byte [OPPOST + 0], 0	   ; if1
	jz	CreateConstants_if1end

	xor	eax, eax
	mov	al, byte [OPPOST + 0]
	push	eax
	push	dword [OPPOSTREF + 0]
	call	SetPost

CreateConstants_if1end:
	movzx	ecx, byte [OPCONSTSIZE + 0]
	mov	edi, OPCONST ; Значение disp в EA
	call	WRITECONST

CreateConstants_N1:
	cmp	byte [OPCONSTFLAG + 1], 1
	jnz	CreateConstants_N41

	movzx	ecx, byte [OPCONSTSIZE + 1]

	cmp	cl, AF		; ABSOLUTE FAR ?
	jnz	CreateConstants_D21

	mov	edi, OPCONST ; YES, CREATE ADDRESS.
	add	edi, 4

	mov	cl, _W		; (32 BIT)

	cmp	byte [ADDRFLAG], 1
	jnz	CreateConstants_D2

	mov	ecx, _D 	; (48 BIT)

CreateConstants_D2:
	call	WRITECONST

	add	edx, [output]

	mov	ebx, edx
	shr	ebx, 4

	and	edx, 15

	push	ebx
	push	edx
	pop	edx		; ???

	mov	eax, [opDescInd]
	mov	[eax], edx

	mov	ecx, _W
	; AFSEG нигде не определен, надо будет разбираться
	; EDI=#AFSEG; //(SEGMENT/SELECTOR)

	call	WRITECONST

	stc
	retn

CreateConstants_D21:
	cmp	byte [OPPOST + 1], 0	 ; if2
	jz	CreateConstants_if2end

	xor	eax, eax
	mov	al, byte [OPPOST + 1]
	push	eax
	push	dword [OPPOSTREF + 4]
	call	SetPost

CreateConstants_if2end:
	mov	edi, OPCONST
	add	edi, 4

	call	WRITECONST

CreateConstants_N41:
	cmp	byte [OPCONSTFLAG + 2], 1 ; if3
	jnz	CreateConstants_if3end

	movzx	ecx, byte [OPCONSTSIZE + 2]

	mov	edi, OPCONST
	add	edi, 8

	call	WRITECONST

CreateConstants_if3end:
	ret
  endp