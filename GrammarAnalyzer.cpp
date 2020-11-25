#include "LexicalAnalyzer.h"
#include "GrammarAnalyzer.h"
#include "SymbolTable.h"
#include "codeGenerator.h"
#include <iostream>
#include <string.h>
#include <map>
#include <iomanip>
#include <algorithm>
#include <string>
#include <fstream>
#include <vector>
string type2String(SymbolType symbolType);
using namespace std;
string cuFuncName = "global";
FuncType cuFuncType = VOID;
int errorFun = 0;
map<string, SymbolItem> funcTable;
map<string, SymbolItem> globalTable;
map<string, vector<SymbolItem>> paras;
SymbolItem *cuSi;
int lineNum = 1;
int OUT = 1;
int haveReturn = 0;
string buffer[4];
string buffer1 = "";
int buffIndex = 0;
ofstream outFile;

string expName;
int tmpCount = 0;
int order = 0;

vector<FourYuanItem> codeItems;

GrammarAnalyzer::GrammarAnalyzer(LexicalAnalyzer &aLexicalAnalyzer, Error& aerror):lexicalAnalyzer(aLexicalAnalyzer),error(aerror) {
}

void GrammarAnalyzer::toFile() {
    outFile << buffer1 << endl;
}

void GrammarAnalyzer::toFile(string str) {
    outFile << str << endl;
}

//＜程序＞ ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
void GrammarAnalyzer::program() {
    outFile.open("output.txt", ios::out);
    GETSYM
    if (SYMTYPE == CONSTTK) {
        constDeclaration();
    }
    if (SYMTYPE == INTTK || SYMTYPE == CHARTK) {
        varDeclaration();
    }
    FourYuanItem fourItem;
    fourItem.codeType = FunctionCall;
    fourItem.target = "main";
    codeItems.push_back(fourItem);
    while (SYMTYPE == INTTK || SYMTYPE == CHARTK || SYMTYPE == VOIDTK) {
        if (SYMTYPE == INTTK || SYMTYPE == CHARTK) {
            returnFun();
        }
        else if (SYMTYPE == VOIDTK) {
            nonReturnFun();
            if (lexicalAnalyzer.index == lexicalAnalyzer.oldIndex) {
                break;
            }
        }
    }
    mainFunc();
    toFile("<程序>");
    FourYuanItem fy;
    fy.codeType = Finish;
    codeItems.push_back(fy);
    printMidCodeToFile(codeItems);
    turnToMips(codeItems);
}

//＜主函数＞ ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’       
void GrammarAnalyzer::mainFunc() {
    updateField("main", VOID);
    FourYuanItem fourItem;
    fourItem.codeType = FunctionDef;
    fourItem.funcType = VOID;
    fourItem.target = "main";
    codeItems.push_back(fourItem);
    GETSYM
    GETSYM
    
    GETSYM
    if (SYMTYPE != RPARENT) {
        error.errorLog(withoutRparent, lineNum);
    }
    else {
        GETSYM
    }
    GETSYM
    compoundStat();
    GETSYM
    toFile("<主函数>");
}

//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
void GrammarAnalyzer::constDeclaration() {
    while (SYMTYPE == CONSTTK) {
        GETSYM
        
        constDefine();
        if (SYMTYPE != SEMICN) {
            error.errorLog(withoutSemicn, lineNum);
            continue;
        }
        GETSYM
    }
    toFile("<常量说明>");
    return;
}

//＜常量定义＞  ::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞} | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
void GrammarAnalyzer::constDefine() {
    string name;
    if (SYMTYPE == INTTK) {
        int number = 0;
        string array = "";
        GETSYM //标识符
        name = *lexicalAnalyzer.token.word;
        GETSYM //=
        GETSYM //数字
        
        if (SYMTYPE == INTCON || SYMTYPE == PLUS || SYMTYPE == MINU) {
            number = integerCon();
        }
        
        else if (SYMTYPE == CHARCON) {
            //error.errorLog(typeMatchError, LINE);
            GETSYM
        }
        addItem(name, CON, INT, number);
        while (SYMTYPE == COMMA) {
            GETSYM //标识符
            name = *lexicalAnalyzer.token.word;
            GETSYM //=
            GETSYM //数字
            
            if (SYMTYPE == INTCON || SYMTYPE == PLUS || SYMTYPE == MINU) {
                number = integerCon();
            }
        
            else if (SYMTYPE == CHARCON) {
                //error.errorLog(typeMatchError, LINE);
                GETSYM
            }
            addItem(name, CON, INT, number);
        }
    }
    else if (SYMTYPE == CHARTK) {
        string value = "";
        GETSYM //标识符
        name = *lexicalAnalyzer.token.word;
        GETSYM //=
        GETSYM //字符
        
        if (SYMTYPE == CHARCON) {
            value = *lexicalAnalyzer.token.word;
        }
        else if (SYMTYPE == INTCON) {
            //error.errorLog(typeMatchError, LINE);
        }
        addItem(name, CON, CHAR, value);
        GETSYM
        while (SYMTYPE == COMMA) {
            GETSYM //标识符
            name = *lexicalAnalyzer.token.word;
            GETSYM //=
            GETSYM //字符
            
            if (SYMTYPE == CHARCON) {
                value = *lexicalAnalyzer.token.word;
            }
            else if (SYMTYPE == INTCON) {
                //error.errorLog(typeMatchError, LINE);
            }
            addItem(name, CON, CHAR, value);
            GETSYM
        }
    }
    toFile("<常量定义>");
    return;
}

//＜整数＞ ::= ［＋｜－］＜无符号整数＞
int GrammarAnalyzer::integerCon() {
    int number = 0;
    if (SYMTYPE == PLUS) {
        GETSYM //数字
        number = integerUnsigned();
    }
    else if (SYMTYPE == MINU) {
        GETSYM //数字
        number = -integerUnsigned();
    }
    else if (SYMTYPE == INTCON) {
        number = integerUnsigned();
    }
    toFile("<整数>");
    return number;
}

