/*  Nanowasp - A Microbee emulator
 *  Copyright (C) 2000-2007 David G. Churchill
 *
 *  This file is part of Nanowasp.  It derives from libz80 (see below
 *  for libz80 license notice).  The following changes have been made:
 *
 *  July 2007 - Converted to C++ class
 *            - Numerous bug fixes to Z80 emulation
 *            - Minor optimisation
 *
 *  Nanowasp is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Nanowasp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* =========================================================
 *  libz80 - Z80 emulation library
 * =========================================================
 *
 * (C) Gabriel Gambetta (ggambett@adinet.com.uy) 2000 - 2002
 *
 * Version 1.99
 *
 * ---------------------------------------------------------
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdafx.h"
#include "Z80CPU.h"


#define BR (R1.br)
#define WR (R1.wr)

/* ---------------------------------------------------------
 *  Flag operations
 * --------------------------------------------------------- 
 */

#define SETFLAG(flag) (BR.F |= (flag))
#define RESFLAG(flag) (BR.F &= ~(flag))
#define GETFLAG(flag) ((BR.F & (flag)) != 0)

#define VALFLAG(flag,val) { if (val) SETFLAG(flag); else RESFLAG(flag); }


/* ---------------------------------------------------------
 *  Condition checks
 * --------------------------------------------------------- 
 */

#define COND_    (true)
#define COND_Z   (GETFLAG(F_Z))
#define COND_NZ  (!GETFLAG(F_Z))
#define COND_C   (GETFLAG(F_C))
#define COND_NC  (!GETFLAG(F_C))
#define COND_M   (GETFLAG(F_S))
#define COND_P   (!GETFLAG(F_S))
#define COND_PE  (GETFLAG(F_PV))
#define COND_PO  (!GETFLAG(F_PV))


/* ---------------------------------------------------------
 *  The opcode implementations
 * --------------------------------------------------------- 
 */


#include "codegen/opcodes_table.c"


/* ---------------------------------------------------------
 *  Data operations
 * --------------------------------------------------------- 
 */ 
void Z80CPU::write8 (ushort addr, byte val)
{
    HandlerEntry& he = mem_handlers[addr / mem_block_size];
    he.handler.mem->Write(addr - he.base, val);
}


void Z80CPU::write16 (ushort addr, ushort val)
{
    write8(addr, (byte)(val & 0xFF));
    write8(addr + 1, (byte)((val >> 8) & 0xFF));
}


byte Z80CPU::read8 (ushort addr)
{
    HandlerEntry& he = mem_handlers[addr / mem_block_size];
    return he.handler.mem->Read(addr - he.base);
}


Z80CPU::ushort Z80CPU::read16 (ushort addr)
{
	return ((ushort)read8(addr) | (read8(addr+1) << 8));
}


byte Z80CPU::ioRead (ushort addr)
{
    addr &= 0xFF;
    HandlerEntry& he = port_handlers[addr / port_block_size];
    return he.handler.port->PortRead(addr - he.base);
}


void Z80CPU::ioWrite (ushort addr, byte val)
{
    addr &= 0xFF;
    HandlerEntry& he = port_handlers[addr / port_block_size];
    he.handler.port->PortWrite(addr - he.base, val);
}


/* ---------------------------------------------------------
 *  Flag adjustments
 * --------------------------------------------------------- 
 */

int Z80CPU::parityBit[256] =
{ 
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1
};


void Z80CPU::adjustFlags (byte val)
{
	VALFLAG(F_5, (val & F_5) != 0);
	VALFLAG(F_3, (val & F_3) != 0);
}


void Z80CPU::adjustFlagSZP (byte val)
{
	VALFLAG(F_S, (val & 0x80) != 0);
	VALFLAG(F_Z, (val == 0));
	VALFLAG(F_PV, parityBit[val]);
}


// Adjust flags after AND, OR, XOR
void Z80CPU::adjustLogicFlag (int flagH)
{
    VALFLAG(F_S, (BR.A & 0x80) != 0);
    VALFLAG(F_Z, (BR.A == 0));
    VALFLAG(F_H, flagH);
    VALFLAG(F_N, 0);
    VALFLAG(F_C, 0);
    VALFLAG(F_PV, parityBit[BR.A]);

    adjustFlags(BR.A);
}


