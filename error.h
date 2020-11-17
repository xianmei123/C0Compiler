#ifndef ERROR_H
#define ERROR_H
#include<iostream>
#include<string>

using namespace std;

enum ErrorCode {
	illegalChar,
	illegalString,
	illegalChInFile,
	identRedefine,
	identNotDefine,
	funcStatParaNum,
	funcStatParaType,
	illegalCondition,
	voidFuncReturnError,
	returnFuncReturnError,
	arrayIndexTypeError,
	assignConError,
	withoutSemicn,
	withoutRparent,
	withoutRbrack,
	arrayInitNumError,
	typeMatchError,
	withoutDefault
};

class Error {
public:
	Error(int flag);
	void errorLog(ErrorCode ec, int lineNumber);
	void errorLog(int lineNumber);
};


#endif // ! ERROR_H