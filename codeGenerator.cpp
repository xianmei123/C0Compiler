#include <cstring>
#include "codeGenerator.h"
#include "type.h"
#include <vector>
#include <fstream>
#include <map>
using namespace std;
const string midCodeFileName = "midCode.txt";
const string mipsCodeFileName = "mips.txt";
static int labelCount = 1;
vector<string> strs;
map<string, int> varToStack;
map<string, int> midVarToSpace;


const unsigned int dataBaseAddr = 0x10010000;
unsigned int strMemSize;
unsigned int midVarBaseAddr;
unsigned int cuMidVarAddr;
unsigned int stackBase = 0x7fffeffc;
unsigned int top = 0x7fffeffc;
unsigned int paraVarBaseAddr;
unsigned int paraTopAddr;
unsigned int varBaseAddr;
unsigned int varTopAddr;

string generateTmpName(int num) {
    return "T" + to_string(num);
}

string generateLabel() {
    return "Label" + to_string(labelCount++);
}

void turnToPostfixExpr(vector<PostfixItem> infix, vector<PostfixItem> & postfix) {
    vector<PostfixItem> tmp;
    if (infix.size() == 0) {
        return;
    }
    
    PostfixItem it = infix.at(0);
    if (it.valueType == CHAR && (it.number == '+' || it.number == '+') && it.isOperator == true) {
        if (it.number == '+') {
            infix.erase(infix.begin());
        }
        else {
            PostfixItem t;
            t.valueType = INT;
            t.number = 0;
            infix.insert(infix.begin(), t);
        }
    }
    if (infix.size() == 1) {
        postfix.push_back(infix.at(0));
        return;
    }
    for (int i = 0; i < infix.size(); i++) {
        PostfixItem t = infix.at(i);
        if (t.isOperator) {
            switch (t.number) {
                case '+':
                case '-':{
                    while (tmp.size() != 0) {
                        postfix.push_back(tmp.at(tmp.size() - 1));
                        tmp.pop_back();
                    }
                    tmp.push_back(t);}
                    break;
                case '*':
                case '/':{
                    while (tmp.size() != 0) {
                        PostfixItem t0 = tmp.at(tmp.size() - 1);
                        if (t0.number == '*' || t0.number == '/') {
                            postfix.push_back(t0);
                            tmp.pop_back();
                        }
                        else {
                            break;
                        }
                    }
                    tmp.push_back(t);}
                    break;
                default:
                    postfix.push_back(t);
            }
        }
        else {
            postfix.push_back(t);
        }
    }
    while (tmp.size() != 0) {
        postfix.push_back(tmp.at(tmp.size() - 1));
        tmp.pop_back();
    }
}

