/***********************************************************************

 CmdLine.c
 
 Author: David Petrovic
 GitHub: https://github.com/davepet1234/CmdLine

***********************************************************************/

// ### 
// Useful link: Command-Line Interfaces: Structure & Syntax
// https://dev.to/paulasantamaria/command-line-interfaces-structure-syntax-2533
// ###

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib/BaseLibInternals.h>
#include "CmdLineInternal.h"


#define DEBUG_MODE 0
#if DEBUG_MODE
#define TRACE(x) Print x
#else
#define TRACE(x)
#endif


// locals functions
STATIC BOOLEAN ReturnValue(IN CONST CHAR16 *String, IN VALUE_TYPE ValueType, IN DATA *Data, OUT VALUE_RET_PTR ValueRetPtr);
STATIC BOOLEAN GetEnumVal(IN ENUM_STR_ARRAY *EnumStrArray, IN CONST CHAR16 *Str, OUT UINTN *Value);
STATIC INTN EFIAPI StriCmp(IN CONST CHAR16 *FirstString, IN CONST CHAR16 *SecondString);
STATIC BOOLEAN HasHexPrefix(IN CONST CHAR16 *String);
STATIC BOOLEAN IsHexString(IN CONST CHAR16 *String);
STATIC BOOLEAN IsDecimalString(IN CONST CHAR16 *String);
STATIC VOID TableError(IN UINTN i, IN CHAR16 *errStr);
STATIC BOOLEAN ArgNameDefined(IN CHAR16 *HelpStr);
STATIC UINTN GetArgName(IN CHAR16 *HelpStr, OUT CHAR16* ArgName, IN UINTN ArgNameSize, IN BOOLEAN Mandatory, IN CONST CHAR16 *DefaultArgName);
STATIC VOID ShowHelp(IN CONST CHAR16 *ProgName, IN UINTN ManParmCount, IN PARAMETER_TABLE *ParamTable, IN SWITCH_TABLE *SwTable, IN CONST CHAR16 *ProgHelpStr);

// globals
STATIC CHAR16 BreakSwStr1[] = L"-b";
STATIC CHAR16 BreakSwStr2[] = L"-break";
STATIC CONST CHAR16 BreakSwStr[] = L"enable page break mode";

STATIC CHAR16 HelpSwStr1[] = L"-h";
STATIC CHAR16 HelpSwStr2[] = L"-help";
STATIC CONST CHAR16 HelpSwStr[] = L"display this help and exit";

STATIC CONST CHAR16 DefaultArgName[] = L"arg";

/**
 * ParseCmdLine()
 * 
 **/