//＜无符号整数＞  ::= ＜数字＞｛＜数字＞｝
int GrammarAnalyzer::integerUnsigned() {
    int number = lexicalAnalyzer.token.number;
    GETSYM
    toFile("<无符号整数>");
    return number;
}
//＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
void GrammarAnalyzer::varDeclaration() {
    int runNum = 0;
    while (SYMTYPE == INTTK || SYMTYPE == CHARTK) {
        SymbolType sym1 = SYMTYPE;
        SAVE
        buffer[buffIndex] = buffer[buffIndex].append(type2String(SYMTYPE));
        buffer[buffIndex] = buffer[buffIndex].append(" ");
        buffer[buffIndex] = buffer[buffIndex].append(*lexicalAnalyzer.token.word);
        buffIndex++;
        OUT = 0;
        if (!varDefine()) {
            BACKTRACK
            SYMTYPE = sym1;
            OUT = 1;
            if (runNum > 0) {
                toFile("<变量说明>");
            }
            return;
        }
        if (SYMTYPE != SEMICN) {
            error.errorLog(withoutSemicn, lineNum);
        }
        else {
            GETSYM
        }
        runNum++;
    }
    toFile("<变量说明>");
}
//＜变量定义＞ ::= ＜变量定义无初始化＞|＜变量定义及初始化＞
bool GrammarAnalyzer::varDefine() {
    ValueType valueType = CHAR;
    if (SYMTYPE == CHARTK) {
        valueType = CHAR;
    }
    else if (SYMTYPE == INTTK) {
        valueType = INT;
    }
    int flag = 0;
    if (!varDefineNonInit(valueType, flag)) {
        return false;
    }
    toFile("<变量定义>");
    return true;
}
//＜变量定义无初始化＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']'|＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']'){,(＜标识符＞|＜标识符＞'['＜无符号整数＞']'|＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']' )}
bool GrammarAnalyzer::varDefineNonInit(ValueType valueType, int flag) {
    string name;
    GETSYM //标识符
    buffer[buffIndex] = buffer[buffIndex].append(type2String(SYMTYPE));
    buffer[buffIndex] = buffer[buffIndex].append(" ");
    buffer[buffIndex] = buffer[buffIndex].append(*lexicalAnalyzer.token.word);
    buffIndex++;
    name = *lexicalAnalyzer.token.word;
    
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    GETSYM //[|=|,|;
    buffer[buffIndex] = buffer[buffIndex].append(type2String(SYMTYPE));
    buffer[buffIndex] = buffer[buffIndex].append(" ");
    buffer[buffIndex] = buffer[buffIndex].append(*lexicalAnalyzer.token.word);
    buffIndex++;
    if (SYMTYPE != LBRACK && SYMTYPE != ASSIGN 
        && SYMTYPE != SEMICN && SYMTYPE != COMMA) {
        
        if (flag != 0) {
            addItem(name, VAR, valueType, 1, 1, 0);
            toFile("<变量定义无初始化>");
            return true;
        }
        for (int i = 0; i < buffIndex; i++) {
            buffer[i].clear();
        }
        buffIndex = 0;
        if (SYMTYPE == INTTK || SYMTYPE == CHARTK || SYMTYPE == VOIDTK) {
            //error.errorLog(withoutSemicn, lineNum);
            addItem(name, VAR, valueType, 1, 1, 0);
            toFile("<变量定义无初始化>");
            return true;
        }
        return false;
    }
    else {
        if (flag == 0) {
            for (int i = 0; i < buffIndex - 1; i++) {
            outFile << buffer[i].c_str() << endl;
            buffer[i].clear();
            }
            buffer1 = buffer[buffIndex - 1];
            buffer[buffIndex - 1].clear();
            OUT = 1;
        }
        else {
            for (int i = 0; i < buffIndex; i++) {
            buffer[i].clear();
            }
            buffIndex = 0;
        }
        
        
    }
    
    buffIndex = 0;
    int i = 1;
    int j = 1;
    
    if (SYMTYPE == LBRACK) {
        GETSYM
        if (SYMTYPE == CHARCON) {
            error.errorLog(arrayIndexTypeError, LINE);
            GETSYM
        }
        else {
            i = integerUnsigned();
        }
        if (SYMTYPE != RBRACK) {
            error.errorLog(withoutRbrack, LINE);
        }
        else {
            
            GETSYM //[|=|,|;
            
        }
        if (SYMTYPE == LBRACK) {
            GETSYM
            if (SYMTYPE == CHARCON) {
                error.errorLog(arrayIndexTypeError, LINE);
                GETSYM
            }
            else {
                j = integerUnsigned();
            }
            if (SYMTYPE != RBRACK) {
                error.errorLog(withoutRbrack, LINE);
            }
            else {
                
                GETSYM //=|,|;
            }
            if (SYMTYPE == COMMA) {
                addItem(name, VAR, valueType, i, j, 2);
                varDefineNonInit(valueType, 1);
            }
            else if (SYMTYPE == SEMICN) {
                addItem(name, VAR, valueType, i, j, 2);
                toFile("<变量定义无初始化>");
                return true;
            }
            else if (SYMTYPE == ASSIGN) {
                if (isDefined(name)) {
                    error.errorLog(identRedefine, LINE);
                    skipto(SEMICN);
                    return true;
                }
                addItem(name, VAR, valueType, i, j, 2);
                varDefineInit(valueType, 2, name, VAR, i, j);
            }
            else {
                error.errorLog(withoutSemicn, lineNum);
                return true;
            }
        }
        else if (SYMTYPE == COMMA) {
            addItem(name, VAR, valueType, i, j, 1);
            varDefineNonInit(valueType, 1);
        }
        else if (SYMTYPE == SEMICN) {
            addItem(name, VAR, valueType, i, j, 1);
            toFile("<变量定义无初始化>");
            return true;
        }
        else if (SYMTYPE == ASSIGN) {
            if (isDefined(name)) {
                error.errorLog(identRedefine, LINE);
                skipto(SEMICN);
                return true;
            }
            addItem(name, VAR, valueType, i, j, 1);
            varDefineInit(valueType, 1, name, VAR, i, j);
        }
        else {
            error.errorLog(withoutSemicn, lineNum);
            return true;
        }
    }
    else if (SYMTYPE == COMMA) {
        addItem(name, VAR, valueType, i, j, 0);
        varDefineNonInit(valueType, 1);
    }
    else if (SYMTYPE == SEMICN) {
        addItem(name, VAR, valueType, i, j, 0);
        toFile("<变量定义无初始化>");
        return true;
    }
    else if (SYMTYPE == ASSIGN) {
        if (isDefined(name)) {
            error.errorLog(identRedefine, LINE);
            skipto(SEMICN);
            return true;
        }
        addItem(name, VAR, valueType, i, j, 0);
        varDefineInit(valueType, 0, name, VAR, i, j);
    }
    else {
        error.errorLog(withoutSemicn, lineNum);
        return true;
    }
    //toFile("<变量定义无初始化>");
    return true;
}
//＜变量定义及初始化＞  ::= ＜类型标识符＞＜标识符＞=＜常量＞|＜类型标识符＞＜标识符＞'['＜无符号整数＞']'='{'＜常量＞{,＜常量＞}'}'|＜类型标识符＞＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']'='{''{'＜常量＞{,＜常量＞}'}'{, '{'＜常量＞{,＜常量＞}'}'}'}'
void GrammarAnalyzer::varDefineInit(ValueType valueType, int dim, string name, ItemType itemType, int x, int y) {
    int flag = 0;
    int in = 0;
    GETSYM
    
    if (dim == 0) {
        FourYuanItem fy;
        fy.codeType = AssignState;
        
        if (cuFuncName == "global") {
            fy.target = "G" + name;
        }
        else {
            fy.target = "L" + cuFuncName + "_" + name;
        }
        
        
        if (constant(&in) != valueType && flag == 0) {
            flag = 1;
            error.errorLog(typeMatchError, LINE);
        }
        
        (*cuSi).value.ch = in;
        (*cuSi).value.number = in;
        
        fy.right = to_string(0);
        fy.rightValue = 0;
        fy.left = to_string(in);
        fy.valueType = valueType;
        fy.targetValue = in;
        fy.targetCh = in;
        fy.leftValue = in;
        fy.op = '+';
        fy.x = 0;
        fy.y = 0;
        codeItems.push_back(fy);
    }
    else if (dim == 1) {
        FourYuanItem fourItem;
        fourItem.dim = 1;
        fourItem.codeType = AssignArray;
        if (cuFuncName == "global") {
            fourItem.arrayName = "G" + name;
        }
        else {
            fourItem.arrayName = "L" + cuFuncName + "_" + name;
        }
        int count = 1;
        GETSYM
        if (constant(&in) != valueType && flag == 0) {
            flag = 1;
            error.errorLog(typeMatchError, LINE);
        }
        fourItem.x = x;
        fourItem.leftValue = count - 1;
        fourItem.left = to_string(count - 1);
        fourItem.dim = 1;
        fourItem.targetValue = in;
        fourItem.target = to_string(in);
        fourItem.valueType = valueType;
        fourItem.targetCh = in;
        codeItems.push_back(fourItem);
        while (SYMTYPE == COMMA) {
            GETSYM
            if (constant(&in) != valueType && flag == 0) {
                flag = 1;
                error.errorLog(typeMatchError, LINE);
            }
            
            fourItem.leftValue = count;
            fourItem.left = to_string(count);
            fourItem.targetValue = in;
            fourItem.target = to_string(in);
            fourItem.targetCh = in;
            codeItems.push_back(fourItem);
            count++;
        }
        if (count != x && flag == 0) {
            flag = 1;
            error.errorLog(arrayInitNumError, LINE);
        }
        
        GETSYM
    }
    else if (dim == 2) {
        FourYuanItem fourItem;
        fourItem.codeType = AssignArray;
        fourItem.dim = 2;
        int countx = 1;
        int county = 1;
        if (cuFuncName == "global") {
            fourItem.arrayName = "G" + name;
        }
        else {
            fourItem.arrayName = "L" + cuFuncName + "_" + name;
        }
        GETSYM //{
        GETSYM
        if (constant(&in) != valueType && flag == 0) {
            flag = 1;
            error.errorLog(typeMatchError, LINE);
        }
        fourItem.x = x;
        fourItem.y = y;
        fourItem.dim = 2;
        fourItem.leftValue = 0;
        fourItem.left = to_string(0);
        fourItem.rightValue = 0;
        fourItem.right = to_string(0);
        fourItem.targetValue = in;
        fourItem.target = to_string(in);
        fourItem.targetCh = in;
        fourItem.valueType = valueType;
        codeItems.push_back(fourItem);
        while (SYMTYPE == COMMA) {
            GETSYM
            if (constant(&in) != valueType && flag == 0) {
                flag = 1;
                error.errorLog(typeMatchError, LINE);
            }
            
            fourItem.rightValue = county;
            fourItem.right = to_string(county);
            fourItem.targetValue = in;
            fourItem.targetCh = in;
            fourItem.target = to_string(in);
            codeItems.push_back(fourItem);
            county++;
        }
        if (county != y && flag == 0) {
            flag = 1;
            error.errorLog(arrayInitNumError, LINE);
        }
        
        //}
        GETSYM //,|}
        while (SYMTYPE == COMMA) {
            GETSYM //{
            GETSYM
            county = 1;
            if (constant(&in) != valueType && flag == 0) {
                flag = 1;
                error.errorLog(typeMatchError, LINE);
            }
         
            fourItem.rightValue = county - 1;
            fourItem.right = to_string(county - 1);
            fourItem.leftValue = countx;
            fourItem.left = to_string(countx);
            fourItem.targetValue = in;
            fourItem.targetCh = in;
            fourItem.target = to_string(in);
            codeItems.push_back(fourItem);
            while (SYMTYPE == COMMA) {
                GETSYM
                county++;
                if (constant(&in) != valueType && flag == 0) {
                    flag = 1;
                    error.errorLog(typeMatchError, LINE);
                }
                
                fourItem.rightValue = county - 1;
                fourItem.right = to_string(county - 1);
                fourItem.targetValue = in;
                fourItem.targetCh = in;
                fourItem.target = to_string(in);
               
                codeItems.push_back(fourItem);
            }
            if (county != y && flag == 0) {
                flag = 1;
                error.errorLog(arrayInitNumError, LINE);
            }
            GETSYM
            countx++;
        }
        if (countx != x && flag == 0) {
            flag = 1;
            error.errorLog(arrayInitNumError, LINE);
        }
        
        GETSYM
        
    }
    toFile("<变量定义及初始化>");
}
//＜常量＞   ::=  ＜整数＞|＜字符＞
void GrammarAnalyzer::constant(ValueType valueType) {
    if (SYMTYPE == PLUS || SYMTYPE == MINU || SYMTYPE == INTCON) {
        if (valueType == CHAR) {
            error.errorLog(typeMatchError, LINE);
            GETSYM
        }
        else if (valueType == INT) {
            integerCon();
        }
        
    }
    else {
        if (valueType == INT) {
            error.errorLog(typeMatchError, LINE);
            integerCon();
        }
        else {
            GETSYM
        }
    }
    toFile("<常量>");
}

