/*
Copyright (c) 2012-2015, Alexey Frunze
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*****************************************************************************/
/*                                                                           */
/*                                Smaller C                                  */
/*                                                                           */
/*                 A simple and small single-pass C compiler                 */
/*                                                                           */
/*                           GP32 code generator                             */
/*                                                                           */
/*****************************************************************************/

#define MAX_GLOBALS_TABLE_LEN 16384

STATIC
void GenInit(void)
{
	// initialization of target-specific code generator
	SizeOfWord = 4;
	OutputFormat = FormatSegmented;
	/*CodeHeaderFooter[0] = "\tsection .text";
	DataHeaderFooter[0] = "\tsection .data";
	RoDataHeaderFooter[0] = "\tsection .rodata";
	BssHeaderFooter[0] = "\tsection .bss";*/
	CodeHeaderFooter[0] = "";
	DataHeaderFooter[0] = "";
	RoDataHeaderFooter[0] = "";
	BssHeaderFooter[0] = "";
}

STATIC
int GenInitParams(int argc, char** argv, int* idx)
{
	(void)argc;
	(void)argv;
	(void)idx;
	// initialization of target-specific code generator with parameters
	
	return 0;
}

STATIC
void GenInitFinalize(void)
{
	// finalization of initialization of target-specific code generator
}

STATIC
void GenStartCommentLine(void)
{
	printf2("\t; ");
}

STATIC
void GenWordAlignment(int bss)
{
	(void)bss;
	printf2("\talign($2)\n");
}

STATIC
void GenLabel(char* Label, int Static)
{
	{
		if (!Static && GenExterns)
			printf2("\tglobal(._%s)\n", Label);
		printf2("._%s\n", Label);
	}
	GenAddGlobal(Label, 1);
}

STATIC
void GenPrintLabel(char* Label)
{
	{
		if (isdigit(*Label))
			printf2(".L%s", Label);
		else
			printf2("._%s", Label);
	}
}

STATIC
void GenNumLabel(int Label)
{
	printf2(".L%d\n", Label);
}

STATIC
void GenPrintNumLabel(int label)
{
	printf2(".L%d", label);
}

STATIC
void GenZeroData(unsigned Size, int bss)
{
	(void)bss;
	printf2("\tfill($%d)\n", Size);
}

STATIC
void GenIntData(int Size, int Val)
{
	Val = truncInt(Val);
	if (Size == 1)
		printf2("\tdb($%d)\n", Val);
	else if (Size == 2)
		printf2("\tdw($%d)\n", Val);
	else if (Size == 4)
		printf2("\tdd($%d)\n", Val);
}

STATIC
void GenStartAsciiString(void)
{
	printf2("\ttext(");
}

STATIC
void GenEndAsciiString(void)
{
	printf2(")\n");
}

char GlobalsTable[MAX_GLOBALS_TABLE_LEN];
int GlobalsTableLen = 0;


STATIC
void GenAddGlobal(char* s, int use)
{
	int i = 0;
	int l;
	if (GenExterns)
	{
		while (i < GlobalsTableLen)
		{
			if (!strcmp(GlobalsTable + i + 2, s))
			{
				GlobalsTable[i] |= use;
				return;
			}
			i += GlobalsTable[i + 1] + 2;
		}
		l = strlen(s) + 1;
		if (GlobalsTableLen + l + 2 > MAX_GLOBALS_TABLE_LEN)
			error("Table of globals exhausted\n");
		GlobalsTable[GlobalsTableLen++] = use;
		GlobalsTable[GlobalsTableLen++] = l;
		memcpy(GlobalsTable + GlobalsTableLen, s, l);
		GlobalsTableLen += l;
	}
}

STATIC
void GenAddrData(int Size, char* Label, int ofs)
{
	ofs = truncInt(ofs);
	if (Size == 1)
		printf2("\t.db\t");
	else if (Size == 2)
		printf2("\t.dw\t");
	else if (Size == 4)
		printf2("\t.dd\t");
	GenPrintLabel(Label);
	if (ofs)
		printf2(" %+d", ofs);
	puts2("");
	if (!isdigit(*Label))
		GenAddGlobal(Label, 2);
}

STATIC
int GenFxnSizeNeeded(void)
{
	return 0;
}

STATIC
void GenRecordFxnSize(char* startLabelName, int endLabelNo)
{
	(void)startLabelName;
	(void)endLabelNo;
}

#define Gp32InstrNop		0x00

#define Gp32InstrMov		0x01

#define Gp32InstrLB			0x02
#define Gp32InstrLW			0x03
#define Gp32InstrL			0x04
#define Gp32InstrSB			0x05
#define Gp32InstrSW			0x06
#define Gp32InstrS			0x07

#define Gp32InstrAdd		0x0A
#define Gp32InstrSub		0x0B
#define Gp32InstrRSub		0x0C
#define Gp32InstrAnd		0x0D
#define Gp32InstrOr			0x0E
#define Gp32InstrXor		0x0F
#define Gp32InstrNot		0x10
#define Gp32InstrLLS		0x11
#define Gp32InstrLRS		0x12
#define Gp32InstrARS		0x13
#define Gp32InstrMul		0x14
#define Gp32InstrSDiv		0x15
#define Gp32InstrDiv		0x16

#define Gp32InstrRJmp		0x17
#define Gp32InstrRCall		0x18
#define Gp32InstrCall		0x19
#define Gp32InstrRet		0x1A
#define Gp32InstrPush		0x1B
#define Gp32InstrPop		0x1C

#define Gp32InstrJE			0x20
#define Gp32InstrJAE		0x21
#define Gp32InstrJBE		0x22
#define Gp32InstrJA			0x23
#define Gp32InstrJB			0x24
#define Gp32InstrJZ			0x25
#define Gp32InstrJNZ		0x26
#define Gp32InstrJNE		0x27
#define Gp32InstrIf			0x28

int GenMemSize;
int GenMemEnable[2] = { 0, 0 };
int GenNowOperand;