/* ---------------------------------------------------------
 *  Generic operations
 * --------------------------------------------------------- 
 */

 
/** Do an arithmetic operation (ADD, SUB, ADC, SBC y CP) */
byte Z80CPU::doArithmetic (byte value, int withCarry, int isSub)
{
	ushort res; /* To detect carry */
    ushort carry;

    if (withCarry && GETFLAG(F_C))
        carry = 1;
    else
        carry = 0;

	if (isSub)
	{
		res = BR.A - value - carry;
		SETFLAG(F_N);
		VALFLAG(F_H, (((BR.A & 0x0F) - (value & 0x0F) - carry) & 0x10) != 0);
	    VALFLAG(F_PV, (((BR.A & 0x80) != (value & 0x80)) && ((BR.A & 0x80) != (res & 0x80))) != 0);
	}
	else
	{
		res = BR.A + value + carry;
		RESFLAG(F_N);
		VALFLAG(F_H, (((BR.A & 0x0F) + (value & 0x0F) + carry) & 0x10) != 0);
	    VALFLAG(F_PV, (((BR.A & 0x80) == (value & 0x80)) && ((BR.A & 0x80) != (res & 0x80))) != 0);
	}
	VALFLAG(F_S, ((res & 0x80) != 0));
	VALFLAG(F_C, ((res & 0x100) != 0));
	VALFLAG(F_Z, ((res & 0xFF) == 0));

	adjustFlags(BR.A);

	return (byte)(res & 0xFF);
}


void Z80CPU::doAND (byte value)
{
	BR.A &= value;
	adjustLogicFlag(1);
}


void Z80CPU::doOR (byte value)
{
	BR.A |= value;
	adjustLogicFlag(0);
}


void Z80CPU::doXOR (byte value)
{
	BR.A ^= value;
	adjustLogicFlag(0);
}


void Z80CPU::doBIT (int b, byte val)
{
	if (val & (1 << b))
        RESFLAG(F_Z | F_PV);
    else
        SETFLAG(F_Z | F_PV);

    SETFLAG(F_H);
    RESFLAG(F_N);

    RESFLAG(F_S);
    if ((b == 7) && !GETFLAG(F_Z))
        SETFLAG(F_S);

    RESFLAG(F_5);
    if ((b == 5) && !GETFLAG(F_Z))
        SETFLAG(F_5);

    RESFLAG(F_3);
    if ((b == 3) && !GETFLAG(F_Z))
        SETFLAG(F_3);
}


byte Z80CPU::doSetRes (int bit, int pos, byte val)
{
    if (bit)
		val |= (1 << pos);
    else
		val &= ~(1 << pos);
    return val;
}



byte Z80CPU::doIncDec (byte val, int isDec)
{
    if (isDec)
    {
        VALFLAG(F_PV, val == 128);  // val is unsigned, but considering signed overflow.  128u = -128s
        val--;
        VALFLAG(F_H, (val & 0x0F) == 0x0F);
    }
    else
    {
        VALFLAG(F_PV, val == 127);
        val++;
        VALFLAG(F_H, (val & 0x0F) == 0);
    }

    VALFLAG(F_S, ((val & 0x80) != 0));
    VALFLAG(F_Z, (val == 0));
    VALFLAG(F_N, isDec);

    adjustFlags(BR.A);

    return val;
}


byte Z80CPU::doRLC (int adjFlags, byte val)
{
    VALFLAG(F_C, (val & 0x80) != 0);
    val <<= 1;
    if (GETFLAG(F_C))
        val |= 0x01;

    adjustFlags(val);
    RESFLAG(F_H | F_N);

    if (adjFlags)
        adjustFlagSZP(val);

    return val;
}


byte Z80CPU::doRL (int adjFlags, byte val)
{
    int CY = GETFLAG(F_C);
    VALFLAG(F_C, (val & 0x80) != 0);
    val <<= 1;
    val |= (byte)CY;

    adjustFlags(val);
    RESFLAG(F_H | F_N);

    if (adjFlags)
        adjustFlagSZP(val);

    return val;
}


