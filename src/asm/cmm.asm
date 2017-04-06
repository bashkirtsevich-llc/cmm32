; ����� ������ ����� ����� � ����� ������ �����������
format PE GUI 4.0 DLL
entry  DllEntryPoint

DEFINE_MAKE_EXE = 0	; ������, ������������ ��� �������� ���������������� ������ (1 - ����� exe ����� 0 - ������ ������������ ����)

include   'win32a.inc'
; ---------------------------------------------------------------------------
; ������� ������� ���������
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
  ; �������� �-� ������ ��������� �����������

	push	ebp
	mov	ebp, esp

	; ��� ���� ����� abort � ���, �������� �����, ���� ���������� ���������, ���� ��������� RestoreState
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
	call	GetMem		; ��������� ������ ��� ����������
	call	TokInit 	; ������������� �������
	call	Compile 	; ����������
	; -------------------------------------------------------------------
	; �������������� ��������� ��������� � ������� �����, ���� ������ ������ � CMM32CompileEnd
	popad
	add	esp, 4
CMM32CompileEnd:
	pop	ebp
	ret	4*5
  endp
; ---------------------------------------------------------------------------
; �������� ���������� �����������
; ---------------------------------------------------------------------------
  include 'enums.inc'			 ; ���������
  include 'init.inc'			 ; ������� �������������
  include 'compiler.inc'		 ; ��� �����������
  include 'exe.inc'			 ; ��� ��� EXE
  include 'tree.inc'			 ; ���������� ������ � ���������
  include 'tokscan.inc' 		 ; ����������
  include 'directiv.inc'		 ; ���������� ��� ������ � �����������
  include 'parser.inc'			 ; ������
  include 'generate.inc'		 ; �������������
  include 'common.inc'			 ; ��������������� �-��
  include 'callbacks.inc'		 ; ��������
; ---------------------------------------------------------------------------
; ����������
; ---------------------------------------------------------------------------
section   '.data' data readable writeable;
  include 'data.inc'			 ; ����������
  include 'string_const.inc'		 ; ��������� ���������
; ---------------------------------------------------------------------------
; ���������
; ---------------------------------------------------------------------------
section   '.data2' data readable	 ;
  include 'opcodes.inc' 		 ; ��������� �������
  include 'tree_consts.inc'		 ; ��������� ��������
  include 'directiv_const.inc'		 ; ��������� ��������
  include 'generate_const.inc'		 ; ��������� ��������������
  include 'tokscan_const.inc'		 ; ��������� �����������
; ---------------------------------------------------------------------------
; ������
; ---------------------------------------------------------------------------
section   '.idata' import data readable  ;
  include 'import.inc'			 ; �������� ������������� �-��
; ---------------------------------------------------------------------------
; �������
; ---------------------------------------------------------------------------
section   '.edata' export data readable
; ��� �������������� ���������
  export  'CMM.DLL',\
	 CMM32Compile, 'Compile'
; ---------------------------------------------------------------------------
;
; ---------------------------------------------------------------------------
section   '.reloc' fixups data discardable