ValueType GrammarAnalyzer::constant(int *in) {
    if (SYMTYPE == PLUS || SYMTYPE == MINU || SYMTYPE == INTCON) {
        int i = integerCon();
        *in = i;
        toFile("<常量>");
        return INT;
    }
    else if (SYMTYPE == CHARCON) {
        int i = (*lexicalAnalyzer.token.word)[0];
        GETSYM
        
        *in = i;
        toFile("<常量>");
        return CHAR;
    }
    toFile("<常量>");
    return INT;
}

//＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
void GrammarAnalyzer::returnFun() {
    FuncType funcType = RETURNCHAR;
    if (SYMTYPE == INTTK) {
        funcType = RETURNINT;
    }
    else if (SYMTYPE == CHARTK) {
        funcType = RETURNCHAR;
    }
    string funcName = declarationHead();
   
    updateField(funcName, funcType);
    GETSYM
    paraTable();
    if (SYMTYPE != RPARENT) {
        error.errorLog(withoutRparent, lineNum);
    }
    else {
        GETSYM //{
    }
    GETSYM
    compoundStat();
    GETSYM
    toFile("<有返回值函数定义>");
}
//＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}' 
void GrammarAnalyzer::nonReturnFun() {
    SymbolType sym1 = VOIDTK;
    SAVE
    buffer[buffIndex] = buffer[buffIndex].append(type2String(SYMTYPE));
    buffer[buffIndex] = buffer[buffIndex].append(" ");
    buffer[buffIndex] = buffer[buffIndex].append(*lexicalAnalyzer.token.word);
    buffIndex++;
    OUT = 0;
    string funcName;
    GETSYM //标识符
    buffer[buffIndex] = buffer[buffIndex].append(type2String(SYMTYPE));
    buffer[buffIndex] = buffer[buffIndex].append(" ");
    buffer[buffIndex] = buffer[buffIndex].append(*lexicalAnalyzer.token.word);
    buffIndex++;
    if (SYMTYPE == MAINTK) {
        BACKTRACK
        SYMTYPE = sym1;
        OUT = 1;
        for (int i = 0; i < buffIndex; i++) {
            buffer[i].clear();
        }
        buffIndex = 0;
        return;
    }
    else {
        for (int i = 0; i < buffIndex - 1; i++) {
            outFile << buffer[i].c_str() << endl;
            buffer[i].clear();
        }
        buffer1 = buffer[buffIndex - 1];
        buffer[buffIndex - 1].clear();
        buffIndex = 0;
        OUT = 1;
        funcName = *lexicalAnalyzer.token.word;
    }
    transform(funcName.begin(), funcName.end(), funcName.begin(), ::tolower);
    updateField(funcName, VOID);
    FourYuanItem fourItem;
    fourItem.codeType = FunctionDef;
    fourItem.funcType = VOID;
    fourItem.target = funcName;
    codeItems.push_back(fourItem);
    GETSYM //(
    GETSYM

    paraTable();
    if (SYMTYPE != RPARENT) {
        error.errorLog(withoutRparent, lineNum);
    }
    else {
        GETSYM //{
    }
    
    GETSYM
    compoundStat();
    GETSYM
    /*FourYuanItem fy;
    fy.codeType = ReturnEmpty;
    codeItems.push_back(fy);*/
    toFile("<无返回值函数定义>");
}
//＜声明头部＞   ::=  int＜标识符＞ |char＜标识符＞
string GrammarAnalyzer::declarationHead() {
    FourYuanItem fourItem;
    fourItem.codeType = FunctionDef;
    if (SYMTYPE == INTTK) {
        fourItem.funcType = RETURNINT;
        fourItem.valueType = INT;
        GETSYM
    }
    else if (SYMTYPE == CHARTK) {
        fourItem.funcType = RETURNCHAR;
        fourItem.valueType = CHAR;
        GETSYM
    }
    string funcName = *lexicalAnalyzer.token.word;
    transform(funcName.begin(), funcName.end(), funcName.begin(), ::tolower);
    fourItem.target = funcName;
    codeItems.push_back(fourItem);
    GETSYM
    toFile("<声明头部>");
    return funcName;
}
//＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞  
void GrammarAnalyzer::paraTable() {
    int order = 1;
    ValueType valueType = CHAR;
    if (SYMTYPE == LBRACE) {
        //error.errorLog(withoutRparent, lineNum);
        //printf("eeeeeeeeeeeeeeeee\n");
        return;
    }
    if (errorFun == 1 && SYMTYPE == RPARENT) {
        toFile("<参数表>");
        return;
    }
    if (errorFun == 0 && SYMTYPE != CHARTK && SYMTYPE != INTTK) {
        addItem("null", cuFuncName, PARA, valueType, order);
        toFile("<参数表>");
        return;
    }
    else if (SYMTYPE == CHARTK) {
        valueType = CHAR;
    }
    else if (SYMTYPE == INTTK) {
        valueType = INT;
    }
    string name;
    GETSYM //标识符
    name = *lexicalAnalyzer.token.word;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    addItem(name, PARA, valueType);
    FourYuanItem fourItem;
    fourItem.codeType = ParamDef;
    fourItem.valueType = valueType;
    if (cuFuncName == "global") {
        fourItem.target = "G" + name;
    }
    else {
        fourItem.target = "L" + cuFuncName + "_" + name;
    }
    //fourItem.target = name;
    codeItems.push_back(fourItem);
    if (errorFun == 0) {
        addItem(name, cuFuncName, PARA, valueType, order);
    }
    GETSYM //，|}
    while (SYMTYPE == COMMA) {
        order++;
        GETSYM //类型标识符
        if (SYMTYPE == CHARTK) {
            valueType = CHAR;
        }
        else if (SYMTYPE == INTTK) {
            valueType = INT;
        }
        GETSYM //标识符
        name = *lexicalAnalyzer.token.word;
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        addItem(name, PARA, valueType);
        FourYuanItem fourItem;
        fourItem.codeType = ParamDef;
        fourItem.valueType = valueType;
        if (cuFuncName == "global") {
            fourItem.target = "G" + name;
        }
        else {
            fourItem.target = "L" + cuFuncName + "_" + name;
        }
        codeItems.push_back(fourItem);
        if (errorFun == 0) {
            addItem(name, cuFuncName, PARA, valueType, order);
        }
        GETSYM //，|}
    }
    toFile("<参数表>");
}
//＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
void GrammarAnalyzer::compoundStat() {
    if (SYMTYPE == CONSTTK) {
        constDeclaration();
    }
    if (SYMTYPE == INTTK || SYMTYPE == CHARTK) {
        varDeclaration();
    }
    statList(1);
    toFile("<复合语句>");
}
//＜语句列＞   ::= ｛＜语句＞｝
void GrammarAnalyzer::statList(int flag) {
    map<string, SymbolItem>::iterator iter;
    if (flag == 1) {
        haveReturn = 0;
    }
    while (1) {
        if (SYMTYPE == RBRACE) {
            break;
        }
        stat();
    }
    
    if (cuFuncType != VOID && haveReturn == 0 && flag == 1) {
        error.errorLog(returnFuncReturnError, LINE);
    }
    if (cuFuncType == VOID && haveReturn == 0 && flag == 1) {
        FourYuanItem fy;
        fy.codeType = ReturnEmpty;
        codeItems.push_back(fy);
    }
    toFile("<语句列>");
}
//＜语句＞    ::= ＜循环语句＞｜＜条件语句＞| ＜有返回值函数调用语句＞;  |＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜情况语句＞｜＜空＞;|＜返回语句＞; | '{'＜语句列＞'}'  
void GrammarAnalyzer::stat() {
    if (SYMTYPE == SEMICN) {
        GETSYM
    }
    else if (SYMTYPE == WHILETK || SYMTYPE == FORTK) {
        loopStat();

    }
    else if (SYMTYPE == IFTK) {
        conditionalStat();
    }
    else if (SYMTYPE == IDENFR) {
        GETITEM
        ItemType iType = ITEM.getItemType();
        FuncType fType = ITEM.getFuncType();
        ValueType vtl = ITEM.getValueType();
        string name = ITEM.getName();
        string funcName = ITEM.getFuncName();
        if (iType == VAR || iType == CON || iType == PARA) {
            if (iType == CON) {
                error.errorLog(assignConError, LINE);
                skipto(SEMICN);
                GETSYM
                return;
            }
            assignStat(vtl, funcName); 
            if (SYMTYPE != SEMICN) {
                error.errorLog(withoutSemicn, lineNum);
            }
            else {
                GETSYM
            }
        }
        else if (iType == FUNC && (fType == RETURNCHAR || fType == RETURNINT)) {
            funStatReturn(name);
            if (SYMTYPE != SEMICN) {
                error.errorLog(withoutSemicn, lineNum);
            }
            else {
                GETSYM
            }
        }
        else if (iType == FUNC && fType == VOID) {
            funStatNonReturn(name);
            if (SYMTYPE != SEMICN) {
                error.errorLog(withoutSemicn, lineNum);
            }
            else {
                GETSYM
            }
            
        }
       
    }
    else if (SYMTYPE == SCANFTK) {
        scanfStat();
        if (SYMTYPE != SEMICN) {
            error.errorLog(withoutSemicn, lineNum);
        }
        else {
            GETSYM
        }
    }
    else if (SYMTYPE == PRINTFTK) {
        printfStat();
        if (SYMTYPE != SEMICN) {
            error.errorLog(withoutSemicn, lineNum);
        }
        else {
            GETSYM
        }
    }
    else if (SYMTYPE == SWITCHTK) {
        caseStat();
    }
    else if (SYMTYPE == RETURNTK) {
        haveReturn = 1;
        returnStat();
        if (SYMTYPE != SEMICN) {
            error.errorLog(withoutSemicn, lineNum);
        }
        else {
            GETSYM
        }
    }
    else if (SYMTYPE == LBRACE) {
        GETSYM
        statList(0);
        GETSYM
    }
    else {
        GETSYM
        error.errorLog(LINE);
    }
    toFile("<语句>");
}
//＜循环语句＞   ::=  while '('＜条件＞')'＜语句＞| for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞ 
void GrammarAnalyzer::loopStat() {
    FourYuanItem fy;
    if (SYMTYPE == WHILETK) {
        string label1 = generateLabel();
        string label2 = generateLabel();
        fy.codeType = Label;
        fy.target = label1;
        codeItems.push_back(fy);
        GETSYM //(
        GETSYM
        
        condition();
        fy.codeType = BNZ;
        fy.target = label2;
        codeItems.push_back(fy);
        if (SYMTYPE != RPARENT) {
            error.errorLog(withoutRparent, lineNum);
        }
        else {
            GETSYM
        }
        stat();
        fy.codeType = Goto;
        fy.target = label1;
        codeItems.push_back(fy);
        fy.codeType = Label;
        fy.target = label2;
        codeItems.push_back(fy);
    }
    else if (SYMTYPE == FORTK) {
        string label1 = generateLabel();
        string label2 = generateLabel();
        // string label3 = generateLabel();
        // string label4 = generateLabel();
        GETSYM //(
        GETSYM //标识符
        string str1 = *lexicalAnalyzer.token.word;
        transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
        fy.codeType = AssignState;
        //fy.target = "G" + str1;
        fy.op = '+';
        fy.right = to_string(0);
        fy.rightValue = 0;
        map<string, SymbolItem>::iterator iter;
        iter = funcTable.find(str1);
        if (iter == funcTable.end()) {
            iter = globalTable.find(str1);
            if (iter == globalTable.end()) {
                error.errorLog(identNotDefine, LINE);
                skipto(SEMICN); //+-*/;,)]><>>=<=!===
                //printf("dont find\n");
            }
        }
        ItemType iType = ITEM.getItemType();
        string funcName = ITEM.getFuncName();
        if (funcName == "global") {
            fy.target = "G" + str1;
        }
        else {
            fy.target = "L" + funcName + "_" + str1;
        }
        if (iType == CON) {
            error.errorLog(assignConError, LINE);
        }
        GETSYM //=
        GETSYM
        string tmpName0;
        expr(tmpName0);
        fy.left = tmpName0;
        codeItems.push_back(fy);
        if (SYMTYPE != SEMICN) {
            error.errorLog(withoutSemicn, lineNum);
        }
        else {
            GETSYM
        }
        fy.codeType = Label;
        fy.target = label1;
        codeItems.push_back(fy);
        condition();
        fy.codeType = BNZ;
        fy.target = label2;
        codeItems.push_back(fy);
        if (SYMTYPE != SEMICN) {
            error.errorLog(withoutSemicn, lineNum);
        }
        else {
            GETSYM
        }
        FourYuanItem fy1;
        str1 = *lexicalAnalyzer.token.word;
        transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
        
        fy1.codeType = AssignState;
        
        iter = funcTable.find(str1);
        if (iter == funcTable.end()) {
            iter = globalTable.find(str1);
            if (iter == globalTable.end()) {
                error.errorLog(identNotDefine, LINE);
                skipto(SEMICN); //+-*/;,)]><>>=<=!===
                //printf("dont find\n");
            }
        }
        iType = ITEM.getItemType();
        if (iType == CON) {
            error.errorLog(assignConError, LINE);
        }
        funcName = ITEM.getFuncName();
        if (funcName == "global") {
            fy1.target = "G" + str1;
        }
        else {
            fy1.target = "L" + funcName + "_" + str1;
        }
        GETSYM //=
        GETSYM //标识符
        str1 = *lexicalAnalyzer.token.word;
        transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
        iter = funcTable.find(str1);
        if (iter == funcTable.end()) {
            iter = globalTable.find(str1);
            if (iter == globalTable.end()) {
                error.errorLog(identNotDefine, LINE);
                skipto(SEMICN); //+-*/;,)]><>>=<=!===
                //printf("dont find\n");
            }
        }
        //iType = ITEM.getItemType();
        funcName = ITEM.getFuncName();
        if (funcName == "global") {
            fy1.left = "G" + str1;
        }
        else {
            fy1.left = "L" + funcName + "_" + str1;
        }
        GETSYM //+|-
        fy1.op = (*lexicalAnalyzer.token.word).at(0);
        GETSYM
        
        fy1.rightValue = stride();
        fy1.right = to_string(fy1.rightValue);
        
        if (SYMTYPE != RPARENT) {
            error.errorLog(withoutRparent, lineNum);
        }
        else {
            GETSYM
        }
        stat();
        codeItems.push_back(fy1);
        fy.codeType = Goto;
        fy.target = label1;
        codeItems.push_back(fy);
        fy.codeType = Label;
        fy.target = label2;
        codeItems.push_back(fy);
    }
    toFile("<循环语句>");
}
//＜条件＞    ::=  ＜表达式＞＜关系运算符＞＜表达式＞ 
void GrammarAnalyzer::condition() {
    FourYuanItem fy;
    string tmpName0;
    string tmpName1;
    ValueType vt1 = expr(tmpName0);
    switch (SYMTYPE) {
        case LSS:
            fy.codeType = LSSm;
            break;
        case LEQ:
            fy.codeType = LEQm;
            break;
        case GRE:
            fy.codeType = GREm;
            break;
        case GEQ:
            fy.codeType = GEQm;
            break;
        case EQL:
            fy.codeType = EQLm;
            break;
        case NEQ:
            fy.codeType = NEQm;
            break;
        default:
            break;
    }
    GETSYM //关系运算符
    ValueType vt2 = expr(tmpName1);
    fy.left = tmpName0;
    fy.right = tmpName1;
    codeItems.push_back(fy);
    if (vt1 != INT || vt2 != INT) {
        error.errorLog(illegalCondition, LINE);
    }
    toFile("<条件>");
}
//＜步长＞::= ＜无符号整数＞
int GrammarAnalyzer::stride() {
    int i = integerUnsigned();
    toFile("<步长>");
    return i;
}
//＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
ValueType GrammarAnalyzer::expr(string &tmpName) {
    vector<PostfixItem> infix, postfix;
    int flag = 0;
    if (SYMTYPE == PLUS || SYMTYPE == MINU) {
        PostfixItem postFixItem;
		postFixItem.valueType = CHAR;
		postFixItem.number = (SYMTYPE == PLUS) ? '+' : '-'; 
		postFixItem.isOperator = true;
        if (SYMTYPE == MINU) {
            PostfixItem postFixItem1;
            postFixItem1.valueType = INT;
            postFixItem1.number = 0;
            postFixItem1.isOperator = false;
            infix.push_back(postFixItem1);
            infix.push_back(postFixItem);
        }
		
        flag = 1;
        GETSYM
    }
    ValueType vt = item(infix);
    while (SYMTYPE == PLUS || SYMTYPE == MINU) {
        PostfixItem postFixItem;
		postFixItem.valueType = CHAR;
		postFixItem.number = (SYMTYPE == PLUS) ? '+' : '-'; 
		postFixItem.isOperator = true;
		infix.push_back(postFixItem);
        flag = 1;
        GETSYM
        item(infix);
    }
    turnToPostfixExpr(infix, postfix);
    bool isSure;
    int value;
    tmpName = calculateExpr(postfix, tmpCount, value, isSure, codeItems);
    printf("tmp : %s \n", tmpName.c_str());
    
    if (flag == 1 && vt == CHAR) {
        vt = INT;
    }
    codeItems.at(codeItems.size() - 1).valueType = vt;
    toFile("<表达式>");
    return vt;
}

