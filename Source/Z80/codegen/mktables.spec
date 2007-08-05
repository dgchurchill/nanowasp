#   Nanowasp - A Microbee emulator
#   Copyright (C) 2000-2007 David G. Churchill
# 
#   This file is part of Nanowasp.  It derives from libz80 (see Z80CPU.c
#   for libz80 license notice).  The following changes have been made:
# 
#   July 2007 - Converted to C++ class
#             - Numerous bug fixes to Z80 emulation
#             - Minor optimisation
# 
#   Nanowasp is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
# 
#   Nanowasp is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
# Z80 -> C specification


#
# Add / Sub / Adc / Sbc
#
(ADC|SBC|ADD|SUB) A,(A|B|C|D|E|H|L|IXh|IXl|IYh|IYl)
	BR.A = doArithmetic(BR.%2, F1_%1, F2_%1);

(ADC|SBC|ADD|SUB) A,\(HL\)
	BR.A = doArithmetic(read8(WR.HL), F1_%1, F2_%1);

(ADC|SBC|ADD|SUB) A,\((IX|IY)\+d\)
	BR.A = doArithmetic(read8(WR.%2 + (signed char)(read8(PC++))), F1_%1, F2_%1);
	
(ADC|SBC|ADD|SUB) A,n
	BR.A = doArithmetic(read8(PC++), F1_%1, F2_%1);


# Particular cases; ADC doesn't set flags 3, 4, 5 correctly
ADC HL,(SP|BC|DE|HL)
	unsigned long res = WR.HL + WR.%1 + GETFLAG(F_C);
	RESFLAG(F_N);
	VALFLAG(F_S, (res & 0x8000) != 0);
	VALFLAG(F_Z, res == 0);
	VALFLAG(F_C, (res & 0x10000) != 0);
	VALFLAG(F_PV, (WR.HL & 0x8000) == (WR.%1 & 0x8000) && (WR.HL & 0x8000) != ((ushort)res & 0x8000));
	WR.HL = res & 0xFFFF;

SBC HL,(SP|BC|DE|HL)
	unsigned long res = WR.HL - WR.%1 - GETFLAG(F_C);
	SETFLAG(F_N);
	VALFLAG(F_S, (res & 0x8000) != 0);
	VALFLAG(F_Z, res == 0);
	VALFLAG(F_C, (res & 0x10000) != 0);
	VALFLAG(F_PV, (WR.HL & 0x8000) != (WR.%1 & 0x8000) && (WR.HL & 0x8000) != ((ushort)res & 0x8000));
	WR.HL = res & 0xFFFF;

# Not all cominations listed are valid, generated pairs are limited by opcodes.lst	
ADD (IX|IY|HL),(SP|BC|DE|HL|IX|IY)
	unsigned long res = WR.%1 + WR.%2;
	RESFLAG(F_N);
	VALFLAG(F_C, (res & 0x10000) != 0);
	WR.%1 = res & 0xFFFF;
	
#
# Logic operations
#
(AND|XOR|OR) \(HL\)
	do%1(read8(WR.HL));

(AND|XOR|OR) \((IX|IY)\+d\)
	do%1(read8(WR.%2 + (signed char)(read8(PC++))));

(AND|XOR|OR) (A|B|C|D|E|H|L|IXh|IXl|IYh|IYl)
	do%1(BR.%2);

(AND|XOR|OR) n
	do%1(read8(PC++));


#
# Bit operations
#
BIT ([0-7]),(A|B|C|D|E|H|L)
	doBIT(%1, BR.%2);

BIT ([0-7]),\(HL\)
	doBIT(%1, read8(WR.HL));

BIT ([0-7]),\((IX|IY)\+d\)
	doBIT(%1, read8(WR.%2 + (signed char)(read8(PC++))));
	
(SET|RES) ([0-7]),(A|B|C|D|E|H|L)
	BR.%3 = doSetRes(SR_%1, %2, BR.%3);

(SET|RES) ([0-7]),\(HL\)
	write8(WR.HL, doSetRes(SR_%1, %2, read8(WR.HL)));

(SET|RES) ([0-7]),\((IX|IY)\+d\)
	int off = (signed char)(read8(PC++));
	write8(WR.%3 + off, doSetRes(SR_%1, %2, read8(WR.%3 + off)));
	
	
#
# Jumps and calls
#
CALL (C|M|NZ|NC|P|PE|PO|Z)?,?\(nn\)
	if (COND_%1)
	{
		doPush(PC+2);
		PC = read16(PC);
	}
	else
		PC += 2;
	
JP \((HL|IX|IY)\)
	PC = WR.%1;
	
