﻿dword ACTUALMNEMDESC; // Указатель на описание текущей мнемоники
byte ACTUALMNEMONIC;	// Код текущей мнемоники
byte OPERANDSIZE; // Размер операнда(B,W,D,BW,WB,DW)
byte OPDESC[3]; 	// Типы операндов (R,E,D,CC,SR,TR,DR,CR)
byte OPDATA[3]; 	// Информация по операнду (RG,CO,EA)
dword OPCONST[3]; // Значение константы в операнде
byte OPCONSTFLAG[3];	// Наличие константы в операнде
byte OPCONSTSIZE[3];	// Размерность константы в операнде
byte OPPOST[3];
dword OPPOSTREF[3];
byte PFLAG; 	// Значение P-флага (номер класса).
byte RANDFLAG;		// TRUE: RAND: требуется префикс 0x66
byte ADDRFLAG;		// TRUE: ADDR: требуется префикс 0x67
byte WBIT;		// Бит WORD в OPCODE
byte DBIT;		// Бит DIRECTION в OPCODE
byte OPLENGTH;		// Количество операндов в текущей инструкции
byte EA_M;		// Байт XRM
byte EA_X;
byte EA_R;		// R NIBBLE IN XRM BYTE.
byte EA_S;		//SEGMENT REGISTER
byte EA_R2; 	//BASE REGISTER
byte EA_I;		//INDEX REGISTER
byte EADESC[2]; 	//CONTAINS INFORMATION ABOUT AN PARSED EFFECTIVE ADRESS.
byte EALENGTH;		// Количество регистров в EA
byte EA_SIBFLAG;	// Флаг наличия Sib в 32-битном EA
byte EA_SCALING;	// Индекс в Sib
dword EA_SCALE; 	// Значение для Scale в Sib
dword EADescInd;	// Индекс в EADESC
byte SEGREGISTER; // SEGMENT OVERRIDE PREFIX NEEDED OR 0.
byte SYSRNUM; 	// CONTROL/DEBUG/TASKREG INDEX
byte SYSRTYPE;		// SYSTEM REGISTER TYPE (CR,CR4,DR,TR)
byte SYSRCODE;		// ENCODED REGISTER TYPE
byte OVERRIDE;		//SEGMENT OVERRIDE PREFIX NEEDED OR 0.
dword opDescInd;
dword prevTok;
// ----- Ассемблирование строки