ValueType GrammarAnalyzer::expr(vector<PostfixItem> &infix) {
    int flag = 0;
    if (SYMTYPE == PLUS || SYMTYPE == MINU) {
        PostfixItem postFixItem;
        postFixItem.valueType = CHAR;
        postFixItem.number = (SYMTYPE == PLUS) ? '+' : '-';
        postFixItem.isOperator = true;
        if (SYMTYPE == MINU) {
            PostfixItem postFixItem1;
            postFixItem1.valueType = INT;
            postFixItem1.number = 0;
            postFixItem1.isOperator = false;
            infix.push_back(postFixItem1);
            infix.push_back(postFixItem);
        }
        flag = 1;
        GETSYM
    }
    ValueType vt = item(infix);
    while (SYMTYPE == PLUS || SYMTYPE == MINU) {
        PostfixItem postFixItem;
        postFixItem.valueType = CHAR;
        postFixItem.number = (SYMTYPE == PLUS) ? '+' : '-';
        postFixItem.isOperator = true;
        infix.push_back(postFixItem);
        flag = 1;
        GETSYM
        item(infix);
    }
 

    if (flag == 1 && vt == CHAR) {
        vt = INT;
    }
    toFile("<表达式>");
    return vt;
}

//＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}    
ValueType GrammarAnalyzer::item(vector<PostfixItem> &infix) {
    int flag = 0;
    ValueType vt = factor(infix);
    while (SYMTYPE == DIV || SYMTYPE == MULT) {
        PostfixItem postFixItem;
		postFixItem.valueType = CHAR;
		postFixItem.number = (SYMTYPE == DIV) ? '/' : '*'; 
		postFixItem.isOperator = true;
		infix.push_back(postFixItem);
        flag = 1;
        GETSYM
        factor(infix);
    }
    if (flag == 1 && vt == CHAR) {
        vt = INT;
    }
    toFile("<项>");
    return vt;
}
//＜因子＞    ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|＜标识符＞'['＜表达式＞']''['＜表达式＞']'|'('＜表达式＞')'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
ValueType GrammarAnalyzer::factor(vector<PostfixItem> &infix) {
    PostfixItem postFixItem;
    ValueType vt = INT;
    if (SYMTYPE == IDENFR) {
        string str1 = *lexicalAnalyzer.token.word;
        transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
        map<string, SymbolItem>::iterator iter;
        iter = funcTable.find(str1);
        if (iter == funcTable.end()) {
            iter = globalTable.find(str1);
            if (iter == globalTable.end()) {
                error.errorLog(identNotDefine, LINE);
                skipto(FACTOROP); //+-*/;,)]><>>=<=!===
                //printf("dont find\n");
                return INT;
            }
        }
        ItemType iType = iter->second.getItemType();
        FuncType fType = iter->second.getFuncType();
        string funcName = iter->second.getFuncName();
        string name = iter->second.getName();
        vt = iter->second.getValueType();
        if (iType == FUNC) {
            if (fType == RETURNCHAR || fType == RETURNINT) {
                funStatReturn(name);
                FourYuanItem fy;
                fy.codeType = AssignState;
                fy.left = "RET";
                fy.right = "0";
                fy.op = '+';
                fy.rightValue = 0;
                fy.target = generateTmpName(tmpCount++);
                codeItems.push_back(fy);
                postFixItem.valueType = String;
                postFixItem.str = fy.target;
                postFixItem.isOperator = false;
                postFixItem.isCharVar = fType == RETURNCHAR ? true : false;
                infix.push_back(postFixItem);
                toFile("<因子>");
                return vt;
            }
            else {
                error.errorLog(LINE);
                funStatNonReturn(name);
                //toFile("<因子>");
                return INT;
            }
        }
        else if (iType == VAR || iType == CON || iType == PARA) {
            int x = iter->second.getDimx();
            int y = iter->second.getDimy();
            int dim = iter->second.getDim();
            if (dim == 2) {
                FourYuanItem fy;
                fy.codeType = AssignByArray;
                fy.x = x;
                fy.y = y;
                GETSYM
                if (SYMTYPE != LBRACK) {
                    error.errorLog(LINE);
                }
                else {
                    GETSYM
                }
                string tmpName0;
                ValueType arrayIndex = expr(tmpName0);
                fy.left = tmpName0;
                if (arrayIndex != INT) {
                    error.errorLog(arrayIndexTypeError, LINE);
                }
                if (SYMTYPE != RBRACK) {
                    error.errorLog(withoutRbrack, LINE);
                }
                else {
                    GETSYM
                }
                if (SYMTYPE != LBRACK) {
                    error.errorLog(LINE);
                }
                else {
                    GETSYM
                }
                string tmpName1;
                arrayIndex = expr(tmpName1);
                fy.right = tmpName1;
                fy.valueType = vt;
                fy.target = generateTmpName(tmpCount++);
                if (funcName == "global") {
                    fy.arrayName = "G" + name;
                }
                else {
                    fy.arrayName = "L" + funcName + "_" + name;
                }
                fy.dim = 2;
                codeItems.push_back(fy);
                if (arrayIndex != INT) {
                    error.errorLog(arrayIndexTypeError, LINE);
                }
                if (SYMTYPE != RBRACK) {
                    error.errorLog(withoutRbrack, LINE);
                }
                else {
                    GETSYM
                }
                postFixItem.valueType = String;
                if (vt == CHAR) {
                    postFixItem.isCharVar = true;
                }
                else {
                    postFixItem.isCharVar = false;
                }
                postFixItem.isOperator = false;
                postFixItem.str = fy.target;
                //printf("eee %s\n", postFixItem.str.c_str());
                infix.push_back(postFixItem);
                toFile("<因子>");
                return vt;
            }
            else if (dim == 1) {
                FourYuanItem fy;
                fy.codeType = AssignByArray;
                if (funcName == "global") {
                    fy.arrayName = "G" + name;
                }
                else {
                    fy.arrayName = "L" + funcName + "_" + name;
                }
                fy.x = x;
                fy.y = 0;
                fy.dim = 1;
                fy.valueType = vt;
                fy.target = generateTmpName(tmpCount++);
                printf("fytarget: %s \n", fy.target.c_str());
                GETSYM
                if (SYMTYPE == LBRACK) {
                    GETSYM
                    string tmpName0;
                    
                    ValueType arrayIndex = expr(tmpName0);
                    fy.left = tmpName0;
                    codeItems.push_back(fy);
                    postFixItem.valueType = String;
                    if (vt == CHAR) {
                        postFixItem.isCharVar = true;
                    }
                    else {
                        postFixItem.isCharVar = false;
                    }
                    postFixItem.isOperator = false;
                    postFixItem.str = fy.target;
                    printf("poststr: %s \n", postFixItem.str.c_str());
                    infix.push_back(postFixItem);
                    if (arrayIndex != INT) {
                        error.errorLog(arrayIndexTypeError, LINE);
                    }
                    if (SYMTYPE != RBRACK) {
                        error.errorLog(withoutRbrack, LINE);
                    }
                    else {
                        GETSYM
                    }
                    toFile("<因子>");
                    return vt;
                }
                else {
                    error.errorLog(LINE);
                }
               
            }
            else {
                postFixItem.valueType = vt;
                if (iType == CON) {
                    if (vt == INT) {
                        postFixItem.number = iter->second.value.number;
                    }
                    else {
                        postFixItem.number = iter->second.value.ch;
                    }
                    postFixItem.isOperator = false;
                }
                else {
                    postFixItem.valueType = String;
                    if (vt == CHAR) {
                        postFixItem.isCharVar = true;
                    }
                    else {
                        postFixItem.isCharVar = false;
                    }
                    postFixItem.isOperator = false;
                    if (funcName == "global") {
                        postFixItem.str = "G" + name;
                    }
                    else {
                        postFixItem.str = "L" + funcName + "_" + name;
                    }
                }
                infix.push_back(postFixItem);
                GETSYM
                toFile("<因子>");
                return vt;
            }
        }
    }
    else if (SYMTYPE == LPARENT) {
        
        GETSYM
        postFixItem.isOperator = true;
        postFixItem.valueType = String;
        postFixItem.number = '(';
        infix.push_back(postFixItem);
        vt = expr(infix);
        
        
        if (vt == CHAR) {
            vt = INT;
        }
        postFixItem.number = ')';
        infix.push_back(postFixItem);
        if (SYMTYPE != RPARENT) {
            error.errorLog(withoutRparent, lineNum);
        }
        else {
            GETSYM
        }
    }
    else if (SYMTYPE == MINU || SYMTYPE == PLUS || SYMTYPE == INTCON) {
        postFixItem.valueType = INT;
        postFixItem.number = integerCon();
        postFixItem.isCharVar = false;
        postFixItem.isOperator = false;
        infix.push_back(postFixItem);
        vt = INT;
    }
    else if (SYMTYPE == CHARCON) {
        postFixItem.valueType = CHAR;
        postFixItem.number = (*lexicalAnalyzer.token.word)[0];
        postFixItem.isCharVar = true;
        postFixItem.isOperator = false;
        infix.push_back(postFixItem);
        vt = CHAR;
        GETSYM
    }
    else {
        GETSYM
        error.errorLog(LINE);
    }
    toFile("<因子>");
    return vt;
}
//＜条件语句＞  ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
void GrammarAnalyzer::conditionalStat() {
    string label1 = generateLabel();
    string label2 = generateLabel();
    FourYuanItem fy;
    
    GETSYM //(
    GETSYM
    condition();
    fy.codeType = BNZ;
    fy.target = label2;
    codeItems.push_back(fy);
    if (SYMTYPE != RPARENT) {
        error.errorLog(withoutRparent, lineNum);
    }
    else {
        GETSYM
    }
    stat();
    fy.codeType = Goto;
    fy.target = label1;
    codeItems.push_back(fy);
    fy.codeType = Label;
    fy.target = label2;
    codeItems.push_back(fy);
    if (SYMTYPE == ELSETK) {
        GETSYM
        stat();
    }
    fy.codeType = Label;
    fy.target = label1;
    codeItems.push_back(fy);
    toFile("<条件语句>");
}
//＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'    
void GrammarAnalyzer::funStatReturn(string funcName) {
    GETSYM //(
    GETSYM
    FourYuanItem fy;
    valueParaTable(funcName);

    fy.codeType = FunctionCall;
    fy.target = funcName;
    codeItems.push_back(fy);
    if (SYMTYPE != RPARENT) {
        error.errorLog(withoutRparent, lineNum);
    }
    else {
        GETSYM
    }
    toFile("<有返回值函数调用语句>");
}
//＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'         
void GrammarAnalyzer::funStatNonReturn(string funcName) {
    GETSYM //(
    GETSYM
    FourYuanItem fy;
    valueParaTable(funcName);
    fy.codeType = FunctionCall;
    fy.target = funcName;
    codeItems.push_back(fy);
    if (SYMTYPE != RPARENT) {
        error.errorLog(withoutRparent, lineNum);
    }
    else {
        GETSYM
    }
    toFile("<无返回值函数调用语句>");
}
//＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}｜＜空＞ 
void GrammarAnalyzer::valueParaTable(string funcName) {
    FourYuanItem fy;
    transform(funcName.begin(), funcName.end(), funcName.begin(), ::tolower);
    map<string, vector<SymbolItem>>::iterator iter;
    iter = paras.find(funcName);
    if (iter == paras.end()) {
        error.errorLog(LINE);
        skipto(RPARENT);
        return;
    }
    
    vector<SymbolItem> *paraVector = &(iter->second);
    int size = (*paraVector).size();
    // if (SYMTYPE == SEMICN || SYMTYPE == MINU || SYMTYPE == PLUS || SYMTYPE == DIV || SYMTYPE == MULT || SYMTYPE == COMMA || SYMTYPE == RBRACK 
    // || SYMTYPE == LEQ || SYMTYPE == LSS || SYMTYPE == GEQ || SYMTYPE == GRE || SYMTYPE == EQL || SYMTYPE == NEQ) {
    //     return;
    // }
    if (SYMTYPE == SEMICN || SYMTYPE == DIV || SYMTYPE == MULT || SYMTYPE == COMMA || SYMTYPE == RBRACK 
    || SYMTYPE == LEQ || SYMTYPE == LSS || SYMTYPE == GEQ || SYMTYPE == GRE || SYMTYPE == EQL || SYMTYPE == NEQ) {
        return;
    }
    if (SYMTYPE == RPARENT) {
        if (size == 0) {
            toFile("<值参数表>");
            return;
        }
        else {
            toFile("<值参数表>");
            error.errorLog(funcStatParaNum, LINE);
            return;
        }
    }
    vector<string> paras;
    size_t i = 0;
    for (i = 0; i < size; i++) {
        string tmpName0;
        ValueType vt = expr(tmpName0);
        paras.push_back(tmpName0);
        
        SymbolItem si = (*paraVector)[i];
        if (vt != si.getValueType()) {
            error.errorLog(funcStatParaType, LINE);
            skipto(RPARENT);
            return;
        }
        if (SYMTYPE == COMMA) {
            GETSYM
        }
        else {
            break;
        }
        
    }
    if (i != size - 1) {
        error.errorLog(funcStatParaNum, LINE);
        if (SYMTYPE != RPARENT) {
            skipto(RPARENT);
            return;
        }
        //GETSYM
    }
    fy.codeType = ValueParamDeliver;
    for (int i = 0; i < paras.size(); i++) {
        fy.target = paras.at(i);
        codeItems.push_back(fy);
    }
   
    toFile("<值参数表>");
}
//＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞|＜标识符＞'['＜表达式＞']''['＜表达式＞']' =＜表达式＞
void GrammarAnalyzer::assignStat(ValueType vtl, string funcName) {
    string str1 = *lexicalAnalyzer.token.word; 
    transform(str1.begin(), str1.end(), str1.begin(), ::tolower); 
    map<string, SymbolItem>::iterator iter;
    iter = funcTable.find(str1);
    if (iter == funcTable.end()) {
        iter = globalTable.find(str1);
        if (iter == globalTable.end()) {
            error.errorLog(identNotDefine, LINE);
            skipto(SEMICN);
            return;
        }
    }
    int x = ITEM.getDimx();
    int y = ITEM.getDimy();
    FourYuanItem fy;
    fy.x = x;
    fy.y = y;
    fy.codeType = AssignState;
    if (funcName == "global") {
        fy.target = "G" + str1;
    }
    else {
        fy.target = "L" + funcName + "_" + str1;
    }
    fy.arrayName = fy.target;
    fy.right = to_string(0);
    fy.rightValue = 0;
    //printf("%s\n", (*lexicalAnalyzer.token.word).c_str());
    GETSYM
    if (SYMTYPE == ASSIGN) {
        GETSYM
        string tmpName0;
        ValueType vtr = expr(tmpName0);
        //printf("%s\n", tmpName0.c_str());
        fy.dim = 0;
        fy.op = '+';
        fy.left = tmpName0;
        codeItems.push_back(fy);
        if (vtr != vtl) {
            //error.errorLog(typeMatchError, LINE);
        }
    }
    else if (SYMTYPE == LBRACK) {
        GETSYM
        //expr();
        string tmpName0;
        ValueType arrayIndex = expr(tmpName0);
        
        if (arrayIndex != INT) {
            error.errorLog(arrayIndexTypeError, LINE);
        }
        if (SYMTYPE != RBRACK) {
            error.errorLog(withoutRbrack, LINE);
        }
        else {
            GETSYM
        }
        
        if (SYMTYPE == ASSIGN) {
            GETSYM
            string tmpName2;
            ValueType vtr = expr(tmpName2);
            fy.dim = 1;
            fy.codeType = AssignArray;
            fy.left = tmpName0;
            fy.target = tmpName2;
            codeItems.push_back(fy);
            if (vtr != vtl) {
                //error.errorLog(typeMatchError, LINE);
            }
        }
        else if (SYMTYPE == LBRACK) {
            GETSYM
            string tmpName1;
            arrayIndex = expr(tmpName1);
            if (arrayIndex != INT) {
                error.errorLog(arrayIndexTypeError, LINE);
            }
            if (SYMTYPE != RBRACK) {
                error.errorLog(withoutRbrack, LINE);
            }
            else {
                GETSYM
            }
            if (SYMTYPE == ASSIGN) {
                GETSYM
                string tmpName2;
                ValueType vtr = expr(tmpName2);
                fy.codeType = AssignArray;
                fy.dim = 2;
                fy.left = tmpName0;
                fy.right = tmpName1;
                fy.target = tmpName2;
                codeItems.push_back(fy);
                if (vtr != vtl) {
                    //error.errorLog(typeMatchError, LINE);
                }
            }
        }
    }
    toFile("<赋值语句>");
}
//＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞＜缺省＞‘}’
void GrammarAnalyzer::caseStat() {
    string label1 = generateLabel();
    //string label2 = generateLabel();
    FourYuanItem fy;
    GETSYM //(
    GETSYM
    string tmpName0;
    ValueType vt = expr(tmpName0);
    if (SYMTYPE != RPARENT) {
        error.errorLog(withoutRparent, lineNum);
    }
    else {
        GETSYM //{
    }
    GETSYM
    caseTable(vt, tmpName0, label1);
    defaultStat();
    fy.codeType = Label;
    fy.target = label1;
    codeItems.push_back(fy);
    GETSYM
    toFile("<情况语句>");
}
//＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞}     
void GrammarAnalyzer::caseTable(ValueType vt, string tmpName, string label) {
    while (SYMTYPE == CASETK) {
        string nextLabel = generateLabel();
        caseSubStat(vt, tmpName, label, nextLabel);
        FourYuanItem fy;
        fy.codeType = Label;
        fy.target = nextLabel;
        codeItems.push_back(fy);
    }
    toFile("<情况表>");
}
//＜情况子语句＞  ::=  case＜常量＞：＜语句＞
void GrammarAnalyzer::caseSubStat(ValueType vt, string tmpName, string label, string nextLabel) {
    FourYuanItem fy;
    fy.codeType = EQLm;
    fy.left = tmpName;
    int in;
    GETSYM
    if (constant(&in) != vt) {
        error.errorLog(typeMatchError, LINE);
    }
    fy.right = to_string(in);
    fy.rightValue = in;
    codeItems.push_back(fy);
    fy.codeType = BNZ;
    fy.target = nextLabel;
    codeItems.push_back(fy);
    GETSYM
    stat();
    fy.codeType = Goto;
    fy.target = label;
    codeItems.push_back(fy);
    toFile("<情况子语句>");
}
//＜缺省＞   ::=  default :＜语句＞
void GrammarAnalyzer::defaultStat() {
    if (SYMTYPE != DEFAULTTK) {
        error.errorLog(withoutDefault, LINE);
        return;
    }
    GETSYM
    GETSYM
    stat();
    toFile("<缺省>");
}
//＜读语句＞    ::=  scanf '('＜标识符＞')' 
void GrammarAnalyzer::scanfStat() {
    FourYuanItem fy;
    GETSYM //(
        
    GETSYM //标识符
    string str1 = *lexicalAnalyzer.token.word; 
    transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
    
    map<string, SymbolItem>::iterator iter; 
    iter = funcTable.find(str1); 
    if (iter == funcTable.end()) { 
        iter = globalTable.find(str1); 
        if (iter == globalTable.end()) { 
            error.errorLog(identNotDefine, LINE); 
            skipto(SEMICN); 
            return; 
        } 
    }
    string funcName = ITEM.getFuncName();
    if (funcName == "global") {
        fy.target = "G" + str1;
    }
    else {
        fy.target = "L" + funcName + "_" + str1;
    }
    ItemType it = ITEM.getItemType();
    ValueType vt = ITEM.getValueType();
    if (vt == INT) {
        fy.codeType = ReadInt;
    }
    else if (vt == CHAR) {
        fy.codeType = ReadChar;
    }
    codeItems.push_back(fy);
    if (it == CON) {
        error.errorLog(assignConError, LINE);
    }
    GETSYM //)
    
    if (SYMTYPE != RPARENT) {
        error.errorLog(withoutRparent, lineNum);
    }
    else {
        GETSYM
    }
    toFile("<读语句>");
}
//＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
void GrammarAnalyzer::printfStat() {
    FourYuanItem fy;
    GETSYM //(
       
    GETSYM //STRCON|expr
    if (SYMTYPE == STRCON) {
        fy.codeType = PrintStr;
        string str1 = *lexicalAnalyzer.token.word; 
        //transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
        fy.target = str1;
       
        GETSYM

        toFile("<字符串>");
        if (SYMTYPE == COMMA) {
            fy.codeType = PrintBeforeExpr;
            codeItems.push_back(fy);
            GETSYM
            string tmpName0;
            ValueType vt = expr(tmpName0);
            fy.target = tmpName0;
            fy.codeType = PrintIdent;
            fy.valueType = vt;
            codeItems.push_back(fy);
            if (SYMTYPE != RPARENT) {
                error.errorLog(withoutRparent, lineNum);
            }
            else {
                GETSYM
            }
        }
        else if (SYMTYPE == RPARENT) {
            codeItems.push_back(fy);
            GETSYM
        }
        else {
            error.errorLog(withoutRparent, lineNum);
        }
    }
    else {
        string tmpName0;
        ValueType vt = expr(tmpName0);
        fy.target = tmpName0;
        fy.codeType = PrintIdent;
        fy.valueType = vt;
        codeItems.push_back(fy);
        if (SYMTYPE != RPARENT) {
            error.errorLog(withoutRparent, lineNum);
        }
        else {
            GETSYM
        }
        
    }
    toFile("<写语句>");
}
//＜返回语句＞   ::=  return['('＜表达式＞')']   
void GrammarAnalyzer::returnStat() {
    FourYuanItem fy;
    
    GETSYM
    
    if (SYMTYPE == SEMICN) {
       
         fy.codeType = ReturnEmpty;
         codeItems.push_back(fy);
        if (cuFuncType != VOID) {
            error.errorLog(returnFuncReturnError, LINE);
            //printf("dd\n");
        }
        toFile("<返回语句>");
        return;
    }
    else if (SYMTYPE == LPARENT) {
        GETSYM
        
        if (SYMTYPE == RPARENT) {
            if (cuFuncType == VOID) {
                error.errorLog(voidFuncReturnError, LINE);
                toFile("<返回语句>");
                GETSYM
                return;
            }
            else {
                error.errorLog(returnFuncReturnError, LINE);
                toFile("<返回语句>");
                GETSYM
                return;
                //printf("dde\n");
            }
        }
        string tmpName0;
        ValueType vt = expr(tmpName0);
        if (vt == INT) {
            
            if (cuFuncType == RETURNCHAR) {
                error.errorLog(returnFuncReturnError, LINE);
            }
            else if (cuFuncType == VOID) {
                error.errorLog(voidFuncReturnError, LINE);
            }
        }
        else if (vt == CHAR) {
            if (cuFuncType == RETURNINT) {
                error.errorLog(returnFuncReturnError, LINE);
            }
            else if (cuFuncType == VOID) {
                error.errorLog(voidFuncReturnError, LINE);
            }
        }
        fy.codeType = ReturnIdent;
        fy.target = tmpName0;
        fy.valueType = vt;
        codeItems.push_back(fy);
        if (SYMTYPE != RPARENT)
        {
            error.errorLog(withoutRparent, lineNum);
        }
        else {
            GETSYM
        }
        
        
    }
    else {
        if (cuFuncType  == VOID) {
            toFile("<返回语句>");
            return;
        }
    }
    toFile("<返回语句>");
}