JP (C|M|NZ|NC|P|PE|PO|Z)?,?\(nn\)
	if (COND_%1)
		PC = read16(PC);
	else
		PC += 2;
	
JR (C|NZ|NC|Z)?,?\(PC\+e\)
	if (COND_%1)
		PC += (signed char)(read8(PC)) + 1;
	else
		++PC;

RETI
	IFF1 = IFF2;
	%RET		
		
RETN
	IFF1 = IFF2;
	%RET


RET (C|M|NZ|NC|P|PE|PO|Z)
	if (COND_%1)
		PC = doPop();
		
RET
	PC = doPop();

	
DJNZ \(PC\+e\)
	BR.B--;
	if (BR.B)
		PC += (signed char)(read8(PC)) + 1;
	else
		++PC;


RST (0|8|10|18|20|28|30|38)H
	doPush(PC);
	PC = 0x0%1;
	
	
#
# Misc
#
CCF
	VALFLAG(F_H, GETFLAG(F_C));
	VALFLAG(F_C, !GETFLAG(F_C));
	RESFLAG(F_N);
	
SCF
	SETFLAG(F_C);
	RESFLAG(F_N | F_H);

CPL
	BR.A = ~BR.A;
	SETFLAG(F_H | F_N);
	
DAA
	doDAA();
	
EX \(SP\),(HL|IX|IY)
	ushort tmp = read16(WR.SP);
	write16(WR.SP, WR.%1);
	WR.%1 = tmp;

EX AF,AF'
	ushort tmp = R1.wr.AF;
	R1.wr.AF = R2.wr.AF;
	R2.wr.AF = tmp;

EX DE,HL
	ushort tmp = WR.DE;
	WR.DE = WR.HL;
	WR.HL = tmp;

EXX
	ushort tmp;	
	tmp = R1.wr.BC;
	R1.wr.BC = R2.wr.BC;
	R2.wr.BC = tmp;	
	
	tmp = R1.wr.DE;
	R1.wr.DE = R2.wr.DE;
	R2.wr.DE = tmp;	
	
	tmp = R1.wr.HL;
	R1.wr.HL = R2.wr.HL;
	R2.wr.HL = tmp;

HALT
	/* What should we do here? */


#
# Compare
#
CP \(HL\)
	byte val = read8(WR.HL);
	doArithmetic(val, 0, 1);	
	adjustFlags(val);

CP \((IX|IY)\+d\)
	byte val = read8(WR.%1 + (signed char)(read8(PC++)));
	doArithmetic(val, 0, 1);	
	adjustFlags(val);

CP (A|B|C|D|E|H|L|IXh|IXl|IYh|IYl)
	doArithmetic(BR.%1, 0, 1);	
	adjustFlags(BR.%1);

CP n
	byte val = read8(PC++);
	doArithmetic(val, 0, 1);	
	adjustFlags(val);

CPDR
	byte carry = GETFLAG(F_C);
	%CP (HL)
	WR.HL--;
	WR.BC--;
	VALFLAG(F_C, carry);
	if (WR.BC != 0 && !GETFLAG(F_Z))
		PC -= 2;  // Still going
	else
		VALFLAG(F_PV, WR.BC != 0);
	
CPD
	byte carry = GETFLAG(F_C);
	%CP (HL)
	WR.HL--;
	WR.BC--;
	VALFLAG(F_PV, WR.BC != 0);
	VALFLAG(F_C, carry);
	

CPIR
	byte carry = GETFLAG(F_C);
	%CP (HL)
	WR.HL++;
	WR.BC--;
	VALFLAG(F_C, carry);
	if (WR.BC != 0 && !GETFLAG(F_Z))
		PC -= 2;  // Still going
	else
		VALFLAG(F_PV, WR.BC != 0);

	
CPI
	byte carry = GETFLAG(F_C);
	%CP (HL)
	WR.HL++;
	WR.BC--;
	VALFLAG(F_PV, WR.BC != 0);
	VALFLAG(F_C, carry);


#
# INC and DEC
#
(INC|DEC) \(HL\)
	write8(WR.HL, doIncDec(read8(WR.HL), ID_%1));

(INC|DEC) \((IX|IY)\+d\)
	int off = (signed char)(read8(PC++));
	write8(WR.%2 + off, doIncDec(read8(WR.%2 + off), ID_%1));
	
(INC|DEC) (A|B|C|D|E|H|L|IXh|IXl|IYh|IYl)
	BR.%2 = doIncDec(BR.%2, ID_%1);

INC (BC|DE|HL|SP|IX|IY)
	++WR.%1;
	
DEC (BC|DE|HL|SP|IX|IY)
	--WR.%1;
	
#
# Interrupts
#
(EI|DI)
	IFF1 = IFF2 = IE_%1;

