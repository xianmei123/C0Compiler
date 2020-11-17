#ifndef TYPE_H
#define TYPE_H
#include <string>
using namespace std;
enum ItemType
{
    CON,
    VAR,
    FUNC,
    PARA
};

enum ValueType
{
    INT,
    CHAR,
    String,
    ARRAY
};

enum FuncType
{
    VOID,
    RETURNINT,
    RETURNCHAR
};

enum SymbolType
{
    CONSTTK,
    INTTK,
    CHARTK,
    VOIDTK,
    MAINTK,
    IFTK,
    ELSETK,
    SWITCHTK,
    CASETK,
    DEFAULTTK,
    WHILETK,
    FORTK,
    SCANFTK,
    PRINTFTK,
    RETURNTK,

    IDENFR,
    INTCON,
    CHARCON,
    STRCON,

    PLUS,
    MINU,
    MULT,
    DIV,
    LSS,
    LEQ,
    GRE,
    GEQ,
    EQL,
    NEQ,
    COLON,
    ASSIGN,
    SEMICN,
    COMMA,
    LPARENT,
    RPARENT,
    LBRACK,
    RBRACK,
    LBRACE,
    RBRACE,

    FINISH,
    FACTOROP
};

enum MidCodeType
{
    VarDef,
    ValueParamDeliver, // 值参传递
    FunctionCall,
    AssignArray,
    AssignByArray,
    AssignState,
    Label,
    FunctionDef,
    ParamDef,
    Goto, // 无条件跳转
    BZ,  // 等于
    BNZ,  // 不等于
    LSSm,  // 小于
    LEQm, // 小于等于
    GREm,  // 大于
    GEQm, // 大于等于
    EQLm,
    NEQm,
    ReadChar,
    ReadInt,
    PrintStr,
    PrintBeforeExpr,
    PrintChar,
    PrintInt,
    PrintIdent,
    ReturnInt,
    ReturnChar,
    ReturnEmpty,
    ReturnIdent
};

struct FourYuanItem {
    MidCodeType codeType;
    ValueType valueType;
    FuncType funcType;
    string target;
    char targetCh;
    string left;
    int leftValue;
    string right;
    int rightValue;
    int targetValue;
    char op;
    bool isArray;
    int x;
    int y;
    int order;
    string arrayName;
    int dim;
};

struct PostfixItem {
	ValueType valueType;
	string str;					  
	int number;					  
	bool isCharVar;				  
	bool isOperator;			
};

#endif