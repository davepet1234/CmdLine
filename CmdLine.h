/***********************************************************************

 CmdLine.h
 
 Author: David Petrovic
 GitHub: https://github.com/davepet1234/CmdLine

***********************************************************************/

#ifndef CMD_LINE_H
#define CMD_LINE_H

#include <Uefi.h>
#include <Library/ShellLib.h>
#include "CmdLineInternal.h"

//-------------------------------------
// Parameter Table Macros
//-------------------------------------

/**
  PARAMTABLE_START - Begins the parameter table

  ArrayName     Defines name of parameter table
**/
#define PARAMTABLE_START(ArrayName) \
    PARAMETER_TABLE ArrayName[] = {

/**
  PARAMTABLE_STR - Adds string parameter to table

  ValueRetPtr   Ptr to CHAR16 string to hold value entered
  StrSize       Size of above string
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_STR(ValueRetPtr, StrSize, HelpStr) \
    {VALTYPE_STRING, {.MaxStrSize=StrSize}, {.pChar16=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_DEC - Adds decimal parameter to table

  ValueRetPtr   Ptr to UINTN to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_DEC(ValueRetPtr, HelpStr) \
    {VALTYPE_DECIMAL, {0}, {.pUintn=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_HEX - Adds hexidecimal parameter to table

  ValueRetPtr   Ptr to UINTN to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_HEX(ValueRetPtr, HelpStr) \
    {VALTYPE_HEXIDECIMAL, {0}, {.pUintn=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_INT - Adds integer parameter (decimal or hex) to table

  ValueRetPtr   Ptr to UINTN to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_INT(ValueRetPtr, HelpStr) \
    {VALTYPE_INTEGER, {0}, {.pUintn=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_ENUM - Adds enum parameter to table (string entry)

  ValueRetPtr   Ptr to enum to hold value entered
  EnumArray     Ptr to array defining enum value to string
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define PARAMTABLE_ENUM(ValueRetPtr, EnumArray, HelpStr) \
    {VALTYPE_ENUM, EnumArray, {.pEnum=ValueRetPtr}, HelpStr},

/**
  PARAMTABLE_END - Ends the parameter table
**/
#define PARAMTABLE_END \
    {VALTYPE_NONE,{0},{0},NULL}};



//-------------------------------------
// Switch Table Macros
//-------------------------------------