IM ([012])
	IM = %1;


#
# IO ports
#
IN (A|B|C|D|E|F|H|L),\(C\)
	BR.%1 = ioRead(BR.C);
	adjustFlagSZP(BR.%1);
	RESFLAG(F_H | F_N);

IN A,\(n\)
	byte port = read8(PC++);	
	BR.A = ioRead(port);

INDR
	%IND
	if (BR.B)
		PC -= 2;  // Still going
	else
	{
		SETFLAG(F_Z);
		RESFLAG(F_5 | F_3 | F_S);
	}

IND
	byte val = ioRead(BR.C);
	write8(WR.HL, val);
	WR.HL--;
	BR.B = doIncDec(BR.B, ID_DEC);
	VALFLAG(F_N, (val & 0x80) != 0);
	
INIR
	%INI
	if (BR.B)
		PC -= 2;  // Still going
	else
	{
		SETFLAG(F_Z);
		RESFLAG(F_5 | F_3 | F_S);
	}

INI
	byte val = ioRead(BR.C);
	write8(WR.HL, val);
	WR.HL++;
	BR.B = doIncDec(BR.B, ID_DEC);
	VALFLAG(F_N, (val & 0x80) != 0);

#
# Loads
#
LD \((BC|DE|HL)\),A
	write8(WR.%1, BR.A);

LD \(HL\),(B|C|D|E|H|L)
	write8(WR.HL, BR.%1);

LD \(HL\),n
	write8(WR.HL, read8(PC++));
	
LD \((IX|IY)\+d\),(A|B|C|D|E|H|L)
	write8(WR.%1 + (signed char)(read8(PC++)), BR.%2);
	
LD \((IX|IY)\+d\),n
	write8(WR.%1 + (signed char)(read8(PC)), read8(PC+1));
	PC += 2;
	
LD \(nn\),A
	write8(read16(PC), BR.A);
	PC += 2;
	
LD (BC|DE|HL|IX|IY|SP),(BC|DE|HL|IX|IY|SP)
	WR.%1 = WR.%2;

LD \(nn\),(BC|DE|HL|IX|IY|SP)
	write16(read16(PC), WR.%1);
	PC += 2;
	
LD A,\((BC|DE)\)
	BR.A = read8(WR.%1);

LD (A|B|C|D|E|H|L),\(HL\)
	BR.%1 = read8(WR.HL);

LD (A|B|C|D|E|H|L),\((IX|IY)\+d\)
	BR.%1 = read8(WR.%2 + (signed char)(read8(PC++)));

LD (A|B|C|D|E|H|L),\(nn\)
	BR.%1 = read8(read16(PC));
	PC += 2;
	
LD (A|B|C|D|E|H|L|IXh|IXl|IYh|IYl),(A|B|C|D|E|H|L|IXh|IXl|IYh|IYl)
	BR.%1 = BR.%2;

LD (A|B|C|D|E|H|L),(SL|SR)A \((IX|IY)\+d\)
	int off = (signed char)(read8(PC++));
	BR.%1 = do%2(read8(WR.%3 + off), 1);
	write8(WR.%3 + off, BR.%1);	
	
LD (A|B|C|D|E|H|L),(SL|SR)L \((IX|IY)\+d\)
	int off = (signed char)(read8(PC++));
	BR.%1 = do%2(read8(WR.%3 + off), 0);
	write8(WR.%3 + off, BR.%1);	
	  
LD (A|B|C|D|E|H|L),(RL|RLC|RR|RRC) \((IX|IY)\+d\)
	int off = (signed char)(read8(PC++));
	BR.%1 = do%2(1, read8(WR.%3 + off));
	write8(WR.%3 + off, BR.%1);

LD (A|B|C|D|E|H|L),(SET|RES) ([0-7]),\((IX|IY)\+d\)
	int off = (signed char)(read8(PC++));
	BR.%1 = doSetRes(SR_%2, %3, read8(WR.%4 + off));
	write8(WR.%4 + off, BR.%1);	

LD A,(I|R)
	BR.A = %1;
		
	adjustFlags(BR.A);
	RESFLAG(F_H | F_N);
	VALFLAG(F_PV, IFF2);
	VALFLAG(F_S, (BR.A & 0x80) != 0);
	VALFLAG(F_Z, (BR.A == 0));
	
LD (I|R),A
	%1 = BR.A;

LD (A|B|C|D|E|H|L|IXh|IXl|IYh|IYl),n
	BR.%1 = read8(PC++);
	
LD (BC|DE|HL|SP),\(nn\)
	WR.%1 = read16(read16(PC));
	PC += 2;

