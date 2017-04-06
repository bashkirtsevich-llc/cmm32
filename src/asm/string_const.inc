; MAIN
  aNoInputFileSpe db 'No input file specified', 0
  main_str2 db 'MAP', 0
  main_str3 db 'DEBUG', 0
  main_str4 db 'ERROR > Unknown command line option:', 0

;
  errmsg_msg db 0Dh,0Ah
	     db 'ERROR> '
; outofmemory2
  outofmemory2_str db 'Not enough memory for the compiler', 27h, 's buffers.', 0Dh, 0Ah, 0

; expectederror
  expectederror_msg db 27h,'%s',27h,' expected',0

; compile
;  Compile_str1 db 'Compiling ...',0Dh,0Ah,0
  Compile_str2 db 'Errors: %d',9,'Lines:',9,'%u',0Dh,0Ah,0
  Compile_str3 db 0Dh,0Ah
	       db 'Run File Saved (%ld bytes).',0Dh,0Ah,0
  Compile_str4 db 'DLL: %d',9,'API: %d ',0Dh,0Ah,0
; StartMapfile
  StartMapfile_str1 db 'MAP FILE BEGIN',0Dh,0Ah,0

; Preview
  Preview_str1	  db 'File %s included.',0Dh,0Ah
		  db 0Dh,0Ah, 0

  Preview_str2	  db 'unuseable input',0

; maxwordpostserror
  maxwordpostserror_msg db 'maximum number of word post location references exceeded',0

; CompileAll
  CompileAll_str3 db 'Main not found',0
  CompileAll_str4 db '==>%3d %8lXh %8lXh %6Xh',9,'%s',0Dh,0Ah,0

; outofmemory
  outofmemory_msg db 'Compiler out of memory',0

; numexpected
  numexpected_msg db 27h,'number',27h,' expected',0

; DoPosts
  DoPosts_str1	  db 'Bad post type',0Dh,0Ah,0

; AddConstToTree
  AddConstToTree_str1 db 'Compiler out of memory for identifier tree',0
  AddConstToTree_str2 db 'string found in tree when trying to add to',9,'it',0

; AddLocalvar
  AddLocalvar_str1 db 'Compiler out of memory for local symbol linked list',0

; PrintMemsizes
  PrintMemsizes_str1 db 'Code:%u bytes,',9,'Post: %u bytes',0Dh,0Ah,0

; FinishMapfile
  FinishMapfile_str1 db 'Component Sizes:',0Dh,0Ah,0
  FinishMapfile_str2 db 'Run file size: %ld bytes',0Dh,0Ah,0
  FinishMapfile_str3 db 'END OF MAP FILE',0

; DisplayTree
  DisplayTree_str1 db 'ALL GLOBAL IDENTIFIERS LIST:',0Dh,0Ah,0
  DisplayTree_str2 db 'tok    type      number    post',9,'IDENTIFIER',0Dh,0Ah,0
  DisplayTree_str3 db 0Dh,0Ah
		   db '    %u Unique Global Identifiers.',0Dh,0Ah
		   db 0Dh,0Ah,0
  DisplayTree_str4 db 'GLOBAL CONSTANT IDENTIFIER LIST:',0Dh,0Ah,0
  DisplayTree_str5 db 0Dh,0Ah
		   db '    %u Unique Global Constant Value Identifiers.',0Dh,0Ah
		   db 0Dh,0Ah,0

; DisplayTreeAll
  DisplayTreeAll_str1 db '%3d %8lXh %8lXh %6Xh',9,'%s',0Dh,0Ah,0
  DisplayTreeAll_str2 db 'Alias=%s',0Dh,0Ah,0
  DisplayTreeAll_str3 db 'File:%s, line=%-d:',0Dh,0Ah
		      db '%s',0Dh,0Ah,0

; DisplayTreeConstants
  DisplayTreeConstants_str1 db '#define %10ld /* %8lX hex */ %s',0Dh,0Ah,0

; unabletoopen
  unabletoopen_str1 db 'unable to open file ',27h,'%s',27h,0

; EMPTY
  EMPTY_str	  db '-ToDo',0Dh,0Ah,0

