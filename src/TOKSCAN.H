// ---- �������������� �������� ������� ��� ���������� ����: \n, \x00,	 �.�.
byte ConvertChar()
dword hold;
{
	IF(cha!='\\')return(cha);
	NextChar(); // ��������� ���������� �� \
	IF(AL>='0')&&(AL<='9'){
// ���������� ���������
		EDX=0;
		AL-='0';
		EAX=AL;
		EDX+=EAX;
		ECX=2;
		loop(ECX){
			EBX=EDX;
			EDX<<=1;
			EBX<<=3;
			EDX+=EBX;
			NextChar();
			EAX=AL;
			IF(AL<'0')||(AL>'9')GOTO ERR;
			AL-='0';
			EDX+=EAX;
		}
		return(DL);
ERR:
		expectederror("decimal digit");
		return(0);
	}
	IF(cha=='a')return('\a');
	IF(cha=='b')	return('\b');
	IF(cha=='f')	return('\f');
	IF(cha=='l')	return(10);
	IF(cha=='n')	return(13);
	IF(cha=='p')	return('_');
	IF(cha=='r')	return(13);
	IF(cha=='t')	return('\t');
	IF(cha=='v')	return('\v');
	IF(cha=='x'){ // HEX ���������
		ECX=2;
		hold=0;
		loop(ECX){
			$PUSH ECX;
			NextChar();
			CharUpperA(AL);
			EBX=AL;
			IF(AL>='0')&&(AL<='9')GOTO LX1;
			IF(AL<'A')&&(AL>'F')GOTO ERR1;
LX1:
			EDX=hold;
			EDX<<=4;
			IF(BL>='A')BL-='A'-10-'0';
			BL-='0';
			EDX+=EBX;
			hold=EDX;
			$POP ECX;
		}
		return(hold);
ERR1:
		$POP ECX;
		expectederror("hexdecimal digit");
		return(0);
	}
	return(cha);
}