STATIC
void GenPrintInstr(int instr, int val)
{
	GenMemEnable[0] = 0;
	GenMemEnable[1] = 0;
	
	char* p = "";

	(void)val;

	switch (instr)
	{
	case Gp32InstrNop	: p = "nop"; break;

	case Gp32InstrMov	: p = "mov"; break;

	case Gp32InstrLB	: p = "mov"; GenMemSize = 1; GenMemEnable[1] = 1; break;
	case Gp32InstrLW	: p = "mov"; GenMemSize = 2; GenMemEnable[1] = 1; break;
	case Gp32InstrL		: p = "mov"; GenMemSize = 4; GenMemEnable[1] = 1; break;
	case Gp32InstrSB	: p = "mov"; GenMemSize = 1; GenMemEnable[0] = 1; break;
	case Gp32InstrSW	: p = "mov"; GenMemSize = 2; GenMemEnable[0] = 1; break;
	case Gp32InstrS		: p = "mov"; GenMemSize = 4; GenMemEnable[0] = 1; break;
	
	case Gp32InstrAdd	: p = "add"; break;
	case Gp32InstrSub	: p = "sub"; break;
	case Gp32InstrRSub	: p = "sub"; break;
	case Gp32InstrAnd	: p = "and"; break;
	case Gp32InstrOr	: p = "or"; break;
	case Gp32InstrXor	: p = "xor"; break;
	case Gp32InstrNot	: p = "not"; break;
	case Gp32InstrLLS	: p = "sal"; break;
	case Gp32InstrLRS	: p = "sar"; break;
	case Gp32InstrARS	: p = "sar"; break;
	case Gp32InstrMul	: p = "umul"; break;
	case Gp32InstrSDiv	: p = "idiv"; break;
	case Gp32InstrDiv	: p = "udiv"; break;

	case Gp32InstrRJmp	: p = "jmp"; break;
	case Gp32InstrRCall	: p = "call"; break;
	case Gp32InstrCall	: p = "call"; break;
	case Gp32InstrRet	: p = "ret"; break;
	case Gp32InstrPush	: p = "push"; break;
	case Gp32InstrPop	: p = "pop"; break;

	case Gp32InstrIf	: p = "cmp"; break;
	
	case Gp32InstrJE	: p = "je"; break;
	case Gp32InstrJAE	: p = "jae"; break;
	case Gp32InstrJBE	: p = "jbe"; break;
	case Gp32InstrJA	: p = "ja"; break;
	case Gp32InstrJB	: p = "jb"; break;
	case Gp32InstrJZ	: p = "jz"; break;
	case Gp32InstrJNZ	: p = "jnz"; break;
	case Gp32InstrJNE	: p = "jne"; break;
	}

	printf2("\t%s", p);
}

#define Gp32OpReg0											0x00
//...
#define Gp32OpRegY											0x21
#define Gp32OpRegBp											0x22
#define Gp32OpRegSp											0x20
//...
#define Gp32OpRegFlags										0x23

#define Gp32OpIndReg0										0x30
//...
#define Gp32OpIndRegY										0x51
#define Gp32OpIndRegBp										0x52
#define Gp32OpIndRegSp										0x50
//...
#define Gp32OpIndRegFlags									0x53

#define Gp32OpConst											0x80
#define Gp32OpLabel											0x81
#define Gp32OpNumLabel										0x82

#define MAX_TEMP_REGS 29 // this many temp registers used beginning with R1 to hold subexpression results
#define TEMP_REG_A 30 // two temporary registers used for momentary operations, similarly to the MIPS AT/R1 register
#define TEMP_REG_B 31

#define tokRevMinus		0x100 // reversed subtraction, RSB
#define tokRevIdent		0x101
#define tokRevLocalOfs	0x102

int GenRegsUsed; // bitmask of registers used by the function being compiled

int GenMemPrefix[] = { 'x', 'b', 'w', 'x', 'd' };

STATIC
void GenPrintOperand(int op, long val)
{
	if (op >= Gp32OpReg0 && op <= Gp32OpRegFlags)
	{
		GenRegsUsed |= 1 << op;
		switch (op)
		{
		case Gp32OpRegBp:		printf2("%%r34");		 break;
		case Gp32OpRegSp:		printf2("%%r32");		 break;
		case Gp32OpRegY:		printf2("%%r33");		 break;
		case Gp32OpRegFlags:	printf2("%%r35"); break;
		default:				printf2("%%r%d", op);
		}
	}
	else if (op >= Gp32OpIndReg0 && op <= Gp32OpIndRegFlags)
	{
		if (val) {
			if(GenMemEnable[GenNowOperand]) printf2("%c@[", GenMemPrefix[GenMemSize]);
			else printf2("@[", val);
		} else {
			if(GenMemEnable[GenNowOperand]) printf2("%c", GenMemPrefix[GenMemSize]);
		}
		GenPrintOperand(op - Gp32OpIndReg0, 0);
		val = truncInt(val);
		if (val)
			printf2(":%d]", val);
	}
	else
	{
		if(GenMemEnable[GenNowOperand]) printf2("%c", GenMemPrefix[GenMemSize]);
		switch (op)
		{
		case Gp32OpConst: printf2("$%10ld", truncInt(val)); break;
		case Gp32OpLabel: GenPrintLabel(IdentTable + val); break;
		case Gp32OpNumLabel: GenPrintNumLabel(val); break;

		default:
			//error("WTF!\n");
			errorInternal(100);
			break;
		}
	}
}

STATIC
void GenPrintOperandSeparator(void)
{
	printf2(",");
}

STATIC
void GenPrintOperandStarter(void)
{
	printf2("(");
}

STATIC
void GenPrintOperandEnder(void)
{
	printf2(")");
}

STATIC
void GenPrintNewLine(void)
{
	puts2("");
}

STATIC
void GenPrintInstrNoOperand(int instr, long instrval)
{
	GenPrintInstr(instr, instrval);
	GenPrintOperandStarter();
	GenPrintOperandEnder();
	GenPrintNewLine();
}

STATIC
void GenPrintInstr1Operand(int instr, int instrval, int operand, long operandval)
{
	GenPrintInstr(instr, instrval);
	GenPrintOperandStarter();
	GenNowOperand = 0;
	GenPrintOperand(operand, operandval);
	GenPrintOperandEnder();
	GenPrintNewLine();
}

STATIC
void GenPrintInstr2Operands(int instr, int instrval, int operand1, long operand1val, int operand2, long operand2val)
{
	GenPrintInstr(instr, instrval);
	GenPrintOperandStarter();
	GenNowOperand = 0;
	GenPrintOperand(operand1, operand1val);
	GenPrintOperandSeparator();
	GenNowOperand = 1;
	GenPrintOperand(operand2, operand2val);
	GenPrintOperandEnder();
	GenPrintNewLine();
}

STATIC
void GenExtendRegIfNeeded(int reg, int opSz)
{
	if (opSz == -1)
	{
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 TEMP_REG_A, 0,
													 reg, 0);
		GenPrintInstr2Operands(Gp32InstrLRS, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 7);
		GenPrintInstr2Operands(Gp32InstrMul, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 0xFFFFFF00);
		GenPrintInstr2Operands(Gp32InstrOr, 0,
													 reg, 0,
													 TEMP_REG_A, 0);
	}
	else if (opSz == 1)
	{
		GenPrintInstr2Operands(Gp32InstrAnd, 0,
													 reg, 0,
													 Gp32OpConst, 0xFF);
	}
	else if (opSz == -2)
	{
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 TEMP_REG_A, 0,
													 reg, 0);
		GenPrintInstr2Operands(Gp32InstrLRS, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 15);
		GenPrintInstr2Operands(Gp32InstrMul, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 0xFFFF0000);
		GenPrintInstr2Operands(Gp32InstrOr, 0,
													 reg, 0,
													 TEMP_REG_A, 0);
	}
	else if (opSz == 2)
	{
		GenPrintInstr2Operands(Gp32InstrAnd, 0,
													 reg, 0,
													 Gp32OpConst, 0xFFFF);
	}
}

STATIC
void GenJumpUncond(int label)
{
	GenPrintInstr1Operand(Gp32InstrRJmp, 0,
												Gp32OpNumLabel, label);
}

extern int GenWreg; // GenWreg is defined below