SHELL_STATUS ParseCmdLine(IN CONST CHAR16 *ProgName, IN UINTN ManParmCount, IN PARAMETER_TABLE *ParamTable, IN SWITCH_TABLE *SwTable, OUT UINTN *NumParams, IN CHAR16 *ProgHelpStr)
{
    SHELL_STATUS ShellStatus = SHELL_INVALID_PARAMETER;
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN OptCount = 0;
    UINTN i, j;
    CHAR16 *ProblemParam = NULL;
    UINTN Memsize;
    UINTN ParamCount, TableParamCount;    

    // initialise switch present flags
    BOOLEAN SwPresent[MAX_SWITCH_ENTRIES] = {0};

    // determine how many items for options table
    i = 0;
    while (SwTable[i].SwitchNecessity != NO_SW) {
        if (SwTable[i].SwStr1) OptCount++;
        if (SwTable[i].SwStr2) OptCount++;
        i++;
    }
    // add break and help options
    OptCount += 4;
    
    // allocate memory for options table
    Memsize = (OptCount+1) * sizeof(SHELL_PARAM_ITEM);
    SHELL_PARAM_ITEM *ParamList = AllocateZeroPool(Memsize);
    if (!ParamList) {
        return SHELL_OUT_OF_RESOURCES;
    }
    
    // construct options table
    i = 0; j = 0;
    while (SwTable[i].SwitchNecessity != NO_SW) {
        if (SwTable[i].SwStr1) {
            ParamList[j].Name = SwTable[i].SwStr1;
            ParamList[j].Type = SwTable[i].ValueType == VALTYPE_NONE ? TypeFlag : TypeValue;
            j++;
        }
        if (SwTable[i].SwStr2) {
            ParamList[j].Name = SwTable[i].SwStr2;
            ParamList[j].Type = SwTable[i].ValueType == VALTYPE_NONE ? TypeFlag : TypeValue;
            j++;
        }
        i++;
    }
    // add break and help options
    ParamList[j].Name = BreakSwStr1;
    ParamList[j].Type = TypeFlag;
    j++;
    ParamList[j].Name = BreakSwStr2;
    ParamList[j].Type = TypeFlag;
    j++;
    ParamList[j].Name = HelpSwStr1;
    ParamList[j].Type = TypeFlag;
    j++;
    ParamList[j].Name = HelpSwStr2;
    ParamList[j].Type = TypeFlag;
    j++;
    ParamList[j].Name = NULL;
    ParamList[j].Type = TypeMax;
   
#if DEBUG_MODE
    for (i=0; i<=OptCount; i++) {
        TRACE((L"\"%s\", ", ParamList[i].Name));
        switch (ParamList[i].Type) {
        case TypeFlag:  TRACE((L"TypeFlag\n")); break;
        case TypeValue: TRACE((L"TypeValue\n")); break;
        case TypeMax:   TRACE((L"TypeMax\n")); break;
        default:        TRACE((L"Unknown(%d)\n", ParamList[i].Type)); break;
        }
    }
#endif

    // parse command line
    LIST_ENTRY *Package = NULL;
    Status = ShellCommandLineParseEx(ParamList, &Package, &ProblemParam, TRUE, FALSE);
    if (EFI_ERROR(Status)) {
        ShellPrintEx(-1, -1, L"%H%s%N: Unknown option - '%H%s%N'\r\n", ProgName, ProblemParam);
        FreePool(ProblemParam);
        goto Error_exit;
    }

    //------------
    // BREAK
    //------------

    if (ShellCommandLineGetFlag(Package, BreakSwStr1) || ShellCommandLineGetFlag(Package, BreakSwStr2)) {
        ShellSetPageBreakMode(TRUE);
    } else {
        ShellSetPageBreakMode(FALSE);
    }

    //------------
    // HELP 
    //------------

    if (ShellCommandLineGetFlag(Package, HelpSwStr1) || ShellCommandLineGetFlag(Package, HelpSwStr2)) {
        ShowHelp(ProgName, ManParmCount, ParamTable, SwTable, ProgHelpStr);
        ShellStatus = SHELL_ABORTED;
        goto Error_exit;
    }

    //------------
    // PARAMETERS 
    //------------
        
    // determine number of parameters in table
    TableParamCount = 0;
    while (ParamTable[TableParamCount].ValueType != VALTYPE_NONE) {
        TableParamCount++;
    }
    
    // check the number of parameters
    ParamCount = ShellCommandLineGetCount(Package);
    if (NumParams) {
        // return number of actual parameters (ignore program name)
        *NumParams = ParamCount-1;
    }
    if (ParamCount > TableParamCount + 1) {
        ShellPrintEx(-1, -1, L"%H%s%N: Too many parameters\r\n", ProgName);
        goto Error_exit;
    }
    if (ParamCount < ManParmCount + 1) {
        ShellPrintEx(-1, -1, L"%H%s%N: Too few parameters\r\n", ProgName);
        goto Error_exit;
    }
    i = 0;
    for (i=0; i<ParamCount-1; i++) {
        CONST CHAR16 *ValueStr;
        if (ParamTable[i].ValueRetPtr.pVoid == NULL) {
            TableError(i, L"Parameter: Null 'RetValPtr'");
            goto Error_exit;
        }
        ValueStr = ShellCommandLineGetRawValue(Package, i+1);
        if (!ReturnValue(ValueStr, ParamTable[i].ValueType, &ParamTable[i].Data, ParamTable[i].ValueRetPtr)) {
            switch (ParamTable[i].ValueType) {
            case VALTYPE_STRING:
                ShellPrintEx(-1, -1, L"%H%s%N: Parameter %d is not a valid string - '%H%s%N'\r\n", ProgName, i+1, ValueStr);
                break;                
            case VALTYPE_DECIMAL:
                ShellPrintEx(-1, -1, L"%H%s%N: Parameter %d is not a valid decimal value - '%H%s%N'\r\n", ProgName, i+1, ValueStr);
                break;                
            case VALTYPE_HEXIDECIMAL:
                ShellPrintEx(-1, -1, L"%H%s%N: Parameter %d is not a valid hex value - '%H%s%N'\r\n", ProgName, i+1, ValueStr);
                break;
            case VALTYPE_INTEGER:
                ShellPrintEx(-1, -1, L"%H%s%N: Parameter %d is not a valid integer value - '%H%s%N'\r\n", ProgName, i+1, ValueStr);
                break;
            case VALTYPE_ENUM:
                ShellPrintEx(-1, -1, L"%H%s%N: Parameter %d is not a valid option - '%H%s%N'\r\n", ProgName, i+1, ValueStr);
                break;
            default:
                TableError(i, L"Parameter: Invalid 'ValueType'");
                goto Error_exit;
            }
            goto Error_exit;
        }
    }

    //------------
    // SWITCHES 
    //------------

    // check for duplicate switches
    Status = ShellCommandLineCheckDuplicate(Package, &ProblemParam);    
    if (EFI_ERROR(Status)) {
        ShellPrintEx(-1, -1, L"%H%s%N: Duplicate switch - '%H%s%N'\r\n", ProgName, ProblemParam);
        FreePool(ProblemParam);
        goto Error_exit;
    }

    // process switches
    i = 0;
    while (SwTable[i].SwitchNecessity != NO_SW) {
        BOOLEAN found = FALSE;
        CHAR16 *SwStr = NULL;
        if (SwTable[i].SwStr1) {
            if (ShellCommandLineGetFlag(Package, SwTable[i].SwStr1)) {
                found = TRUE;
                SwStr = SwTable[i].SwStr1;
            }
        }
        if (SwTable[i].SwStr2) {
            if (ShellCommandLineGetFlag(Package, SwTable[i].SwStr2)) {
                if (!found) {
                    found = TRUE;
                    SwStr = SwTable[i].SwStr2;
                } else {
                    ShellPrintEx(-1, -1, L"%H%s%N: Duplicate switch - '%H%s%N'\r\n", ProgName, SwStr);
                    goto Error_exit;
                }
            }
        }
        if (found) {
            CONST CHAR16 *SwString;
            SwPresent[i] = TRUE;
            SwString = ShellCommandLineGetValue(Package, SwStr);
            if (!SwString && SwTable[i].ValueNecessity == MAN_VALUE) {
                ShellPrintEx(-1, -1, L"%H%s%N: Switch '%H%s%N' requires a value\r\n", ProgName, SwStr);
                goto Error_exit;
            }
            if (SwTable[i].ValueRetPtr.pVoid == NULL) {
                TableError(i, L"Switch: Null 'RetValPtr'");
                goto Error_exit;
            }
            if (SwTable[i].ValueType == VALTYPE_NONE) {
                if (SwTable[i].Data.FlagValue) {
                    // flag with value
                    *(SwTable[i].ValueRetPtr.pUintn) = SwTable[i].Data.FlagValue;
                } else {
                    // true/false flag 
                    *(SwTable[i].ValueRetPtr.pBoolean) = TRUE;
                }
            } else {
                if (!ReturnValue(SwString, SwTable[i].ValueType, &SwTable[i].Data, SwTable[i].ValueRetPtr)) {
                    switch (SwTable[i].ValueType) {
                    case VALTYPE_STRING:
                        ShellPrintEx(-1, -1, L"%H%s%N: Switch '%H%s%N' has invalid string value - '%H%s%N'\r\n", ProgName, SwStr, SwString);
                        break;                
                    case VALTYPE_DECIMAL:
                        ShellPrintEx(-1, -1, L"%H%s%N: Switch '%H%s%N' has invalid decimal value - '%H%s%N'\r\n", ProgName, SwStr, SwString);
                        break;                
                    case VALTYPE_HEXIDECIMAL:
                        ShellPrintEx(-1, -1, L"%H%s%N: Switch '%H%s%N' has invalid hex value - '%H%s%N'\r\n", ProgName, SwStr, SwString);
                        break;
                    case VALTYPE_INTEGER:
                        ShellPrintEx(-1, -1, L"%H%s%N: Switch '%H%s%N' has invalid integer value - '%H%s%N'\r\n", ProgName, SwStr, SwString);
                        break;
                    case VALTYPE_ENUM:
                            ShellPrintEx(-1, -1, L"%H%s%N: Switch '%H%s%N' has invalid option - '%H%s%N'\r\n", ProgName, SwStr, SwString);
                        break;
                    default:
                        TableError(i, L"Switch: Invalid 'ValueType'");
                        goto Error_exit;
                    }
                    goto Error_exit;
                }
            }
        }
        i++;
        if (i >= MAX_SWITCH_ENTRIES) {
            TableError(i, L"Exceeded maximum switch count");
            return SHELL_OUT_OF_RESOURCES;
        }
    }
    
    // check mandatory switches
    i = 0;
    while (SwTable[i].SwitchNecessity != NO_SW) {
        if (SwTable[i].SwitchNecessity == MAN_SW && !SwPresent[i]) {
            ShellPrintEx(-1, -1, L"%H%s%N: Missing switch - '%H%s%N'\r\n", ProgName, SwTable[i].SwStr1);
            goto Error_exit;
        }
        i++;
    }
    ShellStatus = SHELL_SUCCESS;
    
Error_exit:
    if (Package) {
        ShellCommandLineFreeVarList(Package);
        Package = NULL;
    }
    if (ParamList) {
        FreePool(ParamList);
        ParamList = NULL;
    }

    return ShellStatus;
}