byte Z80CPU::doRRC (int adjFlags, byte val)
{
    VALFLAG(F_C, (val & 0x01) != 0);
    val >>= 1;
    if (GETFLAG(F_C))
        val |= 0x80;

    adjustFlags(val);
    RESFLAG(F_H | F_N);

    if (adjFlags)
        adjustFlagSZP(val);

    return val;
}


byte Z80CPU::doRR (int adjFlags, byte val)
{
    int CY = GETFLAG(F_C);
    VALFLAG(F_C, (val & 0x01));
    val >>= 1;
    val |= (CY << 7);

    adjustFlags(val);
    RESFLAG(F_H | F_N);

    if (adjFlags)
        adjustFlagSZP(val);

    return val;
}


byte Z80CPU::doSL (byte val, int isArith)
{
    VALFLAG(F_C, (val & 0x80) != 0);
    val <<= 1;

    if (!isArith)
        val |= 1;

    adjustFlags(val);
    RESFLAG(F_H | F_N);
    adjustFlagSZP(val);

    return val;
}


byte Z80CPU::doSR (byte val, int isArith)
{
    byte b = val & 0x80;

    VALFLAG(F_C, (val & 0x01) != 0);
    val >>= 1;

    if (isArith)
        val |= b;

    adjustFlags(val);
    RESFLAG(F_H | F_N);
    adjustFlagSZP(val);

    return val;
}


void Z80CPU::doPush (ushort val)
{
    WR.SP -= 2;
	write16(WR.SP, val);
}


Z80CPU::ushort Z80CPU::doPop ()
{	
	ushort val;
	
	val = read16(WR.SP);
	WR.SP += 2;

    return val;
}


void Z80CPU::doDAA ()
{
    byte lnib = BR.A & 0x0F;

    if (GETFLAG(F_N))
    {
        // Last op was subtract

        if (BR.A >= 0x9A)
            SETFLAG(F_C);  // A decimal borrow occurred

        // Adjust the high nibble if a (decimal or hex) borrow occurred
        if (GETFLAG(F_C))  // Carry can be set above or by a previous instruction
            BR.A -= 0x60;

        // Low nibble
        // If nibble is less than/equal to 9 and there was no half carry then everything is OK,
        // otherwise an adjustment must be made.
        if (lnib > 9 || GETFLAG(F_H))
        {
            // Half carry flag is set according to whether the *adjustment* caused a carry
            // If this point is reached with H clear then the nibble must be greater than 9
            // so it will stay clear, hence there is no need to explicitly set the flag only
            // to reset it in certain cases.
            if (lnib >= 0x06)
                RESFLAG(F_H);
            BR.A -= 0x06;
        }
    }
    else
    {
        // Last op was add

        // Low nibble
        // If nibble is less than/equal to 9 and there was no half carry then everything is OK,
        // otherwise an adjustment must be made.
        if (lnib > 9)
        {
            BR.A += 0x06;
            SETFLAG(F_H);  // Half carry flag is set according to whether the *adjustment* caused a carry
            if (BR.A < 0x06)
                SETFLAG(F_C);
        }
        else if (GETFLAG(F_H))
        {
            BR.A += 0x06;
            RESFLAG(F_H);  // Half carry flag is set according to whether the *adjustment* caused a carry
        }

        // High nibble
        // Same again but with high nibble and full carry.  Note that the adjustment of the
        // Low nibble may have altered the high nibble so this must be done second.
        if ((BR.A >> 4) > 9 || GETFLAG(F_C))
        {
            BR.A += 0x60;
            SETFLAG(F_C);
        }
    }

    adjustFlagSZP(BR.A);
	adjustFlags(BR.A);
}

#include "codegen/opcodes_impl.c"


/* ---------------------------------------------------------
 *  The top-level functions
 * --------------------------------------------------------- 
 */ 

