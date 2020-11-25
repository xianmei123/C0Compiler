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
map<string, int> varToAddr;
map<string, int> tmpToAddr;
map<string, int> globalToAddr;
map<string, int> funcVarSize;
map<string, int> funcTmpSize;

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
int tmpSize = 0;

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
    if (it.valueType == CHAR && (it.number == '+' || it.number == '-') && it.isOperator == true) {
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
                        
                        PostfixItem t0 = tmp.at(tmp.size() - 1);
                        if (t0.number != '(') {
                            postfix.push_back(t0);
                            tmp.pop_back();
                        }
                        else {
                            break;
                        }
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
                case ')':{
                    while (tmp.at(tmp.size() - 1).number != '(') {
                        PostfixItem t0 = tmp.at(tmp.size() - 1);
                        postfix.push_back(t0);
                        tmp.pop_back();
                    }
                    
                    tmp.pop_back();}
                    break;
                case '(':
                    tmp.push_back(t);
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
        //printf("ddddd %s \n", t.str.c_str());
        t = postfix.at(0);
        if (t.str.size() > 0 && t.str.at(0) == 'T') {
            fy.left = t.str;
            fy.right = to_string(0);
            fy.rightValue = 0;
            fy.target = t.str;
            printf("target : %s \n", t.str.c_str());
            return t.str;
        }
        printf("target : %s \n", t.str.c_str());
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
    int gOff = 0;
    int lOff = 12;
    int tOff = 4;
    string func = " ";
    int size = 0;
    for (int i = 0; i < fourItems.size(); i++) {
        FourYuanItem fy = fourItems.at(i);
        switch (fy.codeType) {
            case VarDef:{
                if (fy.dim == 0) {
                    size = 4;
                }
                else if (fy.dim == 1) {
                    size = fy.x * 4;
                }
                else {
                    size = fy.x * fy.y * 4;
                }
                if (fy.target.at(0) == 'G') {
                    globalToAddr.insert(map<string, int>::value_type(fy.target, gOff));
                    //printf("%s %d\n", fy.target.c_str(), gOff);
                    gOff = gOff + size;
                    //printf("%s %d\n", fy.target.c_str(), gOff);
                }
                else if (fy.target.at(0) == 'L') {
                    varToAddr.insert(map<string, int>::value_type(fy.target, lOff));
                    
                    lOff = lOff + size;
                }
                //off = off + size;
                if (fy.valueType == CHAR) {
                    out << "char " + fy.target << endl;
                    //printf("char %s\n", fy.target.c_str());
                }
                else if (fy.valueType == INT) {
                    out << "int " + fy.target << endl;
                    //printf("char %s\n", fy.target.c_str());
                }}
                break;
            case AssignArray:
                if (fy.dim == 2) {
                    out << fy.arrayName << "[" + fy.left + "][" + fy.right + "] = " << fy.target << endl;
                }
                else if (fy.dim == 1) {
                    out << fy.arrayName << "[" + fy.left + "] = " << fy.target << endl;
                }
                break;
            case AssignByArray:
                if (fy.target.at(0) == 'T') {
                    tmpToAddr.insert(map<string, int>::value_type(fy.target, tOff));
                    tOff += 4;
                }
                if (fy.dim == 1) {
                    out << fy.target << " = " + fy.arrayName + "[" + fy.left + "]" << endl;
                }
                else if (fy.dim == 2) {
                    out << fy.target << " = " + fy.arrayName + "[" + fy.left + "][" + fy.right + "]" << endl;
                }
                break;
            case AssignState:
                if (fy.target.at(0) == 'T') {
                    tmpToAddr.insert(map<string, int>::value_type(fy.target, tOff));
                    tOff += 4;
                }
                out << fy.target << " = " << fy.left << " " << fy.op << " " << fy.right << endl;
                break;
            case Finish:
                funcVarSize.insert(map<string, int>::value_type(func, lOff - 4));
                funcTmpSize.insert(map<string, int>::value_type(func, tOff - 4));
                break;
            case FunctionDef:
                funcVarSize.insert(map<string, int>::value_type(func, lOff - 4));
                funcTmpSize.insert(map<string, int>::value_type(func, tOff - 4));
                gOff = 0;
                lOff = 12;
                tOff = 4;
                func = fy.target;
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
                varToAddr.insert(map<string, int>::value_type(fy.target, lOff));
                printf("para: %s off:%d \n", fy.target.c_str(), lOff);
                lOff += 4;
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
    string s1 = s.substr(1, s.size());
    //printf("%s dd\n", s1.c_str());
    int num = 0;
    for (int i = 0; i < s1.size(); i++) {
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

string mapToVar(ofstream& out, string name, int flag) {
    int offset = 0;
    string reg = "";
    if (flag == 0) {
        reg = "$t8";
    }
    else if (flag == 1) {
        reg = "$t9";
    }
    map<string, int>::iterator iter;
    if (name.size() > 0 && name.at(0) == 'G') {
        iter = globalToAddr.find(name);
        if (iter != globalToAddr.end()) {
            offset = iter->second;
        }
        if (flag == 2) {
            out << "sw " << "$t8," << offset << "($k0)" << endl;
        }
        else {
            out << "lw " << reg + "," << offset << "($k0)" << endl;
        }
        return reg;
    }
    else if (name.size() > 0 && name.at(0) == 'L') {
        iter = varToAddr.find(name);
        if (iter != varToAddr.end()) {
            offset = iter->second;
        }
        if (flag == 2) {
            out << "sw " << "$t8," << -offset << "($fp)" << endl;
        }
        else {
            out << "lw " << reg + "," << -offset << "($fp)" << endl;
            printf("para :%s, off:%d\n", name.c_str(), offset);
        }
        return reg;
    }
    else if (name.size() > 0 && name.at(0) == 'T') {
        iter = tmpToAddr.find(name);
        if (iter != tmpToAddr.end()) {
            offset = iter->second;
        }
        /*else {
            out << "sw " << reg + "," << -tmpSize - 4 << "($sp)" << endl;
            tmpSize = tmpSize + 4;
            tmpToAddr.insert(map<string, int>::value_type(name, tmpSize));
        }*/
        if (flag == 2) {
            out << "sw " << "$t8," << -offset << "($sp)" << endl;
        }
        else {
            out << "lw " << reg + "," << -offset << "($sp)" << endl;
        }
        
        return reg;
    }
    
}

void assignStatMips(ofstream &out, FourYuanItem &fy) {
    int offset = 0;
    string l, r, t;
    int ll, rr, tt;
    bool lIsCon = false, rIsCon = false, tIsCon = false;
    //out << fy.target << " = " << fy.left << " " << fy.op << " " << fy.right << endl;
    if (fy.left.size() > 0 && (fy.left.at(0) == 'G' || fy.left.at(0) == 'L' || fy.left.at(0) == 'T')) {
        l = mapToVar(out, fy.left, 0);
    }
    else if (fy.left == "RET") {
        l = "$v1";
    }
    else {
        ll = fy.leftValue;
        //printf("l%d\n", fy.leftValue);
        lIsCon = true;
    }
    if (fy.right.size() > 0 && (fy.right.at(0) == 'G' || fy.right.at(0) == 'L' || fy.right.at(0) == 'T')) {
        r = mapToVar(out, fy.right, 1);
    }
    else if (fy.right == "RET") {
        r = "$v1";
    }
    else {
        rr = fy.rightValue;
        //printf("r%d\n", fy.rightValue);
        rIsCon = true;
    }
    if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
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
        mapToVar(out, fy.target, 2);
        
    }
}

int calculateArrayOff(FourYuanItem &fy, ofstream &out) {
    string type;
    if (fy.arrayName.size() > 0 && fy.arrayName.at(0) == 'L') {
        type = "sub ";
    }
    else {
        type = "add ";
    }
    string name = fy.arrayName;
    map<string, int>::iterator iter;
    int offset = 0;
    iter = globalToAddr.find(name);
    if (iter != globalToAddr.end()) {
        offset = iter->second;
    }
    iter = varToAddr.find(name);
    if (iter != varToAddr.end()) {
        offset = iter->second;
        //printf("offset %d\n", offset);
    }
    printf("str %s offset %d\n", name.c_str(), offset);
    string l, r, t;
    int ll, rr, tt;
    bool lIsCon = false, rIsCon = false, tIsCon = false;
    if (fy.left.size() > 0 && (fy.left.at(0) == 'G' || fy.left.at(0) == 'L' || fy.left.at(0) == 'T')) {
        l = mapToVar(out, fy.left, 0);
    }
    else if (fy.left == "RET") {
        l = "$v1";
    }
    else {
        ll = fy.leftValue;
        //printf("l%d\n", fy.leftValue);
        lIsCon = true;
    }
    if (fy.right.size() > 0 && (fy.right.at(0) == 'G' || fy.right.at(0) == 'L' || fy.right.at(0) == 'T')) {
        r = mapToVar(out, fy.right, 1);
    }
    else if (fy.right == "RET") {
        r = "$v1";
    }
    else {
        rr = fy.rightValue;
        //printf("r%d\n", fy.rightValue);
        rIsCon = true;
        
    }
    //if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
    //    t = mapToVar(out, fy.target, 0);
    //}
    //else if (fy.target == "RET") {
    //    t = "$v1";
    //}
    //else {
    //    tt = fy.targetValue;
    //    //printf("r%d\n", fy.rightValue);
    //    tIsCon = true;
    //    out << "li $t8," << tt << endl;
    //    t = "$t8";
    //}
    
    string base;
    if (fy.arrayName.size() > 0 && fy.arrayName.at(0) == 'G') {
        base = "$k0";
    }
    else if (fy.arrayName.size() > 0 && fy.arrayName.at(0) == 'L') {
        base = "$fp";
    }
    if (fy.dim == 1) {
        if (lIsCon == true) {
            
            if (fy.arrayName.at(0) == 'G') {
                offset = offset + fy.leftValue * 4;
            }
            else {
                offset = -offset - fy.leftValue * 4;
            }
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    t = mapToVar(out, fy.target, 1);
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << offset << "(" + base + ")" << endl;
                
            }
            else {
                out << "lw $t8," << offset << "(" + base + ")" << endl;
                
                mapToVar(out, fy.target, 2);
            }
        }
        else {
            out << "mul " << l << "," + l + "," << 4 << endl;
            out << "addi " << l << "," + l + "," << offset << endl;
            out << type << l << "," + base + "," << l << endl;
            
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    t = mapToVar(out, fy.target, 1);
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << "0(" + l + ")" << endl;
            }
            else {
                out << "lw $t8," << "0(" + l + ")" << endl;
                mapToVar(out, fy.target, 2);
            }
        }
    }
    else if (fy.dim == 2) {
        if (lIsCon == true && rIsCon == true) {
            if (fy.arrayName.at(0) == 'G') {
                offset = offset + fy.leftValue * fy.y * 4 + fy.rightValue * 4;
            }
            else {
                offset = -offset - fy.leftValue * fy.y * 4 - fy.rightValue * 4;
            }
            
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    t = mapToVar(out, fy.target, 1);
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << offset << "(" + base + ")" << endl;
            }
            else {
                out << "lw $t8," << offset << "(" + base + ")" << endl;
                mapToVar(out, fy.target, 2);
            }
            //out << type + t + "," << offset << "(" + base + ")" << endl;
        }
        else if (lIsCon == true && rIsCon == false) {
            offset = offset + fy.leftValue * fy.y * 4;
            
            out << "mul " << r << "," + r + "," << 4 << endl;
            out << "addi " << r << "," + r + "," << offset << endl;
            out << type << r << "," + base + "," << r << endl;
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    t = mapToVar(out, fy.target, 1);
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << "0(" + r + ")" << endl;
            }
            else {
                out << "lw $t8," << "0(" + r + ")" << endl;
                mapToVar(out, fy.target, 2);
            }
            //out << type + r + "," << "0(" + base + ")" << endl;
        }
        else if (lIsCon == false && rIsCon == true) {
            
            offset = offset + fy.rightValue * 4;
            int si = fy.y * 4;
            out << "mul " << l << "," + l + "," << si << endl;
            out << "addi " << l << "," + l + "," << offset << endl;
            out << type << l << "," + base + "," << l << endl;
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    t = mapToVar(out, fy.target, 1);
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << "0(" + l + ")" << endl;
            }
            else {
                out << "lw $t8," << "0(" + l + ")" << endl;
                mapToVar(out, fy.target, 2);
            }
            //out << type + l + "," << "0(" + base + ")" << endl;
        }
        else {
            int si = fy.y * 4;
            out << "mul " << l << "," + l + "," << si << endl;
            out << "mul " << r << "," + r + "," << 4 << endl;
            out << "addi " << l << "," + l + "," << offset << endl;
            out << "add " << l << "," + l + "," << r << endl;
            out << type << l << "," + base + "," << l << endl;
            //out << type + l + "," << "0(" + base + ")" << endl;
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    t = mapToVar(out, fy.target, 1);
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << "0(" + l + ")" << endl;
            }
            else {
                out << "lw $t8," << "0(" + l + ")" << endl;
                mapToVar(out, fy.target, 2);
            }
        }
        
    }
    return offset;
}