/**
 * Function: ReturnValue
 * 
 **/
STATIC BOOLEAN ReturnValue(IN CONST CHAR16 *String, IN VALUE_TYPE ValueType, IN DATA *Data, OUT VALUE_RET_PTR ValueRetPtr)
{
    UINTN Value;
    
    if (!ValueRetPtr.pVoid) {
        return FALSE;
    }

    switch (ValueType) {
    case VALTYPE_STRING:
        StrnCpyS(ValueRetPtr.pChar16, Data->MaxStrSize, String, Data->MaxStrSize-1);
        break;
    case VALTYPE_DECIMAL:
        if (!IsDecimalString(String)) {
            return FALSE;
        }
        Value = StrDecimalToUintn(String);
        *ValueRetPtr.pUintn = Value;
        break;                
    case VALTYPE_HEXIDECIMAL:
        if (!IsHexString(String)) {
            return FALSE;
        }
        Value = StrHexToUintn(String);
        *ValueRetPtr.pUintn = Value;
        break;
    case VALTYPE_INTEGER:
        if (IsDecimalString(String)) {
            Value = StrDecimalToUintn(String);
            *ValueRetPtr.pUintn = Value;
        } else if (HasHexPrefix(String)) {
            if (!IsHexString(String)) {
                return FALSE;
            }
            Value = StrHexToUintn(String);
            *ValueRetPtr.pUintn = Value;
        } else {
            return FALSE;
        }
        break;
    case VALTYPE_ENUM:
        if(GetEnumVal(Data->EnumStrArray, String, &Value)) {
            *ValueRetPtr.pEnum = (unsigned int)Value;
        } else {
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    
    return TRUE;
}

/**
 * Function: GetEnumVal
 * 
 **/
STATIC BOOLEAN GetEnumVal(IN ENUM_STR_ARRAY *EnumStrArray, IN CONST CHAR16 *Str, OUT UINTN *Value)
{
    BOOLEAN found = FALSE;
    UINTN i = 0;
    while (EnumStrArray[i].Str) {
        if (StriCmp(Str, EnumStrArray[i].Str) == 0) {
            if (Value) {
                *Value = EnumStrArray[i].Value;
            }
            found = TRUE;
            break;
        }
        i++;
    }
    return found;
}

/**
 * Function: StriCmp
 * 
 **/
STATIC INTN EFIAPI StriCmp(IN CONST CHAR16 *FirstString, IN CONST CHAR16 *SecondString)
{
  CHAR16  UpperFirstString;
  CHAR16  UpperSecondString;

  UpperFirstString  = CharToUpper(*FirstString);
  UpperSecondString = CharToUpper(*SecondString);
  while ((*FirstString != L'\0') && (*SecondString != L'\0') && (UpperFirstString == UpperSecondString)) {
    FirstString++;
    SecondString++;
    UpperFirstString  = CharToUpper(*FirstString);
    UpperSecondString = CharToUpper(*SecondString);
  }

  return UpperFirstString - UpperSecondString;
}

/**
 * Function: HasHexPrefix
 * 
 **/
STATIC BOOLEAN HasHexPrefix(IN CONST CHAR16 *String)
{
    BOOLEAN LeadingZero = FALSE;
    
    while ((*String == L' ') || (*String == L'\t')) {
        String++;
    }
    while (*String == L'0') {
        LeadingZero = TRUE;
        String++;
    }
    if (CharToUpper(*String) == L'X') {
        if (!LeadingZero) {
            return FALSE;
        }    
    } else {
        return FALSE;
    }
    return TRUE; 
}

/**
 * Function: IsHexString
 * 
 **/
STATIC BOOLEAN IsHexString(IN CONST CHAR16 *String)
{
    BOOLEAN LeadingZero = FALSE;
    
    while ((*String == L' ') || (*String == L'\t')) {
        String++;
    }
    while (*String == L'0') {
        LeadingZero = TRUE;
        String++;
    }
    if (CharToUpper(*String) == L'X') {
        if (!LeadingZero) {
            return FALSE;
        }    
        String++; // Skip the 'X'
    }
    while (InternalIsHexaDecimalDigitCharacter(*String)) {    
        String++;
    }
    return *String == L'\0' ? TRUE : FALSE;
}

/**
 * Function: IsDecimalString
 * 
 **/
STATIC BOOLEAN IsDecimalString(IN CONST CHAR16 *String)
{
    while ((*String == L' ') || (*String == L'\t')) {
        String++;
    }
    while (InternalIsDecimalDigitCharacter(*String)) {
        String++;
    }
    return *String == L'\0' ? TRUE : FALSE;
}

/**
 * TableError()
 * 
 **/
STATIC VOID TableError(IN UINTN i, IN CHAR16 *errStr)
{
    Print(L"TBLERR(%d): %s\n", i, errStr);
}

/**
 * ArgNameDefined()
 * 
 **/
STATIC BOOLEAN ArgNameDefined(IN CHAR16 *HelpStr)
{
    return HelpStr[0] == L'[' ? TRUE:FALSE;
}

/**
 * GetArgName()
 * 
 **/
STATIC UINTN GetArgName(IN CHAR16 *HelpStr, OUT CHAR16* ArgName, IN UINTN ArgNameSize, IN BOOLEAN Mandatory, IN CONST CHAR16 *DefaultArgName)
{
    UINTN HelpStartIdx = 0;             // start of help text
    CONST CHAR16 *ArgPtr = NULL;
    UINTN ArgLen = 0;

    if (ArgNameDefined(HelpStr)) {
        UINTN i = 1;  // skip start brace
        while (TRUE) {
            CHAR16 c = HelpStr[i];
            if (c == L']') {
                // valid argument name found
                ArgPtr = &HelpStr[1];   // char after '['
                ArgLen = i-1;
                break;
            }
            if (c == L'\0') {
                // no end brace for arg name
                break;
            }
            i++;
        }
        HelpStartIdx = i+1;
    }
    if (!ArgPtr && DefaultArgName) {
        // no argument name specified so use default
        ArgPtr = DefaultArgName;
        ArgLen = StrLen(DefaultArgName);
    }
    if (ArgPtr) {
        if (Mandatory) {
            StrnCpyS(ArgName, ArgNameSize, ArgPtr, (ArgLen >= ArgNameSize) ? ArgNameSize-1 : ArgLen);
        } else {
            // optional argument
            ArgName[0] = L'[';
            ArgName[1] = L'\0';
            StrnCatS(ArgName, ArgNameSize-1, ArgPtr, (ArgLen >= ArgNameSize-3) ? ArgNameSize-3 : ArgLen);
            StrCatS(ArgName, ArgNameSize, L"]");
        }
    } else {
        // no argument name specified or default given
        ArgName[0] = L'\0';
    }

    return HelpStartIdx;
}
/**
 * Function: ShowHelp
 * 
 **/
STATIC VOID ShowHelp(IN CONST CHAR16 *ProgName, IN UINTN ManParmCount, IN PARAMETER_TABLE *ParamTable, IN SWITCH_TABLE *SwTable, IN CONST CHAR16 *ProgHelpStr)
{
    const UINTN ArgNameSize = 24;
    CHAR16 ArgName[ArgNameSize];
    UINTN HelpIdx;

    // initialise padding string
    const UINTN PadSize = 20;
    CHAR16 pad[PadSize];
    for (UINTN i=0; i<PadSize; i++) {
        pad[i] = L' ';
    }
    pad[PadSize-1] = L'\0';

    // program description
    Print(L"\n");
    
    if (ProgHelpStr) {
        Print(L"%s\n\n", ProgHelpStr);
    }

    // usage
    Print(L"Usage: %s", ProgName);
    UINTN i = 0;
    while (ParamTable[i].ValueType != VALTYPE_NONE) {
        GetArgName(ParamTable[i].HelpStr, ArgName, ArgNameSize, (i+1 <= ManParmCount), DefaultArgName);
        Print(L" %s", ArgName);
        i++;
    }
    Print(L" [options]\n");

    // Parameter help
    Print(L"\n Parameters:\n");
    i = 0;
    while (ParamTable[i].ValueType != VALTYPE_NONE) {
        HelpIdx = GetArgName(ParamTable[i].HelpStr, ArgName, ArgNameSize, (i+1 <= ManParmCount), DefaultArgName);
        // get rid of spaces below ###
        Print(L"  %s%s     %s\n", ArgName, &pad[StrLen(ArgName)], &ParamTable[i].HelpStr[HelpIdx]);
        i++;
    }
    // Switch help
    Print(L"\n Options:\n");
    i = 0;
    while (SwTable[i].SwitchNecessity != NO_SW) {
        HelpIdx = GetArgName(SwTable[i].HelpStr, ArgName, ArgNameSize, TRUE, (SwTable[i].ValueType == VALTYPE_NONE) ? NULL : DefaultArgName);
        CHAR16 SeperatorChar = L',';
        CHAR16 *SwStr1 = SwTable[i].SwStr1;
        CHAR16 *SwStr2 = SwTable[i].SwStr2;
        if (!SwStr1) {
            SwStr1 = &pad[(PadSize-1)-2]; // 2 spaces for short switch
            SeperatorChar = L' ';
        }
        if (!SwStr2) {
            SwStr2 = &pad[PadSize-1]; // null str
            SeperatorChar = L' ';
        }
        UINTN TotalLen = StrLen(SwStr2) + StrLen(ArgName);
        CHAR16 *PadStr = (TotalLen > PadSize-1) ? &pad[(PadSize-1)-1] : &pad[TotalLen];
        Print(L"  %s%c %s %s%s%s", SwStr1, SeperatorChar, SwStr2, ArgName, PadStr, &SwTable[i].HelpStr[HelpIdx]);
        if (SwTable[i].ValueType == VALTYPE_ENUM) {
            // print all valid options for enum switches
            Print(L" (");
            UINTN j = 0;
            while (SwTable[i].Data.EnumStrArray[j].Str) {
                Print(L"%s", SwTable[i].Data.EnumStrArray[j].Str);
                j++;
                if (SwTable[i].Data.EnumStrArray[j].Str) {
                    Print(L"|");
                }
            }
            Print(L")");
        }
        Print(L"\n");
        i++;
    }

    // break switch
    Print(L"  %s, %s %s%s\n", BreakSwStr1, BreakSwStr2, &pad[StrLen(BreakSwStr2)], BreakSwStr);
    // help switch
    Print(L"  %s, %s %s%s\n\n", HelpSwStr1, HelpSwStr2, &pad[StrLen(HelpSwStr2)], HelpSwStr);
}