/**
  SWTABLE_START - Begins the switch table

  ArrayName     Defines name of switch table
**/
#define SWTABLE_START(ArrayName) \
    SWITCH_TABLE ArrayName[] = {

/**
  SWTABLE_OPT_FLAG - Adds a optional switch with no value (true/false) to table 

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to BOOLEAN, set to TRUE if switch present
  HelpStr       Ptr to CHAR16 help string for switch
**/
#define SWTABLE_OPT_FLAG(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    {SwStr1, SwStr2, OPT_SW, VALTYPE_NONE, NO_VALUE, {0}, {.pBoolean=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_FLGVAL - Adds an optional switch with no value (has default value) to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to UINTN, set to 'Value' if switch present
  Value         Value to set if switch present
  HelpStr       Ptr to CHAR16 help string for switch
**/
#define SWTABLE_OPT_FLGVAL(SwStr1, SwStr2, ValueRetPtr, Value, HelpStr) \
    {SwStr1, SwStr2, OPT_SW, VALTYPE_NONE, NO_VALUE, {.FlagValue=Value}, {.pUintn=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_STR - Adds an optional string switch to table
  SWTABLE_MAN_STR - Adds a mandatory string switch to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to CHAR16 string to hold value entered
  StrSize       Size of above string
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_STR(SwStr1, SwStr2, ValueRetPtr, StrSize, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_STRING, MAN_VALUE, {.MaxStrSize=StrSize}, {.pChar16=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_STR(SwStr1, SwStr2, ValueRetPtr, StrSize, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_STRING, MAN_VALUE, {.MaxStrSize=StrSize}, {.pChar16=ValueRetPtr}, HelpStr},


/**
  SWTABLE_OPT_DEC - Adds an optional decimal switch to table
  SWTABLE_MAN_DEC - Adds a mandatory decimal switch to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to UINTN to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_DEC(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_DECIMAL, MAN_VALUE, {0}, {.pUintn=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_DEC(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_DECIMAL, MAN_VALUE, {0}, {.pUintn=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_HEC - Adds an optional hexidecimal switch to table
  SWTABLE_MAN_HEC - Adds a mandatory hexidecimal switch to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to UINTN to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_HEX(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_HEXIDECIMAL, MAN_VALUE, {0}, {.pUintn=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_HEX(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_HEXIDECIMAL, MAN_VALUE, {0}, {.pUintn=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_INT - Adds an optional integer (decimal or hex) switch to table
  SWTABLE_MAN_INT - Adds a mandatory integer (decimal or hex) switch to table

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to UINTN to hold value entered
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_INT(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_INTEGER, MAN_VALUE, {0}, {.pUintn=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_INT(SwStr1, SwStr2, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_INTEGER, MAN_VALUE, {0}, {.pUintn=ValueRetPtr}, HelpStr},

/**
  SWTABLE_OPT_ENUM - Adds an optional enum switch to table (string entry)
  SWTABLE_MAN_ENUM - Adds a mandatory enum switch to table (string entry)

  SwStr1        Ptr to CHAR16 defining short switch name
  SwStr2        Ptr to CHAR16 defining long switch name
  ValueRetPtr   Ptr to enum to hold value entered
  EnumArray     Ptr to array defining enum value to string
  HelpStr       Ptr to CHAR16 help string for parameter
**/
#define SWTABLE_OPT_ENUM(SwStr1, SwStr2, ValueRetPtr, EnumArray, HelpStr) \
    { SwStr1, SwStr2, OPT_SW, VALTYPE_ENUM, MAN_VALUE, {.EnumStrArray=EnumArray}, {.pEnum=ValueRetPtr}, HelpStr},
#define SWTABLE_MAN_ENUM(SwStr1, SwStr2, EnumArray, ValueRetPtr, HelpStr) \
    { SwStr1, SwStr2, MAN_SW, VALTYPE_ENUM, MAN_VALUE, {.EnumStrArray=EnumArray}, {.pEnum=ValueRetPtr}, HelpStr},

/**
  SWTABLE_END -Ends the switch table
**/
#define SWTABLE_END \
    {NULL,NULL,NO_SW,VALTYPE_NONE,FALSE,{0},{0},NULL}};

//-------------------------------------
// Enum to String Table Macros
//-------------------------------------

/**
  ENUMSTR_START - Begins the Enum to String array

  ArrayName     Defines name of Enum to String array
**/
#define ENUMSTR_START(ArrayName) \
    ENUM_STR_ARRAY ArrayName[] = {

/**
  ENUMSTR_ENTRY - Adds a mapping to the Enum to String array

  Value         Enum value
  Str           Associated string
**/
#define ENUMSTR_ENTRY(Value, Str) \
    {Value, Str},

/**
  ENUMSTR_END - Ends the Enum to String array
**/
#define ENUMSTR_END \
    {0,NULL}};

//-------------------------------------
// Defines
//-------------------------------------

// Functional options
#define NO_HELP         0x0001
#define FORCE_BREAK     0x0002

//-------------------------------------
// Functions
//-------------------------------------

/**
  ParseCmdLine - Parses the command line
  
  ProgName      Name of shell app
  ManParmCount  Number of manatory parameters required
  ParamTable    Ptr to PARAMETER_TABLE defining the expected parameters
                If no parameters required set this to NULL
  SwTable       Ptr to SWITCH_TABLE defining the expected switches
                If no switches required set this to NULL
  ProgHelpStr   Ptr to help string for program
  FuncOpt       Functional options (bit values to be ORed)
                    NO_HELP         no command line help
                    FORCE_BREAK     force the line break option
  NumParams     Ptr to return the number of parameter entered (optional)
  
  Returns       SHELL_SUCCESS if all parameters/switches are valid
                SHELL_INVALID_PARAMETER if problem encountered with parameter/switches passed on cmd line
                SHELL_OUT_OF_RESOURCES if internal memory error
                SHELL_ABORTED if help displayed
**/
extern SHELL_STATUS ParseCmdLine(IN CONST CHAR16 *ProgName, IN UINTN ManParmCount, IN PARAMETER_TABLE *ParamTable, IN SWITCH_TABLE *SwTable, IN CHAR16 *ProgHelpStr, IN UINT16 FuncOpt, OUT UINTN *NumParams);


#endif // CMD_LINE_H
