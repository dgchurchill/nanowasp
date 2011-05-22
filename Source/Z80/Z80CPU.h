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

/* =============================================================================
 *  libz80 - Z80 emulation library
 * =============================================================================
 *
 * (C) Gabriel Gambetta (ggambett@adinet.com.uy) 2000 - 2002
 *
 * Version 1.99
 *
 * -----------------------------------------------------------------------------
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

#ifndef _Z80_H_
#define _Z80_H_

#include "../Device.h"
#include "../MemoryDevice.h"
#include "../NullMemory.h"
#include "../PortDevice.h"
#include "../NullPort.h"

class Microbee;


class Z80CPU : public Device
{
public:
    typedef unsigned short ushort;
    typedef unsigned char byte;


    /** 
     * A Z80 register set.
     * An union is used since we want independent access to the high and low bytes of the 16-bit registers.
     */
    typedef union 
    {
	    /** Word registers. */
	    struct
	    {
		    ushort AF, BC, DE, HL, IX, IY, SP;
	    } wr;
    	
	    /** Byte registers. Note that SP can't be accesed partially. */
	    struct
	    {
		    byte F, A, C, B, E, D, L, H, IXl, IXh, IYl, IYh;
	    } br;
    } Z80Regs;


    /** The Z80 flags */
	static const byte F_C  =   1;/**< Carry */
	static const byte F_N  =   2;  /**< Sub / Add */
	static const byte F_PV =   4;  /**< Parity / Overflow */
	static const byte F_3  =   8;  /**< Reserved */
	static const byte F_H  =  16;  /**< Half carry */
	static const byte F_5  =  32;  /**< Reserved */
	static const byte F_Z  =  64;  /**< Zero */
	static const byte F_S  = 128;  /**< Sign */


    Z80Regs	R1;		/**< Main register set (R) */
    Z80Regs R2;		/**< Alternate register set (R') */
    ushort	PC;		/**< Program counter */
    byte	R;		/**< Refresh */
    byte	I;
    byte	IFF1;	/**< Interrupt Flipflop 1 */
    byte	IFF2;	/**< Interrupt Flipflop 2 */
    byte	IM;		/**< Instruction mode */
	

    static const unsigned int MemSize = 65536;
    static const unsigned int PortSize = 256;


    //! Construct based on XML \p config_ (primarily used by DeviceFactory)
    Z80CPU(Microbee &mbee_, const TiXmlElement &config_);


    //! Register \p handler at \p addr in the memory address space
    void RegMemoryDevice(word addr, MemoryDevice* handler);
    //! Register \p handler at \p addr in the port address space
    void RegPortDevice(word addr, PortDevice* handler);

    /** Resets the processor. */
    void Reset();

    Microbee::time_t Execute(Microbee::time_t time, Microbee::time_t micros);

    virtual bool Executable() { return true; }

    Microbee::time_t GetTime();


    /** Decode the next instruction to be executed.
     * dump and decode can be NULL if such information is not needed
     *
     * @param dump A buffer which receives the hex dump
     * @param decode A buffer which receives the decoded instruction
     */
    void Z80Debug (char *dump, char *decode);

    
    /** Generates a hardware interrupt.
     * Some interrupt modes read a value from the data bus; this value must be provided in this function call, even
     * if the processor ignores that value in the current interrupt mode.
     *
     * @param value The value to read from the data bus
     */
    void Z80INT (byte value);


    /** Generates a non-maskable interrupt. */
    void Z80NMI ();


