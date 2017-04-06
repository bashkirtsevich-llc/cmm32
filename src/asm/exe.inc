;
; функционал формирования выходного файла
; много чего из этого модуля можно выбросить, если делать чтобы выдавало чистый код
;
; ---------------------------------------------------------------------------
; Формирование IMPORT секции
; ---------------------------------------------------------------------------
if DEFINE_MAKE_EXE
  proc	DoLink
  i		  = -28h
  j		  = -24h
  DirTable	  = -20h
  AddrTable	  = -1Ch
  dllName	  = -18h
  apiName	  = -14h
  byName	  = -10h
  hold		  = -0Ch
  dll		  = -8
  api_		   = -4

	enter	28h, 0

	mov	eax, [outptr]
	sub	eax, [output]
	mov	[OptImportTableAdr], eax

	mov	eax, [outptr]
	mov	[ebp+DirTable], eax

	cmp	[APIcount], 0	  ; if1
	jnz	DoLink_if1else

	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptBaseOfCode]
	add	eax, 4
	call	OUTDWORD

	mov	eax, 80000001h
	call	OUTDWORD

	xor	eax, eax
	call	OUTDWORD

	mov	eax, [ebp+DirTable]
	sub	eax, [output]
	add	eax, 40
	add	eax, [OptBaseOfCode]
	call	OUTDWORD

	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptBaseOfCode]
	sub	eax, 12
	call	OUTDWORD

	mov	eax, [outptr]
	mov	[ebp+DirTable], eax
	mov	eax, [outptr]
	add	eax, 20
	mov	[outptr], eax

	; kernel32.dll
	mov	eax, 'kern'
	call	OUTDWORD

	mov	eax, 'el32'
	call	OUTDWORD

	mov	eax, '.dll'
	call	OUTDWORD

	mov	eax, [outptr]
	mov	[ebp+apiName], eax

	jmp	DoLink_if1end

DoLink_if1else:
	mov	eax, [DLLcount]
	inc	eax
	imul	eax, 20

	add	[outptr], eax	  ; на начало LookupTables
	mov	dword [ebp+i], 0

DoLink_loop1start:
	mov	eax, [DLLcount]   ; Просмотр списка DLL
	cmp	[ebp+i], eax	; loop1 condition
	jnb	DoLink_if1end

	mov	eax, [ebp+i]
	shl	eax, 2

	;mov     eax, DLLlist[eax] ; как это изобразить на fasm? MOV EAX, DWORD PTR DS:[EAX+40CD18]
	mov	eax, [DLLlist + eax] ; как-то так?

	mov	[ebp+dll], eax
	mov	ebx, [ebp+dll]	; Ссылка на DLL в списке идентификаторов

	mov	eax, [ebx+rectype]
	mov	[ebp+byName], eax ; Импорт по имени?

	cmp	dword [ebx+recmodline], 0 ; if2 (Были вызовы api_ из данной DLL?)
	jz	DoLink_if2end

	mov	eax, [outptr]
	mov	[ebp+dllName], eax

	mov	esi, [ebx+recid] ; Указатель на имя DLL

DoLink_loop2start:
	lodsb
	call	OP		; OP(byte AL);
	test	al, al		; loop condition
	jnz	DoLink_loop2start ; Сохраним имя DLL

	mov	eax, [outptr]
	mov	[ebp+AddrTable], eax ; фиксируем начало AddrTable для данного DLL

	mov	eax, [ebx + recmodline]
	inc	eax
	shl	eax, 2
	add	eax, [ebp+AddrTable]
	mov	[ebp+apiName], eax

	mov	dword [ebp+j], 0

DoLink_loop3start:
	mov	eax, [posts]	  ; Ищем в post
	cmp	[ebp+j], eax	; loop condition
	jnb	DoLink_loop3end

	mov	ebx, [ebp+j]
	shl	ebx, 2
	add	ebx, [posttype]

	cmp	dword [ebx], POST_API ; if3
	jnz	DoLink_if3end

	mov	ebx, [ebp+j]
	shl	ebx, 2
	add	ebx, [postnum]
	mov	eax, [ebx]
	mov	[ebp+api_], eax

	mov	ecx, [ebp+api_]

	mov	eax, [ebp+dll]
	cmp	[ecx+rectype], eax ; if4 (Из обрабатываемого DLL?)
	jnz	DoLink_if3end

	cmp	dword [ecx+recmodline], 0 ; if5 (Еще не внесен в Lookup)
	jnz	DoLink_if3end

	mov	eax, [outptr]
	sub	eax, [output]
	add	eax, [OptBaseOfCode]
	mov	[ecx+recmodline], eax

	cmp	dword [ebp+byName], 0 ; if6
	jz	DoLink_if6else

	mov	eax, [ebp+apiName]
	sub	eax, [output]
	add	eax, [OptBaseOfCode]
	dec	eax		; -2
	dec	eax
	call	OUTDWORD

	mov	eax, [outptr]
	mov	[ebp+hold], eax

	mov	eax, [ebp+apiName]
	mov	[outptr], eax

	mov	eax, [ecx+newid]

	test	eax, eax	; if7 (Есть алиас?)
	jz	DoLink_if7end

	invoke	lstrcmpA, [ecx+recid], eax

	test	eax, eax	; if8
	jz	DoLink_if7end

	mov	esi, [ebp+api_]
	mov	esi, [esi+newid]

	jmp	DoLink_L1

