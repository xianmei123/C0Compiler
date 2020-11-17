#include <iostream>
#include <fstream>
#include "error.h"

using namespace std;

int out = 0;
ofstream errorFile;

Error::Error(int flag) {
	errorFile.open("error.txt", ios::out);
}

void Error::errorLog(ErrorCode ec, int lineNumber) {
	switch (ec) {
		case illegalChar: 
			if (out) {
				errorFile << "At line: " << lineNumber << " character value error." << endl;
			}
			errorFile << lineNumber << " a" << endl;
			break;
		case illegalString:
			if (out) {
				errorFile << "At line: " << lineNumber << " string value error." << endl;
			}
			errorFile << lineNumber << " a" << endl;
			break;
		case illegalChInFile:
			if (out) {
				errorFile << "At line: " << lineNumber << " exists illegal character in file." << endl;
			}
			errorFile << lineNumber << " a" << endl;
			break;
		case identRedefine:
			if (out) {
				errorFile << "At line: " << lineNumber << " redefine the identifier." << endl;
			}
			errorFile << lineNumber << " b" << endl;
			break;
		case identNotDefine:
			if (out) {
				errorFile << "At line: " << lineNumber << " the identifier has not been defined." << endl;
			}
			errorFile << lineNumber << " c" << endl;
			break;
		case funcStatParaNum:
			if (out) {
				errorFile << "At line: " << lineNumber << " number of parameters of the function call statement do not match." << endl;
			}
			errorFile << lineNumber << " d" << endl;
			break;
		case funcStatParaType:
			if (out) {
				errorFile << "At line: " << lineNumber << " type of parameters of the function call statement do not match." << endl;
			}
			errorFile << lineNumber << " e" << endl;
			break;
		case illegalCondition:
			if (out) {
				errorFile << "At line: " << lineNumber << " the condition has illegal type." << endl;
			}
			errorFile << lineNumber << " f" << endl;
			break;
		case voidFuncReturnError:
			if (out) {
				errorFile << "At line: " << lineNumber << " No return value function has mismatched return statement." << endl;
			}
			errorFile << lineNumber << " g" << endl;
			break;
		case returnFuncReturnError:
			if (out) {
				errorFile << "At line: " << lineNumber << " Function with return values has mismatched return statement." << endl;
			}
			errorFile << lineNumber << " h" << endl;
			break;
		case arrayIndexTypeError:
			if (out) {
				errorFile << "At line: " << lineNumber << " the index of array is wrong type." << endl;
			}
			errorFile << lineNumber << " i" << endl;
			break;
		case assignConError:
			if (out) {
				errorFile << "At line: " << lineNumber << " constant can not be assigned." << endl;
			}
			errorFile << lineNumber << " j" << endl;
			break;
		case withoutSemicn:
			if (out) {
				errorFile << "At line: " << lineNumber << " lack of semicolon." << endl;
			}
			errorFile << lineNumber << " k" << endl;
			break;
		case withoutRparent:
			if (out) {
				errorFile << "At line: " << lineNumber << " lack of rparent." << endl;
			}
			errorFile << lineNumber << " l" << endl;
			break;
		case withoutRbrack:
			if (out) {
				errorFile << "At line: " << lineNumber << " lack of rbrack." << endl;
			}
			errorFile << lineNumber << " m" << endl;
			break;
		case arrayInitNumError:
			if (out) {
				errorFile << "At line: " << lineNumber << " array initialization number does not match." << endl;
			}
			errorFile << lineNumber << " n" << endl;
			break;
		case typeMatchError:
			if (out) {
				errorFile << "At line: " << lineNumber << " case statement has mismatched type." << endl;
			}
			errorFile << lineNumber << " o" << endl;
			break;
		case withoutDefault:
			if (out) {
				errorFile << "At line: " << lineNumber << " lack of default statement." << endl;
			}
			errorFile << lineNumber << " p" << endl;
			break;
		default:break;
	}
}

void Error::errorLog(int lineNumber) {
}