private:
    Microbee &mbee;

    unsigned long freq;  //!< CPU frequency in Hz

    Microbee::time_t emu_time;  //!< Current emulation time

    int cycles;  //!< cycles left to run (signed in case the previous execution overran)


    // TODO: Change union use to dynamic_casts, or get rid of it all together (separate RegPort / RegMem)
    class HandlerEntry
    { 
    public:
      HandlerEntry(MemoryDevice *h, word b) : base(b) { handler.mem = h; }
      HandlerEntry(PortDevice *h, word b) : base(b) { handler.port = h; }

      union
      {
          MemoryDevice* mem;
          PortDevice *port;
      } handler;
      word base;
    };

    unsigned int mem_block_size;  //**< Each block of mem_block_size in the address space can have a different handler
    std::vector<HandlerEntry> mem_handlers;

    unsigned int port_block_size;  //**< Each block of port_block_size in the address space can have a different handler
    std::vector<HandlerEntry> port_handlers;


    static NullMemory null_mem;
    static NullPort null_port;


    /* ---------------------------------------------------------
     *  Flag tricks
     * --------------------------------------------------------- 
     *
     * To avoid repeating entries in the spec files, many operations that look similar are treated as special cases
     * of a more general operation.
     *
     * For example, ADD and ADC are similar in syntax and operation - the difference is that ADC takes the carry flag
     * into account.
     *
     * So we define a general operation doArithmetic(...) which accepts a boolean parameter specifying wheter to do
     * a Carry-operation or not. Then, when we parse, we can say
     *
     * (ADD|ADC) ....
     *		doArithmetic(FLAG_FOR_%1)
     *
     * and everything works fine.
     *
     */
     
    /* Flags for doIncDec() */
    static const int ID_INC = 0;
    static const int ID_DEC = 1;

    /* Flags for enable / disable interrupts */
    static const int IE_DI = 0;
    static const int IE_EI = 1;

    /* Flags for doSetRes() */
    static const int SR_RES = 0;
    static const int SR_SET = 1;

    /* Flags for logical / arithmetic operations */
    static const int IA_L = 0;
    static const int IA_A = 1;

    /* Flags for doArithmetic() - F1 = withCarry, F2 = isSub */
    static const int F1_ADC = 1;
    static const int F1_SBC = 1;
    static const int F1_ADD = 0;
    static const int F1_SUB = 0;

    static const int F2_ADC = 0;
    static const int F2_SBC = 1;
    static const int F2_ADD = 0;
    static const int F2_SUB = 1;

    static int parityBit[256];
 

    typedef enum
    {
	    OP_NONE,
	    OP_BYTE,
	    OP_OFFSET,
	    OP_WORD	
    } Z80OperandType;

    typedef void (Z80CPU::*Z80OpcodeFunc) (); 

    struct Z80OpcodeTable;

    struct Z80OpcodeEntry
    {
	    Z80OpcodeFunc func;
    	
	    int operand_type;
        int cycles;
	    const char *format;	
    	
        Z80OpcodeTable *table;
    };

    struct Z80OpcodeTable
    {
	    int opcode_offset;
	    Z80OpcodeEntry entries[256];
    };



#include "codegen/opcodes_decl.h"
#include "codegen/opcodes_table_decl.h"

    void write8 (ushort addr, byte val);
    void write16 (ushort addr, ushort val);
    byte read8 (ushort addr);
    ushort read16 (ushort addr);
    byte ioRead (ushort addr);
    void ioWrite (ushort addr, byte val);

    void adjustFlags (byte val);
    void adjustFlagSZP (byte val);
    void adjustLogicFlag (int flagH);

    byte doArithmetic (byte value, int withCarry, int isSub);

    void doAND (byte value);
    void doOR (byte value);
    void doXOR (byte value);

    void doBIT (int b, byte val);
    byte doSetRes (int bit, int pos, byte val);

    byte doIncDec (byte val, int isDec);

    byte doRLC (int adjFlags, byte val);
    byte doRL (int adjFlags, byte val);
    byte doRRC (int adjFlags, byte val);
    byte doRR (int adjFlags, byte val);
    byte doSL (byte val, int isArith);
    byte doSR (byte val, int isArith);

    void doPush (ushort val);
    ushort doPop ();

    void doDAA ();
};


#endif
