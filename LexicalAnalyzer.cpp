#include <fstream>
#include <string>
#include <map>
#include <cctype>
#include <algorithm>
#include "LexicalAnalyzer.h"
using namespace std;

string keyWord[] = {"const", "int", "char", "void", "main", "if", "else", 
"switch", "case", "default", "while", "for", "scanf", "printf", "return"};
string KeyCateCode[] = {"CONSTTK", "INTTK", "CHARTK", "VOIDTK", "MAINTK", "IFTK", "ELSETK", 
"SWITCHTK", "CASETK", "DEFAULTTK", "WHILETK", "FORTK", "SCANFTK", "PRINTFTK", "RETURNTK"};
string symbol[] = {"+", "-", "*", "/", "<", "<=", ">",
                ">=", "==", "!=", ":", "=", ";", ",", "(", 
                ")", "[", "]", "{", "}"};
string symCateCode[] = {"PLUS", "MINU", "MULT", "DIV", "LSS", "LEQ", "GRE",
                "GEQ", "EQL", "NEQ", "COLON", "ASSIGN", "SEMICN", "COMMA", "LPARENT", 
                "RPARENT", "LBRACK", "RBRACK", "LBRACE", "RBRACE"};
map<string, SymbolType> mapKeyWord;
map<string, string> mapSymbol;

static string str;
// void judgeEqualSymbol(Union Token token, SymbolType symbolType1, SymbolType symbolType2) {
//     CLEAR_AND_GET
//     str.append(1, c);
//     c = getChar();
//     if (c == '=') {
//         str.append(1, c);
//         symbolType = EQL;
                
//     }
//     else {
//         inFile.unget();
//         symbolType = ASSIGN;
//     }
// }
char LexicalAnalyzer::getChar() {
    char inter = content[index++];
    return inter;
}

LexicalAnalyzer::LexicalAnalyzer(Error &aerror):error(aerror) {
    for (int i = 0; i < 15; i++) {
        mapKeyWord.insert(map<string, SymbolType>::value_type (keyWord[i], SymbolType(i)));
    }
    fileLength = 0;
    lineNum = 1;
    index = 0;
}

//LexicalAnalyzer &LexicalAnalyzer::initialLexicalAnalyzer(Error &aerror) {      
//    static LexicalAnalyzer theLexicalAnalyzer(aerror);
//    return theLexicalAnalyzer;
//}

void LexicalAnalyzer::readFile() {
    fstream inFile;
    inFile.open("testfile.txt", ios::in);
    istreambuf_iterator<char>beg(inFile), end;
    content = string(beg, end);
    //printf("%s",content.c_str());
    inFile.close();
    fileLength = content.size();
    c = getChar();
    //printf("%c\n", c);
}


