#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include <string>
#include <vector>
#include "type.h"
using namespace std;
string generateTmpName(int num);
string generateLabel();
void turnToPostfixExpr(vector<PostfixItem> infix, vector<PostfixItem> &postfix);
string calculateExpr(vector<PostfixItem> &postfix, int &tmpCount, int &value, bool &isSure, vector<FourYuanItem> &fourItems);
void printMidCodeToFile(vector<FourYuanItem> &fourItems);
int calculateOff(int baseOffset);
string mapT(string s);
string mapToVar(ofstream& out, string name, int flag);
void assignStatMips(ofstream& out, FourYuanItem& fy);
int calculateArrayOff(FourYuanItem& fy, ofstream& out);
void turnToMips(vector<FourYuanItem> &fourItems);
#endif