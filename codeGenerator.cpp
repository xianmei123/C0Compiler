#include <cstring>
#include "codeGenerator.h"
#include "type.h"
#include "opt.h"
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
map<string, vector<int>> tmpse;
vector<Block> blocks;
map<string, string> tmpToReg;

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
                    int op = t.number;
                    bool rIsCon = false, lIsCon = false;
                    PostfixItem it = tmp.at(tmp.size() - 1);
                    if (it.valueType == String) {
                        fy.right = it.str;
                        tmp.pop_back();
                    }
                    else {
                        rIsCon = true;
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
                        lIsCon = true;
                        fy.left = to_string(it.number);
                        fy.leftValue = it.number;
                        //printf("ll%d\n", fy.leftValue);
                        tmp.pop_back();
                    }
                    if (lIsCon == true && rIsCon == true) {
                        PostfixItem item1;
                        item1.valueType = INT;
                        if (op == '+') {
                            item1.number = fy.rightValue + fy.leftValue;
                        }
                        else {
                            item1.number = fy.leftValue - fy.rightValue;
                        }
                        item1.str = to_string(item1.number);
                        tmp.push_back(item1);
                        break;
                    }
                    int flag = 0;
                    if (fy.left.at(0) == 'T') {
                        fy.target = fy.left;
                        tmpName0 = fy.target;
                        flag = 1;
                    }
                    if (fy.right.at(0) == 'T') {
                        fy.target = fy.right;
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
                    int op = t.number;
                    bool rIsCon = false, lIsCon = false;
                    PostfixItem it = tmp.at(tmp.size() - 1);
                    if (it.valueType == String) {
                        fy.right = it.str;
                        tmp.pop_back();
                    }
                    else {
                        rIsCon = true;
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
                        lIsCon = true;
                        fy.left = to_string(it.number);
                        fy.leftValue = it.number;
                        tmp.pop_back();
                    }
                    if (lIsCon == true && rIsCon == true) {
                        PostfixItem item1;
                        item1.valueType = INT;
                        if (op == '*') {
                            item1.number = fy.rightValue * fy.leftValue;
                        }
                        else {
                            item1.number = fy.leftValue / fy.rightValue;
                        }
                        item1.str = to_string(item1.number);
                        tmp.push_back(item1);
                        break;
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
    if (tmp.size() == 1 && tmp.at(0).valueType == INT) {
        tmpName0 = generateTmpName(tmpCount++);
        FourYuanItem fy;
        fy.codeType = AssignState;
        fy.left = tmp.at(0).str;
        fy.leftValue = tmp.at(0).number;
        fy.right = "0";
        fy.rightValue = 0;
        fy.targetValue = fy.leftValue;
        fy.op = '+';
        fy.target = tmpName0;
        fourItems.push_back(fy);
    }
    //tmpCount = 0;
    //tmpCount = tmpCount - count + 1;
    //printf("%d %d hh\n", tmpCount,count);
    return tmpName0;
}

void optDag() {
    for (int i = 0; i < blocks.size(); i++) {
        vector<FourYuanItem> items;
        vector<FourYuanItem> tmp;
        items = blocks.at(i).getCodes();
        for (int j = 0; j < items.size(); j++) {
            FourYuanItem fy = items.at(j);
            
            int count = 0;
            if (fy.codeType == AssignState) {
                vector<DagNode> nodes;
                map<string, int> str2Node;
                
               
                vector<string> vars;
                map<string, vector<string>> needAssign;
                while (fy.codeType == AssignState) {
                    string l = fy.left;
                    string r = fy.right;
                    char op = fy.op;
                    DagNode d;
                    int lIndex = -1, rIndex = -1;
                    map<string, int>::iterator iter;
                    iter = str2Node.find(l);
                    if (iter != str2Node.end()) {
                        lIndex = iter->second;
                        //l = nodes.at(lIndex).name;
                    }
                    else {
                        DagNode node1;
                        node1.name = l;
                        node1.lIndex = -1;
                        node1.rIndex = -1;
                        node1.op = ' ';
                        nodes.push_back(node1);
                        lIndex = count;
                        str2Node.insert(map<string, int>::value_type(l, count++));
                    }
                    iter = str2Node.find(r);
                    if (iter != str2Node.end()) {
                        rIndex = iter->second;
                        //r = nodes.at(rIndex).name;
                    }
                    else {
                        DagNode node1;
                        node1.name = r;
                        node1.lIndex = -1;
                        node1.rIndex = -1;
                        node1.op = ' ';
                        nodes.push_back(node1);
                        rIndex = count;
                        str2Node.insert(map<string, int>::value_type(r, count++));
                    }
                    int flag = 0;
                    for (int k = 0; k < nodes.size(); k++) {
                        DagNode node1 = nodes.at(k);
                        if (node1.op == op && node1.lIndex == lIndex && node1.rIndex == rIndex) {
                            nodes[k].name = fy.target;
                            str2Node[fy.target] = k;
                            //str2Node.insert(map<string, int>::value_type(fy.target, k));
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 0) {
                        if (fy.target.at(0) != 'T') {
                            vars.push_back(fy.target);
                        }
                        DagNode node1;
                        node1.name = fy.target;
                        node1.lIndex = lIndex;
                        node1.rIndex = rIndex;
                        node1.op = op;
                        nodes.push_back(node1);
                        str2Node[fy.target] = count++;
                        //str2Node.insert(map<string, int>::value_type(fy.target, count++));
                    }
                    j++;
                    if (j == items.size()) {
                        break;
                    }
                    fy = items.at(j);
                }
                for (int k = 0; k < vars.size(); k++) {
                    int index0 = str2Node[vars[k]];
                    /*if (nodes[index0].name.at(0) == 'T') {
                        nodes[index0].name = vars[k];
                        vars[k] = "0";
                    }*/
                    if (nodes[index0].name == vars[k]) {
                        vars[k] = "0";
                    }
                }
                vector<FourYuanItem> newItems;
                int symb[count];
                for (int k = 0; k < count; k++) {
                    symb[k] = 0;
                }
                for (int k = 0; k < count; k++) {
                    if (symb[k] == 0) {
                        generateDagTmp(nodes, newItems, symb, k, vars, str2Node);
                    }
                }
                /*if (count > 1) {
                    
                    blocks.at(i).setCodes(newItems);
                }*/
                for (int k = 0; k < newItems.size(); k++) {
                    tmp.push_back(newItems.at(k));
                }
                j--;
                
            }
            else {
                tmp.push_back(fy);
            }
            
        }
        blocks.at(i).setCodes(tmp);
        //printf("aaa%d\n",count - 1);
        
        
        /*for (int j = count - 1; j >= 0; j--) {
            DagNode node = nodes.at(j);
            if (node.lIndex != -1) {
                DagNode 
            }
        }*/
        
    }
}

void generateDagTmp(vector<DagNode> &nodes, vector<FourYuanItem> &newItems, int symb[], int index, vector<string> &vars, map<string, int> &str2Node) {
    DagNode node = nodes.at(index);
    int lIndex = node.lIndex;
    int rIndex = node.rIndex;
    string l, r;
    if (lIndex != -1) {
        generateDagTmp(nodes, newItems, symb, lIndex, vars, str2Node);
        l = nodes.at(lIndex).name;
    }
    else {
        l = "0";
    }
    if (rIndex != -1) {
        generateDagTmp(nodes, newItems, symb, lIndex, vars, str2Node);
        r = nodes.at(rIndex).name;
    }
    else {
        r = "0";
    }
    if (l == "0" && r == "0") {
        return;
    }
    else if (symb[index] == 0) {
        FourYuanItem fy;
        fy.target = node.name;
        fy.left = l;
        if (fy.left.size() > 0 && fy.left.at(0) != 'T' && fy.left.at(0) != 'L' && fy.left.at(0) != 'G' && fy.left.at(0) != 'R') {
            printf("%s dddddddd\n", l.c_str());
            fy.leftValue = stoi(l);
        }
        fy.right = r;
        if (fy.right.size() > 0 && fy.right.at(0) != 'T' && fy.right.at(0) != 'L' && fy.right.at(0) != 'G' && fy.right.at(0) != 'R') {
            fy.rightValue = stoi(r);
        }
        fy.op = node.op;
        fy.codeType = AssignState;
        newItems.push_back(fy);
        for (int i = 0; i < vars.size(); i++) {
            if (vars[i] != "0" && str2Node[vars[i]] == index) {
                fy.target = vars[i];
                vars[i] = "0";
                newItems.push_back(fy);
            }
        }
        symb[index] = 1;
    }
}

void optBlock(vector<FourYuanItem>& fourItems) {
    map<string, int> label2Block;
    vector<FourYuanItem> codes;
    int blockIndex = 0;
    for (int i = 0; i < fourItems.size(); i++) {
        FourYuanItem fy = fourItems.at(i);
        switch (fy.codeType) {
        case Finish:
            break;
        case FunctionDef:
        case Label:

            if (codes.size() > 0) {
                Block block = Block(codes);
                blocks.push_back(block);

                blockIndex++;
                codes.clear();
                codes.push_back(fy);
                label2Block.insert(map<string, int>::value_type(fy.target, blockIndex));
            }
            else {
                label2Block.insert(map<string, int>::value_type(fy.target, blockIndex));
                codes.push_back(fy);
            }
            break;
        case BNZ:
        case Goto: {
            codes.push_back(fy);
            Block block = Block(codes);
            blocks.push_back(block);
            blockIndex++;
            codes.clear(); }
            break;
        case FunctionCall: {
            codes.push_back(fy);
            Block block = Block(codes);
            blocks.push_back(block);
            blockIndex++;
            codes.clear(); }
            break;
        case ReturnIdent: 
        case ReturnEmpty: {
            codes.push_back(fy);
            if (i + 1 < fourItems.size() && fourItems.at(i + 1).codeType == Finish) {
                codes.push_back(fourItems.at(i + 1));
            }
            Block block = Block(codes);
            blocks.push_back(block);
            blockIndex++;
            codes.clear(); }
            break;
        default:
            codes.push_back(fy);
            break;

        }
    }

    for (int i = 0; i < blocks.size(); i++) {
        vector<FourYuanItem> items;
        items = blocks.at(i).getCodes();
        
            FourYuanItem fy = items.at(items.size() - 1);
            switch (fy.codeType) {
            case Finish:
            case ReturnChar:
            case ReturnEmpty:
            case ReturnIdent:
            case ReturnInt:
                break;
            case BNZ: {
                blocks.at(i).addToPost(i + 1);
                blocks.at(i).addToPost(label2Block[fy.target]);}
                break;
            case Goto: {
                blocks.at(i).addToPost(label2Block[fy.target]);
                }
                break;
            case FunctionCall: {
                if (fy.target == "main") {
                    blocks.at(i).addToPost(label2Block[fy.target]);
                }
                else {
                    blocks.at(i).addToPost(i + 1);
                }
                }
                break;
            default:
                blocks.at(i).addToPost(i + 1);
                break;

            }

        
    }
}

void liveVar() {
    for (int i = 0; i < blocks.size(); i++) {
        vector<FourYuanItem> items;
        vector<FourYuanItem> tmp;
        items = blocks.at(i).getCodes();
        for (int j = 0; j < items.size(); j++) {
            FourYuanItem fy = items.at(j);
            switch (fy.codeType) {
            case AssignState:
                if (blocks.at(i).hasAdded(fy.target) == false) {
                    blocks.at(i).addToDef(fy.target);
                }
                
                if (fy.left.size() > 0 && blocks.at(i).hasAdded(fy.left) == false &&
                    (fy.left.at(0) == 'T' ||
                    fy.left.at(0) == 'G' || fy.left.at(0) == 'L')) {
                    blocks.at(i).addToUse(fy.left);
                }
                if (fy.right.size() > 0 && blocks.at(i).hasAdded(fy.right) == false &&
                    (fy.right.at(0) == 'T' ||
                    fy.right.at(0) == 'G' || fy.right.at(0) == 'L')) {
                    blocks.at(i).addToUse(fy.right);
                }
                break;
            case AssignArray:
                if (fy.left.size() > 0 && blocks.at(i).hasAdded(fy.left) == false &&
                    (fy.left.at(0) == 'T' ||
                    fy.left.at(0) == 'G' || fy.left.at(0) == 'L')) {
                    blocks.at(i).addToUse(fy.left);
                }
                if (fy.right.size() > 0 && blocks.at(i).hasAdded(fy.right) == false &&
                    (fy.right.at(0) == 'T' ||
                    fy.right.at(0) == 'G' || fy.right.at(0) == 'L')) {
                    blocks.at(i).addToUse(fy.right);
                }
                break;
            case AssignByArray:
            case ReadChar:
            case ReadInt:
                if (blocks.at(i).hasAdded(fy.target) == false) {
                    blocks.at(i).addToDef(fy.target);
                }
                break;
            case PrintChar:
            case PrintInt:
            case PrintIdent:
            case ReturnIdent:
                if (blocks.at(i).hasAdded(fy.target) == false) {
                    blocks.at(i).addToUse(fy.target);
                }
                break;
            default:break;
            }

        }
    }
    bool needCal = true;
    int t = 0;
    while (needCal == true) {
        t++;
        needCal = false;
        for (int i = blocks.size() - 1; i >= 0; i--) {
            vector<int> posts = blocks.at(i).getPost();
            for (int j = 0; j < posts.size(); j++) {
                vector<string> poIn = blocks[posts[j]].getIn();
                for (int k = 0; k < poIn.size(); k++) {
                    if (blocks[i].inOut(poIn[k]) == false) {
                        needCal = true;
                        blocks[i].addToOut(poIn[k]);
                    }
                }
            }
            if (blocks[i].calIn() == true) {
                needCal = true;
            }
        }
    }
}

void tmpConflict() {
   
    for (int i = 0; i < blocks.size(); i++) {
        
        vector<string> tmps;
        tmps = blocks.at(i).getTmps();
        int m = tmps.size();
        int counts[m];
        int graph[m][m];
        for (int j = 0; j < m; j++) {
            vector<int> se1 = tmpse[tmps[j]];
            for (int k = j + 1; k < m; k++) {
                vector<int> se2 = tmpse[tmps[k]];
                if (se2[0] <= se1[1]) {
                    counts[j] = counts[j] + 1;
                    counts[k] = counts[k] + 1;
                    graph[j][k] = 1;
                    graph[k][j] = 1;
                }
                else {
                    break;
                }
            }
        }
        int color[m];
        for (int j = 0; j < m; j++)
            color[j] = -1;
        vector<int> notAss;
        while (graphColoring((int*)graph, m, color, 0) == 0) {
            int index = 0;
            int max = -1;
            for (int j = 0; j < m; j++) {
                if (counts[j] > max) {
                    max = counts[j];
                    index = j;
                }
            }
            counts[index] = 0;
            notAss.push_back(index);
            for (int j = 0; j < m; j++) {
                graph[index][j] = 0;
                graph[j][index] = 0;
                
            }
        }
        printf("color ");
        for (int i = 0; i < notAss.size(); i++) {
            printf("%d ", notAss[i]);
        }
        printf("\n");
        for (int i = 0; i < m; i++) {
            printf("%d ", color[i]);
            bool isNeedMap = true;
            for (int j = 0; j < notAss.size(); j++) {
                if (i == notAss[j]) {
                    isNeedMap = false;
                }
            }

            if (isNeedMap == true) {
                tmpToReg.insert(map<string, string>::value_type(tmps[i], "$t" + to_string(color[i])));
            }
        }
            
        printf("\n");
    }
}

int graphColoring(int *graph, int m, int color[], int v)
{

    if (v == m)
    {
        return 1;
    }
 
    for (int c = 0; c < 8; c++)
    {
  
        if (isSafe(v, graph, m, color, c) == true)
        {
            color[v] = c;
       
            if (graphColoring(graph, m, color, v + 1) == 1) {
                return 1;
            }
           
            color[v] = 0;
        }
    }
    return 0;
}

bool isSafe(int v, int* graph, int m, int color[], int c)
{
    for (int i = 0; i < m; i++)

        if (*(graph + v * m + i) == 1 && c == color[i])
            return false;
    return true;
}

void printBlock() {
    ofstream out(midCodeFileName, ios::out);
    int gOff = 0;
    int lOff = 12;
    int tOff = 4;
    string func = " ";
    int size = 0;
    for (int i = 0; i < blocks.size(); i++) {
        out << "block" << i << endl;
        vector<FourYuanItem> fourItems;
        fourItems = blocks.at(i).getCodes();
        for (int j = 0; j < fourItems.size(); j++) {
            FourYuanItem fy = fourItems.at(j);
            switch (fy.codeType) {
            case VarDef: {
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
            case AssignArray: {
                if (fy.left.size() > 0 && fy.left.at(0) == 'T') {
                    (tmpse[fy.left])[1] = j;
                }
                if (fy.right.size() > 0 && fy.right.at(0) == 'T') {
                    (tmpse[fy.right])[1] = j;
                }
                map<string, vector<int>>::iterator iter;
                if (fy.target.at(0) == 'T') {
                    iter = tmpse.find(fy.target);
                    if (iter != tmpse.end()) {
                        iter->second[1] = j;

                    }
                }
                if (fy.dim == 2) {
                    out << fy.arrayName << "[" + fy.left + "][" + fy.right + "] = " << fy.target << endl;
                }
                else if (fy.dim == 1) {
                    out << fy.arrayName << "[" + fy.left + "] = " << fy.target << endl;
                }}
                break;
            case AssignByArray: {
                if (fy.left.size() > 0 && fy.left.at(0) == 'T') {
                    (tmpse[fy.left])[1] = j;
                }
                if (fy.right.size() > 0 && fy.right.at(0) == 'T') {
                    (tmpse[fy.right])[1] = j;
                }
                map<string, vector<int>>::iterator iter;
                if (fy.target.at(0) == 'T') {
                    
                    iter = tmpse.find(fy.target);
                    if (iter != tmpse.end()) {
                        iter->second[1] = j;

                    }
                    else {
                        vector<int> se;
                        se.push_back(j);
                        se.push_back(-1);
                        tmpse.insert(map<string, vector<int>>::value_type(fy.target, se));
                        blocks[i].addToTmps(fy.target);
                    }
                    tmpToAddr.insert(map<string, int>::value_type(fy.target, tOff));
                    tOff += 4;
                }
                if (fy.dim == 1) {
                    out << fy.target << " = " + fy.arrayName + "[" + fy.left + "]" << endl;
                }
                else if (fy.dim == 2) {
                    out << fy.target << " = " + fy.arrayName + "[" + fy.left + "][" + fy.right + "]" << endl;
                }}
                break;
            case AssignState: {
                map<string, vector<int>>::iterator iter;
                if (fy.target.at(0) == 'T') {
                    iter = tmpse.find(fy.target);
                    if (iter != tmpse.end()) {
                        iter->second[1] = j;

                    }
                    else {
                        vector<int> se;
                        se.push_back(j);
                        se.push_back(-1);
                        tmpse.insert(map<string, vector<int>>::value_type(fy.target, se));
                        blocks[i].addToTmps(fy.target);
                    }
                }
                if (fy.target.at(0) == 'T') {
                    tmpToAddr.insert(map<string, int>::value_type(fy.target, tOff));
                    tOff += 4;
                }
                if (fy.left.size() > 0 && fy.left.at(0) == 'T') {
                    iter = tmpse.find(fy.left);
                    if (iter != tmpse.end()) {
                        iter->second[1] = j;
                    }
                }
                if (fy.right.size() > 0 && fy.right.at(0) == 'T') {
                    iter = tmpse.find(fy.right);
                    if (iter != tmpse.end()) {
                        iter->second[1] = j;
                    }
                }
                out << fy.target << " = " << fy.left << " " << fy.op << " " << fy.right << endl; }
                break;
            case Finish:
                funcVarSize.insert(map<string, int>::value_type(func, lOff - 4));
                funcTmpSize.insert(map<string, int>::value_type(func, tOff - 4));
                break;
            case FunctionDef: {
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
                out << fy.target << "()" << endl; }
                break;
            case ParamDef: {
                varToAddr.insert(map<string, int>::value_type(fy.target, lOff));
                printf("para: %s off:%d \n", fy.target.c_str(), lOff);
                lOff += 4;
                if (fy.valueType == INT) {
                    out << "Param int ";
                }
                else {
                    out << "Param char ";
                }
                out << fy.target << endl; }
                break;
            case ReadChar:
                out << "Read Char " << fy.target << endl;
                break;
            case ReadInt:
                out << "Read Int " << fy.target << endl;
                break;
            case PrintStr:
            case PrintBeforeExpr: {
                out << "Print String " << "\"" << fy.target << "\"" << endl;
                strs.push_back(fy.target); }
                break;
            case PrintChar:
                if (fy.target.at(0) == 'T') {
                    (tmpse[fy.target])[1] = j;
                }
                out << "Print Char "
                    << "\'" << fy.target.at(0) << "\"" << endl;
                break;
            case PrintInt:
                if (fy.target.at(0) == 'T') {
                    (tmpse[fy.target])[1] = j;
                }
                out << " Print Int " << fy.target << endl;
                break;
            case PrintIdent:
                if (fy.target.at(0) == 'T') {
                    (tmpse[fy.target])[1] = j;
                }
                out << "Print Ident " << fy.target << endl;
                break;
            case ReturnIdent:
                if (fy.target.at(0) == 'T') {
                    (tmpse[fy.target])[1] = j;
                }
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
                if (fy.left.at(0) == 'T') {
                    (tmpse[fy.left])[1] = j;
                }
                if (fy.right.at(0) == 'T') {
                    (tmpse[fy.right])[1] = j;
                }
                out << fy.left << " < " << fy.right << endl;
                break;
            case LEQm:
                if (fy.left.at(0) == 'T') {
                    (tmpse[fy.left])[1] = j;
                }
                if (fy.right.at(0) == 'T') {
                    (tmpse[fy.right])[1] = j;
                }
                out << fy.left << " <= " << fy.right << endl;
                break;
            case GREm:
                if (fy.left.at(0) == 'T') {
                    (tmpse[fy.left])[1] = j;
                }
                if (fy.right.at(0) == 'T') {
                    (tmpse[fy.right])[1] = j;
                }
                out << fy.left << " > " << fy.right << endl;
                break;
            case GEQm:
                if (fy.left.at(0) == 'T') {
                    (tmpse[fy.left])[1] = j;
                }
                if (fy.right.at(0) == 'T') {
                    (tmpse[fy.right])[1] = j;
                }
                out << fy.left << " >= " << fy.right << endl;
                break;
            case EQLm:
                if (fy.left.at(0) == 'T') {
                    (tmpse[fy.left])[1] = j;
                }
                if (fy.right.at(0) == 'T') {
                    (tmpse[fy.right])[1] = j;
                }
                out << fy.left << " == " << fy.right << endl;
                break;
            case NEQm:
                if (fy.left.at(0) == 'T') {
                    (tmpse[fy.left])[1] = j;
                }
                if (fy.right.at(0) == 'T') {
                    (tmpse[fy.right])[1] = j;
                }
                out << fy.left << " != " << fy.right << endl;
                break;
            case FunctionCall:
                out << "call " << fy.target << endl;
                break;
            case ValueParamDeliver:
                if (fy.target.at(0) == 'T') {
                    (tmpse[fy.target])[1] = j;
                }
                out << "push " << fy.target << endl;
                break;
            default:
                break;
            }
        }
    }
    out.close();
    tmpConflict();
}

//void printBlock() {
//    ofstream out(midCodeFileName, ios::out);
//    for (int i = 0; i < blocks.size(); i++) {
//        out << "block" << i << endl;
//        vector<FourYuanItem> items;
//        items = blocks.at(i).getCodes();
//        for (int j = 0; j < items.size(); j++) {
//            FourYuanItem fy = items.at(j);
//            switch (fy.codeType) {
//            case VarDef:{
//                if (fy.valueType == CHAR) {
//                    out << "char " + fy.target << endl;
//                    //printf("char %s\n", fy.target.c_str());
//                }
//                else if (fy.valueType == INT) {
//                    out << "int " + fy.target << endl;
//                    //printf("char %s\n", fy.target.c_str());
//                }}
//                       break;
//            case AssignArray:
//                if (fy.dim == 2) {
//                    out << fy.arrayName << "[" + fy.left + "][" + fy.right + "] = " << fy.target << endl;
//                }
//                else if (fy.dim == 1) {
//                    out << fy.arrayName << "[" + fy.left + "] = " << fy.target << endl;
//                }
//                break;
//            case AssignByArray:
//                if (fy.dim == 1) {
//                    out << fy.target << " = " + fy.arrayName + "[" + fy.left + "]" << endl;
//                }
//                else if (fy.dim == 2) {
//                    out << fy.target << " = " + fy.arrayName + "[" + fy.left + "][" + fy.right + "]" << endl;
//                }
//                break;
//            case AssignState:
//                out << fy.target << " = " << fy.left << " " << fy.op << " " << fy.right << endl;
//                break;
//            case Finish:
//                break;
//            case FunctionDef:
//                if (fy.funcType == VOID) {
//                    out << "void ";
//                }
//                else if (fy.funcType == RETURNCHAR) {
//                    out << "char ";
//                }
//                else {
//                    out << "int ";
//                }
//                out << fy.target << "()" << endl;
//                break;
//            case ParamDef:
//                if (fy.valueType == INT) {
//                    out << "Param int ";
//                }
//                else {
//                    out << "Param char ";
//                }
//                out << fy.target << endl;
//                break;
//            case ReadChar:
//                out << "Read Char " << fy.target << endl;
//                break;
//            case ReadInt:
//                out << "Read Int " << fy.target << endl;
//                break;
//            case PrintStr:
//            case PrintBeforeExpr:
//                out << "Print String " << "\"" << fy.target << "\"" << endl;
//                strs.push_back(fy.target);
//                break;
//            case PrintChar:
//                out << "Print Char "
//                    << "\'" << fy.target.at(0) << "\"" << endl;
//                break;
//            case PrintInt:
//                out << " Print Int " << fy.target << endl;
//                break;
//            case PrintIdent:
//                out << "Print Ident " << fy.target << endl;
//                break;
//            case ReturnIdent:
//                out << "ret " << fy.target << endl;
//                break;
//            case ReturnEmpty:
//                out << "ret" << endl;
//                break;
//            case Label:
//                out << fy.target << ":" << endl;
//                break;
//            case BNZ:
//                out << "BNZ " << fy.target << endl;
//                break;
//            case Goto:
//                out << "GOTO " << fy.target << endl;
//                break;
//            case LSSm:
//                out << fy.left << " < " << fy.right << endl;
//                break;
//            case LEQm:
//                out << fy.left << " <= " << fy.right << endl;
//                break;
//            case GREm:
//                out << fy.left << " > " << fy.right << endl;
//                break;
//            case GEQm:
//                out << fy.left << " >= " << fy.right << endl;
//                break;
//            case EQLm:
//                out << fy.left << " == " << fy.right << endl;
//                break;
//            case NEQm:
//                out << fy.left << " != " << fy.right << endl;
//                break;
//            case FunctionCall:
//                out << "call " << fy.target << endl;
//                break;
//            case ValueParamDeliver:
//                out << "push " << fy.target << endl;
//                break;
//            default:
//                break;
//            }
//        }
//        
//    }
//}

void optAssignRight0() {
    for (int i = 0; i < blocks.size(); i++) {
        map<string, int> constTmp;
        vector<FourYuanItem> items;
        items = blocks.at(i).getCodes();
        vector<FourYuanItem> tmp;
        for (int j = 0; j < items.size(); j++) {
            FourYuanItem fy = items.at(j);
            if (fy.codeType == AssignState && fy.target.at(0) == 'T') {
                bool lIsSure = false, rIsSure = false;
                int lv = 0, rv = 0, tv = 0;
                map<string, int>::iterator iter;
                if (fy.left.at(0) == 'T') {
                    iter = constTmp.find(fy.left);
                    if (iter != constTmp.end()) {
                        lv = iter->second;
                        lIsSure = true;
                        fy.leftValue = lv;
                        fy.left = to_string(lv);
                    }
                }
                else if (fy.left == "0" || (fy.leftValue != 0 && fy.left.at(0) != 'G' && fy.left.at(0) != 'T' && fy.left.at(0) != 'L' && fy.left.at(0) != 'R')) {
                    lv = fy.leftValue;
                    lIsSure = true;
                }
                if (fy.right.at(0) == 'T') {
                    iter = constTmp.find(fy.right);
                    if (iter != constTmp.end()) {
                        rv = iter->second;
                        rIsSure = true;
                        fy.rightValue = rv;
                        fy.right = to_string(rv);
                    }
                }
                else if (fy.right == "0" || (fy.rightValue != 0 && fy.right.at(0) != 'G' && fy.right.at(0) != 'T' && fy.right.at(0) != 'L' && fy.right.at(0) != 'R')) {
                    rv = fy.rightValue;
                    rIsSure = true;
                }
                if (rIsSure == true && lIsSure == true) {
                    tv = lv + rv;
                    constTmp.insert(map<string, int>::value_type(fy.target, tv));
                }
                else {
                    tmp.push_back(fy);
                }
            }
            else if (fy.codeType == AssignArray || fy.codeType == AssignState)
            {
                map<string, int>::iterator iter;
                if (fy.left.size() > 0 && fy.left.at(0) == 'T') {
                    iter = constTmp.find(fy.left);
                    int lv = 0;
                    if (iter != constTmp.end()) {
                        lv = iter->second;
                        fy.leftValue = lv;
                        fy.left = to_string(lv);
                    }
                }
                if (fy.right.size() > 0 && fy.right.at(0) == 'T') {
                    iter = constTmp.find(fy.right);
                    int rv = 0;
                    if (iter != constTmp.end()) {
                        rv = iter->second;
                        fy.rightValue = rv;
                        fy.right = to_string(rv);
                    }
                }
                tmp.push_back(fy);
            }
            else {
                FourYuanItem fy1;
                fy1.codeType = AssignState;
                fy1.right = "0";
                fy1.rightValue = 0;
                fy1.op = '+';
                map<string, int>::iterator iter;
                if (fy.left.size() > 0 && fy.left.at(0) == 'T') {
                    iter = constTmp.find(fy.left);
                    int lv = 0;
                    if (iter != constTmp.end()) {
                        lv = iter->second;
                        fy1.leftValue = lv;
                        fy1.left = to_string(lv);
                        fy1.target = fy.left;
                        tmp.push_back(fy1);
                    }
                }
                if (fy.right.size() > 0 && fy.right.at(0) == 'T') {
                    iter = constTmp.find(fy.right);
                    int rv = 0;
                    if (iter != constTmp.end()) {
                        rv = iter->second;
                        fy1.leftValue = rv;
                        fy1.left = to_string(rv);
                        fy1.target = fy.right;
                        tmp.push_back(fy1);
                    }
                }
                tmp.push_back(fy);
            }

        }
        blocks.at(i).setCodes(tmp);
        //items.swap(tmp);
    }
    
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
        if (fy.left.at(0) == 'T') {
            map<string, string>::iterator iter;
         
            iter = tmpToReg.find(fy.left);
            if (iter != tmpToReg.end()) {
                l = iter->second;
            }
            else {
                l = mapToVar(out, fy.left, 0);
            }
        }
        else {
            l = mapToVar(out, fy.left, 0);
        }
        
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
        if (fy.right.at(0) == 'T') {
            map<string, string>::iterator iter;
            
            iter = tmpToReg.find(fy.right);
            if (iter != tmpToReg.end()) {
                r = iter->second;
            }
            else {
                r = mapToVar(out, fy.right, 1);
            }
        }
        else {
            r = mapToVar(out, fy.right, 1);
        }
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
        bool needToMap = true;
        t = "$t8";
        if (fy.target.at(0) == 'T') {
            map<string, string>::iterator iter;
            
            iter = tmpToReg.find(fy.target);
            if (iter != tmpToReg.end()) {
                t = iter->second;
                needToMap = false;
            }
            
        }
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
                out << "addiu " << t << "," + r + "," + to_string(ll) << endl;
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
                out << "addiu " << t << "," + l + "," + to_string(rr) << endl;
                break;
            case '-':
                out << "addiu " << t << "," + l + "," + to_string(-rr) << endl;
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
                out << "addu " << t << "," + l + "," + r << endl;
                break;
            case '-':
                out << "subu " << t << "," + l + "," + r << endl;
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
        if (needToMap == true) {
            mapToVar(out, fy.target, 2);
        }
        
        
    }
}

int calculateArrayOff(FourYuanItem &fy, ofstream &out) {
    string type;
    if (fy.arrayName.size() > 0 && fy.arrayName.at(0) == 'L') {
        type = "subu ";
    }
    else {
        type = "addu ";
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
        if (fy.left.at(0) == 'T') {
            map<string, string>::iterator iter;
           
            iter = tmpToReg.find(fy.left);
            if (iter != tmpToReg.end()) {
                l = iter->second;
            }
            else {
                l = mapToVar(out, fy.left, 0);
            }
        }
        else {
            l = mapToVar(out, fy.left, 0);
        }
        
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
        
        if (fy.right.at(0) == 'T') {
            map<string, string>::iterator iter;
            
            iter = tmpToReg.find(fy.right);
            if (iter != tmpToReg.end()) {
                r = iter->second;
            }
            else {
                r = mapToVar(out, fy.right, 1);
            }
        }
        else {
            r = mapToVar(out, fy.right, 1);
        }
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
                    if (fy.target.at(0) == 'T') {
                        map<string, string>::iterator iter;
                       
                        iter = tmpToReg.find(fy.target);
                        if (iter != tmpToReg.end()) {
                            t = iter->second;
                        }
                        else {
                            t = mapToVar(out, fy.target, 1);
                        }
                    }
                    else {
                        t = mapToVar(out, fy.target, 1);
                    }
                    
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << offset << "(" + base + ")" << endl;
                
            }
            else {
                bool isNeedMap = true;
                t = "$t8";
                if (fy.target.at(0) == 'T') {
                    map<string, string>::iterator iter;
                    
                    iter = tmpToReg.find(fy.target);
                    if (iter != tmpToReg.end()) {
                        t = iter->second;
                        isNeedMap = false;
                    }
                   
                }
                
                out << "lw " + t + "," << offset << "(" + base + ")" << endl;
                if (isNeedMap == true) {
                    mapToVar(out, fy.target, 2);
                }
                
            }
        }
        else {
            out << "mul " << l << "," + l + "," << 4 << endl;
            out << "addiu " << l << "," + l + "," << offset << endl;
            out << type << l << "," + base + "," << l << endl;
            
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    if (fy.target.at(0) == 'T') {
                        map<string, string>::iterator iter;
                     
                        iter = tmpToReg.find(fy.target);
                        if (iter != tmpToReg.end()) {
                            t = iter->second;
                        }
                        else {
                            t = mapToVar(out, fy.target, 1);
                        }
                    }
                    else {
                        t = mapToVar(out, fy.target, 1);
                    }
                    
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << "0(" + l + ")" << endl;
            }
            else {
                bool isNeedMap = true;
                t = "$t8";
                if (fy.target.at(0) == 'T') {
                    map<string, string>::iterator iter;
                  
                    iter = tmpToReg.find(fy.target);
                    if (iter != tmpToReg.end()) {
                        t = iter->second;
                        isNeedMap = false;
                    }

                }
                out << "lw " + t << ",0(" + l + ")" << endl;
                if (isNeedMap == true) {
                    mapToVar(out, fy.target, 2);
                }
                
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
                    if (fy.target.at(0) == 'T') {
                        map<string, string>::iterator iter;
                        
                        iter = tmpToReg.find(fy.target);
                        if (iter != tmpToReg.end()) {
                            t = iter->second;
                        }
                        else {
                            t = mapToVar(out, fy.target, 1);
                        }
                    }
                    else {
                        t = mapToVar(out, fy.target, 1);
                    }
                    
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << offset << "(" + base + ")" << endl;
            }
            else {
                bool isNeedMap = true;
                t = "$t8";
                if (fy.target.at(0) == 'T') {
                    map<string, string>::iterator iter;
                    
                    iter = tmpToReg.find(fy.target);
                    if (iter != tmpToReg.end()) {
                        t = iter->second;
                        isNeedMap = false;
                    }

                }
                out << "lw " + t + "," << offset << "(" + base + ")" << endl;
                if (isNeedMap == true) {
                    mapToVar(out, fy.target, 2);
                }
                
            }
            //out << type + t + "," << offset << "(" + base + ")" << endl;
        }
        else if (lIsCon == true && rIsCon == false) {
            offset = offset + fy.leftValue * fy.y * 4;
            
            out << "mul " << r << "," + r + "," << 4 << endl;
            out << "addiu " << r << "," + r + "," << offset << endl;
            out << type << r << "," + base + "," << r << endl;
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    if (fy.target.at(0) == 'T') {
                        map<string, string>::iterator iter;
                        
                        iter = tmpToReg.find(fy.target);
                        if (iter != tmpToReg.end()) {
                            t = iter->second;
                        }
                        else {
                            t = mapToVar(out, fy.target, 1);
                        }
                    }
                    else {
                        t = mapToVar(out, fy.target, 1);
                    }
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << "0(" + r + ")" << endl;
            }
            else {
                bool isNeedMap = true;
                t = "$t8";
                if (fy.target.at(0) == 'T') {
                    map<string, string>::iterator iter;
                    
                    iter = tmpToReg.find(fy.target);
                    if (iter != tmpToReg.end()) {
                        t = iter->second;
                        isNeedMap = false;
                    }

                }
                out << "lw " + t + "," << "0(" + r + ")" << endl;
                if (isNeedMap == true) {
                    mapToVar(out, fy.target, 2);
                }

               
                
            }
            //out << type + r + "," << "0(" + base + ")" << endl;
        }
        else if (lIsCon == false && rIsCon == true) {
            
            offset = offset + fy.rightValue * 4;
            int si = fy.y * 4;
            out << "mul " << l << "," + l + "," << si << endl;
            out << "addiu " << l << "," + l + "," << offset << endl;
            out << type << l << "," + base + "," << l << endl;
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    if (fy.target.at(0) == 'T') {
                        map<string, string>::iterator iter;
                        
                        iter = tmpToReg.find(fy.target);
                        if (iter != tmpToReg.end()) {
                            t = iter->second;
                        }
                        else {
                            t = mapToVar(out, fy.target, 1);
                        }
                    }
                    else {
                        t = mapToVar(out, fy.target, 1);
                    }
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << "0(" + l + ")" << endl;
            }
            else {
                bool isNeedMap = true;
                t = "$t8";
                if (fy.target.at(0) == 'T') {
                    map<string, string>::iterator iter;
                   
                    iter = tmpToReg.find(fy.target);
                    if (iter != tmpToReg.end()) {
                        t = iter->second;
                        isNeedMap = false;
                    }

                }
                out << "lw " + t + "," << "0(" + l + ")" << endl;
                if (isNeedMap == true) {
                    mapToVar(out, fy.target, 2);
                }

                
            }
            //out << type + l + "," << "0(" + base + ")" << endl;
        }
        else {
            int si = fy.y * 4;
            out << "mul " << l << "," + l + "," << si << endl;
            out << "mul " << r << "," + r + "," << 4 << endl;
            out << "addiu " << l << "," + l + "," << offset << endl;
            out << "addu " << l << "," + l + "," << r << endl;
            out << type << l << "," + base + "," << l << endl;
            //out << type + l + "," << "0(" + base + ")" << endl;
            if (fy.codeType == AssignArray) {
                if (fy.target.size() > 0 && (fy.target.at(0) == 'G' || fy.target.at(0) == 'L' || fy.target.at(0) == 'T')) {
                    if (fy.target.at(0) == 'T') {
                        map<string, string>::iterator iter;
                       
                        iter = tmpToReg.find(fy.target);
                        if (iter != tmpToReg.end()) {
                            t = iter->second;
                        }
                        else {
                            t = mapToVar(out, fy.target, 1);
                        }
                    }
                    else {
                        t = mapToVar(out, fy.target, 1);
                    }
                }
                else {
                    out << "li $t9," << fy.target << endl;
                    t = "$t9";
                }
                out << "sw " + t + "," << "0(" + l + ")" << endl;
            }
            else {
                bool isNeedMap = true;
                t = "$t8";
                if (fy.target.at(0) == 'T') {
                    map<string, string>::iterator iter;
                    
                    iter = tmpToReg.find(fy.target);
                    if (iter != tmpToReg.end()) {
                        t = iter->second;
                        isNeedMap = false;
                    }

                }
                out << "lw " + t + "," << "0(" + l + ")" << endl;
                if (isNeedMap == true) {
                    mapToVar(out, fy.target, 2);
                }
                
                
            }
        }
        
    }
    return offset;
}

