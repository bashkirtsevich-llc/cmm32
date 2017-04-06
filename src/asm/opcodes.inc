;
; ��������� �������
;
; 0-7   byte regs
regAL = 0
regCL = 1
regDL = 2
regBL = 3
regAH = 4
regCH = 5
regDH = 6
regBH = 7
; 8-15  16-bit word regs
regAX = 8
regCX = 9
regDX = 10
regBX = 11
regSP = 12
regBP = 13
regSI = 14
regDI = 15
; 16-23 32-bit regs
regEAX = 16
regECX = 17
regEDX = 18
regEBX = 19
regESP = 20
regEBP = 21
regESI = 22
regEDI = 23
; 24-31 control regs
regCR0 = 24
regCR1 = 25
regCR2 = 26
regCR3 = 27
regCR4 = 28
regCR5 = 29
regCR6 = 30
regCR7 = 31
; 32-39 debug regs
regDR0 = 32
regDR1 = 33
regDR2 = 34
regDR3 = 35
regDR4 = 36
regDR5 = 37
regDR6 = 38
regDR7 = 39
; 40-47 test regs
regTR0 = 40
regTR1 = 41
regTR2 = 42
regTR3 = 43
regTR4 = 44
regTR5 = 45
regTR6 = 46
regTR7 = 47
; 48-53 seg regs
regES = 48
regCS = 49
regSS = 50
regDS = 51
regFS = 52
regGS = 53
; finish

; OPERATION SIZE CODES
_B  = 0 ;OPERATION MAY BE OF SIZE BYTE,WORD OR DWORD SEIN. THE OPCODE'S
	; W BIT INDICATES IF IT'S A BYTE OR WORD OPERATION.
	; FOR DWORD OPERATIONS, THE W BIT MUST BE TRUE (WORD OPERATION)
	; AND THE RAND:PREFIX MUST BE GENERATED.
_W  = 1 ;OPERATION SIZE MAY BE WORD OR DWORD.
_D  = 2 ;OPERATION MAY ONLY BE DWORD.
WB  = 3 ;1. OPERAND WORD, 2. OPERAND BYTE
_DW = 4 ;1. OPERAND DWORD, 2. OPERAND WORD
BW  = 5 ;1. OPERAND BYTE, 2. OPERAND WORD
NB  = 6 ;ONLY BYTE SIZE.
_OW = 7 ;ONLY WORD SIZE.
_OD = 8 ;Only DWORD size
AF  = 9 ;ONLY ABSOLUTE FAR 32/48 BIT ADDRESS

_JB = 1 ;OPERAND IS A JUMP - DESTINATION +/-128
_JW = 2 ;OPERAND IS A JUMP - DESTINATION +/-32678

; 0-2   = OPERATION SIZE CODE
; 3-4   = # OF OPERANDS NEEDED.

; # OF OPERANDS
OL0 = 0
OL1 = 10o
OL2 = 20o
OL3 = 30o

OLBITS	   = 30o ;MASKS FOR # OF OPERANDS.
NOLBITS    = 347o
TRANSBITS  = 7o  ;MASKS FOR OPERATION SIZE CODES.
NTRANSBITS = 370o

; IF OL1, USE THIS OPERAND TYPES:
SOJ = 00000000b  ;JUMP
SOE = 00100000b  ;EA
SOR = 01000000b  ;REG
SOC = 01100000b  ;CONST
SO3 = 10000000b  ;_3
SOS = 10100000b  ;SREG
SOM = 11000000b  ;EA-MEMORY-POINTER
SOO = 11100000b  ;THERE IS INFORMATION USING THE OPERAND SIZE CODE BITS.
SOO_FS = 340o	 ;FS-REGISTER
SOO_GS = 341o	 ;GS-REGISTER
SOO_AF = 342o	 ;ABSOLUTE ADRESS
SOO_CC = 343o	 ;SIGNED BYTE
OPBITS = 11100000b

ERO  = 00100000b ;FIXED EA,REG ORDER
REO  = 01000000b ;FIXED REG,EA ORDER
DER  = 01100000b ;EA,REG OR REG,EA DEFINED BY A D-BIT
ECO  = 10000000b ;EA,CONSTANT; SIZE DEFINED BY W-BIT
ECCO = 10100000b ;EA,CC
RCO  = 11000000b ;REG,CONSTANT; SIZE DEFINED BY W-BIT
EAO  = 11100000b ;EA,? ; SECOND OPERAND DESCRIBED BY OLD DESCIPTORS
		 ;?=_1,rCL,CO

; OPERAND TYPE CODES
R    = 0 ;GENERAL REGISTER
E    = 1 ;EFFEKTIVE ADRESS
CO   = 2 ;CONSTANT
_SR  = 3 ;SEGMENT REGISTER
SYSR = 4 ;SYSTEM REGISTER
_DR  = 5 ;DEBUG REGISTER
_TR  = 6 ;TASK REGISTER
_CR  = 7 ;CONTROL REGISTER
_CR4 = 8 ;
_AF  = 9;ABSOLUTE 32/48 BIT FAR ADDRESS
AR   = 11;AX/AL REGISTER, INDICATED BY OPERATION SIZE
_1   = 12;CONSTANT VALUE 1

; IF OL3, USE THIS:
; INSTRUCTION WITH THREE OPERANDS ARE ALWAYS OF SIZE WORD.
; THE FOLLOWING CODES DESCRIBE THE 3RD OPERAND.
CB3 = 0  ;BYTE CONSTANT
CW3 = 1  ;WORD CONSTANT
CC3 = 2  ;SBYTE CONSTANT
CL3 = 3  ;CL REGISTER