string calculateExpr(vector<PostfixItem> &postfix, int &tmpCount, int &value, bool &isSure, vector<FourYuanItem> &fourItems) {
    PostfixItem t;
    vector<PostfixItem> tmp;
    FourYuanItem fy;
    string tmpName0;
    int count = 0;
    // = generateTmpName(tmpCount++);
    if (postfix.size() == 1) {
        t = postfix.at(0);
        fy.codeType = AssignState;
        printf("ddddd %s \n", t.str.c_str());
        t = postfix.at(0);
        if (t.str.length() > 0 && t.str.at(0) == 'T') {
            fy.left = t.str;
            fy.right = to_string(0);
            fy.rightValue = 0;
            fy.target = t.str;
            return t.str;
        }
        tmpName0 = generateTmpName(tmpCount++);
        if (t.valueType == INT || t.valueType == CHAR) {
            fy.valueType = t.valueType;
            // if (fy.valueType == CHAR) {
            //     printf("sssssssss\n");
            // }
            fy.targetCh = t.number;
            fy.targetValue = t.number;
            fy.left = to_string(t.number);
            fy.leftValue = t.number;
            fy.right = to_string(0);
            fy.rightValue = 0;
        }
        else {
            if (t.isCharVar) {
                fy.valueType = CHAR;
                fy.targetCh = t.number;
            }
            else {
                fy.valueType = INT;
                fy.targetValue = t.number;
            }
            fy.left = t.str;
            fy.right = to_string(0);
            fy.rightValue = 0;
        }
        fy.target = tmpName0;
        fy.op = '+';
        fourItems.push_back(fy);
        return tmpName0;
    }
    fy.valueType = INT;
    //int flag1 = 0;
    for (int i = 0; i < postfix.size(); i++) {
        t = postfix.at(i);
        if (t.isOperator) {
            fy.codeType = AssignState;
            switch (t.number) {
                case '+':
                case '-': {
                    PostfixItem it = tmp.at(tmp.size() - 1);
                    if (it.valueType == String) {
                        fy.right = it.str;
                        tmp.pop_back();
                    }
                    else {
                        fy.right = to_string(it.number);
                        fy.rightValue = it.number;
                        //printf("rr%d\n", fy.rightValue);
                        tmp.pop_back();
                    }
                    it = tmp.at(tmp.size() - 1);
                    if (it.valueType == String) {
                        fy.left = it.str;
                        tmp.pop_back();
                    }
                    else {
                        fy.left = to_string(it.number);
                        fy.leftValue = it.number;
                        //printf("ll%d\n", fy.leftValue);
                        tmp.pop_back();
                    }
                    int flag = 0;
                    if (fy.right.at(0) == 'T') {
                        fy.target = fy.right;
                        tmpName0 = fy.target;
                        flag = 1;
                    }
                    if (fy.left.at(0) == 'T') {
                        fy.target = fy.left;
                        tmpName0 = fy.target;
                        flag = 1;
                    }
                    if (flag == 0) {
                        fy.target = generateTmpName(tmpCount++);
                        count++;
                        tmpName0 = fy.target;
                    }
                    fy.op = t.number;
                    fourItems.push_back(fy);
                    PostfixItem item1;
                    item1.valueType = String;
                    item1.str = fy.target;
                    tmp.push_back(item1);}
                break;
                case '*':
                case '/':{
                    PostfixItem it = tmp.at(tmp.size() - 1);
                    if (it.valueType == String) {
                        fy.right = it.str;
                        tmp.pop_back();
                    }
                    else {
                        fy.right = to_string(it.number);
                        fy.rightValue = it.number;
                        tmp.pop_back();
                    }
                    it = tmp.at(tmp.size() - 1);
                    if (it.valueType == String) {
                        fy.left = it.str;
                        tmp.pop_back();
                    }
                    else {
                        fy.left = to_string(it.number);
                        fy.leftValue = it.number;
                        tmp.pop_back();
                    }
                    int flag = 0;
                    if (fy.right.at(0) == 'T') {
                        fy.target = fy.right;
                        tmpName0 = fy.target;
                        flag = 1;
                    }
                    if (fy.left.at(0) == 'T') {
                        fy.target = fy.left;
                        tmpName0 = fy.target;
                        flag = 1;
                    }
                    if (flag == 0) {
                        fy.target = generateTmpName(tmpCount++);
                        count++;
                        tmpName0 = fy.target;
                    }
                    fy.op = t.number;
                    fourItems.push_back(fy);
                    PostfixItem item1;
                    item1.valueType = String;
                    item1.str = fy.target;
                    tmp.push_back(item1);}
                break;
                default:
                    tmp.push_back(t);
            }
        }
        else {
            tmp.push_back(t);
        }

    }
    //tmpCount = 0;
    tmpCount = tmpCount - count + 1;
    //printf("%d %d hh\n", tmpCount,count);
    return tmpName0;
}