#ifndef USE_SWITCH_TAB
STATIC
void GenJumpIfEqual(int val, int label)
{
#ifndef NO_ANNOTATIONS
	printf2("\t; JumpIfEqual\n");
#endif
	GenPrintInstr2Operands(Gp32InstrIf, 0,
												 GenWreg, 0,
												 Gp32OpConst, val);
	GenPrintInstr1Operand(Gp32InstrJE, 0,
												Gp32OpNumLabel, label);
}
#endif

STATIC
void GenJumpIfZero(int label)
{
#ifndef NO_ANNOTATIONS
	printf2("\t; JumpIfZero\n");
#endif
	GenPrintInstr2Operands(Gp32InstrIf, 0,
												 GenWreg, 0,
												 GenWreg, 0);
	GenPrintInstr1Operand(Gp32InstrJZ, 0,
												Gp32OpNumLabel, label);
}

STATIC
void GenJumpIfNotZero(int label)
{
#ifndef NO_ANNOTATIONS
	printf2("\t; JumpIfNotZero\n");
#endif
	GenPrintInstr2Operands(Gp32InstrIf, 0,
												 GenWreg, 0,
												 GenWreg, 0);
	GenPrintInstr1Operand(Gp32InstrJNZ, 0,
												Gp32OpNumLabel, label);
}

STATIC
void GenSaveRestoreRegs(int save)
{
	int rstart, rstop, rinc, r;
	int mask = GenRegsUsed;
	mask &= ~(1 << Gp32OpReg0); // not preserved
//	mask &= ~(1 << Gp32OpRegY); // TBD??? Y is preserved, right???
	mask &= ~(1 << Gp32OpRegBp); // taken care of
	mask &= ~(1 << Gp32OpRegSp); // taken care of
	mask &= ~(1 << Gp32OpRegFlags); // TBD??? flags aren't preserved, right???

	if (save)
		rstart = Gp32OpReg0, rstop = Gp32OpRegFlags, rinc = 1;
	else
		rstart = Gp32OpRegFlags, rstop = Gp32OpReg0, rinc = -1;

	for (r = rstart; r != rstop + rinc; r += rinc)
	{
		int used = (mask & (1 << r)) != 0;
		if (save || used)
		{
			if(used) printf2(save ? "\tpush(" : "\tpop(");
			else printf2(save ? "\t;push " : "\t;pop");
			GenPrintOperand(r, 0);
			if(used) GenPrintOperandEnder();
			else printf2("");
			GenPrintNewLine();
		}
	}
	GenRegsUsed = mask; // undo changes in GenRegsUsed by GenPrintOperand()
}

void GenIsrProlog(void)
{
	// TBD???
}

void GenIsrEpilog(void)
{
	// TBD???
}

long GenPrologPos;
long GenPrologSiz;

STATIC
void GenWriteFrameSize(void)
{
	unsigned size = -CurFxnMinLocalOfs;
	//if(!size) GenStartCommentLine();
	GenPrintInstr2Operands(Gp32InstrSub, 0,
										Gp32OpRegSp, 0,
										Gp32OpConst, size*4);
	GenSaveRestoreRegs(1);
}

STATIC
void GenUpdateFrameSize(void)
{
	long pos;
	pos = ftell(OutFile);
	fseek(OutFile, GenPrologPos, SEEK_SET);
	GenWriteFrameSize();
	fseek(OutFile, GenPrologSiz, SEEK_SET);
	GenPrologSiz -= GenPrologPos;
	fseek(OutFile, pos, SEEK_SET);
}

STATIC
void GenFxnProlog(void)
{
	GenRegsUsed = 0;

	GenPrintInstr1Operand(Gp32InstrPush, 0,
												Gp32OpRegBp, 0);

	GenPrintInstr2Operands(Gp32InstrMov, 0,
												 Gp32OpRegBp, 0,
												 Gp32OpRegSp, 0);
	
	GenPrintInstr2Operands(Gp32InstrAdd, 0,
												 Gp32OpRegBp, 0,
												 Gp32OpConst, 4);

	GenPrologPos = ftell(OutFile);
	GenWriteFrameSize();
	GenPrologSiz = ftell(OutFile);
	GenPrologSiz -= GenPrologPos;
}

STATIC
void GenGrowStack(int size)
{
	if (!size)
		return;
	GenPrintInstr2Operands(Gp32InstrSub, 0,
												 Gp32OpRegSp, 0,
												 Gp32OpConst, size);
}

STATIC
void GenFxnEpilog(void)
{
	GenUpdateFrameSize();

	GenSaveRestoreRegs(0);

	GenPrintInstr2Operands(Gp32InstrSub, 0,
												 Gp32OpRegBp, 0,
												 Gp32OpConst, 4);

	GenPrintInstr2Operands(Gp32InstrMov, 0,
												 Gp32OpRegSp, 0,
												 Gp32OpRegBp, 0);

	GenPrintInstr1Operand(Gp32InstrPop, 0,
												Gp32OpRegBp, 0);

	GenPrintInstrNoOperand(Gp32InstrRet, 0);
}

STATIC
int GenMaxLocalsSize(void)
{
	return 0x7FFFFFFF;
}

STATIC
int GenGetBinaryOperatorInstr(int tok)
{
	switch (tok)
	{
	case tokPostAdd:
	case tokAssignAdd:
	case '+':
		return Gp32InstrAdd;
	case tokPostSub:
	case tokAssignSub:
	case '-':
		return Gp32InstrSub;
	case tokRevMinus:
		return Gp32InstrRSub;
	case '&':
	case tokAssignAnd:
		return Gp32InstrAnd;
	case '^':
	case tokAssignXor:
		return Gp32InstrXor;
	case '|':
	case tokAssignOr:
		return Gp32InstrOr;
	case '<':
	case '>':
	case tokLEQ:
	case tokGEQ:
	case tokEQ:
	case tokNEQ:
	case tokULess:
	case tokUGreater:
	case tokULEQ:
	case tokUGEQ:
		return Gp32InstrNop;
	case '*':
	case tokAssignMul:
		return Gp32InstrMul;
	case '/':
	case '%':
	case tokAssignDiv:
	case tokAssignMod:
		return Gp32InstrSDiv;
	case tokUDiv:
	case tokUMod:
	case tokAssignUDiv:
	case tokAssignUMod:
		return Gp32InstrDiv;
	case tokLShift:
	case tokAssignLSh:
		return Gp32InstrLLS;
	case tokRShift:
	case tokAssignRSh:
		return Gp32InstrARS;
	case tokURShift:
	case tokAssignURSh:
		return Gp32InstrLRS;

	default:
		//error("Error: Invalid operator\n");
		errorInternal(101);
		return 0;
	}
}

