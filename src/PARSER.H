// ---- ��������� ���������
DoExpr(dword var,vtok,vtype,mnem)
byte varName[2*IDLENGTH];
{
	IF(tok2notstopper()){
		IF(vtok==tk_var)Expression("eax",tk_reg,tk_dword);
		ELSE{
			Expression(var,vtok,vtype);
			return;
		}
	}
	else{ // ���� �������
		tok=GetVarname(#varName);
		IF(tok==tk_reg)wsprintfA(#mapstr,"%s %s,%s",mnem,var,#varName);
		else IF(tok==tk_var){
			IF(vtok==tk_var)Expression("eax",tk_reg,tk_dword);
			ELSE wsprintfA(#mapstr,"%s %s,%s",mnem,var,#varName);
		}
		else IF(tok==tk_number)wsprintfA(#mapstr,"%s %s,%d",mnem,var,DoConstMath());
		ELSE IF(tok==tk_postnumber)wsprintfA(#mapstr,"%s %s#%s",mnem,var,#varName);
		ELSE IF(tok==tk_locnumber){
			wsprintfA(#mapstr,"lea ebx,%s",#varName);
			wsprintfA(#mapstr,"%s %s,ebx",mnem,var);
			return;
		}
		Asm(#mapstr);
		RETURN;
	}
	IF(vtype==tk_byte)||(vtype==tk_char)wsprintfA(#mapstr,"%s %s,al",mnem,var);
	ELSE IF(vtype==tk_word)||(vtype==tk_short)wsprintfA(#mapstr,"%s %s,ax",mnem,var);
	ELSE IF(vtype==tk_dword)||(vtype==tk_int)wsprintfA(#mapstr,"%s %s,eax",mnem,var);
	Asm(#mapstr);
}

// ---- �������� � ���������� ���������
dword Expression(dword dest,dtok,dtype)
byte s[IDLENGTH];
{
	GetInto(dest,dtok,dtype);
	for(;;){
		Term(#s,dtype);
		IF(tok==tk_plus){
			Term(#s,dtype);
			wsprintfA(#mapstr,"add %s,%s",dest,#s);
			Asm(#mapstr);
		}
		else IF(tok==tk_minus){
			Term(#s,dtype);
			wsprintfA(#mapstr,"sub %s,%s",dest,#s);
			Asm(#mapstr);
		}
		else IF(tok==tk_or){
			Term(#s,dtype);
			wsprintfA(#mapstr,"or %s,%s",dest,#s);
			Asm(#mapstr);
		}
		else IF(tok==tk_xor){
			Term(#s,dtype);
			wsprintfA(#mapstr,"xor %s,%s",dest,#s);
			Asm(#mapstr);
		}
		else IF(tok==tk_assign)||(tok==tk_equalto){
			Term(#s,dtype);
			wsprintfA(#mapstr,"cmp %s,%s",dest,#s);
			Asm(#mapstr);
			Asm("sete al");
SE:
			wsprintfA(#mapstr,"movzx %s,al",dest);
			Asm(#mapstr);
		}
		else IF(tok==tk_notequal){
			Term(#s,dtype);
			wsprintfA(#mapstr,"cmp %s,%s",dest,#s);
			Asm(#mapstr);
			Asm("setne al");
			GOTO SE;
		}
		else IF(tok==tk_greater){
			Term(#s,dtype);
			wsprintfA(#mapstr,"cmp %s,%s",dest,#s);
			Asm(#mapstr);
			Asm("setg al");
			$JMP SE;
		}
		else IF(tok==tk_greaterequal){
			Term(#s,dtype);
			wsprintfA(#mapstr,"cmp %s,%s",dest,#s);
			Asm(#mapstr);
			Asm("setge al");
			$JMP SE;
		}
		else IF(tok==tk_less){
			Term(#s,dtype);
			wsprintfA(#mapstr,"cmp %s,%s",dest,#s);
			Asm(#mapstr);
			Asm("setl al");
			$JMP SE;
		}
		ELSE IF(tok==tk_lessequal){
			Term(#s,dtype);
			wsprintfA(#mapstr,"cmp %s,%s",dest,#s);
			Asm(#mapstr);
			Asm("setle al");
			$JMP SE;
		}
		ELSE BREAK;
	}
	return(relation);
}

// ---- ������ ���������� �������� ���������
Factor(dword f,ftype)
{
	NextTok();
	IF(tok==tk_openbracket){
		NextTok();
		PushEAX();
		Expression("eax",tk_reg,ftype);
		Asm("pop ebx; xchg eax,ebx");
		wsprintfA(f,"%s","ebx");
	}
	else IF(tok==tk_number)wsprintfA(f,"%#x",DoConstMath());
	else IF(tok==tk_postnumber)wsprintfA(f,"#%s",#string);
	else IF(tok==tk_proc){
		PushEAX();
		DoAnyProc();
		Asm("pop ebx; xchg eax,ebx");
		wsprintfA(f,"%s","ebx");
	}
	ELSE IF(tok==tk_API){
		PushEAX();
		doAPI();
		Asm("pop ebx; xchg eax,ebx");
		wsprintfA(f,"%s","ebx");
	}
	ELSE IF(tok==tk_var)||(tok==tk_param)||(tok==tk_local)||(tok==tk_openblock)||
		(tok==tk_reg)GetVarname(f);
}

// ----
GetInto(dword dest,dtok,dtype)
dword tk;
{
	tk=0;
DOCASE:
	IF(tok==tk_minus){
F_0:
		tk=tok;
		NextTok();
		GOTO DOCASE;
	 }
	else IF(tok==tk_not){
		relation^=1;
		GOTO F_0;
	}
	else IF(tok==tk_openbracket){
		NextTok();
		Expression(dest,dtok,dtype);
	}
	else IF(tok==tk_number){
		IF(tk)wsprintfA(#mapstr,"mov %s,-%#x",dest,DoConstMath());
		ELSE wsprintfA(#mapstr,"mov %s,%#x",dest,DoConstMath());
		Asm(#mapstr);
		tk=0;
	}
	else IF(tok==tk_postnumber){
		wsprintfA(#mapstr,"mov %s,#%s",dest,#string);
		Asm(#mapstr);
	}
	else IF(tok==tk_locnumber){
		wsprintfA(#mapstr,"lea ebx,%s",#string);
		Asm(#mapstr);
		wsprintfA(#mapstr,"mov %s,ebx",dest);
		Asm(#mapstr);
	}
	ELSE IF(tok==tk_proc)DoAnyProc();
	ELSE IF(tok==tk_API)doAPI();
	ELSE IF(tok==tk_var)||(tok==tk_local)||(tok==tk_param)||(tok==tk_reg)||
		(tok==tk_openblock)GetIntoVar(dest,dtok,dtype);
	ELSE preerror("Wrong expression member");
//		wsprintfA(#string,dest,"mov %s,%s",#mapstr);
//		ESP+=16; Asm(#mapstr);
	IF(tk==tk_minus){
		wsprintfA(#mapstr,"neg %s",dest);
		Asm(#mapstr);
	}
	IF(tk==tk_not){
		wsprintfA(#mapstr,"not %s",dest);
		Asm(#mapstr);
	}
}

// ----
GetIntoVar(dword dName,dTok,dType)
byte varName[2*IDLENGTH];
dword vtype,vtok;
{
	if(dTok==tk_reg){
		if(tok==tk_reg){	// Reg = Reg
			IF(dType==tk_dword){// Reg32=Reg
				IF(type==tk_dword){
					wsprintfA(#mapstr,"mov %s,%s",dName,#string);
					Asm(#mapstr);
				}
				ELSE IF(type==tk_word)||(type==tk_byte){
RDW:
					wsprintfA(#mapstr,"movzx %s,%s",dName,#string);
					Asm(#mapstr);
				}
			}
			else IF(dType==tk_word){	// Reg=Reg
				IF(type==tk_dword){
GERR:
					warning("Not same size\n");
				}
				ELSE IF(type==tk_word){
					wsprintfA(#mapstr,"mov %s,%s",dName,#string);
					Asm(#mapstr);
				}
				ELSE IF(type==tk_byte)GOTO RDW;
			}
			ELSE IF(dType==tk_byte){	// Reg=Reg
				IF(type==tk_dword)||(type==tk_word)GOTO GERR;
				IF(type==tk_byte){
					wsprintfA(#mapstr,"mov %s,%s",dName,#string);
					Asm(#mapstr);
				}
			}
		}
		else if(tok==tk_var)||(tok==tk_param)||(tok==tk_local)||(tok==tk_openblock){	// Reg = Var
			vtype=type; vtok=GetVarname(#varName);
			IF(vtype==tk_dword)||(vtype==tk_int)||(dType==vtype){
				wsprintfA(#mapstr,"mov %s,%s",dName,#varName);
			}
			ELSE IF(vtype==tk_word)||(vtype==tk_byte){
				wsprintfA(#mapstr,"movzx %s,%s",dName,#varName);
			}
			ELSE IF(vtype==tk_short)||(vtype==tk_char){
				wsprintfA(#mapstr,"movsx %s,%s",dName,#varName);
			}
			Asm(#mapstr);
		}
		else IF(tok==tk_number){	// Reg = Const
			wsprintfA(#mapstr,"mov %s,%d",dName,DoConstMath());
			Asm(#mapstr);
		}
		else IF(tok==tk_postnumber){	// Reg = #Var
			GetVarname(#varName);
			wsprintfA(#mapstr,"mov %s,#%s",dName,#varName);
			Asm(#mapstr);
		}
		ELSE IF(tok==tk_locnumber){ // Reg = #locVar
			vtype=type; vtok=GetVarname(#varName);
			wsprintfA(#mapstr,"lea %s,%s",dName,#varName);
			Asm(#mapstr);
		}
	}
	else if(dTok==tk_var){
		if(tok==tk_reg){	// Var = Reg;
			IF(type==tk_dword){
				wsprintfA(#mapstr,"mov dword %s,%s",dName,#string);
				Asm(#mapstr);
			}
			ELSE IF(type==tk_word){
				wsprintfA(#mapstr,"mov word %s,%s",dName,#string);
				Asm(#mapstr);
			}
			ELSE IF(type==tk_byte){
				wsprintfA(#mapstr,"mov byte %s,%s",dName,#string);
				Asm(#mapstr);
			}
		}
		else if(tok==tk_var){ // Var = Var;
			vtype=type;
			vtok=GetVarname(#varName);
			IF(dType==tk_byte)||(dType==tk_char){
				wsprintfA(#mapstr,"mov al,%s",#varName);
				Asm(#mapstr);
				wsprintfA(#mapstr,"mov %s,al",dName);
				Asm(#mapstr);
			}
			else if(dType==tk_word)||(dType==tk_short){
				IF(vtype==tk_byte){
					wsprintfA(#mapstr,"movzx ax,%s",#varName);
					Asm(#mapstr);
				}
				else IF(vtype==tk_char){
					wsprintfA(#mapstr,"movsx ax,%s",#varName);
					Asm(#mapstr);
				}
				ELSE IF(vtype==tk_word)||(vtype==tk_short){
					wsprintfA(#mapstr,"mov ax,%s",#varName);
					Asm(#mapstr);
				}
				ELSE IF(vtype==tk_dword)||(vtype==tk_int){
					wsprintfA(#mapstr,"mov ax,word %s",#varName);
					Asm(#mapstr);
				}
				wsprintfA(#mapstr,"mov %s,ax",dName);
				Asm(#mapstr);
			}
			else if(dType==tk_dword)||(dType==tk_int){
				IF(vtype==tk_byte)||(vtype==tk_word){
					wsprintfA(#mapstr,"movzx eax,%s",#varName);
					Asm(#mapstr);
				}
				ELSE IF(vtype==tk_char)||(vtype==tk_short){
					wsprintfA(#mapstr,"movsx eax,%s",#varName);
					Asm(#mapstr);
				}
				ELSE IF(vtype==tk_dword)||(vtype==tk_int){
					wsprintfA(#mapstr,"mov eax,%s",#varName);
					Asm(#mapstr);
				}
				wsprintfA(#mapstr,"mov %s,eax",dName);
				Asm(#mapstr);
			}
		}
		else IF(tok==tk_number){	// Var = Const;
			wsprintfA(#mapstr,"mov %s,%d",dName,DoConstMath());
			Asm(#mapstr);
		}
		else IF(tok==tk_postnumber){	// Var = #Var;
			vtype=type; vtok=GetVarname(#varName);
			wsprintfA(#mapstr,"mov %s,#%s",dName,#varName);
			Asm(#mapstr);
		}
		ELSE IF(tok==tk_locnumber){ // Var = #locVar;
			vtype=type; vtok=GetVarname(#varName);
			wsprintfA(#mapstr,"lea ebx,%s",#varName);
			Asm(#mapstr);
			wsprintfA(#mapstr,"mov %s,ebx",dName);
			Asm(#mapstr);
		}
	}
}

// ---- ������ ����������: VarName[reg+reg*Scale+disp]
dword GetVarname(dword varName)
dword vtok;
{
	IF(tok==tk_openblock)GOTO G0;
	lstrcpyA(varName,#string);
	vtok=tok;
	IF(vtok==tk_local)vtok=tk_var;
	ELSE IF(vtok==tk_param)vtok=tk_var;
	if(tok2==tk_openblock){
		NextTok();
G0:
		vtok=tk_var;
		lstrcatA(varName,"[");
		for(;;){
			NextTok();
			IF(tok==tk_reg)lstrcatA(varName,#string);
			else IF(tok==tk_plus)lstrcatA(varName,"+");
			else IF(tok==tk_mult)lstrcatA(varName,"*");
			ELSE IF(tok==tk_number){
				wsprintfA(#mapstr,"%d",DoConstMath());
				lstrcatA(varName,#mapstr);
			}
			ELSE IF(tok==tk_postnumber){
				lstrcatA(varName,"#");
				lstrcatA(varName,#string);
			}
			ELSE IF(tok==tk_closeblock){
				lstrcatA(varName,"]");
				BREAK;
			}
			ELSE preerror("Illegal index expression in []");
		}
	}
	return(vtok);
}

// ----
Term(dword t,ttype)
{
	for(;;){
		Factor(t,ttype);
		IF(tok==tk_mult){
			Factor(t,ttype);
			IF(tok==tk_number){
				wsprintfA(#mapstr,"mov ebx,%d",DoConstMath());
				Asm(#mapstr);
				wsprintfA(#mapstr,"mul %s","ebx");
			}
			ELSE wsprintfA(#mapstr,"mul %s",t);
			Asm(#mapstr);
		}
		else IF(tok==tk_div){
			Factor(t,ttype);
			IF(tok==tk_number){
				wsprintfA(#mapstr,"mov ebx,%d",DoConstMath());
				Asm(#mapstr);
				wsprintfA(#mapstr,"div %s","ebx");
			}
			ELSE wsprintfA(#mapstr,"div %s",t);
			Asm(#mapstr);
		}
		else IF(tok==tk_mod){
			Factor(t,ttype);
			IF(tok==tk_number){
				wsprintfA(#mapstr,"mov ebx,%d",DoConstMath());
				Asm(#mapstr);
				wsprintfA(#mapstr,"div %s","ebx");
			}
			ELSE wsprintfA(#mapstr,"div %s",t);
			Asm(#mapstr);
			Asm("xchg eax,edx");
		}
		ELSE IF(tok==tk_and){
			Factor(t,ttype);
			IF(tok==tk_number)wsprintfA(#mapstr,"and ebx,%d",DoConstMath());
			ELSE wsprintfA(#mapstr,"and eax,%s",t);
			Asm(#mapstr);
		}
		ELSE IF(tok==tk_not)Asm("not eax");
		ELSE BREAK;
	}
}

// ---- ���������� EAX � �����
PushEAX()
{
	Asm("push eax");
}

// ---- ���������� EAX � �����
/*PopEAX()
{
	Asm("pop eax");
} */

// ---- ��������� ������-���������: proc("string")
dword AddPoststring()
dword returnvalue;
{
	IF(posts >= MAXPOSTS){
		preerror("cannot add post string, post queue full");
		ExitProcess(-1);
	}
	EBX=posts<<2+posttype;
	DSDWORD[EBX] = POST_STR;
	EBX=posts<<2+postloc;
	DSDWORD[EBX] = outptr;
	posts++;
	returnvalue = MAXDATA-1-poststrptr;
	ESI=#string;
	EBX><poststrptr;
	do{
		$LODSB;
		EDI=output+EBX;
		DSBYTE[EDI]=AL;
		EBX--;
	}while(AL!=0);
	EBX><poststrptr;
	EAX=returnvalue;
}

// ---- ��������� post-����� � ������ �� ���
DoPoststrings()
dword addvalue,addhold,i;
{
	IF(poststrptr==MAXDATA-1)return;
	addvalue = OptImageBase + OptBaseOfCode+outptr-output ;
	EDI><outptr;
	EBX=MAXDATA-1;
D0:
	ESI=output+EBX;
	AL=DSBYTE[ESI];
	$STOSB
	EBX--;
	$CMP EBX,poststrptr;
	$JA D0;
	EDI><outptr;
	i=0;
	while(i<posts){
		EBX=i<<2+posttype;
		if(DSDWORD[EBX]==POST_STR){
			EBX=i<<2+postloc;
			addhold = GetDword(DSDWORD[EBX])+addvalue;
			SetDword(DSDWORD[EBX],addhold);
			posts--;
			EBX=i<<2+postloc;
			ECX=posts<<2+postloc;
			DSDWORD[EBX]=DSDWORD[ECX];
			EBX=i<<2+posttype;
			ECX=posts<<2+posttype;
			DSDWORD[EBX]=DSDWORD[ECX];
			EBX=i<<2+postnum;
			ECX=posts<<2+postnum;
			DSDWORD[EBX]=DSDWORD[ECX];
			i--;
		}
		i++;
	}
	poststrptr = MAXDATA-1;
}

// ---- ������ ������ ��������� ��� ������ ���������
GetParam(dword p)
dword count;		// ������� ������ � ���������
{
	count=0;
	for(;;){
		EAX=0;
		ESI><inptr2;
		$LODSB;
		ESI><inptr2;
		cha2=AL;
		EDI><p;
		$STOSB EDI><p;
		IF(AL==0)BREAK;
		IF(AL==')'){
			IF(count==0){
				EDI><p;
				AL=0;
				EDI--;
				$STOSB;
				EDI><p;
				BREAK;
			}
			count--;
		}
		ELSE IF(AL==','){
			IF(count==0){
				EDI><p;
				EDI--;
				AL=0;
				$STOSB;
				EDI><p;
				DoParam();
				BREAK;
			}
		}
		ELSE IF(AL=='(')count++;
	}
}

// ---- ��������� ������ ��������� ��� ������ ���������
DoParam()
dword vtok;
byte p[250];		// ����� ��� ����� ���������
byte holdcha;
byte s[STRLEN],s2[STRLEN];
byte varName[2*IDLENGTH];
{
	GetParam(#p);
	holdcha=cha2;
	$PUSH linenum2,inptr2,number,tok2,tok,input,inptr,currmod,linenumber,
		endoffile,displaytokerrors;
	lstrcpyA(#s,#string); lstrcpyA(#s2,#string2);
	input=#p;
	inptr = input;
	inptr2 =	input;
	endoffile = 0;	// �� ������ �����
	NextChar();
	cha2 = cha;
	inptr2=inptr;
	linenum2 = 1;
	NextTok();
	for(;;){
		IF(tok==tk_eof)||(tok==tk_closebracket)break;
		IF(tok==tk_comma)NextTok();
		else IF(tok==tk_string){
			OP(byte 0x68);
			OUTDWORD(AddPoststring());
			IF(list){
				wsprintfA(#mapstr,"\t//\tpush #\"%s\"\n",#string);
				fprint(mapfile,#mapstr);
			}
			NextTok();
		}
		else IF(tok2isopperand()){
			Expression("eax",tk_reg,tk_dword);
			PushEAX();
		}
		else{ // �� ���������
			IF(tok==tk_number){
				wsprintfA(#mapstr,"push %#x",DoConstMath());
				Asm(#mapstr);
				NextTok();
			}
			else IF(tok==tk_postnumber){
				wsprintfA(#mapstr,"push #%s",#string);
				Asm(#mapstr);
				NextTok();
			}
			else if(tok==tk_reg){
				IF(type==tk_dword){
					wsprintfA(#mapstr,"push %s",#string);
					Asm(#mapstr);
					NextTok();
				}
				ELSE IF(type==tk_word){
					wsprintfA(#mapstr,"movsx e%s,%s;push e%s",#string,#string,#string);
					Asm(#mapstr);
					NextTok();
				}
				ELSE IF(tok==tk_byte){
					wsprintfA(#mapstr,"movsx e%cx,%s;push e%cx",string[0],#string,string[0]);
					Asm(#mapstr);
					NextTok();
				}
			}
			else IF(tok==tk_var)||(tok==tk_local)||(tok==tk_param){
				vtok=GetVarname(#varName);
				IF(type==tk_dword){
D0: 			wsprintfA(#mapstr,"push %s",#varName);
					Asm(#mapstr);
					NextTok();
				}
				ELSE GOTO D1;
				IF(type==tk_int){ //????
					vtok=GetVarname(#varName);
					GOTO D0;
				}
			}
			ELSE{
D1: 		Expression("eax",tk_reg,tk_dword);
				PushEAX();
			}
		}
	}
	lstrcpyA(#string,#s);
	lstrcpyA(#string2,#s2);
	$POP displaytokerrors,endoffile,linenumber,currmod,inptr,input,tok,tok2;
	$POP number,inptr2,linenum2;
	cha2=holdcha;
}