Asm(dword str)
byte holdcha;
byte source[256],s[STRLEN],s2[STRLEN];
{
	lstrcpyA(#source,str);
	IF(list){
		fprint(mapfile,"\t//\t");
		fprint(mapfile,str);
		fprint(mapfile,"\n");
	}
	holdcha=cha2; //запомнить позицию разбора
	$PUSH linenum2,inptr2,number,tok2,tok,input,inptr,currmod,linenumber,
		endoffile,displaytokerrors,type;
	lstrcpyA(#s,#string);
	lstrcpyA(#s2,#string2);
	input=#source;	//разбираем новую строку
	inptr = input;
	inptr2=input;
	endoffile=0;	// На начале файла
	NextChar();
	cha2 = cha;
	inptr2=inptr;
	linenum2 = 1;
	for(;;){
		NextTok();
		IF(tok==tk_mnemonics)DoMnemonics();
		ELSE IF(tok==tk_eof)BREAK;
		ELSE IF(tok==tk_locallabel)DoLocalPost();
		ELSE IF(tok!=tk_semicolon)preerror("ASM: Bad input format\n");
	}
	lstrcpyA(#string,#s); //востановить
	lstrcpyA(#string2,#s2);
	$POP type,displaytokerrors,endoffile,linenumber,currmod,inptr,input,
			tok,tok2,number,inptr2,linenum2;
	cha2=holdcha;
}

DoLocalPost()
dword i;
{
	tok = tk_number;
	number = outptr-output+OptImageBase+OptBaseOfCode;
	ESI=localptr;
	DSDWORD[ESI+localtok] = tok;
	DSDWORD[ESI+localnumber] = number;
	i=0;
	WHILE(i<posts){
		ECX=i<<2+postnum;
		EAX=DSDWORD[ECX];
		IF(EAX==localptr){
			DSDWORD[ECX]=number;
			EBX=i<<2+posttype;
			DSDWORD[EBX]=POST_LOC;
		}
		i++;
	}
	NextTok();
	NextTok();
}

// ---- Установка ссылки на пока необъявленный идентификатор
SetPost(dword ref,ptype)
{
	IF(posts >= MAXPOSTS)maxwordpostserror();
	EBX=posts<<2+postloc;
	DSDWORD[EBX] = outptr;
	EBX=posts<<2+postnum;
	DSDWORD[EBX] = ref;
	EBX=posts<<2+posttype;
	DSDWORD[EBX] = ptype;
	posts++;
}

// ---- Пропуск до следующей записи в шаблоне инструкции
SCDEND()
{
	$LODSB
	IF(AL!=0){
		$CMP AL,_END
		$JNE SCDEND
		illegaloperand();
		$POP EAX; 	// Выход из MapOperands
	}
}

GETSCALING()
{
	NextTok();		// Получим значение масштабного коэффициента
	IF(tok==tk_number){
DoScale:
		EA_SCALE=number;	// Scale
		EA_SIBFLAG=1; // Отметим наличие Sib в 32-битном EA
	}
	ELSE preerror("ASM: Illegal scaling value\n");
}

// ---- Поиск в шаблоне подходящей к операнду записи (для однооперандной инструкции)
GETOL1()
{
	if(DSBYTE[ESI]!=0){	// Инструкция JMP - не проверяем совпадение размеров
G4:
		$LODSB
		AH=AL&NOLBITS;		// Исключим число операндов
		if(NOTZEROFLAG){// Это все-таки JMP...
			DL=AL&OPBITS; 	// Маска для типа операнда
			IF(DL!=SOO){ 	// В поле размера операнда содержится информация о типе операнда?
				AL&=OLBITS; 	// Выделим количество операндов
				IF(AL!=OL1){ 	// Шаблон для инструкции с одним операндом?
G3:
					do{
						$LODSB			// Просмотрим следующую запись шаблона
						IF(AL==0)GOTO G4; 	// Конец записи?
					}while(AL!=_END);		// Это конец шаблона?
					illegaloperand(); // Нет подходящей записи для такой мнемоники
					$POP EAX;
					$RET		// Выход из MapOperands
				}
	// Обработка записи шаблона
G2:
				AL=AH&TRANSBITS;		// Выделим размер операнда
				$CMP AL,_D
				$JBE G5 		// Операнд м.б. размера: byte, word, dword
				$CMP AL,_OW
				$JNE J0
				OPERANDSIZE=_W;
				GOTO G40;
J0:
				$CMP AL,_OD;
				$JNE J1
				OPERANDSIZE=_D;
				GOTO G40;
J1:
				$CMP AL,NB
				$JB G90 		// Операнд м.б. размера WB,DW OR BW.
				AL-=NB; 		// Пересчитаем для фиксированных размеров: NB->B,OW->W, AF->D
G90:
				$CMP AL,OPERANDSIZE // Размер операнда и размер из шаблона совпали?
				$JNE G3 		// Нет - смотрим следующую запись в шаблоне
				GOTO G40; 		// Размеры совпали - продолжим
G5:
				$CMP AL,OPERANDSIZE // Размер операнда и размер из шаблона совпали?
				$JA G3			// Нет - смотрим следующую запись в шаблоне
				EBX=0;			// WBIT/RANDFLAG=0
				DL=AL;
				AL=OPERANDSIZE;
				IF(DL==_B)&&(AL!=_B)BL++;
//				$CMP DL,_B		// В шаблоне byte?
//				$JNE G20
//				$CMP AL,_B		// Операнд byte?
//				$JE G20
//				BL++; 		// W-бит=TRUE
//G20:
				$CMP AL,_W		// В шаблоне word?
				$JNE G30
				$JA G30
				BH++; 		// Операнд обязательно д.б. word - требуется префикс RAND
G30:
				WBIT=BL;
				RANDFLAG=BH;	// Запомним информацию о префиксе и W-бите
G40:
				AH&=NTRANSBITS; 	// Оставим SOO биты
			}
		}
	}
	AL=AH;			// Возвратим SOO поле
}

// ---- Разборка шаблона для двухоперандной инструкции
GETOL2()
{
G7:
	$LODSB			// Получим байт из шаблона
	AH=AL;
	AL&=OLBITS; 	// Выделим число операндов
	$CMP AL,OL2 	// Число операндов = 2?
	$JE G8			// Да - начнем проверку
G9:
	$LODSB			// Поиск следующей записи в шаблоне
	$OR AL,AL 	// Конец записи?
	$JZ G7			// Да - проверим новую запись
	$CMP AL,_END		// Конец шаблона?
	$JNE G9 		// Смотри дальше иначе - ошибка
	toomuchoperands();
	$POP EAX;
	$RET		// Выход из MapOperands
G8:
	AH&=NOLBITS;		// Исключим число операндов
	AL=AH;
	AL&=TRANSBITS;		// Выделим размер операнда
	$CMP AL,_D
	$JBE G100 	// Операнд м.б. размера: byte, word, dword
G94:
	$CMP AL,NB
	$JB J0	//G95 		// Операнд м.б. размера WB,DW OR BW.
	AL-=NB; // Пересчитаем для фиксированных размеров: NB->B,OW->W, AF->D
G95:
	$CMP AL,OPERANDSIZE // Размер операнда и размер из шаблона совпали?
	$JNE G9 		// Размеры не совпали - ищем следующую запись
	$JMP G11			// Размеры совпали - продолжим
J0:
	$CMP OPDESC[0],CO
	$JNE J1
	$CMP AL,WB;
	$JNE J1
	OPCONSTSIZE[0]=_W;
	OPCONSTSIZE[1]=_B;
	GOTO G11;
J1:
	$CMP AL,_DW;
	$JNE J2;
	RANDFLAG=0;
	OPCONSTSIZE[0]=_D;// OPCONSTSIZE[1]=_W;
	GOTO G11;
J2:
	$CMP AL,BW;
	$JNE G95
	OPCONSTSIZE[0]=_B;
	OPCONSTSIZE[1]=_W;
	GOTO G11;
G100:
	$CMP OPERANDSIZE,_D
	$JA NEAR G9 		// Размер операнда > dword - на следующую запись
	$CMP OPERANDSIZE,AL
	$JB NEAR G9 		// Размеры не совпали - ищем следующую запись
	EBX=0;
	DL=AL;
	AL=OPERANDSIZE;
	$CMP DL,_B		// Размер в шаблоне = byte?
	$JNE G50
	$CMP AL,_B		// Размер операнда = byte?
	$JE G50
	BL++; 		// W-бит=TRUE
G50:
	$CMP AL,_W		// В шаблоне word?
	$JNE G60
	$JA G60
	BH++; 		// Операнд обязательно д.б. word - требуется префикс RAND
G60:
	WBIT=BL;
	RANDFLAG=BH;
G11:
	AH&=NTRANSBITS;
	AL=AH;	// Возвратим SOO поле
}

// ---- Разборка шаблона для трехоперандной инструкции
GETOL3()
{
G12:
	$LODSB
	AH=AL;
	AL&=OLBITS;
	$CMP AL,OL3
	$JE G13
G14:
	$LODSB			//TRY NEXT ENTRY.
	$OR AL,AL
	$JZ G12
	$CMP AL,_END
	$JNE G14
	toomuchoperands();
	$POP EAX;
	$RET		// Выход из MapOperands
G13:
	AH&=NOLBITS;
	$CMP OPERANDSIZE,_D //DWORD ?
	$JE G15
	$CMP OPERANDSIZE,_W //WORD ?
	$JE G16
	preerror("ASM: This instruction required a WORD/DWORD operand\n");
	$RET
G16:
	RANDFLAG=1;
G15:
	AL=AH&0xE0;
}

// ----
CREATE_EA()
{
	EA_M=AL&7;
	EA_X=3;
}

// ----
CREATE_R()
{
	EA_R=AL&7;
}

// ---- Генеррация ModRM и Sib
GETMODRMBYTE()
{
	DL=EALENGTH;		// Количество регистров в EA
	$OR DL,DL 	// Нет регистров в EA?
	$JE NEAR C11		// Да
	$TEST EADESC,_W+_D*8
	$JE NEAR E1 						 // 8-битные регистры нельзя применать в адресе
	$TEST EADESC,_W*8 			// 16-битный регистр?
	$JNE NEAR E4		// 16-битная адресация не разрешена
	GETEADISPX();
	$CMP DH,2
	$JNZ X00
	EAX=opDescInd;
	OPCONSTSIZE[EAX]=_D; // Обязательно 32-битный disp
X00:
	DL--; 	// 1 регистр?
	$JNE N1 	// нет...
	AL=EADESC&7;
	$CMP EA_SIBFLAG,1 // Флаг наличия Sib в 32-битном EA
	$JNE L2 		// Нет Sib
	EA_R2=5;		// Фиксируем базовый регистр
	EA_M=4;
	EA_I=AL;
	EDX=opDescInd;
	$CMP OPCONSTFLAG[EDX],1 // Используем disp?
	$JE L1
	EAX=0;
	OPCONSTFLAG[EDX]=1;
	EDX<<=2;
	OPCONST[EDX]=EAX; // disp=0 в EA
L1:
	EDX=opDescInd;
	OPCONSTSIZE[EDX]=_D;
	EA_X=0; //EA_X=AL;
	$RET
L2:
	EA_M=AL;
	$RET
N1:
	EA_M=4; 					//2 REGISTERS USED.
	EA_SIBFLAG=1;
	AL=EADESC[1]>>3;
	$CMP AL,_W
	$JE E5									//ERROR: INDEX REGISTER ISN'T OF SIZE DWORD
	AL=EADESC;
	AH=EADESC[1];
	EAX&=0x707;
	$CMP AH,5 							 //CAN'T USE BP AS INDEX.
	$JE E6
	EA_R2=AL;
	EA_I=AH;
	$RET
E1:
	preerror("ASM: You can't use byte registers in addresses\n");
	$RET
E4:
	preerror("ASM: 16-bit addressing mode not allowed\n");
	$RET
E5:
	preerror("ASM: You must use a 32-bit registers for scaling\n");
	$RET
E6:
	preerror("ASM: You can't use EBP as an index\n");
	$RET
C11:
	EA_X=0;
	EA_M=5;
	ECX=opDescInd;
	AL=OPCONSTSIZE[ECX];
	IF(AL==_B)OPCONSTSIZE[ECX]=_D;
	ELSE IF(AL==_W)ADDRFLAG=1;
}

// ----
GETEADISPX()
{ 									 //CREATE X NIBBLE OF DISPLACEMENT SIZE.
	DH=0;
	$PUSH ECX
	ECX=opDescInd;
	IF(OPCONSTFLAG[ECX]==1){
		AL=OPCONSTSIZE[ECX];
		DH=2; 	//(D)WORD DISPLACEMENT
		IF(AL==_B)DH--; 								 //SBYTE DISPLACEMENT
	}
	EA_X=DH;
	$POP ECX
}

// ---- Инициализация буфера ассемблера
INIT_LINE()
{
	ECX=#opDescInd-#OPERANDSIZE;
	AL=0;
	EDI=#OPERANDSIZE;
	$REP $STOSB;
	AL=255;
	OPERANDSIZE=AL;
	SEGREGISTER=AL;
}

// ---- Запись переопределения сегмента
WRITEOVERRIDE()
{
	EBX=OVERRIDE;
	IF(BL!=0){
		AL=OVERRIDETAB[EBX]-rES;
		OP();
	}
}

// ---- Запись префикса размерности операнда
WRITERAND()
{
	$PUSH EAX
	IF(RANDFLAG==1){
		AL=0x66;
		OP();
	}
	$POP EAX
}

// ---- Запись константы: CL=TYPE; EDI указатель на значение
WRITECONST()
{
	IF(CL==_B)CL=1;
	ELSE IF(CL==_W)CL=2;
	ELSE IF(CL==_D)CL=4;
	ELSE CL++;
	loop(ECX){
		AL=DSBYTE[EDI];
		EDI++;
		OP();
	}
}

// ---- Обработка Override
GETOVERRIDE()
{
	IF(tok==tk_seg)&&(tok2==tk_colon){
		IF(OVERRIDE==0){
			OVERRIDE=number;
			$STC	// ∙ сегментного регистра
		}
		ELSE preerror("ASM: Double segment override");
		NextTok();
		NextTok();	// Пропускаем :
	}
}

// ---- Вычисление размера операнда: _B,_W,_D,WB,_DW & RAND-FLAG. AL=SIZE
DEF_OPSIZE()
{
	AH=OPERANDSIZE;
	IF(AH==255){
		OPERANDSIZE=AL; // Один операнд
		return;
	}
	IF(AH==AL)return; 	// Размеры совпадают
	IF(AX==0X100){	// RW,RB ?
		RANDFLAG=1;// OPERANDSIZE=WB;
		return;
	}
	IF(AX==0X200){	// RD,RB ?
		IF(ACTUALMNEMDESC==#PCOMMANDS3_){
//	OPERANDSIZE=_D;
			return;
		}
		OPERANDSIZE=WB;
		return;
	}
	IF(AX==0X201){	//RD,RW
		RANDFLAG=1;
		OPERANDSIZE=_DW;
	}
}

// ---- Запись префикса адресации
WRITEADDR()
{
	$PUSH EAX
	IF(ADDRFLAG==1){
		AL=0x67;
		OP();
	}
	$POP EAX
}

// ---- Определение размерности константы
DefConstSize()
{ 		// Определим размерность константы
	EBX=opDescInd;
	DL=_D;
	IF(OPPOST[EBX]==0){
		EBX=opDescInd<<2;
		EAX=OPCONST[EBX];
		DL=_B;			// byte
		IF(long EAX>=-128){ // -128
			$CMP EAX,0XFF 	// 255
			$JNG W2
		}
		DL++; 		// _W - word
		IF(long EAX>=-32768){
			$CMP EAX,0XFFFF 	// 65535
			$JNG W2
		}
		DL++; 		// _D - dword
	}
W2:
	EBX=opDescInd;
	OPCONSTSIZE[EBX]=DL;
	OPCONSTFLAG[EBX]=1;
}

// ---- Обработка мнемоники ассемблера
DoMnemonics()
{
	opDescInd=0;
	EADescInd=0;
	INIT_LINE();	// Очистка буферов
	IF(number<24){
		IF(number<8)EBX=#PCOMMANDS1;
		ELSE IF(number<12){
			number-=8;
			EBX=#PCOMMANDS2;
		}
		ELSE IF(number<20){
			number-=12;
			EBX=#PCOMMANDS3;
		}
		ELSE{
			number-=20;
			EBX=#PCOMMANDS4;
		}
		number+=DSBYTE[EBX];
		PFLAG=number;
		EBX++;
	}
	ELSE{
		number-=24;
		EBX=number<<2;
		EBX=TAB_MNEMONICS[EBX];
		IF(EBX>=#T_DAA)&&(EBX<#T_NOT){	// Проверим на наличие инструкции без операндов
			ACTUALMNEMDESC=EBX;
			IF(tok2==tk_semicolon)NextTok();
			ESI=ACTUALMNEMDESC;
			$JMP CreateCode;// Генерация кода для инструкции без операндов
		}
	}
	ACTUALMNEMDESC=EBX; 	// Запомним указатель на текущий шаблон мнемоники
	for(;;){		// Цикл анализа операндов
		prevTok=tok;
		NextTok();		// Следующий операнд
		FastSearch(#string,#St_Sizes);// Это размер операнда?
		IF(CARRYFLAG){	// Да: byte,word или dword
			OPERANDSIZE=AL; // Запомним _B,_W,_D
			continue;
		}
		GETOVERRIDE();	// Обработка конструкции SEG:
ContLine: 	// Точка для продолжения обработки текущего
		IF(tok==tk_eof)||(tok==tk_semicolon){
			EBX=opDescInd;
			IF(OPDESC[EBX]==E){ // Обработка в EA?
				DefConstSize();
				GETMODRMBYTE(); // EOL - GENERATE MODRM OPCODE BYTE.
			}
			IF(prevTok!=tk_mnemonics){		// Были операнды
				OPLENGTH++;
				IF(OPERANDSIZE==255){
					OPERANDSIZE=_D; 	// Принудительно установим dword
				}
			}
			$JMP MapOperands
		}
		else IF(tok==tk_comma){
			IF(opDescInd==3){
				toomuchoperands();
				break;
			}
			EBX=opDescInd;
			IF(OPDESC[EBX]==E){ // Обработка в EA?
				DefConstSize();
				GETMODRMBYTE(); // EOL - GENERATE MODRM OPCODE BYTE.
			}
			opDescInd++;
			OPLENGTH++;
		}
		else IF(tok==tk_openblock){
			EBX=opDescInd;
			OPDESC[EBX]=E;	// Отметим, что работаем с EA операндом
		}
		else IF(tok==tk_closeblock){	// ]
			DefConstSize();
			GETMODRMBYTE(); // EOL - GENERATE MODRM OPCODE BYTE.
		}
		else IF(tok==tk_minus){
			IF(tok2 == tk_number){
				NextTok();
				number = -number;
				$JMP ContLine;	// Продолжим docase без выборки след.token
			}
		}
		else IF(tok==tk_plus) continue;
		else IF(tok==tk_mult){		// *
			GETSCALING(); // Указан масшаб в Sib
		}
		else if(tok==tk_reg){ // Обработка регистра
G0:
			EBX=opDescInd;
			IF(OPDESC[EBX]==E){ // Обработка в EA?
				IF(type==tk_byte){
					preerror("ASM: No byte register in address\n");
					return;
				}
				IF(EALENGTH<2){ // Количество регистров в EA < 2 ?
					EALENGTH++; // Отметим, что есть еще один регистр в EA
					EBX=EADescInd;
					EADESC[EBX]=number; // Запомним ∙ регистра
					EADescInd++;
				}
				ELSE{ 	// Слишком много регистров в EA
					preerror("ASM: too much registers in combination\n");
					return;
				}
			}
			ELSE{
				OPDATA[EBX]=number; // ∙ регистра
				OPDESC[EBX]=R;
				AH=number&7;
				EA_R=AH;
				IF(opDescInd!=2){
					AL>>=3;
					DEF_OPSIZE();
				}
			}
		}
		else IF(tok==tk_number) { // Обработка константы
			IF(tok2==tk_mult){
				DoScale();
				NextTok();
				continue;
			}
NUM:
			EBX=opDescInd<<2;
			OPCONST[EBX]+=number; // Запомним константу
			DefConstSize(); 	// Определим размерность константы
			IF(OPDESC[EBX]!=E)		// Константа в EA?
				OPDESC[EBX]=CO;
		}
		else IF(tok==tk_postnumber){
			EBX=opDescInd;
			OPPOST[EBX]=POST_DATA;
			EBX<<=2;
			OPPOSTREF[EBX]=treeptr;
			ESI=treeptr;
			DSDWORD[ESI+recpost]++;
			GOTO NUM;
		}
		else IF(tok==tk_proc){
			IF(post){
				EBX=opDescInd;
				OPPOST[EBX]=POST_CALL;
				EBX<<=2;
				OPPOSTREF[EBX]=treeptr;
				ESI=treeptr;
				DSDWORD[ESI+recpost]++;
			}
			$JMP NUM
		}
		else IF(tok==tk_locallabel){
			EBX=opDescInd<<2;
			OPPOSTREF[EBX]=localptr;
I2:
			EBX=opDescInd;
			$CMP ACTUALMNEMDESC,#T_JCXZ;
			$JB I1
			$CMP ACTUALMNEMDESC,#T_CALLFAR;
			$JA I1
			OPPOST[EBX]=POST_CALL;
			$JMP NUM
I1:
			OPPOST[EBX]=POST_DATA;
			AL=_D;
			DEF_OPSIZE();
			$JMP PARSE_EA1
		}
		else IF(tok==tk_undefproc){
I0:
			EBX=opDescInd<<2;
			OPPOSTREF[EBX]=treeptr;
			GOTO I2;
		}
		else IF(tok==tk_id){
			tok = tk_undefproc;
			post = 1;
			number=0;
			AddToTree(#string);
			GOTO I0;
		}
		else IF(tok==tk_var){ // Инициализация EA с константой: EA+disp
			AL=type-tk_byte>>1; // AL=размер адресуемого операнда (_B,_W,_D)
			DEF_OPSIZE();
			EBX=opDescInd;
			IF(post){
				EBX<<=2;
				OPPOSTREF[EBX]=treeptr;
				EBX=opDescInd;
				OPPOST[EBX]=POST_DATA;
				ESI=treeptr;
				DSDWORD[ESI+recpost]++;
			}
PARSE_EA1:
			OPDESC[EBX]=E;
			OPCONSTFLAG[EBX]=1; // Отметим, что работаем с EA операндом
			EBX<<=2;
			OPCONST[EBX]+=number; // Запомним адрес
		}
		else IF(tok==tk_seg){
			EBX=opDescInd;
			OPDATA[EBX]=number-rES; // ∙ сегментного регистра
			SEGREGISTER=AL;
			AL<<=3; // Создать код для XSM поля
			EA_S=AL;
			OPDESC[EBX]=_SR;
			AL=_W;
			DEF_OPSIZE();
		}
		else IF(tok==tk_param)||(tok==tk_local){
PARSE_PAR:
			AL=type-tk_byte>>1;
			DEF_OPSIZE();
			EBX=opDescInd;
			OPDESC[EBX]=E;
			EBX<<=2;	// Отметим, что работаем с EA операндом
			OPCONST[EBX]+=number; // Запомним адрес
			OPCONSTFLAG[EBX]=1;
			number=rEBP;
			$JMP G0;
		}
		else IF(tok==tk_controlreg){
			EBX=opDescInd;
			OPDESC[EBX]=SYSR;
			SYSRNUM=number;
			IF(AL==4)SYSRTYPE=_CR4;
			ELSE{
				SYSRTYPE=_CR;
				SYSRCODE=0;
			}
		}
		ELSE IF(tok==tk_debugreg){
			EBX=opDescInd;
			OPDESC[EBX]=SYSR;
			SYSRNUM=number;
			SYSRTYPE=_DR;
			SYSRCODE=1;
		}
		ELSE IF(tok==tk_testreg){
			EBX=opDescInd;
			OPDESC[EBX]=SYSR;
			SYSRNUM=number;
			SYSRTYPE=_TR;
			SYSRCODE=4;
		}
		ELSE preerror("ASM: Syntax error\n");
	}
}

CreateScale()
{
	IF(ADDRFLAG)return;
	if(EA_SIBFLAG){ 	// Флаг наличия Sib в 32-битном EA
		IF(EA_SCALE==0){	// Отсутствует
			EA_SCALING=0;
		 }
		else IF(EA_SCALE==1)EA_SCALING=0;
		else IF(EA_SCALE==2)EA_SCALING=0x40;
		ELSE IF(EA_SCALE==4)EA_SCALING=0x80;
		ELSE IF(EA_SCALE==8)EA_SCALING=0xC0;
		ELSE{
			EA_SCALING=0;
			IF(EA_SCALE>255)OP(byte 0x69);
			ELSE OP(byte 0x6B);
			AL=EA_I<<3|EA_I|0xC0;
			OP(byte AL);
			IF(EA_SCALE>255)OUTDWORD(EA_SCALE);
			ELSE OP(byte EA_SCALE);
		}
	}
}

// ---- Генерация кода. ESI=ptr на запись о описание мнемоники (запись типа T_...)
CreateCode()
{
	WRITEOVERRIDE();
	CreateScale();
	IF(ADDRFLAG==1){	//ADDR: PREFIX ?
		OP(byte 0x67);
	}
	IF(RANDFLAG==1){	//RAND: PREFIX ?
		OP(byte 0x66);
	}
	EDI=ESI;
	IF(ACTUALMNEMDESC==#T_TEST)DBIT=0;	//DON'T ADD ANYTHING IF TESTING
	$SHL DBIT,1 			 //SHIFT D-BIT TO THE RIGHT POSITION.
NEXT_DESC_BYTE: 	// Обработка байта из дескриптора мнемоники
	EBX=0;
	BL=DSBYTE[EDI];
	EDI++;
NB3:
	$CMP BL,X7M
	$JA NC3
	$CMP BL,X0M
	$JB N24
	AH=BL-X0M;
	AL=EA_X<<3|AH<<3|EA_M;
	OP();
	GOTO NEXT_DESC_BYTE;
N24:
	EBX<<=2;
	EBX+=#Dsc_Jump;
	$JMP NEAR DSDWORD[EBX]
NC3:
	$CMP BL,_END
	$JNE E42
	$JMP CreateConstants	// Конец шаблона для мнемоники
E42:
	preerror("Descriptor damaged\n");
	return;
// OpCode - 1 байт
Dsc_O:
	AL=DSBYTE[EDI]+WBIT+DBIT;
	EDI++;
	OP();
	GOTO NEXT_DESC_BYTE;
// OpCode - 1 слово
Dsc_OW:
	AL=DSBYTE[EDI];
	EDI++;
	OP();
	AL=DSBYTE[EDI]+WBIT+DBIT;
	EDI++;
	OP();
	$JMP NEXT_DESC_BYTE
// OpCode - 1 байт и следующий байт, заданный 8-ричной строкой
Dsc_OS:
	AL=DSBYTE[EDI];
	EDI++;
	OP();
// OpCode - 8-ричная строка с кодом
Dsc_S:
S01:
	CL=3;
	EAX=0;
	loop(CL){
		AL=DSBYTE[EDI];
		EDI++;
		IF(AL=='X'){		//X CHAR
			AL=EA_X;
		}
		ELSE IF(AL=='R')AL=EA_R;
		ELSE IF(AL=='M')AL=EA_M;
		ELSE IF(AL=='S')AL=SEGREGISTER;
		ELSE IF(AL=='N')AL=SYSRNUM;
		ELSE IF(AL=='P')AL=PFLAG;
		ELSE AL-='0';
		AH=AH<<3|AL;
	}
	AL=AH+DBIT+WBIT;
N15:
	OP();
	$JMP NEXT_DESC_BYTE
// OpCode - ModRM байт
Dsc_XRM:
	AL=EA_X<<3|EA_R<<3|EA_M;		// Получить регистр из описания мнемоники
	GOTO N15;
// OpCode - ModRM с P-флагом (арифметические инструкции)
Dsc_XPM:
	AL=EA_X<<3|PFLAG<<3|EA_M;
	GOTO N15;
// OpCode - ModRM с сегментным регистром
Dsc_XSM:
	AL=EA_X<<3|SEGREGISTER<<3|EA_M;
	GOTO N15;
//	JMP NEXT_DESC_BYTE
}

// ---- Разбор закончен -> на генерацию кода по шаблону
MapOperands()
{
//	AL=0; WBIT=AL; DBIT=AL;
	opDescInd=0;
	ESI=ACTUALMNEMDESC; // Указатель на информацию по генерации
	AL=OPLENGTH;		// Количество операндов
	ECX=#OPDESC;		// Указатель на информацию об операндах
	ECX=DSDWORD[ECX];   // Типа нуликовый элемент берем
	IF(ESI!=#T_MOV){		// Инструкция MOV?
		IF(AL!=0){
			$CMP AL,1
			$JE NEAR ONEOP		// Инструкция с одним операндом
			$CMP AL,2
			$JE NEAR TWOOPS 	// Инструкция с двумя операндами
			$CMP AL,3
			$JE NEAR THREEOPS // Инструкция с тремя операндами
			toomuchoperands();
			return;
		}
// ---- Инструкция без операндов
		do{
			$LODSB
			IF(AL==0)goto CreateCode; 	// Генерация кода
		}while(AL!=_END);
		preerror("ASM: Operand required\n");
		return;
// ---- Генерация MOV инструкции
	}
	$PUSH EAX,ECX
	WRITEOVERRIDE();
	CreateScale();
	$POP ECX,EAX
	IF(AL!=2){		// 2 OPERANDS IN INSTRUCTION?
		preerror("ASM: Two operands required\n");
		return;
L2:
		preerror("ASM: Not same size\n");
		return;
	}
L1:
	BL=0;
	AL=OPERANDSIZE;
	IF(AL!=_D){
		$CMP AL,_W
		$JA L2
		$JNE N4
		RANDFLAG=1;
	}
	BL=1;
N4:
	WBIT=BL;		//STORE W-BIT
	DL=0; 	//CLEAR D-BIT
	WRITEADDR();
	EBX=0;
	BL=CL;
	EBX=EBX<<2+#Jmp_Mov;
	$JMP NEAR DSDWORD[EBX]
Mov_ERR:
	preerror("ASM: a constant can't be used as a destination\n");
	return;
Mov_R:
	EBX=0;
	BL=CH;
	EBX=EBX<<2+#Jmp_Mov_R;
	$JMP NEAR DSDWORD[EBX]
Mov_E:
	EBX=0;
	BL=CH;
	EBX=EBX<<2+#Jmp_Mov_E;
	$JMP NEAR DSDWORD[EBX]
Mov_R2R:
	WRITERAND();
	AL=OPDATA[1];
	AH=OPDATA[0];
	DL=DBIT;
	$PUSH EAX
	AL=0o210+WBIT;
	$SHL DL,1
	AL+=DL; 	//D-BIT
	OP();
	$POP EAX
	AL=AL&7<<3;
	AH&=7;
	AL=AL|AH|0o300;
	OP();
	$JMP CreateConstants
Mov_R2E:
	AL=OPDATA&7;	//AL/AX/EAX ?
	$OR AL,AL
	$JNE N1
	$CMP EA_M,6 //AR,[DW] POSSIBLE?
	$JNE N1 		//NO, ONLY AR,[EA]
	$CMP ADDRFLAG,0 //32BIT-EA ?
	$JE N1		//NO, TRY ANOTHER...
	WRITERAND();
	AL=0o240+WBIT;	//INSTRUCTION FOUND.
	OP();
	$JMP CreateConstants
Mov_E2R:
D1:
	AL=OPDATA[1]&7; //[DW],AR POSSIBLE?
	$OR AL,AL
	$JNE Y1
	$CMP ADDRFLAG,0 //32BIT EA ?
	$JNE Y1 		//YES, RAVE ON...
	$CMP EA_M,6
	$JNE Y1
	WRITERAND();
	AL=0o242+WBIT;		//INSTRUCTION FOUND.
	OP();
	$JMP CreateConstants
N1:
	DL=2; 	//SET D-BIT
Y1:
	DL+=0o210;
	WRITERAND();
	AL=DL;
	DL=0;
	AL+=WBIT;
	OP();
	AL=EA_X<<3|EA_R<<3|EA_M;
	OP();
	$JMP CreateConstants
E1:
	preerror("ASM: Not same size\n");
	return;
//EA,CONSTANT ?
Mov_E2C:
	OPCONSTSIZE[1]=OPERANDSIZE;
	$CMP AL,_D
	$JA E1
	$JE X1
	$CMP AL,_W
	$JNE X1
	AL=0x66;
	OP();
X1:
	AL=0o306+WBIT;
	OP();
	AL=EA_X<<6|EA_M;
	OP();
	$JMP CreateConstants
Mov_R2C:
	OPCONSTSIZE[1]=OPERANDSIZE;
	$CMP OPERANDSIZE,_B
	$JNE N2
	AL=OPDATA&7|0o260;
	OP();
	$JMP CreateConstants
N2:
	$CMP OPERANDSIZE,_D 		 //BYTE, WORD OR DWORD?
	$JA NEAR E1 	// Not same size
	IF(OPERANDSIZE==_W){
		AL=0x66;
		OP();
	}
	AL=OPDATA&7|0o270;
	OP();
	$JMP CreateConstants
E21:
	preerror("ASM: Word required\n");
	return; //SEGMENT REGISTER IS ALWAYS WORD.
Mov_S:
	AL=0;
	$CMP CX,_SR*256+E // mov EA,segreg
	$JE O1
	$CMP CX,_SR*256+R // mov segreg,reg
	$JE O2
	$CMP CX,R*256+_SR // mov reg,segreg
	$JE O3
	$CMP CX,E*256+_SR // mov segreg,EA
	$JNE NEAR N12
	AL=2; 		//SET D-BIT
O1:
	$CMP OPERANDSIZE,_W
	$JNE E21
	AL+=0o214;
	OP();
	AL=EA_X<<6|EA_S|EA_M;
	OP();
	$JMP CreateConstants
O2:
	$CMP OPERANDSIZE,_W
	$JNE E21
	AL=0o214;
	OP();
	AL=EA_S|0o300|EA_R; 	 //CREATE XSM BYTE
	OP();
	$STC
	$RET
O3:
	$CMP OPERANDSIZE,_W
	$JNE NEAR E21
	AL=0o216;
	OP();
	AL=EA_S|0o300|EA_R;
	OP();
	$STC
	$RET
E31:
	preerror("ASM: CR1 only readable\n");
	$RET
E32:
	preerror("ASM: SysR must be dword\n");
	$RET
Mov_SYSR:
N12:
	AH=0o40;
	$CMP CX,SYSR*256+R
	$JE O11
	$CMP CX,R*256+SYSR
	$JNE N22		//ERROR: ILLEGAL OPERANDS
	AH=0o42;
	$CMP SYSRTYPE,_CR //CR1 REGISTER USED?
	$JNE O11
	$CMP SYSRNUM,1
	$JE E31 		//YES, ONLY READ FROM IT.
O11:
	AH+=SYSRCODE;
	$CMP OPERANDSIZE,_D //SYSTEM REGISTERS ARE ALWAYS DWORD.
	$JNE E32
	AL=0o17;
	OP();
	$CMP SYSRTYPE,_CR4	//EXCEPTION: CR4
	$JE N22
	AL=AH;
	OP();
	AL=SYSRNUM<<3|0o300|EA_R; //CREATE 3NR OPCODE
	OP();
	$STC
	$RET
N22:
	$CMP CX,SYSR*256+R
	$JNE N32
	AL=0x22;
	OP();
	GOTO L11;
N32:
	AL=0x20;
	OP();
L11:
	AL=0o340|EA_R;
	OP();
	$STC
	$RET
// ---- Инструкция с одним операндом
ONEOP:
	EBX=CL<<2+#Jmp_Op1;
	$JMP NEAR DSDWORD[EBX]
Op1ERR:
	preerror("ASM: only use system registers within MOV instruction\n");
	$RET
// RX
L31:
	SCDEND();
Op1R:
	GETOL1(); 	//GET FIRST GENERATION INFO.
	$CMP AL,SOR 	//SINGLE OPERAND/REGISTER ?
	$JNE X23
	$JMP CreateCode
X23:
	$CMP AL,SOE 	//CONVERT REGISTER INTO EFFECTIVE ADDRESS?
	$JNE L31
C2:
	EA_X=3;
	AL=EA_R;
	EA_M=AL;
	$JMP CreateCode
// EA
L41:
	SCDEND();
Op1E:
	GETOL1();
	$CMP AL,SOE 	//SINGLE OPERAND/EFFECTIVE ADDRESS ?
	$JNE X24;
	$JMP CreateCode
X24:
	$CMP AL,SOM 	//SINGLE OPERAND/MEMORY POINTER ?
	$JNE L41
	$CMP EA_X,0
	$JNE L41
	$CMP EA_M,6 	//[WORD CONSTANT]?
	$JNE L41
C11:
	$JMP CreateCode
// CO
L51:
	SCDEND();
Op1C:
	GETOL1();
	$OR AL,AL 	//JUMP INSTRUCTION?
	$JNE NEAR N13
// Здесь обработка call&jmp
	ECX=OPCONST[0]-OptImageBase-OptBaseOfCode-outptr+output;	// Расчет относительного смещения
	$LODSB		// Получим short или near
	$CMP AL,_JB // short?
	$JNE N14
	ECX-=2;
	$CMP ECX,0xFFFFFF80
	$JL L10
	$CMP ECX,0x7F
	$JG L10
	OPCONSTSIZE[0]=_B;
H1:
	OPCONST[0]=ECX;
	$JMP CreateCode
N14:
	EAX=0;
	$LODSB		// Поправка на размер call или jmp
	ECX-=EAX;
	OPCONSTSIZE[0]=_D;
	GOTO H1;
L10:
	$CMP ACTUALMNEMDESC,#T_JCXZ
	$JB L51
	$CMP ACTUALMNEMDESC,#T_LOOP
	$JA NEAR L51
	preerror("ASM: Jump range too long\n");
	$RET
N13:
	$CMP AL,SO3 	//CONSTANT VALUE 3 ?
	$JNE N23
	$CMP OPCONST[0],3
	$JNE NEAR L51
	OPCONSTFLAG[0]=0; //YES, AVOID CONSTANT GENERATION.
	$JMP CreateCode
N23:
	$CMP AL,SOC 	//SINGLE OPERAND/CONSTANT?
	$JNE X25
	OPCONSTSIZE[0]=OPERANDSIZE;
	$JMP CreateCode
X25:
	$CMP AL,SOO_CC		//SINGLE OPERAND/SIGNED BYTE ?
	$JNE NEAR L51
	IF(OPPOST[0])$JMP L51
	$CMP OPCONST[0],0x7F
	$JG NEAR L51
	$CMP OPCONST[0],0xFFFFFF80
	$JL NEAR L51
	OPCONSTSIZE[0]=_B;
	$JMP CreateCode
// SR
L61:
	SCDEND();
Op1S:
	GETOL1();
	$CMP AL,SOS 	//SINGLE OPERAND/SEGMENT REGISTER?
	$JNE L61
	$JMP CreateCode
// AF
L71:
	SCDEND();
Op1AF:
	GETOL1();
	$CMP AL,SOO_AF
	$JNE L71
	$JMP CreateCode
// ---- Инструкция с двумя операндами
TWOOPS:
	EBX=CL<<2+#Jmp_Op2;
	$JMP NEAR DSDWORD[EBX]		// Переход по типу первого операнда
//Op2ERRC:
//	preerror("ASM: A constant can't be used as a destination\n");
//	return;
Op2ERRS:
	preerror("ASM: segment register can only be used within a MOV or PUSH\n");
	return;
Op2ERRSYS:
	preerror("ASM: only use system registers within MOV instruction\n");
	return;
Op2ERRAF:
	preerror("Absolute FAR addresses can only be used for jumps\n");
	return;
// Первый операнд в 2-операндной инструкции - регистр
Op2R:
	EBX=0;
	BL=CH;
	EBX=EBX<<2+#Jmp_Op2R;
	$JMP NEAR DSDWORD[EBX]		// Переход по типу второго операнда
// Первый операнд в 2-операндной инструкции - EA
Op2E:
	EBX=0;
	BL=CH;
	EBX=EBX<<2+#Jmp_Op2E;
	$JMP NEAR DSDWORD[EBX]		// Переход по типу второго операнда
// mnem EA,RX
L81:
	SCDEND();
OpE2R:
	GETOL2(); 	// Обработка записи из шаблона
	$CMP AL,DER 	//EA & R + D-BIT ?
	$JE  C21
X26:
	$CMP AL,ERO 	//'ERO' ORDER ?
	$JE  C21
	$CMP AL,EAO 	//EA, ?
	$JNE L81			// На пропуск записи в шаблоне
	$CMP OPDATA[1],rCL	//CL REGISTER USED??
	$JNE L81			// На пропуск записи в шаблоне
	$CMP DSBYTE[ESI+1],rCL	//CL IN GENERATION INFO?
	$JNE L81			// На пропуск записи в шаблоне
//	CMP OPERANDSIZE,_B	//YES, CHECK SIZE.
//	JNE L81 		// На пропуск записи в шаблоне
	ESI++;
	ESI++;
C21:
	$JMP CreateCode
L91:
	SCDEND();
OpR2E:
	GETOL2();
	$CMP AL,DER 	//DER ?
	$JNE N43
	DBIT=1; 	//(DIRECTION BIT)
	$JMP CreateCode
N43:
	$CMP AL,REO 	//REO ?
	$JNE L91;
	$JMP CreateCode
//RX,RX ?
W2:
	ESI++;
	GOTO W1;
LA1:
	SCDEND();
OpR2R:
	$CMP DSBYTE[ESI],_B+OL2+EAO // байт+2оп+EAold?
	$JE W2			//EAO FOUND, R+R COMBINATION NOT PERMITTED.
	GETOL2(); 	// Обработка записи шаблона
	$CMP AL,DER 	// EA,reg или reg,EA с D/W-битом?
	$JNE N53
LB2:
	AL=OPDATA[0]; 	// Преобразуем регистр в EA
	CREATE_EA();
	AL=OPDATA[1]; 	// Второй операнд
	CREATE_R();
	$JMP CreateCode
N53:
	$CMP AL,ERO
	$JE LB2
	$CMP AL,REO
	$JNE N63
	AL=OPDATA[1]; //RX,EP
	CREATE_EA();
	AL=OPDATA[0];
	CREATE_R();
	$JMP CreateCode
N63:
	$CMP AL,EAO
	$JNE LA1			// На пропуск записи в шаблоне
W1:
	ECX=2;		//COMPARE 2 OPERANDS.
	opDescInd=0;
LX2:
	$LODSB		// поле из записи шаблона
	$CMP AL,255 	//1ST OPERAND OK.
	$JE L022
	$CMP AL,AR		//AL/AX/EAX?
	$JE OL2X_AR
	$CMP AL,rDX 	//DX?
	$JE OL2X_DX
	$CMP AL,rCL
	$JE OL2X_CL
	GOTO LA1;			// На пропуск записи в шаблоне
OL2X_AR:
	EBX=opDescInd;
	AH=OPDATA[EBX]&7; // Операнд из мнемоники
	$JNZ NEAR LA1 		// На пропуск записи в шаблоне
L022:
	opDescInd++;
	$LOOP LX2
	GOTO L23;
OL2X_DX:
	EBX=opDescInd;
	$CMP OPDATA[EBX],0o12
	$JNE NEAR LA1 		// На пропуск записи в шаблоне
	opDescInd++;
	$LOOP LX2
	GOTO L23;
OL2X_CL:
	EBX=opDescInd;
	$CMP OPDATA[EBX],rCL	//CL
	$JNE NEAR LC1
	opDescInd++;
	$LOOP LX2
	$TEST OPDATA[0],8+16	//1ST REGISTER WORD/DWORD?
	$JZ L23
	WBIT=1; 	//YES, SET W-BIT.
L23:
	AL=OPDATA[0];
	CREATE_EA();
	$JMP CreateCode
//EA,CONSTANT ? 	//EA,CONST?
LB1:
	SCDEND();
OpE2C:
	GETOL2();
	opDescInd=1;
	$CMP AL,ECO
	$JNE N73
	OPCONSTSIZE[1]=OPERANDSIZE;
	$JMP CreateCode
N73:
	$CMP AL,EAO
	$JNE N83
	$CMP OPERANDSIZE,_B
	$JNE N83
	$CMP DSBYTE[ESI],_1
	$JNE N83
	$CMP OPCONST[4],1
	$JNE N83
	OPCONSTFLAG[1]=0;
	ESI++;
	$JMP CreateCode
N83:
	$CMP AL,ECCO		//EA/SIGNED BYTE ?
	$JNE LB1
	$CMP OPCONST[4],0xFFFFFF80
	$JL LB1
	$CMP OPCONST[4],0x7F
	$JG NEAR LB1
	OPERANDSIZE=_B; //OMIT A SINGLE BYTE ON GENERATION.
	$JMP CreateCode
// mnem reg,const
LC1:
	SCDEND();
OpR2C:
	GETOL2();
	opDescInd=1;
	$CMP AL,RCO;
	$JNE Q1
A0:
	OPCONSTSIZE[1]=OPERANDSIZE;
	$JMP CreateCode // reg,const
Q1:
	$CMP AL,ECO;
	$JNE L110
A1:
	AL=EA_R;
	CREATE_EA();
	GOTO A0;
L110:
	$CMP AL,EAO;
	$JE N93
	$CMP AL,ECCO;
	$JNE LC1		//SIGNED BYTE CONST ?
	$CMP OPCONST[4],0xFFFFFF80;
	$JL LC1
	$CMP OPCONST[4],0x7F;
	$JG LC1
	OPERANDSIZE=_B;
	OPCONSTSIZE[1]=_B;
	GOTO A1; //CONVERT REGISTER TO EFFECTIVE ADDRESS AND GENERATE OPCODE.
N93:
	ECX=2;		//COMPARE 2 OPERAND.
B2:
	$LODSB;
	$CMP AL,255;
	$JE L122
	$CMP AL,AR;
	$JE OL2_AR	//AL/AX/EAX?
	$CMP AL,CO;
	$JE OL2_CO	//CONSTANT?
	$CMP AL,rDX	;
	$JE OL2_DX	//DX?
	$CMP AL,_1;
	$JE OL2_1 //CONSTANT VALUE 1?
	$JMP LC1
OL2_AR:
	AH=OPDATA[0]&7;
	$JNZ NEAR LC1
L122:
	$LOOP B2;
	$JMP CreateCode
OL2_CO:
	$CMP OPDESC[1],CO;
	$JNE NEAR LC1
	OPCONSTSIZE[1]=OPERANDSIZE;
	GOTO L122;
OL2_DX:
	$CMP OPDATA[0],0o12;
	$JE  L122;
	$JMP LC1
OL2_1:
	$CMP OPCONSTSIZE[1],_B;
	$JNE NEAR LC1
	$CMP OPCONST[4],1;
	$JNE NEAR LC1
	OPCONSTFLAG[1]=0;
	$JMP A1
LD1:
	SCDEND();
// Первый операнд в 2-операндной инструкции - константа
Op2C:
	GETOL2();
	$CMP AL,EAO
	$JNE LD1
	ECX=2;		//COMPARE 2 OPERANDS.
	opDescInd=0;
B12:
	$LODSB
	$CMP AL,255
	$JE L222
	$CMP AL,AR		//AL/AX/EAX
	$JE XOL2_AR
	$CMP AL,CO
	$JE XOL2_CO
	$CMP AL,rDX 	//DX
	$JE XOL2_DX
	$CMP AL,_1
	$JE XOL2_1
	GOTO LD1;
XOL2_AR:
	EBX=opDescInd;
	AH=OPDATA[EBX]&7;
	$JNZ N21
L222:
	opDescInd++;
	$LOOP B12
	$JMP CreateCode
N21:
	GOTO LD1;
XOL2_CO:
	EBX=opDescInd;
	$CMP OPDESC[EBX],CO
	$JNE LD1
	opDescInd++;
	$LOOP B12
	$JMP CreateCode
XOL2_DX:
	EBX=opDescInd;
	$CMP OPDATA[EBX],0o12
	$JNE NEAR LD1
	opDescInd++;
	$LOOP B12
	$JMP CreateCode
XOL2_1:
	EDX=opDescInd;
	$CMP OPCONSTSIZE[EDX],_B
	$JNE NEAR LD1
	EDX<<=2;
	$CMP OPCONST[EDX],1
	$JNE NEAR LD1
	EDX=opDescInd;
	OPCONSTFLAG[EDX]=0;
	AL=EA_R;
	CREATE_EA();
	$JMP CreateCode
// Трехоперандная инструкция
LE1:
	SCDEND();
THREEOPS:
D11:
	GETOL3();
	$CMP AL,ERO
	$JNE N42
	$CMP CX,R*256+E
	$JE O21
	$CMP CX,R*256+R
	$JNE LE1
	AL=OPDATA[0];
	CREATE_EA();
	AL=OPDATA[1];
	CREATE_R();
	GOTO O21;
N42:
	$CMP AL,REO
	$JNE N52		//ERROR: INVALID OPERANDS.
	$CMP CX,E*256+R
	$JE O21
	$CMP CX,R*256+R
	$JNE LE1
	AL=OPDATA[1];
	CREATE_EA();
	AL=OPDATA[0];
	CREATE_R();
O21:
	BL=AH&TRANSBITS;
	$CMP OPCONSTFLAG[2],1
	$JNE NEAR NA3
	$CMP BL,CC3
	$JNE N52
	$CMP OPCONST[8],0xFFFFFF80
	$JL NEAR LE1
	$CMP OPCONST[8],0x7F
	$JG NEAR LE1
	OPCONSTSIZE[2]=_B;
	$JMP CreateCode
N52:
	$CMP BL,CB3
	$JNE N62
	$CMP OPCONST[8],0xFF
	$JA NEAR LE1
	OPCONSTSIZE[2]=_B;
	$JMP CreateCode
N62:
	$CMP BL,CW3
	$JNE NA3
	$CMP OPCONST[8],0xFFFFFFFF
	$JA NEAR LE1
	$CMP RANDFLAG,1
	$JNE NA2
	OPCONSTSIZE[2]=_W;
	$JMP CreateCode
NA2:
	OPCONSTSIZE[2]=_D;
NA_2:
	$JMP CreateCode
NA3:
	$CMP BL,CL3
	$JNE NEAR LE1
	$CMP OPDESC[2],R
	$JNE NEAR LE1
	$CMP OPDATA[2],rCL
	$JE NA_2
	illegaloperand();
}

CreateConstants()
{
	$CMP EA_SIBFLAG,1 // Флаг наличия Sib в 32-битном EA
	$JNE L3 		// Sib отсутствует
	$CMP ADDRFLAG,1
	$JE L3		//NO, NORMAL XRM
// Запись SIB - байта
	AL=EA_I<<3|EA_R2|EA_SCALING;
	OP();
L3:
	$CMP OPCONSTFLAG[0],1
	$JNE NEAR N1
	IF(OPPOST[0])SetPost(OPPOSTREF[0],OPPOST[0]);
	ECX=OPCONSTSIZE[0];
	EDI=#OPCONST; // Значение disp в EA
	WRITECONST();
N1:
	$CMP OPCONSTFLAG[1],1
	$JNE NEAR N41
	ECX=OPCONSTSIZE[1];
	$CMP CL,AF		//ABSOLUTE FAR ?
	$JNE D21
	EDI=#OPCONST+4; //YES, CREATE ADDRESS.
	CL=_W;		//(32 BIT)
	$CMP ADDRFLAG,1
	$JNE D2
	ECX=_D; 	//(48 BIT)
D2:
	WRITECONST();
	EDX+=output;
	EBX=EDX>>4;
	$AND EDX,15
	$PUSH EBX,EDX
	$POP EDX	//???
	EAX=opDescInd;
	DSDWORD[EAX]=EDX;
	ECX=_W;
	/* AFSEG нигде не определен, надо будет разбираться
	EDI=#AFSEG; //(SEGMENT/SELECTOR)									 */

	WRITECONST();
	$STC
	$RET
D21:
	IF(OPPOST[1])SetPost(OPPOSTREF[4],OPPOST[1]);
	EDI=#OPCONST+4;
	WRITECONST();
N41:
	IF(OPCONSTFLAG[2]==1){
		ECX=OPCONSTSIZE[2];
		EDI=#OPCONST+8;
		WRITECONST();
	}
}

// ----
illegaloperand()
{
	preerror("ASM: Illegal operand\n");
}
// ----
toomuchoperands()
{
	preerror("ASM: Illegal number of operands\n");
}

// JUMP TABLES
dword Jmp_Mov={#Mov_R,#Mov_E,#Mov_ERR,#Mov_S,
	#Mov_SYSR,#Mov_SYSR,#Mov_SYSR,#Mov_SYSR,#Mov_SYSR,#Mov_SYSR};
dword Jmp_Mov_R={#Mov_R2R,#Mov_R2E,#Mov_R2C,#Mov_S,
	#Mov_SYSR,#Mov_SYSR,#Mov_SYSR,#Mov_SYSR,#Mov_SYSR,#Mov_SYSR};
dword Jmp_Mov_E={#Mov_E2R,#Mov_ERR,#Mov_E2C,#Mov_S,
				#Mov_ERR,#Mov_ERR,#Mov_ERR,#Mov_ERR,#Mov_ERR,#Mov_ERR};

dword Jmp_Op1={#Op1R,#Op1E,#Op1C,#Op1S,
				#Op1ERR,#Op1ERR,#Op1ERR,#Op1ERR,#Op1ERR,#Op1AF};
dword Jmp_Op2={#Op2R,#Op2E,#Op2C,#Op2ERRS,
				#Op2ERRSYS,#Op2ERRSYS,#Op2ERRSYS,#Op2ERRSYS,#Op2ERRSYS,#Op2ERRAF};
dword Jmp_Op2R={#OpR2R,#OpR2E,#OpR2C,#Op2ERRS,
				#Op2ERRSYS,#Op2ERRSYS,#Op2ERRSYS,#Op2ERRSYS,#Op2ERRSYS,#Op2ERRAF};
dword Jmp_Op2E={#OpE2R,0,#OpE2C,#Op2ERRS,
	#Op2ERRSYS,#Op2ERRSYS,#Op2ERRSYS,#Op2ERRSYS,#Op2ERRSYS,#Op2ERRAF};
//dword TC_JMP={#T_JMPSHORT,#T_JMPNEAR,#T_JMPFAR};
//dword TC_CALL={0,#T_CALL,#T_CALLFAR};
//dword TC_J={#T_J,#T_JN,1};
dword Dsc_Jump={#CreateConstants,#Dsc_O,#Dsc_OW,#Dsc_OS,#Dsc_S,#Dsc_XRM,#Dsc_XPM,#Dsc_XSM};