DoLink_if7end:
	mov	esi, [ecx+8]

DoLink_L1:
	lodsb
	call	OP		; OP(byte AL);
	test	al, al		; loop condition
	jnz	DoLink_L1	; Сохраним имя api_

	mov	eax, [outptr]
	mov	[ebp+apiName], eax

	mov	eax, [ebp+hold]
	mov	[outptr], eax

	jmp	DoLink_if3end

DoLink_if6else:
	mov	eax, [ecx+recnumber]
	or	eax, 80000000h

	call	OUTDWORD	; ord number

DoLink_if3end:
	inc	dword [ebp+j]
	jmp	DoLink_loop3start

DoLink_loop3end:
	xor	eax, eax
	call	OUTDWORD

	mov	eax, [ebp+hold] ; бессмысленный код
	mov	[outptr], eax

	mov	eax, [ebp+DirTable]
	mov	[outptr], eax

	mov	eax, [ebp+AddrTable]
	sub	eax, [output]
	add	eax, [OptBaseOfCode]
	call	OUTDWORD

	xor	eax, eax
	call	OUTDWORD

	xor	eax, eax
	call	OUTDWORD

	mov	eax, [ebp+dllName]
	sub	eax, [output]
	add	eax, [OptBaseOfCode]
	call	OUTDWORD

	mov	eax, [ebp+AddrTable]
	sub	eax, [output]
	add	eax, [OptBaseOfCode]
	call	OUTDWORD

	mov	eax, [outptr]
	mov	[ebp+DirTable], eax

	mov	eax, [ebp+apiName]
	mov	[outptr], eax

DoLink_if2end:
	inc	dword [ebp+i]
	jmp	DoLink_loop1start

DoLink_if1end:
	mov	eax, [ebp+DirTable]
	mov	[outptr], eax

; вагон лишнего
	xor	eax, eax
	call	OUTDWORD

	xor	eax, eax
	call	OUTDWORD

	xor	eax, eax
	call	OUTDWORD

	xor	eax, eax
	call	OUTDWORD

	xor	eax, eax
	call	OUTDWORD

	mov	eax, [ebp+apiName]
	mov	[outptr], eax

	mov	eax, [outptr]
	sub	eax, [output]
	sub	eax, [OptImportTableAdr]
	mov	[OptImportTableSize], eax

	mov	eax, [OptImportTableAdr]
	add	eax, [OptBaseOfCode]
	mov	[OptImportTableAdr], eax

	leave
	ret
  endp
end if
; ---------------------------------------------------------------------------
; Формирование EXE
; ---------------------------------------------------------------------------
  proc	WriteEXE

	push	ebp
	mov	ebp, esp
	if DEFINE_MAKE_EXE
	mov	eax, [outptr]
	sub	eax, [startptr]
	inc	eax
	mov	[OptCodeSize], eax

	mov	eax, [OptCodeSize]
	xor	edx, edx
	div	[OptSectionAlignment]
	mov	[OptImageSize], eax

	add	[OptImageSize], 2

	mov	eax, [OptImageSize]
	mul	[OptSectionAlignment]
	mov	[OptImageSize], eax

	add	[OptImageSize], 80000h

	mov	eax, [OptCodeSize]
	add	eax, 80000h
	mov	[TxtVirtualSize], eax

	mov	eax, [OptCodeSize]
	xor	edx, edx
	div	[OptFileAlignment]
	mov	[TxtSizeOfRawData], eax

	inc	[TxtSizeOfRawData]

	mov	eax, [TxtSizeOfRawData]
	mul	[OptFileAlignment]
	mov	[TxtSizeOfRawData], eax

	push	160h
	push	exeheader
	call	OnWriteEXEFile

	push	0A0h		; size
	call	WriteZero
	end if
	mov	eax, [outptr]
	sub	eax, [output]
	jz	WriteEXE_if2end ; if2
	mov	eax, [outptr]
	sub	eax, [output]

	push	eax
	push	[output]
	call	OnWriteEXEFile

WriteEXE_if2end:
	if DEFINE_MAKE_EXE
	mov	eax, [OptCodeSize]
	xor	edx, edx
	div	[OptFileAlignment]

	xchg	eax, edx
	xchg	eax, ebx

	mov	eax, [OptFileAlignment]
	sub	eax, ebx

	inc	eax

	push	eax		; size
	call	WriteZero
	end if
	xor	eax, eax

	pop	ebp
	ret
  endp