Microbee::time_t Z80CPU::Execute(Microbee::time_t time, Microbee::time_t micros)
{
    emu_time = time + micros;  // Time once we're done
    cycles += micros * freq / 1000000;  // Number of cycles to execute (+= because last loop may have executed too much)


    while (cycles > 0)
    {
	    Z80OpcodeTable *current = &opcodes_main;
	    Z80OpcodeEntry *entries = current->entries;
	    Z80OpcodeFunc func;

       
	    byte opcode;
	    int offset = 0;
	    do
	    {
		    opcode = read8(PC + offset);
    		
		    PC++;
		    func = entries[opcode].func;
		    if (func != NULL)
		    {			
			    PC -= offset;
			    (this->*func)();
			    PC += offset;
			    break;
		    }
		    else if (entries[opcode].table != NULL)
		    {
			    current = entries[opcode].table;
			    entries = current->entries;
			    offset = current->opcode_offset;
		    }

		    else
		    {
			    /* NOP */
			    break;	
		    }
	    } while (true);

        cycles -= entries[opcode].cycles;
    }

    return 0;  // Run again at next available opportunity
}


#if 0
void Z80CPU::Z80Debug (char *dump, char *decode)
{
	char tmp[20];	
	Z80OpcodeTable *current = &opcodes_main;
	Z80OpcodeEntry *entries = current->entries;
	char *fmt;
	byte opcode;
	ushort parm;
	int offset = 0;
	int prog_count = PC;
	int size = 0;
	
	if (dump)
		dump[0] = 0;
		
	if (decode)
		decode[0] = 0;

	do
	{
		opcode = read8(prog_count + offset);
		size++;
		
		prog_count++;
		R++;
		fmt = entries[opcode].format;
		if (fmt != NULL)
		{			
			prog_count -= offset;
			parm = read16(prog_count);
		
			if (entries[opcode].operand_type == OP_NONE)
				size++;
			else
				size += 2;
			if (entries[opcode].operand_type != OP_WORD)
			{
				parm &= 0xFF;
				size--;
			}
				
			if (decode)
				sprintf(decode, fmt, parm);
			
			prog_count += offset;
			break;
		}
		else if (entries[opcode].table != NULL)
		{
			current = entries[opcode].table;
			entries = current->entries;
			offset = current->opcode_offset;
		}

		else
		{
			if (decode != NULL)
				strcpy(decode, "NOP (ignored)");
			break;	
		}
	} while(1);	
	
	if (dump)
	{
		for (offset = 0; offset < size; offset++)
		{
			sprintf(tmp, "%02X", read8(prog_count + offset));
			strcat(dump, tmp);
		}		
	}
}
#endif


void Z80CPU::Reset()
{
	PC = 0x0000;
	BR.F = 0;
	IM = 0;
	IFF1 = IFF2 = 0;

    emu_time = mbee.GetTime();
    cycles = 0;


#if 0   // For DAA operation comparison
    printf("A,...,,..H,,.C.,,.CH,,N..,,N.H,,NC.,,NCH,,\n");
    BR.A = 0;
    for (int i = 0; i < 256; i++)
    {
        printf("%02X,", i);

        for (int j = 0; j < 8; j++)
        {
            BR.A = i & 0xFF;
            valFlag(F_N, (j & 4) != 0);
            valFlag(F_C, (j & 2) != 0);
            valFlag(F_H, (j & 1) != 0);
            doDAA();

            printf("%02X,%c%c%c%c%c%c%c%c,",
                    BR.A,
                    GETFLAG(F_S) ? 'S' : '.',
                    GETFLAG(F_Z) ? 'Z' : '.',
                    '.',
                    GETFLAG(F_H) ? 'H' : '.',
                    '.',
                    GETFLAG(F_PV) ? 'P' : '.',
                    GETFLAG(F_N) ? 'N' : '.',
                    GETFLAG(F_C) ? 'C' : '.');
        }

        printf("\n");
    }
    throw "die";
#endif

}


void Z80CPU::Z80INT (byte value)
{
    if (!IFF1)
        return;

    if (IM == 0)
    {  	
    	/* FIXME What to do? */
/*        opcode = Val;
        execute();*/
    }
    else if (IM == 1)
    {
        doPush(PC);
        PC = 0x0038;
    }
    else if (IM == 2)
    {
        doPush(PC);
        PC = (I << 8) | value;
    }
}


void Z80CPU::Z80NMI ()
{
	IFF1 = 0;
    doPush(PC);
	PC = 0x0066;	
}