void GrammarAnalyzer::skipto(SymbolType st) {
    if (st == FACTOROP) { //+-*/;,)]><>=<=!===
        while (SYMTYPE != PLUS && SYMTYPE != MINU
         && SYMTYPE != MULT && SYMTYPE != DIV
          && SYMTYPE != LSS && SYMTYPE != LEQ
           && SYMTYPE != GRE && SYMTYPE != GEQ
            && SYMTYPE != EQL && SYMTYPE != NEQ
             && SYMTYPE != SEMICN && SYMTYPE != COMMA
              && SYMTYPE != RPARENT && SYMTYPE != RBRACK) {
            lexicalAnalyzer.getSymbol();
        }
        return;
    }

    while (SYMTYPE != st) {
        lexicalAnalyzer.getSymbol();
    }
}

void GrammarAnalyzer::addItem(string name, string funcName, ItemType itemType, ValueType valueType, int order) {
    transform(funcName.begin(), funcName.end(), funcName.begin(), ::tolower);
    if (name == "null") {
        vector<SymbolItem> paraVector;
        paras.insert(map<string, vector<SymbolItem>>::value_type(funcName, paraVector));
        return;
    }
    SymbolItem symbolItem(name, funcName);
    symbolItem.setItemType(itemType);
    symbolItem.setValueType(valueType);
    symbolItem.setOrder(order);
    symbolItem.setFuncName(cuFuncName);
    map<string, vector<SymbolItem>>::iterator iter;
    iter = paras.find(funcName);
    if (iter == paras.end()) {
        vector<SymbolItem> paraVector;
        paras.insert(map<string, vector<SymbolItem>>::value_type(funcName, paraVector));
    }
    vector<SymbolItem> *paraVector = &paras[funcName];
    (*paraVector).push_back(symbolItem);
}