void turnToMips(vector<FourYuanItem> &fourItems) {
    ofstream out(mipsCodeFileName, ios::out);
    string func;
    int paraTop = 8;
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
    out << "li $k0," << varBaseAddr << endl;
    out << "move $fp,$sp" << endl;
    map<string, int>::iterator iter;
    int size = 0;
    iter = funcVarSize.find("main");
    if (iter != funcVarSize.end()) {
        size = iter->second;
    }
    out << "addi $sp,$sp," << -size - 8 << endl;
    //out << "j main" << endl;
    for (int i = 0; i < fourItems.size(); i++) {
        int offset = 0;
        FourYuanItem fy = fourItems.at(i);
        switch (fy.codeType) {
            case AssignArray:
            case AssignByArray:
                calculateArrayOff(fy, out);
                break;
            case AssignState:
                assignStatMips(out, fy);
                break;
            case FunctionDef:
                func = fy.target;
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
                out << "move " << "$t8,$v0" << endl;
                mapToVar(out, fy.target, 2);}
                break;
            case ReadInt:{
               // out << "Read Int " << fy.target << endl;
                out << "li $v0,5" << endl;
                out << "syscall" << endl;
                out << "move " << "$t8,$v0" << endl;
                mapToVar(out, fy.target, 2); }
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
                string t;
                t = mapToVar(out, fy.target, 0);
                out << "move $a0," << t << endl;
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
                break;//blt,blr,bgt,bge,beq,bne
            case LSSm: 
            case LEQm: 
            case GREm: 
            case GEQm: 
            case EQLm: 
            case NEQm: {
                string type;
                if (fy.codeType == LSSm) {
                    type = "bge ";
                }
                else if (fy.codeType == LEQm) {
                    type = "bgt ";
                }
                else if (fy.codeType == GREm) {
                    type = "ble ";
                }
                else if (fy.codeType == GEQm) {
                    type = "blt ";
                }
                else if (fy.codeType == EQLm) {
                    type = "bne ";
                }
                else if (fy.codeType == NEQm) {
                    type = "beq ";
                }
                string l = mapToVar(out, fy.left, 0);
                string r;
                if (fy.right.size() > 0 && (fy.right.at(0) == 'G' || fy.right.at(0) == 'L' || fy.right.at(0) == 'T')) {
                    r = mapToVar(out, fy.right, 1);
                }
                else {
                    r = fy.right;
                }
                FourYuanItem fyPre = fourItems.at(++i);
                out << type << l + "," << r + "," << fyPre.target << endl; }
                break;
            case Goto: 
                out << "j " << fy.target << endl;
                break;
            case ValueParamDeliver: {
                map<string, int>::iterator iter;
                int size = 0;
                iter = funcTmpSize.find(func);
                if (iter != funcTmpSize.end()) {
                    size = iter->second;
                }
                paraTop += 4;
                string t;
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    t = mapToVar(out, fy.target, 0);
                }
                else {
                    out << "li $t8," << fy.target << endl;
                    t = "$t8";
                }
                out << "sw " << t << "," << -paraTop - size << "($sp)" << endl; }
                break;
            case FunctionCall: {
                map<string, int>::iterator iter;
                int size1 = 0, size2 = 0;
                iter = funcTmpSize.find(func);
                if (iter != funcTmpSize.end()) {
                    size1 = iter->second;
                }
                out << "sw $fp," << -size1 - 4 << "($sp)" << endl;
                out << "sw $ra," << -size1 - 8 << "($sp)" << endl;
                out << "addi $fp,$sp," << -size1  << endl;
                iter = funcVarSize.find(fy.target);
                if (iter != funcVarSize.end()) {
                    size2 = size1 + iter->second;
                }
                out << "addi $sp,$sp," << -size2 - 8 << endl;
                out << "jal " + fy.target << endl;
                if (fy.target == "main") {
                    out << "li $v0,10" << endl;
                    out << "syscall" << endl;
                } 
                else {
                    out << "addi $sp,$sp," << size2 + 8 << endl;
                    out << "lw $fp," << -size1 - 4 << "($sp)" << endl;
                    out << "lw $ra," << -size1 - 8 << "($sp)" << endl;
                }
                paraTop = 8; }
                break;
            case ReturnIdent: {
                string t;
                t = mapToVar(out, fy.target, 0);
                out << "move $v1," << t << endl;
                out << "jr $ra" << endl; }
                break;
            case ReturnEmpty:
                out << "jr $ra" << endl; 
                break;
            default:
                break;
        }
    }
    out.close();
}