; GetImport
  GetImport_str1  db '%s',0
  GetImport_str2  db '%s\%s',0
  GetImport_str3  db 'ERROR: No export directory in file %s.',0Dh,0Ah,0

; SearchLocals
  SearchLocals_str1 db 'Bad *tok4 value in SearchLocals',0

; preerror
  preerror_str	    db '%s (%d)#%d> %s.',0Dh,0Ah,0

; Proc
  Proc_str1	  db 'push ebp;mov ebp,esp',0

; DoExpr
  DoExpr_str2	  db '%s %s,%s',0
  DoExpr_str3	  db '%s %s,%d',0
  DoExpr_str4	  db '%s %s#%s',0
  DoExpr_str5	  db 'lea ebx,%s',0
  DoExpr_str6	  db '%s %s,ebx',0
  DoExpr_str7	  db '%s %s,al',0
  DoExpr_str8	  db '%s %s,ax',0
  DoExpr_str9	  db '%s %s,eax',0

; Expression
  Expression_str1  db 'add %s,%s',0
  Expression_str2  db 'sub %s,%s',0
  Expression_str3  db 'or %s,%s',0
  Expression_str4  db 'xor %s,%s',0
  Expression_str5  db 'cmp %s,%s',0
  Expression_str6  db 'movzx %s,al',0
  Expression_str7  db 'setne al',0
  Expression_str8  db 'setg al',0
  Expression_str9  db 'setge al',0
  Expression_str10 db 'setl al',0
  Expression_str11 db 'setle al',0
  Expression_str12 db 'sete al',0

; Factor
  Factor_str1	  db 'pop ebx; xchg eax,ebx',0
  Factor_str2	  db '%#x',0

; GetInto
  GetInto_str1	  db 'mov %s,-%#x',0
  GetInto_str2	  db 'mov %s,%#x',0
  GetInto_str3	  db 'mov %s,#%s',0
  GetInto_str4	  db 'mov %s,ebx',0
  GetInto_str5	  db 'Wrong expression member',0
  GetInto_str6	  db 'neg %s',0
  GetInto_str7	  db 'not %s',0

; GetIntoVar
  GetIntoVar_str1  db 'mov %s,%s',0
  GetIntoVar_str2  db 'movzx %s,%s',0
  GetIntoVar_str3  db 'movsx %s,%s',0
  GetIntoVar_str4  db 'mov %s,%d',0
  GetIntoVar_str5  db 'lea %s,%s',0
  GetIntoVar_str6  db 'mov dword %s,%s',0
  GetIntoVar_str7  db 'mov word %s,%s',0
  GetIntoVar_str8  db 'mov byte %s,%s',0
  GetIntoVar_str9  db 'mov al,%s',0
  GetIntoVar_str10 db 'mov %s,al',0
  GetIntoVar_str11 db 'movzx ax,%s',0
  GetIntoVar_str12 db 'movsx ax,%s',0
  GetIntoVar_str13 db 'mov ax,%s',0
  GetIntoVar_str14 db 'mov ax,word %s',0
  GetIntoVar_str15 db 'mov %s,ax',0
  GetIntoVar_str16 db 'movzx eax,%s',0
  GetIntoVar_str17 db 'movsx eax,%s',0
  GetIntoVar_str18 db 'mov eax,%s',0
  GetIntoVar_str19 db 'mov %s,eax',0

; GetVarname
  GetVarname_str1 db '[',0
  GetVarname_str2 db '+',0
  GetVarname_str3 db '*',0
  GetVarname_str4 db '#',0
  GetVarname_str5 db ']',0
  GetVarname_str6 db 'Illegal index expression in []',0

; Term
  Term_str1	  db 'mov ebx,%d',0
  Term_str2	  db 'mul %s',0
  Term_str3	  db 'div %s',0
  Term_str4	  db 'xchg eax,edx',0
  Term_str5	  db 'and ebx,%d',0
  Term_str6	  db 'and eax,%s',0
  Term_str7	  db 'not eax',0

; PushEAX
  PushEAX_str1	  db 'push eax',0

; AddPoststring
  AddPoststring_str1 db 'cannot add post string, post queue full',0

; undefinederror
  undefinederror_str1 db 27h,'%s',27h,' undefined',0