LD (BC|DE|HL|SP|IX|IY),\(nn\)
	ushort addr = read16(PC);
	PC += 2;
	WR.%1 = read16(addr);	

LD (BC|DE|HL|SP|IX|IY),nn
	WR.%1 = read16(PC);
	PC += 2;
	

LDIR
	%LDI
	if (WR.BC)
		PC -= 2;  // Repeat this opcode

LDI
	byte val = read8(WR.HL);
	write8(WR.DE, val);
	WR.DE++;
	WR.HL++;
	WR.BC--;
	
	VALFLAG(F_5, ((BR.A + val) & F_5) != 0);
	VALFLAG(F_3, ((BR.A + val) & F_3) != 0);
	RESFLAG(F_H | F_N);
	VALFLAG(F_PV, WR.BC != 0);

LDDR
	%LDD
	if (WR.BC)
		PC -= 2;  // Repeat this opcode

LDD
	byte val = read8(WR.HL);
	write8(WR.DE, val);
	WR.DE--;
	WR.HL--;
	WR.BC--;
	
	VALFLAG(F_5, ((BR.A + val) & F_5) != 0);
	VALFLAG(F_3, ((BR.A + val) & F_3) != 0);
	RESFLAG(F_H | F_N);
	VALFLAG(F_PV, WR.BC != 0);
	
NEG
	byte olda = BR.A;
	BR.A = 0;
	BR.A = doArithmetic(olda, F1_SUB, F2_SUB);

NOP
	/* NOP */
	

OUTI
	ioWrite(BR.C, read8(WR.HL));
	WR.HL++;
	BR.B = doIncDec(BR.B, 1);

OTIR
	%OUTI
	if (BR.B)
		PC -= 2;  // Still going
	else
	{
		SETFLAG(F_Z);
		RESFLAG(F_S | F_5 | F_3);
	}


OUTD
	ioWrite(BR.C, read8(WR.HL));
	WR.HL--;
	BR.B = doIncDec(BR.B, 1);

OTDR
	%OUTD
	if (BR.B)
		PC -= 2;  // Still going
	else
	{
		SETFLAG(F_Z);
		RESFLAG(F_S | F_5 | F_3);
	}

OUT \(C\),0
	ioWrite(BR.C, 0);
	
OUT \(C\),(A|B|C|D|E|H|L)
	ioWrite(BR.C, BR.%1);

OUT \(n\),A
	ioWrite(read8(PC++), BR.A);

POP (AF|BC|DE|HL|IX|IY)
	WR.%1 = doPop();

PUSH (AF|BC|DE|HL|IX|IY)
	doPush(WR.%1);


#
# Rotate & shift
#

(RLC|RRC|RL|RR) \(HL\)
	write8(WR.HL, do%1(1, read8(WR.HL)));
	
(RLC|RRC|RL|RR) (A|B|C|D|E|H|L|IXh|IXl|IYh|IYl)
	BR.%2 = do%1(1, BR.%2);

(RLC|RRC|RL|RR) \((IX|IY)\+d\)
	int off = (signed char)(read8(PC++));	
	write8(WR.%2 + off, do%1(1, read8(WR.%2 + off)));

(RL|RR|RLC|RRC)A
	BR.A = do%1(0, BR.A);
	
	
RLD
	byte Al = BR.A & 0x0F;
	byte hl = read8(WR.HL);
	
	BR.A = (BR.A & 0xF0) | ((hl & 0xF0) >> 4);
	hl = (hl << 4) | Al;
	write8(WR.HL, hl);
	
	VALFLAG(F_S, (BR.A & 0x80) != 0);
	VALFLAG(F_Z, BR.A == 0);
	RESFLAG(F_H | F_N);
	VALFLAG(F_PV, parityBit[BR.A]);


RRD
	byte Al = BR.A & 0x0F;
	byte hl = read8(WR.HL);
	
	BR.A = (BR.A & 0xF0) | (hl & 0x0F);
	hl = (hl >> 4) | (Al << 4);
	write8(WR.HL, hl);
	
	VALFLAG(F_S, (BR.A & 0x80) != 0);
	VALFLAG(F_Z, BR.A == 0);
	RESFLAG(F_H | F_N);
	VALFLAG(F_PV, parityBit[BR.A]);


(SL|SR)(L|A) \(HL\)
	write8(WR.HL, do%1(read8(WR.HL), IA_%2));


(SL|SR)(L|A) \((IX|IY)\+d\)
	int off = (signed char)(read8(PC++));	
	write8(WR.%3 + off, do%1(read8(WR.%3 + off), IA_%2));

(SL|SR)(L|A) (A|B|C|D|E|H|L|IXh|IXl|IYh|IYl)
	BR.%3 = do%1(BR.%3, IA_%2);
	
