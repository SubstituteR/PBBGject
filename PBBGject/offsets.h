#pragma once
int base = 0; //needs to be updated at run-time...
char localVariable = 0;
int retJMP = 0;

void placeJMP(BYTE* address, DWORD jumpTo, DWORD length) //need to clean up.
{
    DWORD oldProtect, newProtect, relativeAddress;
    VirtualProtect(address, length, PAGE_EXECUTE_READWRITE, &oldProtect);
    relativeAddress = (DWORD)(jumpTo - (DWORD)address) - 5;
    *address = 0xE9;
    *((DWORD*)(address + 0x1)) = relativeAddress;
    for (DWORD x = 0x5; x < length; x++)
    {
        *(address + x) = 0x90;
    }
    VirtualProtect(address, length, oldProtect, &newProtect);
}

void writeOPByte(int offset, char nextValue)
{
    char* address = (char*)base + offset + 0x3;
    DWORD prev;
    VirtualProtect(address, 1, PAGE_EXECUTE_READWRITE, &prev);
    *address = nextValue;
    VirtualProtect(address, 1, prev, nullptr);
}

__inline int computeJMP(int start) { return base + start; }

#define JMP(name, start, size) \
 int name ##_start = start; \
 int name ##_end = start+size; \
 int name ##_size = size;

#define ComputeJMP(name) \
     computeJMP(name ##_start)



#define pushStack() \
    __asm pushad \
    __asm pushfd
#define popStack() \
    __asm popfd \
    __asm popad
#define ComputeRET(name) \
    retJMP = base + name ##_end;

#define RET() \
    _asm jmp[retJMP]

#define ReturnAndPopStack(name) \
    ComputeRET(name) \
    popStack() \
    RET() \

#define PlaceJMP(name, func) \
 placeJMP((BYTE*) ComputeJMP(name), (DWORD) func, name ##_size);




JMP(EnableCostume, 0x10641, 0x5);
JMP(FixCharSel, 0x9AD46, 0x7);
JMP(NextChar, 0x105D5, 0x5);
JMP(PrevChar, 0x10585, 0x5);
JMP(DisableCostume, 0x10697, 0x5);

JMP(PrevColor, 0x10817, 0x6);
JMP(NextColor, 0x10833, 0x6);

JMP(CorrectColor, 0x22D3B, 0x5);