; DoParam ; заменить
  aPushS	  db 9,'//',9,'push #"%s"',0Dh,0Ah,0
  aPushX	  db 'push %#x',0
  aPushS_0	  db 'push #%s',0
  aPushS_1	  db 'push %s',0
  aMovsxESSPushES db 'movsx e%s,%s;push e%s',0
  aMovsxECxSPushE db 'movsx e%cx,%s;push e%cx',0

; CmdCycle
  CmdCycle_str1   db 'jmp @L%d',0

; CmdCase
  CmdCase_str1	  db 'jnc @L%d',0
  CmdCase_str2	  db 'jc @L%d',0
  CmdCase_str3	  db 'jnz @L%d',0
  CmdCase_str4	  db 'jz @L%d',0
  CmdCase_str5	  db 'jno @L%d',0
  CmdCase_str6	  db 'jo @L%d',0
  CmdCase_str7	  db 'eax',0
  CmdCase_str8	  db 'test eax,eax;jnz @L%d',0
  CmdCase_str9	  db 'test eax,eax;jz @L%d',0
  CmdCase_str10   db '@L%d:',0

; CmdContinue
  CmdContinue_str1 db 'jmp @L%d',0

; CmdCycle
  CmdCycle_str2   db 'dec %s;jnz @L%d',0

; CmdDo
  CmdDo_str1	  db 27h,'while',27h,' expected following ',27h,'do',27h,0

; CmdIf
  CmdIf_str1	  db 'jmp @L%d;',0Dh,0Ah
		  db '@L%d:',0

; TypeSize
  TypeSize_str	  db 'vartype=%d in TypeSize()',0

; expecting
  expecting_msg   db 'expecting a different token',0

; DeclareLocals
  DeclareLocals_str1 db 'push ebp; mov ebp,esp;',0

  DeclareLocals_str2 db 'sub esp,%d;',0

; LeaveProc
  LeaveProc_str1  db 'leave',0
  LeaveProc_str2  db 'pop ebp',0
  LeaveProc_str3  db 'retf',0
  LeaveProc_str4  db 'retf %d;',0
  LeaveProc_str5  db 'ret',0
  LeaveProc_str6  db 'ret %d;',0

; internalerror_
  internalerror_str1 db '%s(%d)#%d> *** SERIOUS COMPILER INTERNAL ERROR ***',0Dh,0Ah
		     db '>%s.',0Dh,0Ah,0
  internalerror_str2 db 'STRING:%s',0Dh,0Ah,0
  internalerror_str3 db 'TOK:%d',9,'POST:%d',9,'number:%ld',0Dh,0Ah,0
  internalerror_str4 db 'STRING2:%s',0Dh,0Ah,0
  internalerror_str5 db 'TOK2:%d',9,'POST2:%d',9,'number2:%ld',0Dh,0Ah,0

; illegaloperand
  illegaloperand_str db 'ASM: Illegal operand',0Dh,0Ah,0

; toomuchoperands
  toomuchoperands_str db 'ASM: Illegal number of operands',0Dh,0Ah,0

; datatype_expected
  datatype_expected_msg db 'byte, word, short, char, dword or int expected',0

; idexpected
  idexpected_str1 db 'undefined ',27h,'identifier',27h,' expected',0

; unexpectedeof
  unexpectedeof_str1 db 'unexpected END OF FILE',0

; localunresolved
  localunresolved_msg db 'local jump label ',27h,'%s',27h,' unresolved',0

; DoId
  DoId_str1	  db 'call %s;',0

; doAPI
  doAPI_str1	  db 'maximum number of API procedure calls exceeded',0
  doAPI_str2	  db 9,'//',9,'call ',0
  doAPI_str3	  db 0Dh,0Ah,0