void printMidCodeToFile(vector<FourYuanItem> &fourItems) {
    ofstream out(midCodeFileName, ios::out);
    for (int i = 0; i < fourItems.size(); i++) {
        FourYuanItem fy = fourItems.at(i);
        switch (fy.codeType) {
            case VarDef:
                if (fy.valueType == CHAR) {
                    out << "char " + fy.target << endl;
                    //printf("char %s\n", fy.target.c_str());
                }
                else if (fy.valueType == INT) {
                    out << "int " + fy.target << endl;
                    //printf("char %s\n", fy.target.c_str());
                }
                break;
            case AssignArray:
                if (fy.dim == 2) {
                    out << fy.target << "[" + to_string(fy.x) + "][" + to_string(fy.y) + "] = " << fy.left << endl;
                }
                else if (fy.dim == 1) {
                    out << fy.target << "[" + to_string(fy.x) + "] = " << fy.left << endl;
                }
                break;
            case AssignByArray:
                if (fy.dim == 1) {
                    out << fy.target << " = " + fy.arrayName + "[" + fy.left + "]" << endl;
                }
                else if (fy.dim == 2) {
                    out << fy.target << " = " + fy.arrayName + "[" + fy.left + "][" + fy.right + "]" << endl;
                }
                break;
            case AssignState:
                out << fy.target << " = " << fy.left << " " << fy.op << " " << fy.right << endl;
                break;
            case FunctionDef:
                if (fy.funcType == VOID) {
                    out << "void ";
                }
                else if (fy.funcType == RETURNCHAR) {
                    out << "char ";
                }
                else {
                    out << "int ";
                }
                out << fy.target << "()" << endl;
                break;
            case ParamDef:
                if (fy.valueType == INT) {
                    out << "Param int ";
                }
                else {
                    out << "Param char ";
                }
                out << fy.target << endl;
                break;
            case ReadChar:
                out << "Read Char " << fy.target << endl;
                break;
            case ReadInt:
                out << "Read Int " << fy.target << endl;
                break;
            case PrintStr:
            case PrintBeforeExpr:
                out << "Print String " << "\"" << fy.target << "\"" << endl;
                strs.push_back(fy.target);
                break;
            case PrintChar:
                out << "Print Char "
                    << "\'" << fy.target.at(0) << "\"" << endl;
                break;
            case PrintInt:
                out << " Print Int " << fy.target << endl;
                break;
            case PrintIdent:
                out << "Print Ident " << fy.target << endl;
                break;
            case ReturnIdent:
                out << "ret " << fy.target << endl;
                break;
            case ReturnEmpty:
                out << "ret" << endl;
                break;
            case Label:
                out << fy.target << ":" << endl;
                break;
            case BNZ:
                out << "BNZ " << fy.target << endl;
                break;
            case Goto:
                out << "GOTO " << fy.target << endl;
                break;
            case LSSm:
                out << fy.left << " < " << fy.right << endl;
                break;
            case LEQm:
                out << fy.left << " <= " << fy.right << endl;
                break;
            case GREm:
                out << fy.left << " > " << fy.right << endl;
                break;
            case GEQm:
                out << fy.left << " >= " << fy.right << endl;
                break;
            case EQLm:
                out << fy.left << " == " << fy.right << endl;
                break;
            case NEQm:
                out << fy.left << " != " << fy.right << endl;
                break;
            case FunctionCall:
                out << "call " << fy.target << endl;
                break;
            case ValueParamDeliver:
                out << "push " << fy.target << endl;
                break;
            default:
                break;
        }
    }
    out.close();
}

int calculateOff(int baseOffset) {
    return stackBase - top + baseOffset;
}

int calculateMidOff(int baseOffset) {
    return  midVarBaseAddr + baseOffset;
}

string mapReg(string s) {
    string s1 = s.substr(1, s.length());
    //printf("%s dd\n", s1.c_str());
    int num = 0;
    for (int i = 0; i < s1.length(); i++) {
        if (s1.at(i) >= '0' && s1.at(i) <= '9') {
            num = num * 10 + (s1.at(i) - '0');
        }
    }
    //printf("dddd %d\n", num);
    if (num > 7) {
        num = num % 8;
        //printf("s%d\n", num);
        return "$s" + to_string(num);
    }
    
    return "$t" + to_string(num);
}

