/***********************************************************************

 CmdLineTest.c
 
 Author: David Petrovic
 GitHub: https://github.com/davepet1234/CmdLine

 Test application to test and demonstate the command line parser.

 Run "CmdLine.efi -h" to get help 

***********************************************************************/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/MemoryAllocationLib.h>
#include "CmdLine.h"

// Parameter variables
#define STR_MAXSIZE 20
CHAR16  Param1[STR_MAXSIZE] = L"default string";
UINTN   Param2  = 0;
UINTN   Param3  = 0;

// Switch variables
typedef enum { ENUM_BLACK, ENUM_RED, ENUM_GREEN, ENUM_BLUE, ENUM_WHITE } ENUM_COLOUR;
BOOLEAN     Flag        = FALSE;
UINTN       Flag2       = 0;
ENUM_COLOUR Colour      = ENUM_BLACK;
UINTN       DecValue    = 0;
UINTN       HexValue    = 0;
UINTN       IntValue    = 0;
CHAR16      StringValue[STR_MAXSIZE] = L"not initialised";

// Main program help
CHAR16 ProgName[]       = L"CmdLine";
CHAR16 ProgHelpStr[]    = L"Application to test command line parser";

// Parameter table defines 3 arguments
PARAMTABLE_START(ParamTable)
PARAMTABLE_STR(Param1, STR_MAXSIZE, L"[str]string parameter")
PARAMTABLE_HEX(&Param2,             L"[num1]hexidecimal parameter")
PARAMTABLE_DEC(&Param3,             L"[num2]decimal parameter")
PARAMTABLE_END

// String definitions for enum switch
ENUMSTR_START(EnumColourStrs)
ENUMSTR_ENTRY(ENUM_BLACK,  L"black")
ENUMSTR_ENTRY(ENUM_RED,    L"red")
ENUMSTR_ENTRY(ENUM_GREEN,  L"green")
ENUMSTR_ENTRY(ENUM_BLUE,   L"blue")
ENUMSTR_ENTRY(ENUM_WHITE,  L"white")
ENUMSTR_END

// Switch table defines 7 switches
SWTABLE_START(SwitchTable)
SWTABLE_OPT_FLAG(   L"-f",  NULL,           &Flag,                      L"boolean flag")
SWTABLE_OPT_FLGVAL( NULL,   L"-flag2",      &Flag2, 12345678,           L"flag with default value assigned")
SWTABLE_OPT_ENUM(   L"-c",  L"-colour",     &Colour, EnumColourStrs,    L"[val]named option")
SWTABLE_MAN_DEC(    L"-d",  L"-dec",        &DecValue,                  L"[num]decimal value")
SWTABLE_OPT_HEX(    L"-x",  L"-hex",        &HexValue,                  L"[num]hexidecimal value")
SWTABLE_OPT_INT(    L"-i",  NULL,           &IntValue,                  L"[num]integer value")
SWTABLE_OPT_STR(    L"-s",  L"-string",     StringValue, STR_MAXSIZE,   L"[str]string value")
SWTABLE_END

//---------------------------
// Main entry point
//---------------------------

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
    SHELL_STATUS ShellStatus = SHELL_SUCCESS;
    UINTN ParamCount;
        
    // Parse the command line
    ShellStatus = ParseCmdLine(ProgName, 1, ParamTable, SwitchTable, &ParamCount, ProgHelpStr);

    Print(L"========================================\n");
    if (ShellStatus == SHELL_SUCCESS) {
        Print(L"Parameters (%u):\n", ParamCount);
        Print(L"  Param1      = '%s'\n", Param1);
        Print(L"  Param2      = %d, 0x%02x\n", Param2, Param2);
        Print(L"  Param3      = %d, 0x%02x\n", Param3, Param3);
        Print(L"Options:\n");
        Print(L"  Flag        = %u\n", Flag);
        Print(L"  Flag2       = %u\n", Flag2);
        Print(L"  Colour      = %u\n", Colour);
        Print(L"  DecValue    = %u\n", DecValue);
        Print(L"  HexValue    = %u, 0x%02x\n", HexValue, HexValue);
        Print(L"  IntValue    = %u, 0x%02x\n", IntValue, IntValue);
        Print(L"  StringValue = '%s'\n", StringValue);
    }
    Print(L"ShellStatus   = %d\n", ShellStatus);
    Print(L"========================================\n");

    return ShellStatus;
}
