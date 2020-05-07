#pragma once
#include "pch.h"
#include <thread>
#include "offsets.h"

#define TOTAL_CHARACTERS 0x10
#define PALETTES_PER_SET 0x8
#define TOTAL_SETS 0xD //13 > 100 / 8 > 12
#define PALETTE_SIZE 0x400
#define TOTAL_MEM TOTAL_CHARACTERS * TOTAL_SETS * PALETTES_PER_SET * PALETTE_SIZE


char selectedColor = 0;
char selectedCharacter = 0;
char palettes[TOTAL_CHARACTERS][TOTAL_SETS][PALETTES_PER_SET][PALETTE_SIZE]; //[0xF]max chars, 16, max sets, 100, max palette 8, palette sizeof 1024

void injectPalettes()
{
    int set = (selectedColor < 0 ? 100 : selectedColor > 100 ? 0 : selectedColor) / 8;
    memcpy((char*)base + 0xB4CF84, palettes[selectedCharacter][set], PALETTE_SIZE * PALETTES_PER_SET);
}
void storeColorInformation(char nextValue)
{
    selectedColor = nextValue < 0 ? 100 : nextValue > 100 ? 0 : nextValue;
    printf("Color Updated %i\n", selectedColor);
    injectPalettes();
}
void storeCharacterInformation(char selected) 
{ 
    printf("Character Updated %i\n", selected); 
    selectedCharacter = selected; 
}
/*
    Fixes the selected color to be bound between 0-7.
    This is for when the game transitions from the character select
    to the level select menu.
*/
__declspec(naked) void correctColor()
{
    __asm
    {
         mov al, [esi + 0x3A]
         mov localVariable, al
    }
    pushStack();
    localVariable %= 8;
    ComputeRET(CorrectColor);
    popStack();
    __asm
    {
         mov al, localVariable
         cmp al, 07
        jmp[retJMP]
    }
    RET();
}

__declspec(naked) void nextColor()
{
    __asm
    {
         mov eax, [edi + 0x00000194]
         mov cl, [eax + 0x3A]
         mov localVariable, cl
    }
    pushStack();
    storeColorInformation(localVariable);
    ReturnAndPopStack(NextColor);
}

__declspec(naked) void prevColor()
{
    __asm
    {
         mov eax, [edi + 0x00000194]
         mov cl, [eax + 0x3A]
         mov localVariable, cl
    }
    pushStack();
    storeColorInformation(localVariable);
    ReturnAndPopStack(PrevColor);
}

__declspec(naked) void prevCharacter()
{
    __asm
    {
        mov byte ptr [esp + 0xC], 01
        mov localVariable, cl
    }
    pushStack();
    storeCharacterInformation(localVariable);
    ReturnAndPopStack(PrevChar);
}

__declspec(naked) void nextCharacter()
{
    __asm
    {
        mov byte ptr[esp + 0xC], 02
        mov localVariable, cl
    }
    pushStack();
    storeCharacterInformation(localVariable);
    ReturnAndPopStack(NextChar);
}

__declspec(naked) void enableCostume()
{
    __asm
    {
        mov byte ptr[esp + 0xC], 03
        mov localVariable, cl
    }
    pushStack();
    storeCharacterInformation(localVariable);
    ReturnAndPopStack(EnableCostume);
}

__declspec(naked) void disableCostume()
{
    __asm
    {
        mov byte ptr[esp + 0xC], 04
        mov localVariable, cl
    }
    pushStack();
    storeCharacterInformation(localVariable);
    ReturnAndPopStack(DisableCostume);
}

__declspec(naked) void characterSelectHack()
{
    __asm
    {
        mov al, [esi + 0x000000D0];
        mov localVariable, al;
    }
    pushStack();
    localVariable %= 8;
    ComputeRET(FixCharSel);
    popStack();
    __asm mov al, [localVariable];
     RET();
}

void run()
{
    base = (int)GetModuleHandle(NULL);;

    writeOPByte(0x10839, 101); //max colors
    writeOPByte(0x10823, 100); //go from 0 to upper limit

    PlaceJMP(FixCharSel, characterSelectHack);
    PlaceJMP(EnableCostume, enableCostume);
    PlaceJMP(DisableCostume, disableCostume);
    PlaceJMP(NextChar, nextCharacter);
    PlaceJMP(PrevChar, prevCharacter);
    PlaceJMP(PrevColor, prevColor);
    PlaceJMP(NextColor, nextColor);
    PlaceJMP(CorrectColor, correctColor);

    ZeroMemory(palettes, TOTAL_MEM); //init memory

    for (int i = 0; i < TOTAL_MEM; i++) { ((char*)palettes)[i] = rand(); } //FOR DEBUG -> write random color data...
    while (true) { injectPalettes(); } //filthy hack, surely there's a better way?
}

void createConsole()
{
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        createConsole();
        std::thread(run).detach();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        //todo , cleanup
        break;
    }
    return TRUE;
}