void turnToMips() {
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
    out << "addiu $sp,$sp," << -size - 8 << endl;
    //out << "j main" << endl;
    for (int i = 0; i < blocks.size(); i++) {
        //out << "block" << i << endl;
        vector<FourYuanItem> fourItems;
        fourItems = blocks.at(i).getCodes();
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
            case ReadChar: {
                //out << "Read Char " << fy.target << endl;
                //li $v0,5
                //syscall
                out << "li $v0,12" << endl;
                out << "syscall" << endl;
                out << "move " << "$t8,$v0" << endl;
                mapToVar(out, fy.target, 2); }
                         break;
            case ReadInt: {
                // out << "Read Int " << fy.target << endl;
                out << "li $v0,5" << endl;
                out << "syscall" << endl;
                out << "move " << "$t8,$v0" << endl;
                mapToVar(out, fy.target, 2); }
                        break;
            case PrintStr: {
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
                out << "syscall" << endl; }
                         break;
            case PrintBeforeExpr: {
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
            case PrintIdent: {
                //out << "Print Ident " << fy.target << endl;
                string t;
                if (fy.target.at(0) == 'T') {
                    map<string, string>::iterator iter;
                    
                    iter = tmpToReg.find(fy.target);
                    if (iter != tmpToReg.end()) {
                        t = iter->second;
                    }
                    else {
                        t = mapToVar(out, fy.target, 0);
                    }
                }
                else {
                    t = mapToVar(out, fy.target, 0);
                }
                
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
                out << "syscall" << endl; }
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
                string l;
                if (fy.left.at(0) == 'T') {
                    map<string, string>::iterator iter;
                    
                    iter = tmpToReg.find(fy.left);
                    if (iter != tmpToReg.end()) {
                        l = iter->second;
                    }
                    else {
                        l = mapToVar(out, fy.left, 0);
                    }
                }
                else {
                    l = mapToVar(out, fy.left, 0);
                }
                string r;
                if (fy.right.size() > 0 && (fy.right.at(0) == 'G' || fy.right.at(0) == 'L' || fy.right.at(0) == 'T')) {
                    //r = mapToVar(out, fy.right, 1);
                    if (fy.right.at(0) == 'T') {
                        map<string, string>::iterator iter;
                       
                        iter = tmpToReg.find(fy.right);
                        if (iter != tmpToReg.end()) {
                             r= iter->second;
                        }
                        else {
                            r = mapToVar(out, fy.right, 1);
                        }
                    }
                    else {
                        r = mapToVar(out, fy.right, 1);
                    }
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
                    //t = mapToVar(out, fy.target, 0);
                    if (fy.target.at(0) == 'T') {
                        map<string, string>::iterator iter;
                        
                        iter = tmpToReg.find(fy.target);
                        if (iter != tmpToReg.end()) {
                            t = iter->second;
                        }
                        else {
                            t = mapToVar(out, fy.target, 0);
                        }
                    }
                    else {
                        t = mapToVar(out, fy.target, 0);
                    }
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
                out << "addiu $fp,$sp," << -size1 << endl;
                iter = funcVarSize.find(fy.target);
                if (iter != funcVarSize.end()) {
                    size2 = size1 + iter->second;
                }
                out << "addiu $sp,$sp," << -size2 - 8 << endl;
                out << "jal " + fy.target << endl;
                if (fy.target == "main") {
                    out << "li $v0,10" << endl;
                    out << "syscall" << endl;
                }
                else {
                    out << "addiu $sp,$sp," << size2 + 8 << endl;
                    out << "lw $fp," << -size1 - 4 << "($sp)" << endl;
                    out << "lw $ra," << -size1 - 8 << "($sp)" << endl;
                }
                paraTop = 8; }
                             break;
            case ReturnIdent: {
                string t;
                //t = mapToVar(out, fy.target, 0);
                if (fy.target.at(0) == 'T') {
                    map<string, string>::iterator iter;
                    iter = tmpToReg.find(fy.target);
                    if (iter != tmpToReg.end()) {
                        t = iter->second;
                    }
                    else {
                        t = mapToVar(out, fy.target, 0);
                    }
                }
                else {
                    t = mapToVar(out, fy.target, 0);
                }
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
    }
    
    out.close();
}