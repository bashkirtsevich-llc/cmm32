; ---------------------------------------------------------------------------
; константы кодогенератора
; ---------------------------------------------------------------------------
; jump tables
Jmp_Mov dd MapOperands_Mov_R
	dd MapOperands_Mov_E
	dd MapOperands_Mov_ERR
	dd MapOperands_Mov_S
	dd MapOperands_N12
	dd MapOperands_N12
	dd MapOperands_N12
	dd MapOperands_N12
	dd MapOperands_N12
	dd MapOperands_N12
Jmp_Mov_R dd MapOperands_Mov_R2R
	dd MapOperands_Mov_R2E
	dd MapOperands_Mov_R2C
	dd MapOperands_Mov_S
	dd MapOperands_N12
	dd MapOperands_N12
	dd MapOperands_N12
	dd MapOperands_N12
	dd MapOperands_N12
	dd MapOperands_N12
Jmp_Mov_E dd MapOperands_Mov_E2R
	dd MapOperands_Mov_ERR
	dd MapOperands_Mov_E2C
	dd MapOperands_Mov_S
	dd MapOperands_Mov_ERR
	dd MapOperands_Mov_ERR
	dd MapOperands_Mov_ERR
	dd MapOperands_Mov_ERR
	dd MapOperands_Mov_ERR
	dd MapOperands_Mov_ERR
Jmp_Op1 dd MapOperands_Op1R
	dd MapOperands_Op1E
	dd MapOperands_Op1C
	dd MapOperands_Op1S
	dd Op1ERR
	dd Op1ERR
	dd Op1ERR
	dd Op1ERR
	dd Op1ERR
	dd MapOperands_Op1AF
Jmp_Op2 dd MapOperands_Op2R
	dd MapOperands_Op2E
	dd MapOperands_Op2C
	dd MapOperands_Op2ERRS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRAF
Jmp_Op2R dd MapOperands_OpR2R
	dd MapOperands_OpR2E
	dd MapOperands_OpR2C
	dd MapOperands_Op2ERRS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRAF
Jmp_Op2E dd MapOperands_OpE2R
	dd 0			; нули
	dd MapOperands_OpE2C
	dd MapOperands_Op2ERRS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRSYS
	dd MapOperands_Op2ERRAF
Dsc_Jump dd CreateConstants
	dd CreateCode_Dsc_O
	dd CreateCode_Dsc_OW
	dd CreateCode_Dsc_OS
	dd CreateCode_Dsc_S
	dd CreateCode_Dsc_XRM
	dd CreateCode_Dsc_XPM
	dd CreateCode_Dsc_XSM