; DoVar
  DoVar_str1	  db 'mov',0
  DoVar_str2	  db 'dec %s',0
  DoVar_str3	  db 'inc %s',0
  DoVar_str4	  db 'add',0
  DoVar_str5	  db 'sub',0
  DoVar_str6	  db 'and',0
  DoVar_str7	  db 'xor',0
  DoVar_str8	  db 'or',0
  DoVar_str9	  db 'xchg %s,%s',0
  DoVar_str10	  db 'xchg %s,eax',0
  DoVar_str11	  db 'xchg %s,ax',0
  DoVar_str12	  db 'xchg %s,al',0
  DoVar_str13	  db 'cl',0
  DoVar_str14	  db 'shl %s,cl',0
  DoVar_str15	  db 'shr %s,%d',0
  DoVar_str16	  db 'shr %s,cl',0
  DoVar_str17	  db 'shl %s,%d',0

; swaperror
  swaperror_str1  db 'invalid or incompatable swap item',0

; SeekUndefined
  SeekUndefined_str1 db 27h,'%s',27h,' undefined',0Dh,0Ah,0

; expected
  expected_msg	  db 27h,'%c',27h,' expected',0

; stringexpected_str1
  stringexpected_str1 db 27h,'string',27h,' expected',0

; GlobalVar
  aCountDofrom1   db 'count = DoFrom(1);',0Dh,0Ah,0
  aCountDoextract db 'count = DoExtract(1);',0Dh,0Ah,0


; operatorexpected
  operatorexpected_str1 db 'operator identifier expected',0

; GENERATE.INC

; Asm
  Asm_str1	  db 9,'//',9,0
  Asm_str2	  db 'ASM: Bad input format',0Dh,0Ah,0
  Asm_str3	  db 0Dh,0Ah,0

; GETSCALING
  GETSCALING_str1 db 'ASM: Illegal scaling value',0Dh,0Ah,0

; GETOL3
  GETOL3_str1	  db 'ASM: This instruction required a WORD/DWORD operand',0Dh,0Ah,0

; GETMODRMBYTE
  GETMODRMBYTE_str1 db 'ASM: You can',27h,'t use byte registers in addresses',0Dh,0Ah,0
  GETMODRMBYTE_str2 db 'ASM: 16-bit addressing mode not allowed',0Dh,0Ah,0
  GETMODRMBYTE_str3 db 'ASM: You must use a 32-bit registers for scaling',0Dh,0Ah,0
  GETMODRMBYTE_str4 db 'ASM: You can',27h,'t use EBP as an index',0Dh,0Ah,0

; GETOVERRIDE
  aAsmDoubleSegme db 'ASM: Double segment override',0

; DoMnemonics
  DoMnemonics_str1 db 'ASM: No byte register in address',0Dh,0Ah,0
  DoMnemonics_str2 db 'ASM: too much registers in combination',0Dh,0Ah,0
  DoMnemonics_str3 db 'ASM: Syntax error',0Dh,0Ah,0

; CreateCode
  CreateCode_str1 db 'Descriptor damaged',0Dh,0Ah,0

; MapOperands
  MapOperands_str1  db 'ASM: Operand required',0Dh,0Ah,0
  MapOperands_str2  db 'ASM: Two operands required',0Dh,0Ah,0
  MapOperands_str3  db 'ASM: Not same size',0Dh,0Ah,0
  MapOperands_str4  db 'ASM: a constant can',27h,'t be used as a destination',0Dh,0Ah,0
  MapOperands_str5  db 'ASM: Word required',0Dh,0Ah,0
  MapOperands_str6  db 'ASM: CR1 only readable',0Dh,0Ah,0
  MapOperands_str7  db 'ASM: SysR must be dword',0Dh,0Ah,0
  MapOperands_str10 db 'ASM: only use system registers within MOV instruction',0Dh,0Ah,0
  MapOperands_str8  db 'ASM: Jump range too long',0Dh,0Ah,0
  MapOperands_str9  db 'ASM: segment register can only be used within a MOV or PUSH',0Dh,0Ah,0
  MapOperands_str11 db 'Absolute FAR addresses can only be used for jumps',0Dh,0Ah,0

; end GENERATE.INC

; ConvertChar
  ConvertChar_str1 db 'decimal digit',0
  ConvertChar_str2 db 'hexdecimal digit',0

; TokScan
  TokScan_str1	  db 'Maximum length for an identifier exceeded',0
  TokScan_str2	  db 'tokenizer: bad character value',0
  TokScan_str3	  db 'Maximum String Length Exceeded',0
