; Разделители
Delim1		db '#"',27h,'-+*/%|&!^=><@',0
Delim2		db ':;(){}[],.$?~',0

; '#','\"','\'','-','+','*','/','%','|','&','!','^','=','>','<','@'
Jmp_Delim1 dd TokScan_Jmp_Number, TokScan_Jmp_String, TokScan_Jmp_Const, TokScan_Jmp_Minus,\
	      TokScan_Jmp_Plus,   TokScan_Jmp_Mul,    TokScan_Jmp_Div,	 TokScan_Jmp_Mod,   TokScan_Jmp_Or,   TokScan_Jmp_And,\
	      TokScan_Jmp_Not,	  TokScan_Jmp_Xor,    TokScan_Jmp_Equal, TokScan_Jmp_Great, TokScan_Jmp_Less, TokScan_Jmp_Local

tk_delim2 db tk_colon, tk_semicolon, tk_openbracket, tk_closebracket,\			;':', ';', '(', ')',
	     tk_openbrace, tk_closebrace, tk_openblock, tk_closeblock, tk_comma,\	;'{', '}', '[', ']', ',',
	     tk_period, tk_dollar, tk_question, tk_tilda				;'.', '$', '?', '~'