void GrammarAnalyzer::addItem(string name, ItemType itemType, ValueType valueType, int i, int j, int dim) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (isDefined(name)) {
        error.errorLog(identRedefine, LINE);
        return;
    }
    SymbolItem symbolItem(name, cuFuncName);
    symbolItem.setItemType(itemType);
    symbolItem.setValueType(valueType);
    symbolItem.setFuncType(VOID);
    symbolItem.setFuncName(cuFuncName);
    symbolItem.setDimx(i);
    symbolItem.setDimy(j);
    symbolItem.setDim(dim);
    FourYuanItem fy;
    fy.codeType = VarDef;
    fy.isArray = false;
    if (dim > 0) {
        fy.isArray = true;
    }
    fy.valueType = valueType;
    fy.x = i;
    fy.y = j;
    fy.dim = dim;
    if (cuFuncName == "global") {
        fy.target = "G" + name;
    }
    else {
        fy.target = "L" + cuFuncName + "_" + name;
    }
    fy.order = order++;
    codeItems.push_back(fy);
    cuSi = &symbolItem;
    if (cuFuncName == "global") {
        globalTable.insert(map<string, SymbolItem>::value_type(name, symbolItem));
    }
    else {
        funcTable.insert(map<string, SymbolItem>::value_type(name, symbolItem));
    }
}