STATIC
void GenReadIdent(int regDst, int opSz, int label)
{
	int instr = Gp32InstrL;
	if (opSz == -1 || opSz == 1)
	{
		instr = Gp32InstrLB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = Gp32InstrLW;
	}
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpLabel, label);

	if (opSz == -1) {
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(Gp32InstrLRS, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 7);
		GenPrintInstr2Operands(Gp32InstrMul, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 0xFFFFFF00);
		GenPrintInstr2Operands(Gp32InstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	} else if (opSz == -2) {
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(Gp32InstrLRS, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 15);
		GenPrintInstr2Operands(Gp32InstrMul, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 0xFFFF0000);
		GenPrintInstr2Operands(Gp32InstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	}
}

STATIC
void GenReadLocal(int regDst, int opSz, int ofs)
{
	int instr = Gp32InstrL;
	if (opSz == -1 || opSz == 1)
	{
		instr = Gp32InstrLB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = Gp32InstrLW;
	}
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpIndRegBp, ofs);

	if (opSz == -1) {
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(Gp32InstrLRS, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 7);
		GenPrintInstr2Operands(Gp32InstrMul, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 0xFFFFFF00);
		GenPrintInstr2Operands(Gp32InstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	} else if (opSz == -2) {
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(Gp32InstrLRS, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 15);
		GenPrintInstr2Operands(Gp32InstrMul, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 0xFFFF0000);
		GenPrintInstr2Operands(Gp32InstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	}
}

STATIC
void GenReadIndirect(int regDst, int regSrc, int opSz)
{
	int instr = Gp32InstrL;
	if (opSz == -1 || opSz == 1)
	{
		instr = Gp32InstrLB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = Gp32InstrLW;
	}
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 regSrc + Gp32OpIndReg0, 0);

	if (opSz == -1) {
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(Gp32InstrLRS, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 7);
		GenPrintInstr2Operands(Gp32InstrMul, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 0xFFFFFF00);
		GenPrintInstr2Operands(Gp32InstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	} else if (opSz == -2) {
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(Gp32InstrLRS, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 15);
		GenPrintInstr2Operands(Gp32InstrMul, 0,
													 TEMP_REG_A, 0,
													 Gp32OpConst, 0xFFFF0000);
		GenPrintInstr2Operands(Gp32InstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	}
}

STATIC
void GenWriteIdent(int regSrc, int opSz, int label)
{
	int instr = Gp32InstrS;
	if (opSz == -1 || opSz == 1)
	{
		instr = Gp32InstrSB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = Gp32InstrSW;
	}
	GenPrintInstr2Operands(instr, 0,
												 Gp32OpLabel, label,
												 regSrc, 0);
}

STATIC
void GenWriteLocal(int regSrc, int opSz, int ofs)
{
	int instr = Gp32InstrS;
	if (opSz == -1 || opSz == 1)
	{
		instr = Gp32InstrSB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = Gp32InstrSW;
	}
	GenPrintInstr2Operands(instr, 0,
												 Gp32OpIndRegBp, ofs,
												 regSrc, 0);
}

STATIC
void GenWriteIndirect(int regDst, int regSrc, int opSz)
{
	int instr = Gp32InstrS;
	if (opSz == -1 || opSz == 1)
	{
		instr = Gp32InstrSB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = Gp32InstrSW;
	}
	GenPrintInstr2Operands(instr, 0,
												 regDst + Gp32OpIndReg0, 0,
												 regSrc, 0);
}

STATIC
void GenIncDecIdent(int regDst, int opSz, int label, int tok)
{
	int instr = Gp32InstrAdd;

	if (tok != tokInc)
		instr = Gp32InstrSub;

	GenReadIdent(regDst, opSz, label);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpConst, 1);
	GenWriteIdent(regDst, opSz, label);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenIncDecLocal(int regDst, int opSz, int ofs, int tok)
{
	int instr = Gp32InstrAdd;

	if (tok != tokInc)
		instr = Gp32InstrSub;

	GenReadLocal(regDst, opSz, ofs);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpConst, 1);
	GenWriteLocal(regDst, opSz, ofs);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenIncDecIndirect(int regDst, int regSrc, int opSz, int tok)
{
	int instr = Gp32InstrAdd;

	if (tok != tokInc)
		instr = Gp32InstrSub;

	GenReadIndirect(regDst, regSrc, opSz);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpConst, 1);
	GenWriteIndirect(regSrc, regDst, opSz);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenPostIncDecIdent(int regDst, int opSz, int label, int tok)
{
	int instr = Gp32InstrAdd;

	if (tok != tokPostInc)
		instr = Gp32InstrSub;

	GenReadIdent(regDst, opSz, label);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpConst, 1);
	GenWriteIdent(regDst, opSz, label);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpConst, -1);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenPostIncDecLocal(int regDst, int opSz, int ofs, int tok)
{
	int instr = Gp32InstrAdd;

	if (tok != tokPostInc)
		instr = Gp32InstrSub;

	GenReadLocal(regDst, opSz, ofs);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpConst, 1);
	GenWriteLocal(regDst, opSz, ofs);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpConst, -1);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenPostIncDecIndirect(int regDst, int regSrc, int opSz, int tok)
{
	int instr = Gp32InstrAdd;

	if (tok != tokPostInc)
		instr = Gp32InstrSub;

	GenReadIndirect(regDst, regSrc, opSz);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpConst, 1);
	GenWriteIndirect(regSrc, regDst, opSz);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 Gp32OpConst, -1);
	GenExtendRegIfNeeded(regDst, opSz);
}

int CanUseTempRegs;
int TempsUsed;
int GenWreg = Gp32OpReg0; // current working register (R0, R1, R2, etc)
int GenLreg, GenRreg; // left operand register and right operand register after GenPopReg()

/*
	General idea behind GenWreg, GenLreg, GenRreg:

	- In expressions w/o function calls:

		Subexpressions are evaluated in R0, R1, R2, ..., R<MAX_TEMP_REGS>. If those registers
		aren't enough, the stack is used additionally.

		The expression result ends up in R0, which is handy for returning from
		functions.

		In the process, GenWreg is the current working register and is one of: R0, R1, R2, ... .
		All unary operators are evaluated in the current working register.

		GenPushReg() and GenPopReg() advance GenWreg as needed when handling binary operators.

		GenPopReg() sets GenWreg, GenLreg and GenRreg. GenLreg and GenRreg are the registers
		where the left and right operands of a binary operator are.

		When the exression runs out of the temporary registers, the stack is used. While it is being
		used, GenWreg remains equal to the last temporary register, and GenPopReg() sets GenLreg = TEMP_REG_A.
		Hence, after GenPopReg() the operands of the binary operator are always in registers and can be
		directly manipulated with.

		Following GenPopReg(), binary operator evaluation must take the left and right operands from
		GenLreg and GenRreg and write the evaluated result into GenWreg. Care must be taken as GenWreg
		will be the same as either GenLreg (when the popped operand comes from R1-R<MAX_TEMP_REGS>)
		or GenRreg (when the popped operand comes from the stack in TEMP_REG_A).

	- In expressions with function calls:

		GenWreg is always R0. R1-R<MAX_TEMP_REGS> are not used. Instead the stack and TEMP_REG_A and TEMP_REG_B
		are used.
*/

STATIC
void GenWregInc(int inc)
{
	if (inc > 0)
	{
		// Advance the current working register to the next available temporary register
		GenWreg++;
	}
	else
	{
		// Return to the previous current working register
		GenWreg--;
	}
}

STATIC
void GenPushReg(void)
{
	if (CanUseTempRegs && TempsUsed < MAX_TEMP_REGS)
	{
		GenWregInc(1);
		TempsUsed++;
		return;
	}

	GenPrintInstr1Operand(Gp32InstrPush, 0,
												GenWreg, 0);

	TempsUsed++;
}

