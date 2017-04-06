format PE
entry  start

include   'win32a.inc'

section   '.text' code readable executable

  proc	start
;        invoke  _lcreat, exefile, 0
;        mov     [exefilehandle], eax

;        invoke  _lcreat, mapfile, 0
;        mov     [mapfilehandle], eax

;        invoke  _lcreat, msgfile, 0
;        mov     [msgfilehandle], eax

	invoke	Compile, 040000h, onLoadFileProc, onWriteEXEFileProc, onWriteMAPFileProc, onWriteMessageProc
	invoke	ExitProcess, 0
  endp

  proc	onLoadFileProc
  destPtr	 = 8
  fileNamePtr	 = 0Ch

	push	ebp
	mov	ebp, esp

	mov	eax, dword [ebp+destPtr]
	lea	edx, [samplecode]
	mov	[eax], edx

	pop	ebp
	ret	2*4
  endp

  proc	onWriteEXEFileProc
  dataPtr	 = 8
  dataSize	 = 0Ch

	push	ebp
	mov	ebp, esp

	int3
	call	dword [ebp+dataPtr]
;        invoke  _lwrite, [exefilehandle], [ebp+dataPtr], [ebp+dataSize]

	pop	ebp
	ret	2*4
  endp

  proc	onWriteMAPFileProc
  dataPtr	 = 8

	push	ebp
	mov	ebp, esp

;        invoke  _lwrite, [mapfilehandle], [ebp+dataPtr], [ebp+dataSize]

	pop	ebp
	ret	4
  endp

  proc	onWriteMessageProc
  dataPtr	 = 8

	push	ebp
	mov	ebp, esp

;        invoke  lstrlenA, dword [ebp+dataPtr]
;        invoke  _lwrite, [msgfilehandle], [ebp+dataPtr], eax

	pop	ebp
	ret	4
  endp

section   '.data' data readable writable

exefile db '1.exe',0
exefilehandle dd 0

mapfile db '1.map',0
mapfilehandle dd 0

msgfile db '1.txt',0
msgfilehandle dd 0

samplecode db '#map',13,10,\
	      '#list',13,10,\
	      'main()',13,10,\
	      '{',13,10,\
	      '        STOSD;',13,10,\
	      '        foo("qweqwe");',13,10,\
	      '}',13,10,\
	      'dword foo(dword string)',13,10,\
	      '{',13,10,\
	      '        return(0);',13,10,\
	      '}',0

section   '.idata' import data readable
  library kernel,'KERNEL32.DLL',\
	  cmm,'CMM.DLL'

  import kernel,\
	 ExitProcess, 'ExitProcess',\
	 _lcreat, '_lcreat',\
	 _lwrite, '_lwrite',\
	 _lclose, '_lclose',\
	 lstrlenA, 'lstrlenA'

  import cmm,\
	 Compile, 'Compile'