void GrammarAnalyzer::addItem(string name, ItemType itemType, ValueType valueType) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (isDefined(name)) {
        error.errorLog(identRedefine, LINE);
        return;
    }
    SymbolItem symbolItem(name, cuFuncName);
    symbolItem.setItemType(itemType);
    symbolItem.setValueType(valueType);
    symbolItem.setFuncType(VOID);
    symbolItem.setFuncName(cuFuncName);
    if (cuFuncName == "global") {
        globalTable.insert(map<string, SymbolItem>::value_type(name, symbolItem));
    }
    else {
        funcTable.insert(map<string, SymbolItem>::value_type(name, symbolItem));
    }
}

void GrammarAnalyzer::addItem(string name, ItemType itemType, ValueType valueType, int value) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (isDefined(name)) {
        error.errorLog(identRedefine, LINE);
        return;
    }
    SymbolItem symbolItem(name, cuFuncName);
    symbolItem.setItemType(itemType);
    symbolItem.setValueType(valueType);
    symbolItem.setFuncType(VOID);
    symbolItem.setFuncName(cuFuncName);
    symbolItem.value.number = value;
    if (cuFuncName == "global") {
        globalTable.insert(map<string, SymbolItem>::value_type(name, symbolItem));
    }
    else {
        funcTable.insert(map<string, SymbolItem>::value_type(name, symbolItem));
    }
}

