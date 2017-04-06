;
; функции вызываемые только на этапе инициализации
;
; ---------------------------------------------------------------------------
; Выделение памяти для компиляции. При неудаче выход из компилятора
  proc	GetMem
	invoke	LocalAlloc, LPTR, MAXDATA
	mov	[output], eax

	test	eax, eax	; if1
	jnz	GetMem_if1end
	call	outofmemory2

GetMem_if1end:
	invoke	LocalUnlock, [output]

	mov	eax, [output]
	mov	[outptr], eax

	mov	eax, [output]
	mov	[startptr], eax

	invoke	LocalAlloc, LPTR, MAXPOSTS * 4
	mov	[postloc], eax

	test	eax, eax	; if2
	jnz	GetMem_if2end
	call	outofmemory2

GetMem_if2end:
	invoke	LocalAlloc, LPTR, MAXPOSTS * 4
	mov	[postnum], eax

	test	eax, eax	; if3
	jnz	GetMem_if3end
	call	outofmemory2

GetMem_if3end:
	invoke	LocalAlloc, LPTR, MAXPOSTS
	mov	[posttype], eax

	test	eax, eax	; if4
	jnz	GetMem_if4end
	call	outofmemory2

GetMem_if4end:
	invoke	LocalAlloc, LPTR, MAXDBGS * dbg_size
	mov	[dbginfo], eax

	test	eax, eax	; if5
	jnz	GetMem_if5end
	call	outofmemory2

GetMem_if5end:
	mov	eax, [dbginfo]
	mov	[dbgs], eax
	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	TokInit
	push	St_Mnemonics
	push	Mnemonics
	call	InitList

	push	St_Registers
	push	Registers
	call	InitList

	push	St_Directives
	push	Directives
	call	InitList

	push	St_Sizes
	push	Sizes
	call	InitList

	ret
  endp
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
  proc	InitList
  ptr_		  = -4
  keylist	  =  8
  table 	  =  0Ch

	enter	4, 0		; Инициализация списка

	invoke	LocalAlloc, LPTR, SORTSIZE * 256
	mov	[ebp+ptr_], eax

	test	eax, eax	; if1
	jnz	InitList_if1end
	call	outofmemory2

InitList_if1end:
	xchg	eax, edi
	mov	esi, [ebp+keylist]
	xor	ecx, ecx
	mov	bl, 0

InitList_loop1start:
	push	edi
	cmp	byte [esi], '-' ; if2
	jnz	InitList_if2break

	inc	esi
	inc	esi

	inc	bl

	pop	edi
	jmp	InitList_loop1start

InitList_if2break:
	mov	al, bl
	stosb			; Сохраним порядковый номер строки

InitList_loop2start:
	lodsb			; Скопируем строку в ptr_
	cmp	al, ' ' 	; if3
	jb	InitList_if3break
	stosb
	jmp	InitList_loop2start

InitList_if3break:
	cmp	al, _END	; if4
	jz	InitList_if4break
	stosb
	pop	edi
	inc	bl
	add	edi, SORTSIZE
	inc	ecx
	jmp	InitList_loop1start

InitList_if4break:
	mov	esi, [ebp+ptr_]
	push	ecx		; Копирование в ptr_ закончено. Сохраним кол-во строк
	dec	ecx
	jz	InitList_if5end ; if5 (Всего одна строка? - сортировка не нужна)

InitList_loop3start:
	push	esi
	push	ecx
	mov	edi, esi
	add	edi, SORTSIZE

InitList_loop4start:
	push	ecx
	push	esi
	push	edi

	xor	ecx, ecx
	mov	ecx, SORTSIZE

	inc	esi

	inc	edi

	repe	cmpsb

	pop	edi
	pop	esi

	jb	InitList_if6end ; if6 (проверит условие ????????????Сравнение двух строк)
	mov	eax, esi
	mov	edx, edi
	mov	edi, Buffer16

	xor	ecx, ecx
	mov	ecx, SORTSIZE/4      ; Нашли меньшую строку

	rep	movsd
	mov	esi, edx
	mov	edi, eax

	xor	ecx, ecx
	mov	ecx, SORTSIZE/4      ; -поменяем местами
	rep	movsd

	mov	esi, Buffer16
	mov	edi, edx

	xor	ecx, ecx
	mov	ecx, SORTSIZE/4
	rep	movsd

	mov	esi, eax
	mov	edi, edx

InitList_if6end:
	add	edi, SORTSIZE
	pop	ecx
	loop	InitList_loop4start
	pop	ecx
	pop	esi
	add	esi, SORTSIZE
	loop	InitList_loop3start

InitList_if5end:
	mov	edi, [ebp+table] ; Сортировка введенных строк в буфере ptr_ закончена

	xor	ecx, ecx
	mov	ecx, 26

	xor	eax, eax
	rep	stosd		 ; Обнуление table

	pop	ecx

	mov	esi, [ebp+ptr_] ;
	mov	edi, esi	;
	push	esi
	jmp	InitList_L42 ; Форм руем таблицу

InitList_loop5start:
	push	esi
	cmp	[esi+1], ah	; if7
	jz	short InitList_if7end
	dec	edi
	mov	al, _END
	stosb			; Отметим конец таблицы для данного символа в AH

InitList_L42:
	mov	ah, [esi+1]

	xor	ebx, ebx

	mov	bl, ah
	sub	bl, 'A'
	shl	ebx, 2
	add	ebx, [ebp+table] ; дальше  идет крах
	mov	[ebx], edi	 ; Запись указателя в table

InitList_if7end:
	movsb			; Запомним порядковый номер строки
	inc	esi		; Пропуск первого символа - он уже известен

InitList_loop6start:
	lodsb
	stosb
	test	al, al
	jnz	short InitList_loop6start ; Копируем строку в table
	pop	esi
	add	esi, SORTSIZE
	loop	InitList_loop5start
	dec	edi		; Продолжим для следующей строки
	mov	al, _END
	stosb			; table сформирована. Отметим конец

	leave
	ret    8
  endp
; ---------------------------------------------------------------------------