STATIC
void GenPopReg(void)
{
	TempsUsed--;

	if (CanUseTempRegs && TempsUsed < MAX_TEMP_REGS)
	{
		GenRreg = GenWreg;
		GenWregInc(-1);
		GenLreg = GenWreg;
		return;
	}

	GenPrintInstr1Operand(Gp32InstrPop, 0,
												TEMP_REG_A, 0);

	GenLreg = TEMP_REG_A;
	GenRreg = GenWreg;
}

STATIC
void GenPrep(int* idx)
{
	int tok;
	int oldIdxRight, oldIdxLeft, t0, t1;

	if (*idx < 0)
		//error("GenPrep(): idx < 0\n");
		errorInternal(100);

	tok = stack[*idx][0];

	oldIdxRight = --*idx;

	switch (tok)
	{
	case tokAssignMul:
	case tokUDiv:
	case tokUMod:
	case tokAssignUDiv:
	case tokAssignUMod:
		if (stack[oldIdxRight][0] == tokNumInt || stack[oldIdxRight][0] == tokNumUint)
		{
			unsigned m = truncUint(stack[oldIdxRight][1]);
			if (m && !(m & (m - 1)))
			{
				// Change multiplication to left shift, this helps indexing arrays of ints/pointers/etc
				if (tok == tokAssignMul)
				{
					t1 = 0;
					while (m >>= 1) t1++;
					stack[oldIdxRight][1] = t1;
					tok = tokAssignLSh;
				}
				// Change unsigned division to right shift and unsigned modulo to bitwise and
				else if (tok == tokUMod || tok == tokAssignUMod)
				{
					stack[oldIdxRight][1] = (int)(m - 1);
					tok = (tok == tokUMod) ? '&' : tokAssignAnd;
				}
				else
				{
					t1 = 0;
					while (m >>= 1) t1++;
					stack[oldIdxRight][1] = t1;
					tok = (tok == tokUDiv) ? tokURShift : tokAssignURSh;
				}
				stack[oldIdxRight + 1][0] = tok;
			}
		}
	}

	switch (tok)
	{
	case tokNumUint:
		stack[oldIdxRight + 1][0] = tokNumInt; // reduce the number of cases since tokNumInt and tokNumUint are handled the same way
		// fallthrough
	case tokNumInt:
	case tokIdent:
	case tokLocalOfs:
		break;

	case tokPostAdd:
	case tokPostSub:
	case '/':
	case '-':
	case '%':
	case tokUDiv:
	case tokUMod:
	case tokLShift:
	case tokRShift:
	case tokURShift:
	case tokLogAnd:
	case tokLogOr:
	case tokComma: {
		int xor;
	//case '-':
		switch(tok) {
			case '-':
				xor = '-' ^ tokRevMinus; break;
		}
		tok ^= xor;
		GenPrep(idx);
		// fallthrough
	}
	case tokShortCirc:
	case tokGoto:
	case tokUnaryStar:
	case tokInc:
	case tokDec:
	case tokPostInc:
	case tokPostDec:
	case '~':
	case tokUnaryPlus:
	case tokUnaryMinus:
	case tok_Bool:
	case tokVoid:
	case tokUChar:
	case tokSChar:
	case tokShort:
	case tokUShort:
		GenPrep(idx);
		break;

	case '=':
	//case '-':
	case tokAssignAdd:
	case tokAssignSub:
	case tokAssignMul:
	case tokAssignDiv:
	case tokAssignUDiv:
	case tokAssignMod:
	case tokAssignUMod:
	case tokAssignLSh:
	case tokAssignRSh:
	case tokAssignURSh:
	case tokAssignAnd:
	case tokAssignXor:
	case tokAssignOr:
		GenPrep(idx);
		oldIdxLeft = *idx;
		GenPrep(idx);
		// If the left operand is an identifier (with static or auto storage), swap it with the right operand
		// and mark it specially, so it can be used directly
		if ((t0 = stack[oldIdxLeft][0]) == tokIdent || t0 == tokLocalOfs)
		{
			t1 = stack[oldIdxLeft][1];
			memmove(stack[oldIdxLeft], stack[oldIdxLeft + 1], (oldIdxRight - oldIdxLeft) * sizeof(stack[0]));
			stack[oldIdxRight][0] = (t0 == tokIdent) ? tokRevIdent : tokRevLocalOfs;
			stack[oldIdxRight][1] = t1;
		}
		break;

	case '+':
	//case '-':
	case '*':
	case '&':
	case '^':
	case '|':
	case tokEQ:
	case tokNEQ:
	case '<':
	case '>':
	case tokLEQ:
	case tokGEQ:
	case tokULess:
	case tokUGreater:
	case tokULEQ:
	case tokUGEQ:
		GenPrep(idx);
		oldIdxLeft = *idx;
		GenPrep(idx);
		// If the right operand isn't a constant, but the left operand is, swap the operands
		// so the constant can become an immediate right operand in the instruction
		t1 = stack[oldIdxRight][0];
		t0 = stack[oldIdxLeft][0];
		if (t1 != tokNumInt && t0 == tokNumInt)
		{
			int xor;

			t1 = stack[oldIdxLeft][1];
			memmove(stack[oldIdxLeft], stack[oldIdxLeft + 1], (oldIdxRight - oldIdxLeft) * sizeof(stack[0]));
			stack[oldIdxRight][0] = t0;
			stack[oldIdxRight][1] = t1;

			switch (tok)
			{
			case '<':
			case '>':
				xor = '<' ^ '>'; break;
			case tokLEQ:
			case tokGEQ:
				xor = tokLEQ ^ tokGEQ; break;
			case tokULess:
			case tokUGreater:
				xor = tokULess ^ tokUGreater; break;
			case tokULEQ:
			case tokUGEQ:
				xor = tokULEQ ^ tokUGEQ; break;
			case '-':
				xor = '-' ^ tokRevMinus; break;
			default:
				xor = 0; break;
			}
			tok ^= xor;
		}
		if (stack[oldIdxRight][0] == tokNumInt)
		{
			unsigned m = truncUint(stack[oldIdxRight][1]);
			switch (tok)
			{
			case '*':
				// Change multiplication to left shift, this helps indexing arrays of ints/pointers/etc
				if (m && !(m & (m - 1)))
				{
					t1 = 0;
					while (m >>= 1) t1++;
					stack[oldIdxRight][1] = t1;
					tok = tokLShift;
				}
				break;
			case tokULEQ:
				// Change left <= const to left < const+1, but const+1 must be <=0xFFFFFFFFu
				if (m != 0xFFFFFFFF)
				{
					stack[oldIdxRight][1]++;
					tok = tokULess;
				}
				break;
			case tokUGEQ:
				// Change left >= const to left > const-1, but const-1 must be >=0u
				if (m)
				{
					stack[oldIdxRight][1]--;
					tok = tokUGreater;
				}
				break;
			}
		}
		stack[oldIdxRight + 1][0] = tok;
		break;

	case ')':
		while (stack[*idx][0] != '(')
		{
			GenPrep(idx);
			if (stack[*idx][0] == ',')
				--*idx;
		}
		--*idx;
		break;

	default:
		//error("GenPrep: unexpected token %s\n", GetTokenName(tok));
		errorInternal(101);
	}
}