void GrammarAnalyzer::addItem(string name, ItemType itemType, ValueType valueType, string value) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (isDefined(name)) {
        error.errorLog(identRedefine, LINE);
        return;
    }
    SymbolItem symbolItem(name, cuFuncName);
    symbolItem.setItemType(itemType);
    symbolItem.setValueType(valueType);
    symbolItem.setFuncType(VOID);
    symbolItem.setFuncName(cuFuncName);
    symbolItem.value.ch = value.at(0);
    if (cuFuncName == "global") {
        globalTable.insert(map<string, SymbolItem>::value_type(name, symbolItem));
    }
    else {
        funcTable.insert(map<string, SymbolItem>::value_type(name, symbolItem));
    }
}


void GrammarAnalyzer::addItem(string name, FuncType funcType) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    SymbolItem symbolItem(name, cuFuncName);
    if (funcType == RETURNCHAR) {
        symbolItem.setValueType(CHAR);
    }
    else if (funcType == RETURNINT) {
        symbolItem.setValueType(INT);
    }
    symbolItem.setItemType(FUNC);
    symbolItem.setFuncName(cuFuncName);
    symbolItem.setFuncType(funcType);
    globalTable.insert(map<string, SymbolItem>::value_type(name, symbolItem));
}

bool GrammarAnalyzer::isDefined(string name) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    map<string, SymbolItem>::iterator iter;
    if (cuFuncName == "global") {
        iter = globalTable.find(name);
        if (iter != globalTable.end()) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        iter = funcTable.find(name);
        if (iter != funcTable.end()) {
            return true;
        }
        else {
            return false;
        }
    }
    return true;
}

bool GrammarAnalyzer::isNotDefined(string name) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    map<string, SymbolItem>::iterator iter;
    iter = funcTable.find(name);
    if (iter != funcTable.end()) {
        return false;
    }
    iter = globalTable.find(name);
    if (iter != globalTable.end()) {
        return false;
    }
    return true;
}

void GrammarAnalyzer::updateField(string name, FuncType ft) {
    order = 0;
    tmpCount = 0;
    cuFuncName = "global";
    if (!isDefined(name)) {
        errorFun = 0;
        addItem(name, ft);
        printf("%s add to global\n", name.c_str());
    }
    else {
        errorFun = 1;
        printf("find redefine fun %s\n", name.c_str());
        error.errorLog(identRedefine, LINE);
    }
    cuFuncName = name;
    cuFuncType = ft;
    funcTable.erase(funcTable.begin(), funcTable.end());
}

string type2String(SymbolType symbolType) {
    string str;
    switch(symbolType) {
        case CONSTTK :
            str = "CONSTTK";
            break;
        case INTTK :
            str = "INTTK";
            break;
        case CHARTK :
            str = "CHARTK";
            break;
        case VOIDTK :
            str = "VOIDTK";
            break;
        case MAINTK :
            str = "MAINTK";
            break;
        case IFTK :
            str = "IFTK";
            break;
        case ELSETK :
            str = "ELSETK";
            break;
        case SWITCHTK :
            str = "SWITCHTK";
            break;
        case CASETK :
            str = "CASETK";
            break;
        case DEFAULTTK :
            str = "DEFAULTTK";
            break;
        case WHILETK :
            str = "WHILETK";
            break;
        case FORTK :
            str = "FORTK";
            break;
        case SCANFTK :
            str = "SCANFTK";
            break;
        case PRINTFTK :
            str = "PRINTFTK";
            break;
        case RETURNTK :
            str = "RETURNTK";
            break;
        case IDENFR :
            str = "IDENFR";
            break;
        case INTCON :
            str = "INTCON";
            break;
        case CHARCON :
            str = "CHARCON";
            break;
        case STRCON :
            str = "STRCON";
            break;
        case PLUS :
            str = "PLUS";
            break;
        case MINU :
            str = "MINU";
            break;
        case MULT :
            str = "MULT";
            break;
        case DIV :
            str = "DIV";
            break;
        case LSS :
            str = "LSS";
            break;
        case LEQ :
            str = "LEQ";
            break;
        case GRE :
            str = "GRE";
            break;
        case GEQ :
            str = "GEQ";
            break;
        case EQL :
            str = "EQL";
            break;
        case NEQ :
            str = "NEQ";
            break;
        case COLON :
            str = "COLON";
            break;
        case ASSIGN :
            str = "ASSIGN";
            break;
        case SEMICN :
            str = "SEMICN";
            break;
        case COMMA :
            str = "COMMA";
            break;
        case LPARENT :
            str = "LPARENT";
            break;
        case RPARENT :
            str = "RPARENT";
            break;
        case LBRACK :
            str = "LBRACK";
            break;
        case RBRACK :
            str = "RBRACK";
            break;
        case LBRACE :
            str = "LBRACE";
            break;
        case RBRACE :
            str = "RBRACE";
            break;
        default:
            str = "";
            break;
    }
    return str;
}