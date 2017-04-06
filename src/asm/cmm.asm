; «десь только точка входа и вызов логики компил€тора
format PE GUI 4.0 DLL
entry  DllEntryPoint

DEFINE_MAKE_EXE = 0	; дефайн, определ€ющий как выдавать скомпилированные данные (1 - вывод exe файла 0 - только исполн€емого кода)

include   'win32a.inc'
; ---------------------------------------------------------------------------
; √лавна€ функци€ программы
; ---------------------------------------------------------------------------
section   '.text' code readable executable

  proc	DllEntryPoint
	mov	eax, TRUE
	ret
  endp

  proc	CMM32Compile
  ImageBase	     = 8
  onLoadFileProc     = 0Ch
  onWriteEXEFileProc = 10h
  onWriteMAPFileProc = 14h
  onWritemessageProc = 18h
  ; добавить ф-ю вывода сообщений компил€тора

	push	ebp
	mov	ebp, esp

	; это типа такой abort у нас, сохраним адрес, куда необходимо выскочить, если вызоветс€ RestoreState
	push	CMM32CompileEnd
	pushad
	mov	[SavedESPState], esp

if DEFINE_MAKE_EXE
else

	mov	eax, dword [ebp+ImageBase]
	mov	[OptImageBase], eax

	xor	eax, eax
	mov	[OptBaseOfCode], eax

end if

	mov	eax, dword [ebp+onLoadFileProc]
	mov	dword [OnLoadFileProcPtr], eax

	mov	eax, dword [ebp+onWriteEXEFileProc]
	mov	dword [OnWriteEXEFileProcPtr], eax

	mov	eax, dword [ebp+onWriteMAPFileProc]
	mov	dword [OnWriteMAPFileProcPtr], eax

	mov	eax, dword [ebp+onWritemessageProc]
	mov	dword [OnWriteMessageProcPtr], eax
	; -------------------------------------------------------------------
	call	GetMem		; выделение пам€ти дл€ компил€ции
	call	TokInit 	; инициализаци€ списков
	call	Compile 	; компил€ци€
	; -------------------------------------------------------------------
	; восстановление состо€ни€ регистров и очистка стека, если небыло прыжка в CMM32CompileEnd
	popad
	add	esp, 4
CMM32CompileEnd:
	pop	ebp
	ret	4*5
  endp
; ---------------------------------------------------------------------------
; основной функционал компил€тора
; ---------------------------------------------------------------------------
  include 'enums.inc'			 ; константы
  include 'init.inc'			 ; функции инициализации
  include 'compiler.inc'		 ; код компил€тора
  include 'exe.inc'			 ; код дл€ EXE
  include 'tree.inc'			 ; функционал работы с деревь€ми
  include 'tokscan.inc' 		 ; токенайзер
  include 'directiv.inc'		 ; функционал дл€ работы с директивами
  include 'parser.inc'			 ; парсер
  include 'generate.inc'		 ; кодогенератор
  include 'common.inc'			 ; вспомогательные ф-ии
  include 'callbacks.inc'		 ; коллбэки
; ---------------------------------------------------------------------------
; переменные
; ---------------------------------------------------------------------------
section   '.data' data readable writeable;
  include 'data.inc'			 ; переменные
  include 'string_const.inc'		 ; строковые константы
; ---------------------------------------------------------------------------
;  онстанты
; ---------------------------------------------------------------------------
section   '.data2' data readable	 ;
  include 'opcodes.inc' 		 ; константы опкодов
  include 'tree_consts.inc'		 ; константы деревьев
  include 'directiv_const.inc'		 ; константы директив
  include 'generate_const.inc'		 ; константы кодогенератора
  include 'tokscan_const.inc'		 ; константы токенайзера
; ---------------------------------------------------------------------------
; »мпорт
; ---------------------------------------------------------------------------
section   '.idata' import data readable  ;
  include 'import.inc'			 ; перечень импортируемых ф-ий
; ---------------------------------------------------------------------------
; Ёкспорт
; ---------------------------------------------------------------------------
section   '.edata' export data readable
; все экспортируемые процедуры
  export  'CMM.DLL',\
	 CMM32Compile, 'Compile'
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
section   '.reloc' fixups data discardable