void LexicalAnalyzer::getSymbol() {
    token.number = 0;
    while (1) {
        if (index > fileLength) {
            symbolType = FINISH;
            return;
        }
        else if (c == '\n') {
            lineNum = lineNum + 1;
            c = getChar();
        }
        else if (isspace(c)) {
            c = getChar();
        }
        else {
            break;
        }

    }       
    if (c == '\'') {
        CLEAR_AND_GET
        c = getChar();
        //printf("mydddd %c\n", c);
        if (c == '+' || c == '-' || c == '*' || c == '/' || isdigit(c) || isalpha(c) || c == '_') {
            str.append(1, c);
            c = getChar();
            if (c != '\'') {
                error.errorLog(illegalChar, lineNum);
                while (c != '\'') {
                    c = getChar();
                    
                }
                symbolType = CHARCON; 
                c = getChar();
                //printf("mydddde %c %d \n", c,lineNum);
                return;
                
            
            }
            symbolType = CHARCON; 
            c = getChar();    
            return;
        }
        else if (c == '\'') {
            error.errorLog(illegalChar, lineNum);
            symbolType = CHARCON; 
            c = getChar();    
            return;
            
        }
        else {
            error.errorLog(illegalChar, lineNum);
            while (c != '\'') {
                c = getChar();
                
            }
            printf("mydddde %c %d \n", c,lineNum);
            symbolType = CHARCON; 
            c = getChar();    
            return;
        }
    }
    else if (c == '\"') {
        CLEAR_AND_GET
        str.clear();
        c = getChar();
        if (c == '\"') {
            error.errorLog(illegalString, lineNum);
        }
        int flag = 0;
        while (c != '\"') {
            
            if ((int)c == 32 || (int)c == 33 || ((int)c >= 35 && (int)c <= 126)) {
                str.append(1, c);
            }
            else {
                if (flag == 0) {
                    error.errorLog(illegalString, lineNum);
                    printf("");
                    flag = 1;
                }
                
            }
            
            c = getChar();
        }
        c = getChar();
        symbolType = STRCON;       
    }
    else if (isalpha(c) || c == '_') {
        CLEAR_AND_GET
        while (isalpha(c) || c == '_' || isdigit(c)) {
            str.append(1, c);
            c = getChar();
        }
        string str1 = str;
        transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
        map<string, SymbolType>::iterator iter;
        iter = mapKeyWord.find(str1);
        if (iter != mapKeyWord.end()) {
            transform(str.begin(), str.end(), str.begin(), ::tolower);
            symbolType = iter->second;   
        }
        else {
            symbolType = IDENFR;
        }
    }
    else if (isdigit(c)) {
        CLEAR_AND_GET
        token.number = 0;
        while (isdigit(c)) {
            //printf("dd%d\n",token.number);
            token.number = token.number * 10 + (c - '0');
            //printf("%dffff\n", token.number);
            // str.append(1, c);
            c = getChar();
        }
        //printf("%deeee\n", token.number);
        symbolType = INTCON;
    }
    else if (c == '=') {
        //judgeEqualSymbol();
        CLEAR_AND_GET
        str.append(1, c);
        c = getChar();
        if (c == '=') {
            str.append(1, c);
            symbolType = EQL;
            c = getChar();
        }
        else {
            symbolType = ASSIGN;
        }
    }
    else if (c == '<') {
        CLEAR_AND_GET
        str.append(1, c);
        c = getChar();
        if (c == '=') {
            str.append(1, c);
            symbolType = LEQ;
            c = getChar();
        }
        else {
            symbolType = LSS;
        }
    }
    else if (c == '>') {
        CLEAR_AND_GET
        str.append(1, c);
        c = getChar();
        if (c == '=') {
            str.append(1, c);
            symbolType = GEQ;
            c = getChar();  
        }
        else {
            symbolType = GRE;
        }
    }
    else if (c == '!') {
        CLEAR_AND_GET
        str.append(1, c);
        c = getChar();
        if (c == '=') {
            str.append(1, c);
            symbolType = NEQ;
            c = getChar();   
        }
        else {
            //error
        }
    }
    else {
        CLEAR_AND_GET
        str.append(1, c);
        switch(c) {
            case '+' :
                symbolType = PLUS;
                break;
            case '-' :
                symbolType = MINU;
                break;
            case '*' :
                symbolType = MULT;
                break;
            case '/' :
                symbolType = DIV;
                break;
            case ':' :
                symbolType = COLON;
                break;
            case ';':
                symbolType = SEMICN;
                break;
            case ',' :
                symbolType = COMMA;
                break;
            case '(' :
                symbolType = LPARENT;
                break;
            case ')' :
                symbolType = RPARENT;
                break;
            case '[' :
                symbolType = LBRACK;
                break;
            case ']' :
                symbolType = RBRACK;
                break;
            case '{' :
                symbolType = LBRACE;
                break; 
            case '}' :
                symbolType = RBRACE;
                break;   
            default:
                error.errorLog(illegalChInFile, lineNum);
                break;
        }
        c = getChar();
    } 
}
