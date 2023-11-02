#include <iostream>

using namespace std;

// 8-BIT CPU/DATA BUS ; 16-BIT ADDRRESS BUS {64KB MEM}
using Byte = unsigned char;  // 8-BIT
using Word = unsigned short; // 16-BIT

using u32 = unsigned int;

struct Mem
{
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void Initialise()
    {
        for (u32 i = 0; i < MAX_MEM; i++)
        {
            Data[i] = 0;
        }
    }

    // READ 1 BYTE
    Byte operator[](u32 Address) const
    {
        return Data[Address];
    }

    // WRITE 1 BYTE
    Byte &operator[](u32 Address)
    {
        return Data[Address];
    }
};

struct CPU
{

    // 16-BIT REGISTER
    Word PC; // PROGRAM COUNTER , NEXT INS
    Word SP; // 256 BYTE STACK {0100 - 01FF}

    // 8-BIT REGISTERS
    Byte A; // ACCUMULATOR {ALU}
    Byte X; // INDEX REGISTER X {OFFSETS & COUNTERS}
    Byte Y; // INDEX REFISTER Y {OFFSETS & COUNTERS}

    // STATUS FLAGS {BIT-FIELD , EACH JUST A SINGLE BIT}
    Byte C : 1; // CARRY
    Byte Z : 1; // ZERO
    Byte I : 1; // INTERRUPTS
    Byte D : 1; // DECIMAL MODE
    Byte B : 1; // WHEN AN INTERRUPT
    Byte V : 1; // OVERFLOW
    Byte N : 1; // NEGATIVE

    // SPECIFIED ON THE WIKI
    void Reset(Mem &memory)
    {
        PC = 0xFFFC; // RESET ROUTINE
        SP = 0X0100;
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;
        memory.Initialise();
    }

    Byte FetchByte(u32 &Cycles, Mem &memory)
    {
        Byte Data = memory[PC];
        PC++;
        Cycles--;
        return Data;
    }

    Byte FetchWord(u32 &Cycles, Mem &memory)
    {
        // 6502 is little endian -> first byte is least significant
        Word Data = memory[PC];
        PC++;
        Cycles--;

        Data |= (memory[PC] << 8);
        PC++;
        Cycles--;

        return Data;
    }

    Byte ReadByte(u32 &Cycles, Byte Address, Mem &memory)
    {
        Byte Data = memory[Address];
        Cycles--;
        return Data;
    }

    // opcodes
    // LOAD A BYTE OF MEMORY IN ACC , IN IMMEDIATE MODE {FIRST BYTE OP , SECOND DATA} , absolute mode contains full 16 bit address for the target location
    static constexpr Byte
        INS_LDA_IM = 0xA9,
        INS_LDA_ZP = 0xA5, // next byte is address
        INS_LDA_ZPX = 0xB5;
    
    // SET FLAGS AFTER LDA
    void LDASetStatus()
    {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }

    void Execute(u32 Cycles, Mem &memory)
    {
        while (Cycles > 0)
        {
            Byte Ins = FetchByte(Cycles, memory);
            switch (Ins)
            {

            case INS_LDA_IM:
            {
                Byte Value = FetchByte(Cycles, memory);
                A = Value;
                LDASetStatus();
            }
            break;

            case INS_LDA_ZP:
            {
                Byte ZeroPageAddress = FetchByte(Cycles, memory);
                A = ReadByte(Cycles, ZeroPageAddress, memory);
                LDASetStatus();
            }
            break;

            case INS_LDA_ZPX:
            {
                Byte ZeroPageAddress = FetchByte(Cycles, memory);
                ZeroPageAddress += X;
                Cycles--;
                A = ReadByte(Cycles, ZeroPageAddress, memory);
                LDASetStatus();
            }
            break;

            default:
            {
                cout << "FAULTY INSTRUCTION";
            }
            break;
            }
        }
    }
};

int main()
{
    Mem mem;
    CPU cpu;
    cpu.Reset(mem);

    // inline program testing
    mem[0xFFFC] = CPU::INS_LDA_ZP; //tell the accumulator that the next byte is the address in zero page
    mem[0xFFFD] = 0x42; // load value that is present at the address 0x42
    mem[0x0042] = 0x84; //putting in some value at memory location 0x42

    cpu.Execute(3, mem);
    return 0;
}