NullMemory Z80CPU::null_mem = NullMemory(Z80CPU::MemSize);
NullPort Z80CPU::null_port = NullPort(Z80CPU::PortSize);


/*! \p config_ must specify the attribute freq on the <device> */
Z80CPU::Z80CPU(Microbee &mbee_, const TiXmlElement &config_) :
mbee(mbee_),
mem_block_size(MemSize), mem_handlers(1, HandlerEntry(&null_mem, 0x0000)), 
port_block_size(PortSize), port_handlers(1, HandlerEntry(&null_port, 0x00))
{
    int f;
    if (config_.Attribute("freq", &f) == NULL)
        throw ConfigError(&config_, "Z80CPU missing freq attribute");
    if (f <= 0)
        throw ConfigError(&config_, "Z80CPU freq attribute must be positive");
    freq = f;
}


/*! \brief Register a new memory device.
 *
 *  A memory device is responsible for processing memory read/write
 *  requests for a specified region of the address space.  The vector mem_handlers is used to map 
 *  addresses to handlers.  Elements in mem_handlers are arranged in the same order as the memory
 *  address space, and each element covers a range of mem_block_size.  This allows the read/write
 *  requests to be processed quickly.
 */
void Z80CPU::RegMemoryDevice(word addr, MemoryDevice* handler)
{
    // Adjust the block size if necessary
    const unsigned int start = addr, end = start + handler->GetSize();
    unsigned int start_align = 1, end_align = 1, align;

    if (end > MemSize)
        throw OutOfRange();

    while ((start & start_align) == 0 && start_align < 0x10000)
        start_align <<= 1;

    while ((end & end_align) == 0 && end_align < 0x10000)
        end_align <<= 1;

    align = start_align < end_align ? start_align : end_align;


    if (align < mem_block_size)
    {
        // Smaller blocks are required to store the details of this handler, so rebuild 
        // the mem_handlers vector at the new size.

        std::vector<HandlerEntry> new_handlers;
        new_handlers.reserve(MemSize / align);

        std::vector<HandlerEntry>::iterator it = mem_handlers.begin();
        for (; it != mem_handlers.end(); it++)
        {
            for (unsigned int i = 0; i < mem_block_size / align; i++)
                new_handlers.push_back(*it);
        }

        mem_handlers = new_handlers;
        mem_block_size = align;
    }


    // Install the new handler
    for (unsigned int i = start / mem_block_size; i < end / mem_block_size; i++)
    {
        mem_handlers[i].handler.mem = handler;
        mem_handlers[i].base = addr;
    }
}

// TODO: Factor the code from RegMemoryDevice and RegPortDevice
// TODO: Put appropriate const tags on parameters
void Z80CPU::RegPortDevice(word addr, PortDevice* handler)
{
    // Adjust the block size if necessary
    const unsigned int start = addr, end = start + handler->GetSize();
    unsigned int start_align = 1, end_align = 1, align;


    if (end > PortSize)
        throw OutOfRange();

    while ((start & start_align) == 0 && start_align < 0x100)
        start_align <<= 1;

    while ((end & end_align) == 0 && end_align < 0x100)
        end_align <<= 1;

    align = start_align < end_align ? start_align : end_align;


    if (align < port_block_size)
    {
        // Smaller blocks are required to store the details of this handler, so rebuild 
        // the mem_handlers vector at the new size.

        std::vector<HandlerEntry> new_handlers;
        new_handlers.reserve(PortSize / align);

        std::vector<HandlerEntry>::iterator it = port_handlers.begin();
        for (; it != port_handlers.end(); it++)
        {
            for (unsigned int i = 0; i < port_block_size / align; i++)
                new_handlers.push_back(*it);
        }

        port_handlers = new_handlers;
        port_block_size = align;
    }


    // Install the new handler
    for (unsigned int i = start / port_block_size; i < end / port_block_size; i++)
    {
        port_handlers[i].handler.port = handler;
        port_handlers[i].base = addr;
    }
}


Microbee::time_t Z80CPU::GetTime()
{
    return emu_time - (Microbee::time_t)cycles * 1000000 / freq;  // We've got 'cycles' left to run, so back in time we go
}