// ---- �����������
byte Delim1={'#','\"','\'','-','+','*','/','%','|','&','!','^','=','>','<','@',0};
byte Delim2={':',';','(',')','{','}','[',']',',','.','$','?','~',0};
// ---- ����������� ���� token
TokScan(dword tok4,type4,src4,post4,string4,number4)
dword useme,strptr;
dword next;
dword dirPrefix,locPrefix;// ����� ����������� #directive ��� @LocalLabel
{
	dirPrefix=0;
	locPrefix=0;
SC_0:
	strptr=string4;
	next=1;
	EAX=number4;
	DSDWORD[EAX] = 0;
	EAX=type4;
	DSDWORD[EAX] = 0;
	EAX=src4;
	DSDWORD[EAX] = 0;
	EAX=post4;
	DSDWORD[EAX] = 0;
	WhiteSpaces();
	ESI=string4;
	DSBYTE[ESI]=0;
	ECX=17;
	EDI=#Delim1;
	AL=cha;
	$REPNZ $SCASB;
	$JCXZ SC00	// �� ������ ������ �����������
	EDI=EDI-#Delim1-1<<2+#Jmp_Delim1;
	$JMP NEAR DSDWORD[EDI];
SC00:
	ECX=14;
	EDI=#Delim2;
	AL=cha;
	$REPNZ $SCASB;
	$JCXZ SC01	// �� ������ ������ �����������
	EDI=EDI-#Delim2-1+#tk_delim2;
	EAX=DSBYTE[EDI];
	EBX=tok4;
	DSDWORD[EBX]=EAX;
	$JMP ScEx
SC01:
	IF(locPrefix){
		EDI><strptr;
		AL='@';
		$STOSB;
		strptr><EDI;
	}
	IF(cha==0){ 	// ����� �������� ������
		EAX=tok4;
		DSDWORD[EAX]=tk_eof;
		next=0;
	}
	else
	if(IsCharAlphaA(cha))||(cha=='_'){ // ������������� ���������� � �����
		do{
			do{ // �������� ������������� � string4
UP:
				EDI><strptr;
				AL=cha;
				$STOSB;
				strptr><EDI;
				NextChar();
			}while(IsCharAlphaNumericA(cha));
		}while(cha=='_');
		EAX=strptr-string4;
		IF(EAX>=IDLENGTH){
			preerror("Maximum length for an identifier exceeded");
			strptr = string4 + IDLENGTH - 1;
		}
		EDI=strptr;
		AL=0;
		$STOSB
		EBX=tok4;
		DSDWORD[EBX]=tk_id;
		IF(locPrefix)goto FL;
		FastSearch(string4,#St_Directives); // ��� ����������������� �����?
		IF(CARRYFLAG){	// ������� ���������� � ������
			EBX=number4; DSDWORD[EBX]=EAX;	// �������� ���������� �����
			EBX=tok4; 	// ������ ��������������� token
			IF(dirPrefix)DSDWORD[EBX]=tk_directive;
			ELSE DSDWORD[EBX]=tk_command;
			dirPrefix=0;
			next=0;
			$JMP ScEx
		}
		FastSearch(string4,#St_Mnemonics);	// ��� ���������?
		IF(CARRYFLAG){	// ��������� ����������
			EBX=number4;
			DSDWORD[EBX]=EAX; // �������� ����� ���������
			EBX=tok4;
			DSDWORD[EBX]=tk_mnemonics;
			next=0;
			$JMP ScEx
		}
		FastSearch(string4,#St_Registers);	// ��� ��� ��������?
		IF(CARRYFLAG){	// �������
			EBX=number4;
			DSDWORD[EBX]=EAX; // �������� ����� �������
			EAX>>=3;
			$CMP EAX,2;
			$JG R0		// ����������� ��������?
			EBX=tok4;
			DSDWORD[EBX]=tk_reg;
			EBX=type4;
			EAX<<=1;
			DSDWORD[EBX] = EAX + tk_byte;
			GOTO R1;
R0:
			EBX=tok4;
			EAX-=3;
			DSDWORD[EBX]=EAX+tk_controlreg;
R1:
			next=0;
			$JMP ScEx
		}
FL:
		EAX=tok4;
		EAX=DSDWORD[EAX];
		IF(AL==tk_id){
			SearchLocals(tok4,type4,string4,number4); // ���� � ������ ���������?
			EAX=tok4;
			EAX=DSDWORD[EAX];
			IF(AL==tk_id){
				IF(locPrefix){	// @label
					EBX=tok4;
					DSDWORD[EBX]=tk_locallabel;
					IF(displaytokerrors)AddLocalvar(#string,tk_locallabel,0,0);
					locPrefix=0;
					GOTO FL;
				}
				SearchTree(tok4,type4,src4,post4,string4,number4);
			}
		}
		IF(dirPrefix){	// �����������: #ident
			dirPrefix=0;
			EBX=tok4;
			EAX=DSDWORD[EBX];
			IF(AL==tk_id){	// ���� � ������?
				IF(displaytokerrors){
					EAX=post4;
					DSDWORD[EAX]	= 1;	// ������� � ������
					EBX=tok4;
					DSDWORD[EBX]=tk_undefproc;
					AddToTree(string4);
				}
			}
				// ������������� ���� � ������
			EAX=post4;
			EAX=DSDWORD[EAX];
			EBX=tok4;
			IF(EAX){	// ��� �� ������������ �������������?
				DSDWORD[EBX] = tk_postnumber;
			}
			ELSE{
				EAX=tok4;
				EAX=DSDWORD[EAX];
				IF(EAX==tk_param)DSDWORD[EBX] = tk_locnumber;
				ELSE IF(EAX==tk_local)DSDWORD[EBX] = tk_locnumber;
				ELSE DSDWORD[EBX] = tk_number;
			}
		}
		next=0;
		$JMP ScEx
	}
	else if(IsNumber(cha)){ // ������������� ���������� � �����
		EAX=tok4;
		DSDWORD[EAX]=tokens;	// ���� ����������� token
		if(cha=='0'){
			NextChar();
			IF(cha=='X')||(cha=='x'){		// hex - �����
				EAX=tok4;
				DSDWORD[EAX]=tk_number;
HEX:
				NextChar();
				CharUpperA(AL);
				$CMP AL,'0';
				$JL EHEX;
				$CMP AL,'9';
				$JA HEXAF
				AL-='0';
				for(;;){
					EBX=number4;
					ECX=DSDWORD[EBX];
					ECX<<=4;
					EAX+=ECX;
					DSDWORD[EBX]=EAX;
					GOTO	HEX;
HEXAF:
					IF(AL<'A')||(AL>'F')BREAK;
					AL-='7';
				}
EHEX:
			}
			ELSE IF(cha=='B')||(cha=='b'){		// binary �����
				EAX=tok4;
				DSDWORD[EAX]=tk_number;
				for(;;){
					NextChar();
					IF(AL!='0')||(AL!='1')BREAK;
					AL-='0';
					EBX=number4;
					ECX=DSDWORD[EBX];
					ECX<<=1;
					EAX+=ECX;
					DSDWORD[EBX]=EAX;
				}
EBIN:
			}
			ELSE IF(cha=='O')||(cha=='o'){		// octal �����
				EAX=tok4;
				DSDWORD[EAX]=tk_number;
				for(;;){
					NextChar();
					IF(AL<'0')||(AL>'7')BREAK;
					AL-='0';
					EBX=number4;
					ECX=DSDWORD[EBX];
					ECX<<=3;
					EAX+=ECX;
					DSDWORD[EBX]=EAX;
				}
EOCT:
			}
		}
		EAX=tok4;
		EAX=DSDWORD[EAX];
		IF(EAX!=tk_number){ 	// decimal �����
			for(;;){
				EAX=cha;
				IF(AL<'0')||(AL>'9')BREAK;
				AL-='0';
				EBX=number4;
				EDX=DSDWORD[EBX];
				ECX=EDX;
				EDX<<=1;
				ECX<<=3;
				EDX+=ECX;
				EAX+=EDX;
				DSDWORD[EBX]=EAX;
				NextChar();
			}
			EAX=tok4;
			DSDWORD[EAX]=tk_number;
		}
		next=0;
	}
	ELSE{
		IF(displaytokerrors)preerror("tokenizer: bad character value");
		NextChar();
		TokScan(tok4,type4,src4,post4,string4,number4);
		next=0;
	}
	$JMP ScEx
Jmp_Number: 	// #directive || #identifier
	NextChar();
	dirPrefix=1;
	$JMP SC_0
Jmp_Local:		// @LocalLabel
	NextChar();
	locPrefix=1;
	$JMP SC01
Jmp_String: 	// ������ �������� � ""
	do{
		NextChar();
		IF(cha=='\"') // ����������� �������
			BREAK;
		EAX=strptr-string4;
		IF(EAX<STRLEN-1 ){
			AL=ConvertChar();
			EDI=strptr;
			$STOSB;
			IF(AL==13)&&(cha=='n'){
				AL=10;
				$STOSB
			} // ������� char 10 ��� \n
			strptr=EDI;
		}
		ELSE{
			IF(displaytokerrors)preerror("Maximum String Length Exceeded");
			WHILE(cha!='\"'){  // ����� ����������� �������
				IF(endoffile)BREAK;
				NextChar();
			}
			BREAK;
		}
	}while(cha==0);  // ����� ������ ���� !=
	EDI=strptr;
	DSBYTE[EDI]=0;
	EDI-=string4;
	EAX=tok4;
	DSDWORD[EAX]=tk_string;
	EAX=number4;
	DSDWORD[EAX] =	EDI;	// �������� ����� �����
	IF(cha!='\"')expected('\"');
	$JMP	ScEx
Jmp_Const:			// ���������� ���������: 'AbCD'
	NextChar();
	EAX=tok4;
	DSDWORD[EAX]=tk_number;
	EAX=number4;
	DSDWORD[EAX] =	0;
	WHILE(cha != '\''){
		IF(endoffile)BREAK;
		EAX=ConvertChar();
		EBX=number4;
		ECX=DSDWORD[EBX];
		ECX<<=8;
		EAX+=ECX;
		DSDWORD[EBX]=EAX;
		NextChar();
	}
	IF(cha != '\''){
		IF(displaytokerrors)
		expected(0x27/*'\''*/);
	}
	ELSE NextChar();
	next = 0;
	$JMP ScEx
Jmp_Minus:			// -
	NextChar();
	EBX=tok4;
	IF(cha=='=') DSDWORD[EBX]=tk_minusequals; // -=
	ELSE IF(cha=='-') DSDWORD[EBX]=tk_minusminus; // --
	ELSE{
		DSDWORD[EBX]=tk_minus;
		next = 0;
	} // -
	$JMP	ScEx
Jmp_Plus: 		// +
	NextChar();
	EBX=tok4;
	IF(cha=='=')DSDWORD[EBX]=tk_plusequals; // +=
	ELSE IF(cha=='+')  DSDWORD[EBX]=tk_plusplus;		// ++
	ELSE{
		WhiteSpaces();
		EBX=tok4; // �������� ������� ��������
		IF(cha=='-')DSDWORD[EBX]=tk_minus;	// ����������� + -
		ELSE{
			DSDWORD[EBX]=tk_plus;
			next = 0;
		}
	}
	$JMP	ScEx
Jmp_Mul:				// *
	NextChar();
	WhiteSpaces();
	EBX=tok4;
	IF(cha == '-')DSDWORD[EBX] = tk_multminus;	// *-
	ELSE{
		DSDWORD[EBX] = tk_mult;
		next=0; // *
	}
	$JMP	ScEx
Jmp_Div:				// /
	NextChar();
	if(cha=='*'){ // �����������
		NextChar();
		useme = 1;	// � useme - ������� ��������� ������������
		WHILE(useme>0){
			WhiteSpaces();
			IF( cha == '*' ){
				NextChar();
				IF(cha == '/' ){	// �������� �����������
					IF(useme > 0)useme--; // �������� �������
					NextChar();
				}
			}
			ELSE{
				IF( cha == '/' ){
					NextChar();
					IF( cha == '*' ){ // ��������� �����������
						useme++;
						NextChar();
					}
				}
				ELSE	// �� ������������ �����������
					NextChar();
			}
		}
		IF(endoffile){
			EAX=tok4;
			DSDWORD[EAX]=tk_eof;
			IF(useme > 0)&&(displaytokerrors)unexpectedeof();
		}
		ELSE TokScan(tok4,type4,src4,post4,string4,number4);
	}
	ELSE IF(cha=='/'){	// ����������� �� ����� ������ //
		do{
			NextChar();
			IF(endoffile)BREAK;
		} while(cha!=10 );
		IF(endoffile){
			EAX=tok4;
			DSDWORD[EAX]=tk_eof;
		}
		ELSE{
			WhiteSpaces();
			TokScan(tok4,type4,src4,post4,string4,number4);
		}
	}
	ELSE{
		WhiteSpaces();
		IF(cha=='-'){
			EAX=tok4; DSDWORD[EAX]=tk_divminus; // /-
			NextChar();
		}
		ELSE{
			EAX=tok4;
			DSDWORD[EAX]=tk_div;	// /
		}
	}
	next = 0;
	$JMP ScEx
Jmp_Mod:				// %
	NextChar();
	WhiteSpaces();
	IF(cha == '-'){
		EAX=tok4;
		DSDWORD[EAX] = tk_modminus; // %-
	}
	ELSE{
		EAX=tok4;
		DSDWORD[EAX]=tk_mod; next=0;
	}
	$JMP	ScEx
Jmp_Or: 				// |
	NextChar();
	IF(cha=='='){
		EAX=tok4;
		DSDWORD[EAX]=tk_orequals; // |=
	}
	ELSE IF(cha=='|'){
		EAX=tok4;
		DSDWORD[EAX]=tk_oror; 	// ||
	}
	ELSE{
		WhiteSpaces();
		IF(cha=='-'){
			EAX=tok4;
			DSDWORD[EAX]=tk_orminus;	// |-
		}
		ELSE{
			EAX=tok4;
			DSDWORD[EAX]=tk_or;
			next=0; // |
		}
	}
	$JMP	ScEx
Jmp_And:				// &
	NextChar();
	IF(cha=='='){
		EAX=tok4;
		DSDWORD[EAX]=tk_andequals;	// &=
	}
	ELSE IF(cha=='&'){
		EAX=tok4;
		DSDWORD[EAX]=tk_andand; // &&
	}
	ELSE{
		WhiteSpaces();
		IF(cha == '-'){
			EAX=tok4;
			DSDWORD[EAX]=tk_andminus; // &-
		}
		ELSE{
			EAX=tok4;
			DSDWORD[EAX]=tk_and;
			next=0;// &
		}
	}
	$JMP	ScEx
Jmp_Not:				// !
	NextChar();
	IF(cha == '='){
		EAX=tok4;
		DSDWORD[EAX]=tk_notequal; // !=
	}
	ELSE{
		EAX=tok4;
		DSDWORD[EAX]=tk_not;
		next=0; // !
	}
	$JMP	ScEx
Jmp_Xor:				// ^
	NextChar();
	IF(cha == '='){
		EAX=tok4;
		DSDWORD[EAX]=tk_xorequals;	// ^=
	}
	ELSE{
		WhiteSpaces();
		IF(cha == '-'){
			EAX=tok4;
			DSDWORD[EAX]=tk_xorminus; // ^-
		}
		ELSE{
			EAX=tok4;
			DSDWORD[EAX]=tk_xor;
			next=0; // ^
		}
	}
	$JMP	ScEx
Jmp_Equal:				// =
	NextChar();
	IF(cha == '='){
		EAX=tok4;
		DSDWORD[EAX]=tk_equalto;	// ==
	}
	ELSE{
		EAX=tok4;
		DSDWORD[EAX]=tk_assign;
		next=0; // =
	}
	$JMP	ScEx
Jmp_Great:				// >
	NextChar();
	IF(cha=='>'){
		NextChar();
		IF( cha == '=' ){
			EAX=tok4;
			DSDWORD[EAX]=tk_rrequals; // >>=
		}
		ELSE{
			WhiteSpaces();
			IF(cha == '-'){
				EAX=tok4;
				DSDWORD[EAX]=tk_rrminus;	// >>-
			}
			ELSE{
				EAX=tok4;
				DSDWORD[EAX]=tk_rr;
				next=0;// >>
			}
		}
	}
	ELSE IF(cha=='<'){
		EAX=tok4;
		DSDWORD[EAX]=tk_swap; 	// ><
	}
	ELSE IF(cha=='='){
		EAX=tok4;
		DSDWORD[EAX]=tk_greaterequal; // >=
	}
	ELSE{
		EAX=tok4;
		DSDWORD[EAX]=tk_greater;
		next= 0; // >
	}
	GOTO ScEx;
Jmp_Less: 			// <
	NextChar();
	IF(cha=='<'){
		NextChar();
		IF(cha=='='){
			EAX=tok4;
			DSDWORD[EAX]=tk_llequals; // <<=
		}
		ELSE{
			WhiteSpaces();
			IF(cha == '-'){
				EAX=tok4;
				DSDWORD[EAX]=tk_llminus;	// <<-
			}
			ELSE{
				EAX=tok4;
				DSDWORD[EAX]=tk_ll;
				next=0;
			}
		}
	}
	ELSE IF(cha=='>'){
		EAX=tok4;
		DSDWORD[EAX]=tk_notequal; // <>
	}
	ELSE IF(cha=='='){
		EAX=tok4;
		DSDWORD[EAX]=tk_lessequal;	// <=
	}
	ELSE{
		EAX=tok4;
		DSDWORD[EAX]=tk_less;
		next=0; // <
	}
ScEx:
	IF(next)NextChar();
}

// '#','\"','\'','-','+','*','/','%','|','&','!','^','=','>','<','@'
dword Jmp_Delim1={#Jmp_Number,#Jmp_String,#Jmp_Const,#Jmp_Minus,
#Jmp_Plus,#Jmp_Mul,#Jmp_Div,#Jmp_Mod,#Jmp_Or,#Jmp_And,
#Jmp_Not,#Jmp_Xor,#Jmp_Equal,#Jmp_Great,#Jmp_Less,#Jmp_Local};

//								':',				';',				 '(', 					 ')',
byte tk_delim2={tk_colon,tk_semicolon,tk_openbracket,tk_closebracket,
//		'{',				 '}', 				'[',					']',			 ',',
tk_openbrace,tk_closebrace,tk_openblock,tk_closeblock,tk_comma,
//'.',			'$',			'?',			 '~'
tk_period,tk_dollar,tk_question,tk_tilda};