; OPCODE DESCRIPTORS
O   = 1  ;OPCODE IS A BYTE
OW  = 2  ;OPCODE IS A WORD
OS  = 3  ;OPCODE IS A BYTE PLUS A SECOND BYTE THAT IS DESCRIBED BY AN
	 ;OCTAL STRING.
S   = 4  ;OCTAL STRING FOLLOWS FOR OPCODE
XRM = 5  ;MODRM BYTE
XPM = 6  ;MODRM MIT P-FLAG (GRUPPE VON ARITHMETIC INSTRUCTIONS)
XSM = 7  ;MODRM MIT SEGMENTREGISTER

X0M = 8
X1M = 9
X2M = 10
X3M = 11
X4M = 12
X5M = 13
X6M = 14
X7M = 15 ;MODRM WITH CONSTANT R DIGIT

OVERRIDETAB db 46o, 56o, 66o, 76o, 144o, 145o

Registers db 'AL' , 0, 'CL' , 0, 'DL' , 0, 'BL' , 0, 'AH' , 0, 'CH' , 0, 'DH' , 0, 'BH', 0,\
	     'AX' , 0, 'CX' , 0, 'DX' , 0, 'BX' , 0, 'SP' , 0, 'BP' , 0, 'SI' , 0, 'DI', 0,\
	     'EAX', 0, 'ECX', 0, 'EDX', 0, 'EBX', 0, 'ESP', 0, 'EBP', 0, 'ESI', 0, 'EDI', 0,\
	     'CR0', 0, 'CR1', 0, 'CR2', 0, 'CR3', 0, 'CR4', 0, 'CR5', 0, 'CR6', 0, 'CR7', 0,\
	     'DR0', 0, 'DR1', 0, 'DR2', 0, 'DR3', 0, 'DR4', 0, 'DR5', 0, 'DR6', 0, 'DR7', 0,\
	     'TR0', 0, 'TR1', 0, 'TR2', 0, 'TR3', 0, 'TR4', 0, 'TR5', 0, 'TR6', 0, 'TR7', 0,\
	     'ES' , 0, 'CS' , 0, 'SS' , 0, 'DS' , 0, 'FS' , 0, 'GS' , 0, _END , 0

Sizes	   db 'BYTE', 0, 'WORD', 0, 'DWORD',0 , _END, 0

;P FLAG STARTS AT 0
PCOMMANDS1 db 0,\
	      _B+OL2+EAO , AR, CO,S, '0P4', 0,\
	      _B+OL2+DER , S , '0' , 'P'  , '0', XRM, 0,\
	      _W+OL2+ECCO, O , 203o, XPM  , 0,\
	      _B+OL2+ECO , O , 200o, XPM  , _END

;/P FLAG STARTS AT 4
PCOMMANDS2 db 4,\
	      _B+OL1+SOE, O, 0F6h, XPM, _END

;P FLAG STARTS AT 0
PCOMMANDS3 db 0

PCOMMANDS3_ db _B+OL2+EAO , -1, _1   , O  , 0D0h, XPM, 0,\
	       _B+OL2+EAO , -1, regCL, O  , 0D2h, XPM, 0,\
	       _B+OL2+ECCO, O , 0C0h , XPM, _END

;P FLAG STARTS AT 4
PCOMMANDS4 db 4,\
	      _W+OL2+ERO, OS, 0Fh, '2' , 'P', '3', XRM, 0,\
	      WB+OL2+ECO, OW, 0Fh, 272o, XPM, _END

