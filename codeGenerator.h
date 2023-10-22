#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include <string>
#include <vector>
#include <map>
#include "type.h"
using namespace std;
string generateTmpName(int num);
string generateLabel();
void turnToPostfixExpr(vector<PostfixItem> infix, vector<PostfixItem> &postfix);
string calculateExpr(vector<PostfixItem> &postfix, int &tmpCount, int &value, bool &isSure, vector<FourYuanItem> &fourItems);
void optAssignRight0();
void optBlock(vector<FourYuanItem>& fourItems);
void liveVar();
void tmpConflict();
int graphColoring(int* graph, int m, int color[], int v);
bool isSafe(int v, int* graph, int m, int color[], int c);
void printBlock();
void optDag();
void generateDagTmp(vector<DagNode>& nodes, vector<FourYuanItem>& newItems, int symb[], int index, vector<string>& vars, map<string, int>& str2Node);
void printMidCodeToFile(vector<FourYuanItem> &fourItems);
int calculateOff(int baseOffset);
string mapT(string s);
string mapToVar(ofstream& out, string name, int flag);
void assignStatMips(ofstream& out, FourYuanItem& fy);
int calculateArrayOff(FourYuanItem& fy, ofstream& out);
void turnToMips(vector<FourYuanItem> &fourItems);
void turnToMips();
#endif