STATIC
void GenCmp(int* idx, int instr)
{
	// constness: 0 = zero const, 1 = non-zero const, 2 = non-const
	int constness = (stack[*idx - 1][0] == tokNumInt) ? (stack[*idx - 1][1] != 0) : 2;
	int constval = (constness == 1) ? truncInt(stack[*idx - 1][1]) : 0;
	// condbranch: 0 = no conditional branch, 1 = branch if true, 2 = branch if false
	int condbranch = (*idx + 1 < sp) ? (stack[*idx + 1][0] == tokIf) + (stack[*idx + 1][0] == tokIfNot) * 2 : 0;
	int label = condbranch ? stack[*idx + 1][1] : 0;

	GenStartCommentLine(); printf2("GenCmp const=%d cond=%d\n",constness,condbranch);

	if (constness == 2)
		GenPopReg();

	if (condbranch)
	{
		if (constness == 2)
			GenPrintInstr2Operands(Gp32InstrIf, 0,
														 GenLreg, 0,
														 GenRreg, 0);
		else
			GenPrintInstr2Operands(Gp32InstrIf, 0,
														 GenWreg, 0,
														 Gp32OpConst, constval);
		if(condbranch != 1) 
			GenPrintInstr2Operands(Gp32InstrXor, 0,
														 Gp32OpRegFlags, 0,
														 Gp32OpConst, 1 << (instr - 0x20));
		GenPrintInstr1Operand(instr, 0,
													Gp32OpNumLabel, label);
	}
	else
	{
													 
		// Slow, general, catch-all implementation
		if (constness == 2)
			GenPrintInstr2Operands(Gp32InstrIf, 0,
														 GenLreg, 0,
														 GenRreg, 0);
		else
			GenPrintInstr2Operands(Gp32InstrIf, 0,
														 GenWreg, 0,
														 Gp32OpConst, constval);
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 TEMP_REG_B, 0,
													 Gp32OpRegFlags, 0);
		GenPrintInstr2Operands(Gp32InstrAnd, 0,
													 TEMP_REG_B, 0,
													 Gp32OpConst, 1 << (instr - 0x20));
		GenPrintInstr2Operands(Gp32InstrMov, 0,
													 GenWreg, 0,
													 TEMP_REG_B, 0);
	}

	*idx += condbranch != 0;
}

STATIC
int GenIsCmp(int t)
{
	return
		t == '<' ||
		t == '>' ||
		t == tokGEQ ||
		t == tokLEQ ||
		t == tokULess ||
		t == tokUGreater ||
		t == tokUGEQ ||
		t == tokULEQ ||
		t == tokEQ ||
		t == tokNEQ;
}