void turnToMips(vector<FourYuanItem> &fourItems) {
    ofstream out(mipsCodeFileName, ios::out);
    strMemSize = 0;
    out << ".data" << endl;
    out << "str0:    .asciiz    \"\\n\"" << endl;
    strMemSize += 2;
    for (int i = 0; i < strs.size(); i++) {
        out << "str" + to_string(i + 1) + ":    .asciiz    \"" + strs.at(i) + "\"" << endl;
        strMemSize += strs.at(i).size() + 1;
    }
    strMemSize = strMemSize + 4 - (strMemSize % 4);
    paraVarBaseAddr = strMemSize + 4 + dataBaseAddr;
    paraTopAddr = paraVarBaseAddr;
    varBaseAddr = paraVarBaseAddr + 100;
    varTopAddr = varBaseAddr;  
    out << ".text" << endl;
    out << "li $k0," << midVarBaseAddr << endl;
    out << "j main" << endl;
    for (int i = 0; i < fourItems.size(); i++) {
        int offset = 0;
        FourYuanItem fy = fourItems.at(i);
        switch (fy.codeType) {
            case AssignState:{
                string l, r, t;
                int ll, rr, tt;
                bool lIsCon = false, rIsCon = false, tIsCon = false;
                //out << fy.target << " = " << fy.left << " " << fy.op << " " << fy.right << endl;
                if (fy.left.at(0) == 'G') {
                    map<string, int>::iterator iter;
                    iter = varToStack.find(fy.left);
                    if (iter != varToStack.end()) {
                        offset = iter->second;
                    }
                    offset = calculateOff(offset);
                    //lw t8,offset(sp)
                    l = "$t8";
                    out << "lw " << l + "," + to_string(offset) + "($sp)" << endl;
                }
                else if (fy.left.at(0) == 'T') {
                    if (strcmp(fy.left.c_str(), "T7") > 0) {
                        map<string, int>::iterator iter;
                        iter = midVarToSpace.find(fy.left);
                        if (iter != midVarToSpace.end()) {
                            offset = iter->second;
                        }
                        l = "$t8";
                        out << "lw " << l + "," + to_string(offset) + "($k0)" << endl;
                    }
                    else {
                        l = mapReg(fy.left);
                    }
                    
                }
                else {
                    ll = fy.leftValue;
                    //printf("l%d\n", fy.leftValue);
                    lIsCon = true;
                }
                if (fy.right.at(0) == 'G') {
                    map<string, int>::iterator iter;
                    iter = varToStack.find(fy.right);
                    if (iter != varToStack.end()) {
                        offset = iter->second;
                    }
                    offset = calculateOff(offset);
                    r = "$t9";
                    out << "lw " << r + "," + to_string(offset) + "($sp)" << endl; //lw t9,offset(sp)
                }
                else if (fy.right.at(0) == 'T') {
                     if (strcmp(fy.right.c_str(), "T7") > 0) {
                        map<string, int>::iterator iter;
                        iter = midVarToSpace.find(fy.right);
                        if (iter != midVarToSpace.end()) {
                            offset = iter->second;
                        }
                        r = "$t9";
                        out << "lw " << r + "," + to_string(offset) + "($k0)" << endl;
                    }
                    else {
                        r = mapReg(fy.right);
                    }
                }
                else {
                    rr = fy.rightValue;
                    //printf("r%d\n", fy.rightValue);
                    rIsCon = true;
                }
                if (fy.target.at(0) == 'G') {
                    map<string, int>::iterator iter;
                    iter = varToStack.find(fy.target);
                    if (iter != varToStack.end()) {
                        offset = iter->second;
                        offset = calculateOff(offset);
                    }
                    else {
                        out << "addi " << "$sp,$sp," + to_string(-4) << endl;
                        top = top - 4;
                        varToStack.insert(map<string, int>::value_type (fy.target, top - stackBase));
                        offset = 0;
                    }
                    //addi t8,t8,t9 addi t8,t8,1 addi t8,t9,1
                    //sw t8,offse(sp)
                    t = "$t8";
                    if (lIsCon == true && rIsCon == true) {
                        switch (fy.op) {
                            case '+':
                                tt = rr + ll;
                                break;
                            case '-':
                                tt = ll - rr;
                                break;
                            case '*':
                                tt = ll * rr;
                                break;
                            case '/':
                                tt = ll / rr;
                                break;
                            default:
                                break;
                        }
                        out << "li " << t << "," + to_string(tt) << endl;
                        
                    }
                    else if (lIsCon == true && rIsCon == false) {
                        switch (fy.op) {
                            case '+':
                                out << "addi " << t << "," + r + "," + to_string(ll) << endl;
                                break;
                            case '-':
                                out << "li $t8," << to_string(ll) << endl;
                                out << "sub " << t << ",$t8," + r << endl;
                                break;
                            case '*':
                                out << "mul " << t << "," + r + "," + to_string(ll) << endl;
                                break;
                            case '/':
                                out << "li $t8," << to_string(ll) << endl;
                                out << "div " << t << ",$t8," + r << endl;
                                //out << "div " << t << "," + r + "," + to_string(ll) << endl;
                                break;
                            default:
                                break;
                        }
                        //out << "addi " << t << "," + r + "," + to_string(ll) << endl;
                    }
                    else if (lIsCon == false && rIsCon == true) {
                        switch (fy.op) {
                            case '+':
                                out << "addi " << t << "," + l + "," + to_string(rr) << endl;
                                break;
                            case '-':
                                out << "addi " << t << "," + l + "," + to_string(-rr) << endl;
                                break;
                            case '*':
                                out << "mul " << t << "," + l + "," + to_string(rr) << endl;
                                break;
                            case '/':
                                out << "div " << t << "," + l + "," + to_string(rr) << endl;
                                //out << "ddd" << endl;
                                break;
                            default:
                                break;
                        }
                        //out << "addi " << t << "," + l + "," + to_string(rr) << endl;
                    }
                    else {
                        switch (fy.op) {
                            case '+':
                                out << "add " << t << "," + l + "," + r << endl;
                                break;
                            case '-':
                                out << "sub " << t << "," + l + "," + r << endl;
                                break;
                            case '*':
                                out << "mul " << t << "," + l + "," + r << endl;
                                break;
                            case '/':
                                out << "div " << l + "," + r << endl;
                                out << "mflo " << t << endl;
                                break;
                            default:
                                break;
                        }
                        //out << "add " << t << "," + l + "," + r << endl;
                    }
                    out << "sw " << t + "," + to_string(offset) + "($sp)" << endl;
                }
                else if (fy.target.at(0) == 'T') {
                    //printf("%s\n", fy.target.c_str());

                    int isMap = 0;
                    if (strcmp(fy.target.c_str(), "T7") > 0) {
                        isMap = 1;
                        map<string, int>::iterator iter;
                        iter = midVarToSpace.find(fy.target);
                        if (iter != midVarToSpace.end()) {
                            offset = iter->second;
                        }
                        else {
                            //out << "li " << "$k1," + to_string(cuMidVarAddr) << endl;
                            offset = cuMidVarAddr - midVarBaseAddr;
                            midVarToSpace.insert(map<string, int>::value_type (fy.target, offset));
                            
                            cuMidVarAddr += 4;
                        }
                        t = "$t8";
                        
                    }
                    else {
                        t = mapReg(fy.target);
                    }
                    //printf("%s\n", t.c_str());
                    if (lIsCon == true && rIsCon == true) {
                        switch (fy.op) {
                            case '+':
                                tt = rr + ll;
                                break;
                            case '-':
                                tt = ll - rr;
                                break;
                            case '*':
                                tt = ll * rr;
                                break;
                            case '/':
                                tt = ll / rr;
                                break;
                            default:
                                break;
                        }
                        out << "li " << t << "," + to_string(tt) << endl;
                        
                    }
                    else if (lIsCon == true && rIsCon == false) {
                        switch (fy.op) {
                            case '+':
                                out << "addi " << t << "," + r + "," + to_string(ll) << endl;
                                break;
                            case '-':
                                out << "li $t8," << to_string(ll) << endl;
                                out << "sub " << t << ",$t8," + r << endl;
                                //out << "addi " << t << "," + r + "," + to_string(-ll) << endl;
                                break;
                            case '*':
                                out << "mul " << t << "," + r + "," + to_string(ll) << endl;
                                break;
                            case '/':
                                out << "li $t8," << to_string(ll) << endl;
                                out << "div " << t << ",$t8," + r << endl;
                                //out << "div " << t << "," + r + "," + to_string(ll) << endl;
                                break;
                            default:
                                break;
                        }
                        //out << "addi " << t << "," + r + "," + to_string(ll) << endl;
                    }
                    else if (lIsCon == false && rIsCon == true) {
                        switch (fy.op) {
                            case '+':
                                out << "addi " << t << "," + l + "," + to_string(rr) << endl;
                                break;
                            case '-':
                                out << "addi " << t << "," + l + "," + to_string(-rr) << endl;
                                break;
                            case '*':
                                out << "mul " << t << "," + l + "," + to_string(rr) << endl;
                                // out << "mult " <<  l + "," + r << endl;
                                // out << "mflo " <<  t  << endl;
                                break;
                            case '/':
                                out << "div " << t << "," + l + "," + to_string(rr) << endl;
                                break;
                            default:
                                break;
                        }
                        //out << "addi " << t << "," + l + "," + to_string(rr) << endl;
                    }
                    else {
                        switch (fy.op) {
                            case '+':
                                out << "add " << t << "," + l + "," + r << endl;
                                break;
                            case '-':
                                out << "sub " << t << "," + l + "," + r << endl;
                                break;
                            case '*':
                                out << "mul " << t << "," + l + "," + r << endl;
                                break;
                            case '/':
                                out << "div " << l + "," + r << endl;
                                out << "mflo " << t << endl;
                                break;
                            default:
                                break;
                        }
                    }
                    if (isMap == 1) {
                        out << "sw " << t + "," + to_string(offset) + "($k0)" << endl;
                    }
                   
                    // if (lIsCon == true && rIsCon == true) {
                    //     out << "li " << t << "," + to_string(rr + ll) << endl;
                    // }
                    // else if (lIsCon == true && rIsCon == false) {
                    //     out << "addi " << t << "," + r + "," + to_string(ll) << endl;
                    // }
                    // else if (lIsCon == false && rIsCon == true) {
                    //     out << "addi " << t << "," + l + "," + to_string(rr) << endl;
                    // }
                    // else {
                    //     out << "add " << t << "," + l + "," + r << endl;
                    // }
                }}
                break;
            case FunctionDef:
                out << fy.target << ":" << endl;
                break;
            case ParamDef:
                if (fy.valueType == INT) {
                    //out << "Param int ";
                }
                else {
                    //out << "Param char ";
                }
                //out << fy.target << endl;
                break;
            case ReadChar:{
                //out << "Read Char " << fy.target << endl;
                //li $v0,5
                //syscall
                out << "li $v0,12" << endl;
                out << "syscall" << endl;
                map<string, int>::iterator iter;
                iter = varToStack.find(fy.target);
                if (iter != varToStack.end()) {
                    offset = iter->second;
                    offset = calculateOff(offset);
                }
                else {
                    out << "addi " << "$sp,$sp," + to_string(-4) << endl;
                    top = top - 4;
                    varToStack.insert(map<string, int>::value_type (fy.target, top - stackBase));
                    offset = 0;
                }
                out << "sw " << "$v0," + to_string(offset) + "($sp)" << endl;}
                break;
            case ReadInt:{
               // out << "Read Int " << fy.target << endl;
                out << "li $v0,5" << endl;
                out << "syscall" << endl;
                map<string, int>::iterator iter;
                iter = varToStack.find(fy.target);
                if (iter != varToStack.end()) {
                    offset = iter->second;
                    offset = calculateOff(offset);
                }
                else {
                    out << "addi " << "$sp,$sp," + to_string(-4) << endl;
                    top = top - 4;
                    varToStack.insert(map<string, int>::value_type (fy.target, top - stackBase));
                    offset = 0;
                }
                out << "sw " << "$v0," + to_string(offset) + "($sp)" << endl;}
                break;
            case PrintStr:{
                //out << "Print String " << "\"" << fy.target << "\"" << endl;
                string str1;
                for (int ii = 0; ii < strs.size(); ii++) {
                    if (strs.at(ii) == fy.target) {
                        str1 = "str" + to_string(ii + 1);
                    }
                }
                out << "la $a0," << str1 << endl;
                out << "li $v0,4" << endl;
                out << "syscall" << endl;
                out << "la $a0,str0" << endl;
                out << "li $v0,4" << endl;
                out << "syscall" << endl;}
                break;
            case PrintBeforeExpr:{
                //out << "Print String " << "\"" << fy.target << "\"" << endl;
                string str1;
                for (int ii = 0; ii < strs.size(); ii++) {
                    if (strs.at(ii) == fy.target) {
                        str1 = "str" + to_string(ii + 1);
                    }
                }
                out << "la $a0," << str1 << endl;
                out << "li $v0,4" << endl;
                out << "syscall" << endl;
                }
                break;
            case PrintChar:
                //out << "Print Char " << "\'" << fy.target.at(0) << "\"" << endl;
                
                break;
            case PrintInt:
                //out << " Print Int " << fy.target << endl;
                break;
            case PrintIdent:{
                //out << "Print Ident " << fy.target << endl;
                if (strcmp(fy.target.c_str(), "T1") > 0) {
                    map<string, int>::iterator iter;
                    iter = midVarToSpace.find(fy.target);
                    if (iter != midVarToSpace.end()) {
                            offset = iter->second;
                    } 
                   
                    out << "lw " << "$a0," + to_string(offset) + "($k0)" << endl;
                        
                }
                else {
                    out << "move $a0," << mapReg(fy.target) << endl;
                }
                
                if (fy.valueType == CHAR) {
                    out << "li $v0,11" << endl;
                }
                else {
                    out << "li $v0,1" << endl;
                }
                out << "syscall" << endl;
                out << "la $a0,str0" << endl;
                out << "li $v0,4" << endl;
                out << "syscall" << endl;}
                break;
            case Label:
                out << fy.target << ":" << endl;
                break;
            case LSSm: {
                if (fy.right.at(0) == 'G') {
                    map<string, int>::iterator iter;
                    iter = varToStack.find(fy.right);
                    if (iter != varToStack.end()) {
                        offset = iter->second;
                    }
                    offset = calculateOff(offset);
                    // r = "$t9";
                    // out << "lw " << r + "," + to_string(offset) + "($sp)" << endl; //lw t9,offset(sp)
                }
                else if (fy.right.at(0) == 'T') {
                     if (strcmp(fy.right.c_str(), "T7") > 0) {
                        map<string, int>::iterator iter;
                        iter = midVarToSpace.find(fy.right);
                        if (iter != midVarToSpace.end()) {
                            offset = iter->second;
                        }
                        // r = "$t9";
                        // out << "lw " << r + "," + to_string(offset) + "($k0)" << endl;
                    }
                    else {
                        // r = mapReg(fy.right);
                    }
                }
                FourYuanItem fyPre = fourItems.at(++i);
                out << "blt " << fy.left << fy.right << fyPre.target << endl;}
                break;
            case LEQm: {
                FourYuanItem fyPre = fourItems.at(++i);
                out << "ble " << fy.left + "," << fy.right + "," << fyPre.target << endl;}
                break;
            case GREm: {
                FourYuanItem fyPre = fourItems.at(++i);
                out << "bgt " << fy.left + "," << fy.right + "," << fyPre.target << endl;}
                break;
            case GEQm: {
                FourYuanItem fyPre = fourItems.at(++i);
                out << "bge " << fy.left + "," << fy.right + "," << fyPre.target << endl;}
                break;
            case EQLm: {
                FourYuanItem fyPre = fourItems.at(++i);
                out << "beq " << fy.left + "," << fy.right + "," << fyPre.target << endl;}
                break;
            case NEQm: {
                FourYuanItem fyPre = fourItems.at(++i);
                out << "bne " << fy.left + "," << fy.right + "," << fyPre.target << endl;}
                break;
            case Goto: 
                out << "j " << fy.target << endl;
                break;
            default:
                break;
        }
    }
    out.close();
}