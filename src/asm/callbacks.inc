;
; ������� ��� ������� �-���
;
; ---------------------------------------------------------------------------
; ���������� �������� �����
; ---------------------------------------------------------------------------
  proc	OnLoadFile
  destPtr	 = 8
  fileNamePtr	 = 0Ch

	push	ebp
	mov	ebp, esp

	push	dword [ebp+fileNamePtr]
	push	dword [ebp+destPtr]
	call	dword [OnLoadFileProcPtr]

	pop	ebp
	ret	4*2
  endp
; ---------------------------------------------------------------------------
; ���������� ������ ������������ �����
; ---------------------------------------------------------------------------
  proc	OnWriteEXEFile
  dataPtr	 = 8
  dataSize	 = 0Ch

	push	ebp
	mov	ebp, esp

	push	dword [ebp+dataSize]
	push	dword [ebp+dataPtr]
	call	dword [OnWriteEXEFileProcPtr]

	pop	ebp
	ret	8
  endp
; ---------------------------------------------------------------------------
; ���������� ������ MAP �����
; ---------------------------------------------------------------------------
  proc	OnWriteMAPFile
  dataPtr	 = 8

	push	ebp
	mov	ebp, esp

	push	dword [ebp+dataPtr]
	call	dword [OnWriteMAPFileProcPtr]

	pop	ebp
	ret	4
  endp
; ---------------------------------------------------------------------------
; ���������� ������ ���������
; ---------------------------------------------------------------------------
  proc	OnWriteMessage
  dataPtr	 = 8

	push	ebp
	mov	ebp, esp

	push	dword [ebp+dataPtr]
	call	dword [OnWriteMessageProcPtr]

	pop	ebp
	ret	4
  endp
; ---------------------------------------------------------------------------