; ������������ ����������
T_DAA	       db O , 047o,_END
T_DAS	       db O , 057o,_END
T_AAA	       db O , 067o,_END
T_AAD	       db OW, 0325o,012o,_END
T_AAM	       db OW, 0324o,012o,_END
T_AAS	       db O , 077o,_END
T_CWDE	       db O , 0230o,_END
T_CBW	       db OW, 066h,0230o,_END
T_CDQ	       db O , 0231o,_END
T_CWD	       db OW, 066h,0231o,_END
T_XLATB        db O , 0327o,_END
T_NOP	       db O , 0220o,_END
T_WAIT	       db O , 0233o,_END
T_LOCK	       db O , 0360o,_END
T_HLT	       db O , 0364o,_END
T_INTO	       db O , 0316o,_END
T_IRET	       db O , 0317o,_END
T_POPFD        db O , 0235o,_END
T_POPF	       db OW, 066h,0235o,_END
T_PUSHFD       db O , 0234o,_END
T_PUSHF        db OW, 066h,0234o,_END ;PUSHF / (#) PUSHFD
T_SAHF	       db O , 0236o,_END
T_LAHF	       db O , 0237o,_END
T_CMC	       db O , 0365o,_END
T_CLC	       db O , 0370o,_END
T_STC	       db O , 0371o,_END
T_CLI	       db O , 0372o,_END
T_STI	       db O , 0373o,_END
T_CLD	       db O , 0374o,_END
T_STD	       db O , 0375o,_END
T_PUSHAD       db O , 0140o,_END
T_PUSHA        db OW, 066h,0140o,_END
T_POPAD        db O , 0141o,_END
T_POPA	       db OW, 066h,0141o,_END
T_INSB	       db O , 0154o,_END
T_INSW	       db OW, 066h,0155o,_END
T_INSD	       db O , 0155o,_END
T_OUTSB        db O , 0156o,_END
T_OUTSW        db OW, 066h,0157o,_END
T_OUTSD        db O , 0157o,_END
T_MOVSB        db O , 0244o,_END
T_MOVSW        db OW, 066h,0245o,_END
T_MOVSD        db O , 0245o,_END
T_CMPSB        db O , 0246o,_END
T_CMPSW        db OW, 066h,0247o,_END
T_CMPSD        db O , 0247o,_END
T_STOSB        db O , 0252o,_END
T_STOSW        db OW, 066h,0253o,_END
T_STOSD        db O , 0253o,_END
T_LODSB        db O , 0254o,_END
T_LODSW        db OW, 066h,0255o,_END
T_LODSD        db O , 0255o,_END
T_SCASB        db O , 0256o,_END
T_SCASW        db OW, 066h,0257o,_END
T_SCASD        db O , 0257o,_END
T_REP	       db O , 0362o,_END
T_REPE	       db O , 0363o,_END
T_RAND	       db O , 0146o,_END ;RAND:
T_ADDR	       db O , 0147o,_END ;ADDR:
T_LEAVE        db O , 0311o,_END
T_CLTS	       db OW, 0Fh,6,_END
T_INVD	       db OW, 0Fh,010o,_END
T_WBINVD       db OW, 0Fh,011o,_END
T_WRMSR        db OW, 0Fh,030h,_END ;WRMSR WRITE        EDXEAX TO MODEL SPECIFIC REG #ECX
T_CPUID        db OW, 0Fh,0A2h,_END ;CPUID IF EAX=1 SET EDXEAX TO CPU IDENTIFICATION VALUES
T_RDMSR        db OW, 0Fh,032h,_END ;RDMSR READ MODEL SPECIFIC REG #ECX TO EDXEAX
T_RDTSC        db OW, 0Fh,031h,_END ;RDTSC READ TIME STAMP COUNTER TO EDXEAX
T_RSM	       db OW, 0Fh,0AAh,_END ;RSM        RESUME FROM SYSTEM MANAGEMENT MODE
;===== INTEL PENTIUM PRO INSTRUCTIONS
T_RDPMC        db OW, 0Fh,033h,_END ;READ PERFORMANCE MONITORING COUNTERS
T_UD2	       db OW, 0Fh,0Bh,_END ;UNDEFINED INSTRUCTION EXCEPTION
T_EMMX	       db OW, 0Fh,077h,_END ;EMPTY MMX STATE
T_SALC	       db O , 0D6h,_END
T_ICEBP        db O , 0F1h,_END
T_PSHIMW       db OW, 0Fh,071h,_END
T_PSHIMD       db OW, 0Fh,072h,_END
T_PSHIMQ       db OW, 0Fh,073h,_END

T_NOT	       db _B+OL1+SOE,O,0F6h,X2M,_END
T_NEG	       db _B+OL1+SOE,O,0F6h,X3M,_END

T_INC	       db _W+OL1+SOR,S,'10R',0,\
		  _B+OL1+SOE,O,0xFE,X0M,_END

T_DEC	       db _W+OL1+SOR,S,'11R',0,\
		  _B+OL1+SOE,O,0xFE,X1M,_END

T_TEST	       db _B+OL2+ERO,O,0204o,XRM,0,\ ;ALPHA:WAS 204O
		  _B+OL2+REO,O,0204o,XRM,0,\ ;"
		  _B+OL2+EAO,AR,CO,O,0250o,0,\
		  _B+OL2+ECO,O,0F6h,X0M,_END
; EXTENDED 386 INTEGER MULTIPLICATION
T_IMUL	       db _B+OL1+SOE,O,0F6h,X5M,0,\		     ;IMUL EB
		  _B+OL2+ECO,O,0C0h,X1M,0,\
		  _B+OL2+EAO,-1,_1,O,0D0h,X1M,0,\
		  _B+OL2+EAO,-1,regCL,O,0D2h,X1M,0,\
		  _W+OL1+REO,OW,0Fh,0257o,XRM,0,\  ;IMUL RW, EW
		  OL3+REO+CC3,O,0153o,XRM,0,\	   ;IMUL RW, EW, DC
		  OL3+REO+CW3,O,0151o,XRM,_END	   ;IMUL RW, EW, DW

T_SHLD	      db OL3+ERO+CB3,OW,0Fh,0244o,XRM,0,\
		 OL3+ERO+CL3,OW,0Fh,0245o,XRM,_END

T_SHRD	      db OL3+ERO+CB3,OW,0Fh,0254o,XRM,0,\
		 OL3+ERO+CL3,OW,0Fh,0255o,XRM,_END

T_MOVSX       db WB+OL2+REO,OW,0Fh,0276o,XRM,0,\
		_DW+OL2+REO,OW,0Fh,0277o,XRM,_END

T_MOVZX       db WB+OL2+REO,OW,0Fh,0266o,XRM,0,\
		_DW+OL2+REO,OW,0Fh,0267o,XRM,_END

T_BSWAP       db _D+OL1+SOR,OS,0Fh,'31R',0,_END
T_BSF	      db _W+OL2+REO,OW,0Fh,0274o,XRM,_END
T_BSR	      db _W+OL2+REO,OW,0Fh,0275o,XRM,_END
T_CMPXCHG     db _B+OL2+ERO,OW,0Fh,0B0h,XRM,_END
T_CMPXCHG486  db _B+OL2+ERO,OW,0Fh,0A7h,XRM,_END
T_CMPXCHG8B   db _B+OL2+ERO,OW,0Fh,0C7h,XRM,_END
T_XADD	      db _B+OL2+ERO,OW,0Fh,0C0h,XRM,_END
T_XCHG	      db _W+OL2+EAO,-1,AR,S,'22M',0,\ ;ALPHA:WAS "22R" BUT GENERATED NOP
		 _W+OL2+EAO,AR,-1,S,'22R',0,\
		 _B+OL2+REO,O,0206o,XRM,0,\
		 _B+OL2+ERO,O,0206o,XRM,_END

T_MOV	      db 0 ;DUMMY BYTE
T_LEA	      db _D+OL2+REO,O,0215o,XRM,_END	  ;LEA RW, EN    (X != 3)
T_LSS	      db _D+OL2+REO,OW,0Fh,0262o,XRM,_END ;LSS RW, EF  (X != 3)
T_LFS	      db _D+OL2+REO,OW,0Fh,0264o,XRM,_END ;LFS RW, EF  (X != 3)
T_LGS	      db _D+OL2+REO,OW,0Fh,0265o,XRM,_END ;LGS RW, EF  (X != 3)
T_LES	      db _D+OL2+REO,O,0304o,XRM,_END	  ;LES RW, EF    (X != 3)
T_LDS	      db _D+OL2+REO,O,0305o,XRM,_END	  ;LDS RW, EF    (X != 3)
T_SET0	      db NB+OL1+SOE,OW,0Fh,0220o,X0M,_END
T_SET1	      db NB+OL1+SOE,OW,0Fh,0221o,X0M,_END
T_SET2	      db NB+OL1+SOE,OW,0Fh,0222o,X0M,_END
T_SET3	      db NB+OL1+SOE,OW,0Fh,0223o,X0M,_END
T_SET4	      db NB+OL1+SOE,OW,0Fh,0224o,X0M,_END
T_SET5	      db NB+OL1+SOE,OW,0Fh,0225o,X0M,_END
T_SET6	      db NB+OL1+SOE,OW,0Fh,0226o,X0M,_END
T_SET7	      db NB+OL1+SOE,OW,0Fh,0227o,X0M,_END
T_SET8	      db NB+OL1+SOE,OW,0Fh,0230o,X0M,_END
T_SET9	      db NB+OL1+SOE,OW,0Fh,0231o,X0M,_END
T_SETA	      db NB+OL1+SOE,OW,0Fh,0232o,X0M,_END
T_SETB	      db NB+OL1+SOE,OW,0Fh,0233o,X0M,_END
T_SETC	      db NB+OL1+SOE,OW,0Fh,0234o,X0M,_END
T_SETD	      db NB+OL1+SOE,OW,0Fh,0235o,X0M,_END
T_SETE	      db NB+OL1+SOE,OW,0Fh,0236o,X0M,_END
T_SETF	      db NB+OL1+SOE,OW,0Fh,0237o,X0M,_END
;
T_JCXZ	      db OL1,_JB,O,0343o,_END
T_LOOPNZ      db OL1,_JB,O,0340o,_END
T_LOOPZ       db OL1,_JB,O,0341o,_END
T_LOOP	      db OL1,_JB,O,0342o,_END

T_J0	      db OL1,_JB,O,0160o,0,\
		 OL1,_JW,6,OW,017o,0200o,_END

T_J1	      db OL1,_JB,O,0161o,0,\
		 OL1,_JW,6,OW,017o,0201o,_END

T_J2	      db OL1,_JB,O,0162o,0,\
		 OL1,_JW,6,OW,017o,0202o,_END

T_J3	      db OL1,_JB,O,0163o,0,\
		 OL1,_JW,6,OW,017o,0203o,_END

T_J4	      db OL1,_JB,O,0164o,0,\
		 OL1,_JW,6,OW,017o,0204o,_END

T_J5	      db OL1,_JB,O,0165o,0,\
		 OL1,_JW,6,OW,017o,0205o,_END

T_J6	      db OL1,_JB,O,0166o,0,\
		 OL1,_JW,6,OW,017o,0206o,_END

T_J7	      db OL1,_JB,O,0167o,0,\
		 OL1,_JW,6,OW,017o,0207o,_END

T_J8	      db OL1,_JB,O,0170o,0,\
		 OL1,_JW,6,OW,017o,0210o,_END

T_J9	      db OL1,_JB,O,0171o,0,\
		 OL1,_JW,6,OW,017o,0211o,_END

T_JA	      db OL1,_JB,O,0172o,0,\
		 OL1,_JW,6,OW,017o,0212o,_END

T_JB	      db OL1,_JB,O,0173o,0,\
		 OL1,_JW,6,OW,017o,0213o,_END

T_JC	      db OL1,_JB,O,0174o,0,\
		 OL1,_JW,6,OW,017o,0214o,_END

T_JD	      db OL1,_JB,O,0175o,0,\
		 OL1,_JW,6,OW,017o,0215o,_END

T_JE	      db OL1,_JB,O,0176o,0,\
		 OL1,_JW,6,OW,017o,0216o,_END

T_JF	      db OL1,_JB,O,0177o,0,\
		 OL1,_JW,6,OW,017o,0217o,_END

T_JMP	      db OL1,_JB,O,0353o,0,\	   ;JMP SHORT CB
		 OL1,_JW,5,O,0351o,0,\	   ;JMP NEAR CW
		 _OW+OL1+SOE,O,0377o,X4M,0 ; JMP NEAR EN

T_JMPFAR      db _D+OL1+SOE,O,0377o,X5M,0,\ ; JMP FAR EF
		OL1+SOO_AF,O,0352o,_END ;JMP FAR AF

T_JMPSHORT    db OL1,_JB,O,0353o,_END			;JMP SHORT CB

T_JMPNEAR     db OL1,_JW,5,O,0351o,0,\			  ;JMP NEAR CW
		_OW+OL1+SOE,O,0377o,X4M,_END ; JMP NEAR EN

T_CALL	      db OL1,_JW,5,O,0350o,0,\			    ;CALL CW
		_OW+OL1+SOE,O,0377o,X2M,0      ;CALL EN

T_CALLFAR     db OL1+SOO_AF,O,0232o,0,\  ;CALL AF
		_D+OL1+SOE,O,0377o,X3M,_END ;CALL FAR EF

T_RETF	      db 0,O,0313o,0,\		     ;RETF/RET FAR
		_OW+OL1+SOC,O,0312o,_END ;RETF DW/RET FAR DW

T_ENTER       db WB+OL2+EAO,CO,CO,O,0310o,_END
T_BOUND       db _W+OL2+REO,O,0142o,XRM,_END	       ;BOUND RW, ED
;/*byte T_INT={  NB+OL1+SO3,O,0o314,0, // !!! No interrupts in Win32
;                NB+OL1+SOC,O,0o315,_END};*/
T_IN	      db NB+OL2+EAO,AR,CO,O,0344o,0,\	  ;IN AL, DB
		WB+OL2+EAO,AR,CO,O,0345o,0,\	  ;IN AX,CO
		_B+OL2+EAO,AR,regDX,O,0354o,0,\   ;IN AL,DX
		_OW+OL2+EAO,AR,regDX,O,0355o,_END ;IN AX, DX

T_OUT	      db NB+OL2+EAO,CO,AR,O,0346o,0,\	   ;OUT DB, AL
		 BW+OL2+EAO,CO,AR,O,0347o,0,\	   ;OUT DB, AX
		 WB+OL2+EAO,regDX,AR,O,0356o,0,\   ;OUT DX, AL
		 _OW+OL2+EAO,regDX,AR,O,0357o,_END ;OUT DX, AX

T_PUSH	      db OL1+SOO_CC,O,0152o,0,\
		 _D+OL1+SOR,S,'12R',0,\
		 _D+OL1+SOE,O,0377o,X6M,0,\
		 _D+OL1+SOC,O,0150o,0,\
		 OL1+SOO_FS,OW,0Fh,0240o,0,\
		 OL1+SOO_GS,OW,0Fh,0250o,0,\
		 _OW+OL1+SOS,S,'0S6',0,\ ;WIEDER ANS ENDE PACKEN
		 _END

T_RET	      db _OW+OL1+SOC,O,0302o,0,\ ;RET DW
		 O,0303o,0,_END        ;RET

T_POP	      db _D+OL1+SOR,S,'13R',0,\
		 _D+OL1+SOE,O,0217o,X0M,0,\
		 OL1+SOO_FS,OW,0Fh,0241o,0,\
		 OL1+SOO_GS,OW,0Fh,0251o,0,\
		 _OW+OL1+SOS,S,'0S7',0,_END

T_ARPL	      db _W+OL2+ERO,O,0143o,XRM,_END	;ARPL ES, RW
T_SLDT	      db _OW+OL1+SOE,OW,0Fh,0,X0M,_END	;SLDT EW
T_STR	      db _OW+OL1+SOE,OW,0Fh,0,X1M,_END	;STR EW
T_LLDT	      db _OW+OL1+SOE,OW,0Fh,0,X2M,_END	;LLDT EW
T_LTR	      db _OW+OL1+SOE,OW,0Fh,0,X3M,_END	;LTR EW
T_VERR	      db _OW+OL1+SOE,OW,0Fh,0,X4M,_END	;VERR EW
T_VERW	      db _OW+OL1+SOE,OW,0Fh,0,X5M,_END	;VERW EW
T_LAR	      db _OW+OL1+REO,OW,0Fh,2,XRM,_END	;LAR RW, EW
T_LSL	      db _OW+OL1+REO,OW,0Fh,3,XRM,_END	;LSL RW, EW
T_SGDT	      db _OW+OL1+SOM,OW,0Fh,1,X0M,_END	;SGDT EP
T_SIDT	      db _OW+OL1+SOM,OW,0Fh,1,X1M,_END	;SIDT EP
T_LGDT	      db _OW+OL1+SOM,OW,0Fh,1,X2M,_END	;LGDT EP
T_LIDT	      db _OW+OL1+SOM,OW,0Fh,1,X3M,_END	;LIDT EP
T_SMSW	      db _OW+OL1+SOE,OW,0Fh,1,X4M,_END	;SMSW EW
T_LMSW	      db _OW+OL1+SOE,OW,0Fh,1,X6M,_END	;LMSW EW
;===== X486 INSTRUCTIONS
T_INVLPD      db _OW+OL1+SOM,OW,0Fh,020o,X7M,_END ;INVLPG EA
;===== INTEL PENTIUM INSTRUCTIONS
T_CMPX8       db _OW+OL1+SOE,OW,0Fh,0C7h,X1M,_END ;CMPX8 EW 5 IF EDXEAX=MQ THEN MQ:=ECXEBX, ELSE EAXEDX:=MQ
T_CMOV	      db _W+OL2,REO,OW,0Fh,040h,XRM,_END
;===== MMX INSTRUCTIONS
T_EMMS	      db _END
T_UMOV	      db _B+OL2,DER,OW,0Fh,010h,XRM,_END

_Q    = 1
MMXE  = 1
MMXP  = 1
MMXPI = 1
MMXPE = 1
MMXEP = 1

T_PUNPCKLBW   db _D+OL2,MMXE,OW,0Fh,060h,_END
T_PUNPCKLWD   db _D+OL2,MMXE,OW,0Fh,061h,_END
T_PUNPCKLDQ   db _D+OL2,MMXE,OW,0Fh,062h,_END
T_PACKSSWB    db _Q+OL2,MMXE,OW,0Fh,063h,_END
T_PCMPGTB     db _Q+OL2,MMXE,OW,0Fh,064h,_END
T_PCMPGTW     db _Q+OL2,MMXE,OW,0Fh,065h,_END
T_PCMPGTD     db _Q+OL2,MMXE,OW,0Fh,066h,_END
T_PACKUSWB    db _Q+OL2,MMXE,OW,0Fh,067h,_END
T_PCMPEQB     db _Q+OL2,MMXE,OW,0Fh,074h,_END
T_PCMPEQW     db _Q+OL2,MMXE,OW,0Fh,075h,_END
T_PCMPEQD     db _Q+OL2,MMXE,OW,0Fh,076h,_END
T_PSRLW       db _Q+OL2,MMXE,OW,0Fh,0D1h,_END
T_PSRLD       db _Q+OL2,MMXE,OW,0Fh,0D2h,_END
T_PSRLQ       db _Q+OL2,MMXE,OW,0Fh,0D3h,_END
T_PMULLW      db _Q+OL2,MMXE,OW,0Fh,0D5h,_END
T_PSRAW       db _Q+OL2,MMXE,OW,0Fh,0E1h,_END
T_PSRAD       db _Q+OL2,MMXE,OW,0Fh,0E2h,_END
T_PMULHW      db _Q+OL2,MMXE,OW,0Fh,0E5h,_END
T_PSLLW       db _Q+OL2,MMXE,OW,0Fh,0F1h,_END
T_PSLLD       db _Q+OL2,MMXE,OW,0Fh,0F2h,_END
T_PSLLQ       db _Q+OL2,MMXE,OW,0Fh,0F3h,_END
T_PMADDWD     db _Q+OL2,MMXE,OW,0Fh,0F5h,_END
T_PUNPCKHBW   db _Q+OL2,MMXE,OW,0Fh,068h,_END
T_PUNPCKHWD   db _Q+OL2,MMXE,OW,0Fh,069h,_END
T_PUNPCKHDQ   db _Q+OL2,MMXE,OW,0Fh,06Ah,_END
T_PACKSSDW    db _Q+OL2,MMXE,OW,0Fh, 06Bh,_END

T_MOVD	      db _D+OL2,MMXPE,OW,0Fh,06Eh,0,\
		 _Q+OL2,MMXEP,OW,0Fh,07Eh,_END

T_MOVQ	      db _Q+OL2,MMXE,OW,0Fh,06Fh,0,\
		 _Q+OL2,MMXE,OW,0Fh,07Fh,_END

T_PSUBUSB     db _Q+OL2,MMXE,OW,0Fh,0D8h,_END
T_PSUBUSW     db _Q+OL2,MMXE,OW,0Fh,0D9h,_END
T_PAND	      db _Q+OL2,MMXE,OW,0Fh,0DBh,_END
T_PADDUSB     db _Q+OL2,MMXE,OW,0Fh,0DCh,_END
T_PADDUSW     db _Q+OL2,MMXE,OW,0Fh,0DDh,_END
T_PANDN       db _Q+OL2,MMXE,OW,0Fh,0DFh,_END
T_PSUBSB      db _Q+OL2,MMXE,OW,0Fh,0E8h,_END
T_PSUBSW      db _D+OL2,MMXE,OW,0Fh,0E9h,_END
T_POR	      db _Q+OL2,MMXE,OW,0Fh,0EBh,_END
T_PADDSB      db _Q+OL2,MMXE,OW,0Fh,0ECh,_END
T_PADDSW      db _Q+OL2,MMXE,OW,0Fh,0EDh,_END
T_PXOR	      db _Q+OL2,MMXE,OW,0Fh,0EFh,_END
T_PSUBB       db _Q+OL2,MMXE,OW,0Fh,0F8h,_END
T_PSUBW       db _Q+OL2,MMXE,OW,0Fh,0F9h,_END
T_PSUBD       db _Q+OL2,MMXE,OW,0Fh,0FAh,_END
T_PADDB       db _Q+OL2,MMXE,OW,0Fh,0FCh,_END
T_PADDW       db _Q+OL2,MMXE,OW,0Fh,0FDh,_END
T_PADDD       db _Q+OL2,MMXE,OW,0Fh,0FEh,_END
T_PSRL	      db _Q+OL2,MMXPI,OW,0Fh,_END
T_PSRA	      db _Q+OL2,MMXPI,OW,0Fh,_END
T_PSLL	      db _Q+OL2,MMXE,OW,0Fh,_END

Mnemonics db 'ADD', 0, 'OR', 0, 'ADC', 0, 'SBB/BC', 0, 'AND', 0, 'SUB', 0, 'XOR', 0, 'CMP', 0,\
	     'MUL', 0, '-', 0, 'DIV', 0, 'IDIV', 0,\
	     'ROL', 0, 'ROR', 0, 'RCL', 0, 'RCR', 0, 'SHL/AL', 0, 'SHR', 0, '-', 0, 'SAR', 0,\
	     'BT', 0, 'BTS', 0, 'BTR', 0, 'BTC', 0,\
	     'NOT', 0, 'NEG', 0, 'INC', 0, 'DEC', 0, 'TEST', 0,  'IMUL', 0, 'SHLD', 0, 'SHRD', 0,\
	     'DAA', 0, 'DAS', 0, 'AAA', 0, 'AAS', 0, 'AAM', 0, 'AAD', 0,\
	     'MOVSX', 0, 'MOVZX', 0, 'CBW', 0, 'CWDE', 0, 'CWD', 0, 'CDQ', 0,\
	     'BSWAP', 0, 'XLAT/LATB', 0, 'BSF', 0, 'BSR', 0,\
	     'CMPXCHG', 0, 'CMPXCHG486', 0, 'CMPXCHG8B', 0, 'XADD', 0,\
	     'NOP', 0, 'WAIT', 0, 'LOCK', 0, 'HLT', 0,\
	     'INTO', 0, 'IRET', 0,\
	     'POPF', 0, 'POPFD', 0, 'PUSHF', 0, 'PUSHFD', 0, 'SAHF', 0, 'LAHF', 0,\
	     'CMC', 0, 'CLC', 0, 'STC', 0, 'CLI', 0, 'STI', 0, 'CLD', 0, 'STD', 0,\
	     'PUSH', 0, 'PUSHA', 0, 'PUSHAD', 0, 'POP', 0, 'POPA', 0, 'POPAD', 0,\
	     'XCHG', 0, 'MOV', 0, 'LEA', 0, 'LSS', 0, 'LFS', 0, 'LGS', 0, 'LES', 0, 'LDS', 0,\
	     'ADDR', 0, 'RAND', 0,\
	     'IN', 0, 'OUT', 0, 'INSB', 0, 'INSW', 0, 'INSD', 0, 'OUTSB', 0, 'OUTSW', 0, 'OUTSD', 0,\
	     'MOVSB', 0, 'MOVSW', 0, 'MOVSD', 0, 'CMPSB', 0, 'CMPSW', 0, 'CMPSD', 0,\
	     'STOSB', 0, 'STOSW', 0, 'STOSD', 0, 'LODSB', 0, 'LODSW', 0, 'LODSD', 0,\
	     'SCASB', 0, 'SCASW', 0, 'SCASD', 0, 'REP/EPNE/EPNZ', 0, 'REPE/EPZ', 0,\
	     'JCXZ/ECXZ', 0, 'LOOP', 0, 'LOOPZ/OOPE', 0, 'LOOPNZ/OOPNE', 0,\
	     'JO', 0, 'JNO', 0, 'JC/B/NAE', 0, 'JNC/AE/NB', 0,\
	     'JE/Z', 0, 'JNE/NZ', 0, 'JBE/NA', 0, 'JA/NBE', 0,\
	     'JS', 0, 'JNS', 0, 'JP/PE', 0, 'JNP/PO', 0, 'JL/NGE', 0, 'JGE/NL', 0,\
	     'JLE/NG', 0, 'JG/NLE', 0,\
	     'SETO', 0, 'SETNO', 0, 'SETC/ETB/ETNAE', 0, 'SETNC/ETAE/ETNB', 0,\
	     'SETE/ETZ', 0, 'SETNE/ETNZ', 0, 'SETBE/ETNA', 0, 'SETA/ETNBE', 0,\
	     'SETS', 0, 'SETNS', 0, 'SETP/ETPE', 0, 'SETNP/ETPO', 0, 'SETL/ETNGE', 0, 'SETGE/ETNL', 0,\
	     'SETLE/ETNG', 0, 'SETG/ETNLE', 0,\
	     'JMPS', 0, 'JMPN', 0, 'JMPF', 0, 'JMP', 0,\
	     'CALL', 0, 'CALLF', 0, 'RET', 0, 'RETF', 0,\
	     'ENTER', 0, 'LEAVE', 0, 'BOUND', 0, 'ARPL', 0,\
	     'SLDT', 0, 'STR', 0, 'LLDT', 0, 'LTR', 0, 'VERR', 0, 'VERW', 0, 'LAR', 0, 'LSL', 0,\
	     'SGDT', 0, 'SIDT', 0, 'LGDT', 0, 'LIDT', 0, 'SMSW', 0, 'LMSW', 0, 'CLTS', 0,\
	     'INVD', 0, 'WBINVD', 0, 'INVLPD', 0,\
	     'WRMSR', 0, 'CMPX8/MPXCHG8B', 0, 'CPUID', 0, 'RDMSR', 0, 'RDTSC', 0, 'RSM', 0,\
	     'RDPMC', 0, 'UD2', 0, 'EMMX', 0, 'SETALC', 0,\
	     'MOVD', 0, 'MOVQ', 0,\
	     'PACKUS/PACKUSWB', 0,\
	     'PACKSSWB', 0, 'PACKSSDW', 0,\
	     'PUNPCKHBW', 0, 'PUNPCKHWD', 0,\
	     'PUNPCKHDQ', 0,\
	     'PUNPCKLBW', 0, 'PUNPCKLWD', 0,\
	     'PUNPCKLDQ', 0,\
	     'PADDB', 0, 'PADDW', 0, 'PADDD', 0,\
	     'PADDSB', 0, 'PADDSW', 0,\
	     'PADDUSB', 0, 'PADDUSW', 0,\
	     'PSUBB', 0, 'PSUBW', 0, 'PSUBD', 0,\
	     'PSUBSB', 0, 'PSUBSW', 0, 'PSUBUSB', 0, 'PSUBUSW', 0,\
	     'PMULH/PMULHW', 0, 'PMULL/PMULLW', 0, 'PMADD/PMADDWD', 0,\
	     'PSLLW', 0, 'PSLLD', 0, 'PSLLQ', 0, 'PSRLW', 0, 'PSRLD', 0, 'PSRLQ', 0,\
	     'PSRAW', 0, 'PSRAD', 0, 'PCMPEQB', 0, 'PCMPEQW', 0, 'PCMPEQD', 0,\
	     'PCMPGTB', 0, 'PCMPGTW', 0, 'PCMPGTD', 0, 'PAND', 0, 'PANDN', 0, 'POR', 0, 'PXOR', 0, 'EMMS', 0, _END, 0

TAB_MNEMONICS dd T_NOT, T_NEG, T_INC, T_DEC, T_TEST,\
		 T_IMUL, T_SHLD, T_SHRD,\
		 T_DAA, T_DAS, T_AAA, T_AAS, T_AAM, T_AAD,\
		 T_MOVSX, T_MOVZX, T_CBW, T_CWDE, T_CWD, T_CDQ,\
		 T_BSWAP, T_XLATB,\
		 T_BSF, T_BSR, T_CMPXCHG, T_CMPXCHG486, T_CMPXCHG8B, T_XADD,\
		 T_NOP, T_WAIT, T_LOCK, T_HLT,\  ;T_INT,
		 T_INTO, T_IRET,\
		 T_POPF, T_POPFD, T_PUSHF, T_PUSHFD,\
		 T_SAHF, T_LAHF,\
		 T_CMC, T_CLC, T_STC, T_CLI, T_STI, T_CLD, T_STD,\
		 T_PUSH, T_PUSHA, T_PUSHAD,\
		 T_POP, T_POPA, T_POPAD,\
		 T_XCHG, T_MOV,\
		 T_LEA, T_LSS, T_LFS, T_LGS, T_LES, T_LDS,\
		 T_ADDR, T_RAND,\
		 T_IN, T_OUT,\
		 T_INSB, T_INSW, T_INSD,\
		 T_OUTSB, T_OUTSW, T_OUTSD,\
		 T_MOVSB, T_MOVSW, T_MOVSD,\
		 T_CMPSB, T_CMPSW, T_CMPSD,\
		 T_STOSB, T_STOSW, T_STOSD,\
		 T_LODSB, T_LODSW, T_LODSD,\
		 T_SCASB, T_SCASW, T_SCASD,\
		 T_REP,\
		 T_REPE,\
		 T_JCXZ, T_LOOP, T_LOOPZ, T_LOOPNZ,\
		 T_J0, T_J1, T_J2, T_J3,\
		 T_J4, T_J5, T_J6, T_J7,\
		 T_J8, T_J9, T_JA, T_JB,\
		 T_JC, T_JD, T_JE, T_JF,\
		 T_SET0, T_SET1, T_SET2, T_SET3,\
		 T_SET4, T_SET5, T_SET6, T_SET7,\
		 T_SET8, T_SET9, T_SETA, T_SETB,\
		 T_SETC, T_SETD, T_SETE, T_SETF,\
		 T_JMPSHORT, T_JMPNEAR, T_JMPFAR, T_JMP,\
		 T_CALL, T_CALLFAR,\
		 T_RET, T_RETF,\
		 T_ENTER, T_LEAVE,\
		 T_BOUND, T_ARPL,\
		 T_SLDT, T_STR, T_LLDT, T_LTR, T_VERR, T_VERW,\
		 T_LAR, T_LSL,\
		 T_SGDT, T_SIDT, T_LGDT, T_LIDT,\
		 T_SMSW, T_LMSW, T_CLTS,\
		 T_INVD, T_WBINVD, T_INVLPD,\				;INTEL PENTIUM INSTRUCTIONS
		 T_WRMSR, T_CMPX8, T_CPUID, T_RDMSR, T_RDTSC, T_RSM,\	;INTEL PENTIUM PRO INSTRUCTIONS
		 T_RDPMC, T_UD2, T_EMMX, T_SALC,\			;MMX INSTRUCTIONS
		 T_MOVD, T_MOVQ,\		  ;MOVE MMX REG
		 T_PACKUSWB,\					   ;PACK MMX REG WITH UNSIGNED SATURATION
		 T_PACKSSWB, T_PACKSSDW,\	  ;PACK MMX REG WITH SIGNED SATURATION
		 T_PUNPCKHBW, T_PUNPCKHWD, T_PUNPCKHDQ,\  ;UNPACK HIGH ORDER
		 T_PUNPCKLBW, T_PUNPCKLWD, T_PUNPCKLDQ,\  ;UNPACK LOW ORDER
		 T_PADDB, T_PADDW, T_PADDD,\   ;ADD MMX REG WITH WRAP-AROUND
		 T_PADDSB, T_PADDSW,\		  ;" WITH SIGNED SATURATION
		 T_PADDUSB, T_PADDUSW,\ 	  ;" WITH UNSIGNED SATURATION
		 T_PSUBB, T_PSUBW, T_PSUBD,\   ;SUBTRACT MMX REG
		 T_PSUBSB, T_PSUBSW,\
		 T_PSUBUSB, T_PSUBUSW,\
		 T_PMULHW,\
		 T_PMULLW,\
		 T_PMADDWD,\
		 T_PSLLW, T_PSLLD, T_PSLLQ,\
		 T_PSRLW, T_PSRLD, T_PSRLQ,\
		 T_PSRAW, T_PSRAD,\
		 T_PCMPEQB, T_PCMPEQW, T_PCMPEQD,\
		 T_PCMPGTB, T_PCMPGTW, T_PCMPGTD,\
		 T_PAND,\
		 T_PANDN,\
		 T_POR,\
		 T_PXOR,\
		 T_EMMS