// Improved register/stack-based code generator
// DONE: test 32-bit code generation
STATIC
void GenExpr0(void)
{
	int i;
	int gotUnary = 0;
	int maxCallDepth = 0;
	int callDepth = 0;
	int t = sp - 1;

	if (stack[t][0] == tokIf || stack[t][0] == tokIfNot || stack[t][0] == tokReturn)
		t--;
	GenPrep(&t);

	for (i = 0; i < sp; i++)
		if (stack[i][0] == '(')
		{
			if (++callDepth > maxCallDepth)
				maxCallDepth = callDepth;
		}
		else if (stack[i][0] == ')')
		{
			callDepth--;
		}

	CanUseTempRegs = maxCallDepth == 0;
	TempsUsed = 0;
	if (GenWreg != Gp32OpReg0)
		errorInternal(102);

	for (i = 0; i < sp; i++)
	{
		int tok = stack[i][0];
		int v = stack[i][1];

#ifndef NO_ANNOTATIONS
		switch (tok)
		{
			case tokNumInt: GenStartCommentLine(); printf2("%d\n", truncInt(v)); break;
		//case tokNumUint: printf2(" ; %uu\n", truncUint(v)); break;
		case tokIdent: case tokRevIdent: GenStartCommentLine(); printf2("%s\n", IdentTable + v); break;
		case tokLocalOfs: case tokRevLocalOfs: GenStartCommentLine(); printf2("local ofs\n"); break;
		case ')': GenStartCommentLine(); printf2(") fxn call\n"); break;
		case tokUnaryStar: GenStartCommentLine(); printf2("* (read dereference)\n"); break;
		case '=': GenStartCommentLine(); printf2("= (write dereference)\n"); break;
		case tokShortCirc: GenStartCommentLine(); printf2("short-circuit "); break;
		case tokGoto: GenStartCommentLine(); printf2("sh-circ-goto "); break;
		case tokLogAnd: GenStartCommentLine(); printf2("short-circuit && target\n"); break;
		case tokLogOr: GenStartCommentLine(); printf2("short-circuit || target\n"); break;
		case tokIf: GenStartCommentLine(); printf2(__FUNCTION__ ); printf2(" if\n"); break;
		case tokIfNot: GenStartCommentLine(); printf2(__FUNCTION__ ); printf2(" !if\n"); break;
		case tokReturn: break;
		case tokRevMinus: GenStartCommentLine(); printf2("-r\n"); break;
		default: GenStartCommentLine(); printf2("%s\n", GetTokenName(tok)); break;
		}
#endif

		switch (tok)
		{
		// TBD??? forward tokNumInt and tokIdent to ',', push them directly, w/o extra moves
		case tokNumInt:
			if (!(i + 1 < sp && ((t = stack[i + 1][0]) == '+' ||
													 t == '-' ||
													 t == tokRevMinus ||
													 t == '*' ||
													 t == '&' ||
													 t == '^' ||
													 t == '|' ||
													 t == tokLShift ||
													 t == tokRShift ||
													 t == tokURShift ||
													 t == '/' ||
													 t == tokUDiv ||
													 t == '%' ||
													 t == tokUMod ||
													 GenIsCmp(t))))
			{
				if (gotUnary)
					GenPushReg();

				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 GenWreg, 0,
															 Gp32OpConst, v);
			}
			gotUnary = 1;
			break;

		case tokIdent:
			if (gotUnary)
				GenPushReg();
			if (!(i + 1 < sp && ((t = stack[i + 1][0]) == ')' ||
													 t == tokUnaryStar ||
													 t == tokInc ||
													 t == tokDec ||
													 t == tokPostInc ||
													 t == tokPostDec)))
			{
				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 GenWreg, 0,
															 Gp32OpLabel, v);
			}
			gotUnary = 1;
			break;

		case tokLocalOfs:
			if (gotUnary)
				GenPushReg();
			if (!(i + 1 < sp && ((t = stack[i + 1][0]) == tokUnaryStar ||
													 t == tokInc ||
													 t == tokDec ||
													 t == tokPostInc ||
													 t == tokPostDec)))
			{
				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 GenWreg, 0,
															 Gp32OpRegBp, 0);
				GenPrintInstr2Operands(Gp32InstrAdd, 0,
															 GenWreg, 0,
															 Gp32OpConst, v);
			}
			gotUnary = 1;
			break;

		case '(':
			if (gotUnary)
				GenPushReg();
			gotUnary = 0;
			break;

		case ',':
			break;

		case ')':
			if (stack[i - 1][0] == tokIdent)
			{
				GenPrintInstr1Operand(Gp32InstrRCall, 0,
															Gp32OpLabel, stack[i - 1][1]);
			}
			else
			{
				GenPrintInstr1Operand(Gp32InstrCall, 0,
															GenWreg, 0);
			}
			GenGrowStack(-v);
			break;

		case tokUnaryStar:
			if (stack[i - 1][0] == tokIdent)
				GenReadIdent(GenWreg, v, stack[i - 1][1]);
			else if (stack[i - 1][0] == tokLocalOfs)
				GenReadLocal(GenWreg, v, stack[i - 1][1]);
			else
				GenReadIndirect(GenWreg, GenWreg, v);
			break;

		case tokUnaryPlus:
			break;
		case '~':
			GenPrintInstr1Operand(Gp32InstrNot, 0,
														GenWreg, 0);
			break;
		case tokUnaryMinus:
			GenPrintInstr2Operands(Gp32InstrRSub, 0,
														 GenWreg, 0,
														 Gp32OpConst, 0);
			break;

		case '+':
		case '-':
		case tokRevMinus:
		case '*':
		case '&':
		case '^':
		case '|':
		case tokLShift:
		case tokRShift:
		case tokURShift:
		case '/':
		case tokUDiv:
		case '%':
		case tokUMod:
			if (stack[i - 1][0] == tokNumInt)
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				//GenPopReg();
				GenPrintInstr2Operands(instr, 0,
															 GenWreg, 0,
															 Gp32OpConst, stack[i - 1][1]);
			}
			else
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				GenPopReg();
				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 TEMP_REG_B, 0,
															 GenLreg, 0);
				GenPrintInstr2Operands(instr, 0,
															 TEMP_REG_B, 0,
															 GenRreg, 0);
				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 GenWreg, 0,
															 TEMP_REG_B, 0);
			}
			if (tok == '%' || tok == tokUMod)
				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 GenWreg, 0,
															 Gp32OpRegY, 0);
			break;
		case tokInc:
		case tokDec:
			if (stack[i - 1][0] == tokIdent)
			{
				GenIncDecIdent(GenWreg, v, stack[i - 1][1], tok);
			}
			else if (stack[i - 1][0] == tokLocalOfs)
			{
				GenIncDecLocal(GenWreg, v, stack[i - 1][1], tok);
			}
			else
			{
				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 TEMP_REG_A, 0,
															 GenWreg, 0);
				GenIncDecIndirect(GenWreg, TEMP_REG_A, v, tok);
			}
			break;
		case tokPostInc:
		case tokPostDec:
			if (stack[i - 1][0] == tokIdent)
			{
				GenPostIncDecIdent(GenWreg, v, stack[i - 1][1], tok);
			}
			else if (stack[i - 1][0] == tokLocalOfs)
			{
				GenPostIncDecLocal(GenWreg, v, stack[i - 1][1], tok);
			}
			else
			{
				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 TEMP_REG_A, 0,
															 GenWreg, 0);
				GenPostIncDecIndirect(GenWreg, TEMP_REG_A, v, tok);
			}
			break;

		case tokPostAdd:
		case tokPostSub:
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				GenPopReg();
				if (GenWreg == GenLreg)
				{
					GenPrintInstr2Operands(Gp32InstrMov, 0,
																 TEMP_REG_B, 0,
																 GenLreg, 0);

					GenReadIndirect(GenWreg, TEMP_REG_B, v);
					GenPrintInstr2Operands(Gp32InstrMov, 0,
																 TEMP_REG_A, 0,
																 GenWreg, 0);
					GenPrintInstr2Operands(instr, 0,
																 TEMP_REG_A, 0,
																 GenRreg, 0);
					GenWriteIndirect(TEMP_REG_B, TEMP_REG_A, v);
				}
				else
				{
					// GenWreg == GenRreg here
					GenPrintInstr2Operands(Gp32InstrMov, 0,
																 TEMP_REG_B, 0,
																 GenRreg, 0);

					GenReadIndirect(GenWreg, GenLreg, v);
					GenPrintInstr2Operands(Gp32InstrMov, 0,
																 TEMP_REG_B, 0,
																 GenWreg, 0);
					GenPrintInstr2Operands(instr, 0,
																 TEMP_REG_B, 0,
																 TEMP_REG_B, 0);
					GenWriteIndirect(GenLreg, TEMP_REG_B, v);
				}
			}
			break;

		case tokAssignAdd:
		case tokAssignSub:
		case tokAssignMul:
		case tokAssignAnd:
		case tokAssignXor:
		case tokAssignOr:
		case tokAssignLSh:
		case tokAssignRSh:
		case tokAssignURSh:
		case tokAssignDiv:
		case tokAssignUDiv:
		case tokAssignMod:
		case tokAssignUMod:
		//case '-':
		//case tokRevMinus:
			if (stack[i - 1][0] == tokRevLocalOfs || stack[i - 1][0] == tokRevIdent)
			{
				int instr = GenGetBinaryOperatorInstr(tok);

				if (stack[i - 1][0] == tokRevLocalOfs)
					GenReadLocal(TEMP_REG_B, v, stack[i - 1][1]);
				else
					GenReadIdent(TEMP_REG_B, v, stack[i - 1][1]);

				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 TEMP_REG_A, 0,
															 TEMP_REG_B, 0);
				GenPrintInstr2Operands(instr, 0,
															 TEMP_REG_A, 0,
															 GenWreg, 0);
				
				GenPrintInstr2Operands(Gp32InstrMov, 0,
															 GenWreg, 0,
															 TEMP_REG_A, 0);

				if (tok == tokAssignMod || tok == tokAssignUMod)
					GenPrintInstr2Operands(Gp32InstrMov, 0,
																 GenWreg, 0,
																 Gp32OpRegY, 0);

				if (stack[i - 1][0] == tokRevLocalOfs)
					GenWriteLocal(GenWreg, v, stack[i - 1][1]);
				else
					GenWriteIdent(GenWreg, v, stack[i - 1][1]);
			}
			else
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				int lsaved, rsaved;
				GenPopReg();
				if (GenWreg == GenLreg)
				{
					GenPrintInstr2Operands(Gp32InstrMov, 0,
																 TEMP_REG_B, 0,
																 GenLreg, 0);
					lsaved = TEMP_REG_B;
					rsaved = GenRreg;
				}
				else
				{
					// GenWreg == GenRreg here
					GenPrintInstr2Operands(Gp32InstrMov, 0,
																 TEMP_REG_B, 0,
																 GenRreg, 0);
					rsaved = TEMP_REG_B;
					lsaved = GenLreg;
				}

				GenReadIndirect(GenWreg, GenLreg, v); // destroys either GenLreg or GenRreg because GenWreg coincides with one of them
				GenPrintInstr2Operands(instr, 0,
															 GenWreg, 0,
															 rsaved, 0);

				if (tok == tokAssignMod || tok == tokAssignUMod)
					GenPrintInstr2Operands(Gp32InstrMov, 0,
																 GenWreg, 0,
																 Gp32OpRegY, 0);

				GenWriteIndirect(lsaved, GenWreg, v);
			}
			GenExtendRegIfNeeded(GenWreg, v);
			break;

		case '=':
			if (stack[i - 1][0] == tokRevLocalOfs)
			{
				GenWriteLocal(GenWreg, v, stack[i - 1][1]);
			}
			else if (stack[i - 1][0] == tokRevIdent)
			{
				GenWriteIdent(GenWreg, v, stack[i - 1][1]);
			}
			else
			{
				GenPopReg();
				GenWriteIndirect(GenLreg, GenRreg, v);
				if (GenWreg != GenRreg)
					GenPrintInstr2Operands(Gp32InstrMov, 0,
																 GenWreg, 0,
																 GenRreg, 0);
			}
			GenExtendRegIfNeeded(GenWreg, v);
			break;

		case '<':				GenCmp(&i, Gp32InstrJB	); break;
		case tokLEQ:			GenCmp(&i, Gp32InstrJBE	); break;
		case '>':				GenCmp(&i, Gp32InstrJA	); break;
		case tokGEQ:			GenCmp(&i, Gp32InstrJAE	); break;
		case tokULess:			GenCmp(&i, Gp32InstrJB	); break;
		case tokULEQ:			GenCmp(&i, Gp32InstrJBE	); break;
		case tokUGreater:		GenCmp(&i, Gp32InstrJA	); break;
		case tokUGEQ:			GenCmp(&i, Gp32InstrJAE	); break;
		case tokEQ:				GenCmp(&i, Gp32InstrJE	); break;
		case tokNEQ:			GenCmp(&i, Gp32InstrJNE	); break;

		case tok_Bool:
			GenPrintInstr2Operands(Gp32InstrIf, 0,
														 GenWreg, 0,
														 GenWreg, 0);
			GenPrintInstr2Operands(Gp32InstrMov, 0,
														 GenWreg, 0,
														 Gp32OpRegFlags, 1);
			GenPrintInstr2Operands(Gp32InstrLRS, 0,
														 GenWreg, 0,
														 Gp32OpConst, 6);
			GenPrintInstr2Operands(Gp32InstrAnd, 0,
														 GenWreg, 0,
														 Gp32OpRegFlags, 1);
			break;

		case tokSChar:
			GenPrintInstr2Operands(Gp32InstrMov, 0,
														 TEMP_REG_A, 0,
														 GenWreg, 0);
			GenPrintInstr2Operands(Gp32InstrLRS, 0,
														 TEMP_REG_A, 0,
														 Gp32OpConst, 7);
														 
			GenPrintInstr2Operands(Gp32InstrMul, 0,
														 TEMP_REG_A, 0,
														 Gp32OpConst, 0xFFFFFF00);
			GenPrintInstr2Operands(Gp32InstrOr, 0,
														 GenWreg, 0,
														 TEMP_REG_A, 0);
			break;
		case tokUChar:
			GenPrintInstr2Operands(Gp32InstrAnd, 0,
														 GenWreg, 0,
														 Gp32OpConst, 0xFF);
			break;
		case tokShort:
			GenPrintInstr2Operands(Gp32InstrLRS, 0,
														 TEMP_REG_A, 0,
														 Gp32OpConst, 15);
														 
			GenPrintInstr2Operands(Gp32InstrMul, 0,
														 TEMP_REG_A, 0,
														 Gp32OpConst, 0xFFFF0000);
			GenPrintInstr2Operands(Gp32InstrOr, 0,
														 GenWreg, 0,
														 TEMP_REG_A, 0);
			break;
		case tokUShort:
			GenPrintInstr2Operands(Gp32InstrAnd, 0,
														 GenWreg, 0,
														 Gp32OpConst, 0xFFFF);
			break;

		case tokShortCirc:
#ifndef NO_ANNOTATIONS
			if (v >= 0)
				printf2("&&\n");
			else
				printf2("||\n");
#endif
			if (v >= 0) {
				printf2("\t; tokShortCirc +\n");
				GenJumpIfZero(v); // &&
			} else {
				printf2("\t; tokShortCirc -\n");
				GenJumpIfNotZero(-v); // ||
			}
			gotUnary = 0;
			break;
		case tokGoto:
#ifndef NO_ANNOTATIONS
			printf2("goto\n");
#endif
			GenJumpUncond(v);
			gotUnary = 0;
			break;
		case tokLogAnd:
			printf2("\t; tokLogAnd\n");
			GenNumLabel(v);
			break;
		case tokLogOr:
			GenNumLabel(v);
			break;

		case tokVoid:
			gotUnary = 0;
			break;

		case tokRevIdent:
		case tokRevLocalOfs:
		case tokComma:
			break;
		case tokReturn:
			printf2("\t; tokReturn\n");
			break;

		case tokIf:
			printf2("\t; tokIf\n");
			GenJumpIfNotZero(stack[i][1]);
			break;
		case tokIfNot:
			printf2("\t; tokIfNot\n");
			GenJumpIfZero(stack[i][1]);
			break;

		default:
			//error("Error: Internal Error: GenExpr0(): unexpected token %s\n", GetTokenName(tok));
			errorInternal(103);
			break;
		}
	}

	if (GenWreg != Gp32OpReg0)
		printf("GenWreg=%d\n",GenWreg), errorInternal(104);
}

STATIC
void GenDumpChar(int ch)
{
	if(ch > 0) printf2("\tdb($%u)\n", ch & 0xFFu);
}

STATIC
void GenExpr(void)
{
	if (GenExterns)
	{
		int i;
		for (i = 0; i < sp; i++)
			if (stack[i][0] == tokIdent && !isdigit(IdentTable[stack[i][1]]))
			GenAddGlobal(IdentTable + stack[i][1], 2);
	}
	GenExpr0();
}

STATIC
void GenFin(void)
{
	if (StructCpyLabel)
	{
		int lbl = LabelCnt++;

		puts2(CodeHeaderFooter[0]);

		GenNumLabel(StructCpyLabel);
		puts2("\tpush(%r1)\n"
					"\tpush(%r2)\n"
					"\tpush(%r3)\n"
					"\tmov(%r1,d@[%sp:$16])\n" // size
					"\tmov(%r2,d@[%sp:$20])\n" // source
					"\tmov(%r3,d@[%sp:$24])"); // destination
		GenNumLabel(lbl);
		puts2("\tmov(%r0,b%r2)\n"
					"\tadd(%r2,$1)\n"
					"\tadd(%r1,$-1)\n"
					"\tmov(b%r3,%r0)\n"
					"\tadd(%r3,$1)\n"
					"\tcmp(%r1,$0)");
		printf2("\tjne("); GenPrintNumLabel(lbl);
		puts2(")");
		puts2("\tmov(%r0,d@[%sp:$24])\n" // destination
					"\tpop(%r3)\n"
					"\tpop(%r2)\n"
					"\tpop(%r1)\n"
					"\tret()");

		puts2(CodeHeaderFooter[1]);
	}
	
	if (GenExterns)
	{
		int i = 0;

		puts2("");
		while (i < GlobalsTableLen)
		{
			if (GlobalsTable[i] == 2)
			{
				//printf2("\textern\t");
				//GenPrintLabel(GlobalsTable + i + 2);
				printf2("\tglobal(._%s)\n", GlobalsTable + i + 2);
				//puts2("");
			}
			i += GlobalsTable[i + 1] + 2;
		}
	}
}
