#ifndef GRAMMA_ANALYZER__H
#define GRAMMA_ANALYZER__H
#include <string>
#include <vector>
#include "LexicalAnalyzer.h"
#include "SymbolTable.h"
#include "type.h"
#include "error.h"
#define GETSYM                                                                 \
    if (OUT == 1 && !buffer1.empty())                                          \
    {                                                                          \
        toFile();                                                              \
    }                                                                          \
    lineNum = LINE;                                                            \
    lexicalAnalyzer.getSymbol();                                               \
    if (OUT == 1 && SYMTYPE != FINISH)                                         \
    {                                                                          \
        buffer1.clear();                                                       \
        buffer1 = buffer1.append(type2String(SYMTYPE));                        \
        buffer1 = buffer1.append(" ");                                         \
        if (SYMTYPE == INTCON)                                                 \
        {                                                                      \
            buffer1 = buffer1.append(to_string(lexicalAnalyzer.token.number)); \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            buffer1 = buffer1.append(*lexicalAnalyzer.token.word);             \
        }                                                                      \
    }
#define SYMTYPE lexicalAnalyzer.symbolType
#define SAVE                                          \
    lexicalAnalyzer.oldIndex = lexicalAnalyzer.index;
#define BACKTRACK                                     \
    lexicalAnalyzer.index = lexicalAnalyzer.oldIndex; \
    lexicalAnalyzer.c = lexicalAnalyzer.content[lexicalAnalyzer.index - 1];
#define LINE lexicalAnalyzer.lineNum

#define GETITEM string str1 = *lexicalAnalyzer.token.word; \
        transform(str1.begin(), str1.end(), str1.begin(), ::tolower); \
        map<string, SymbolItem>::iterator iter; \
        iter = funcTable.find(str1); \
        if (iter == funcTable.end()) { \
            iter = globalTable.find(str1); \
            if (iter == globalTable.end()) { \
                error.errorLog(identNotDefine, LINE); \
                skipto(SEMICN); \
                GETSYM \
                return; \
            } \
        }
#define ITEM iter->second
class GrammarAnalyzer {
    public:
        
        GrammarAnalyzer(LexicalAnalyzer &aLexicalAnalyzer, Error& error);
        void toFile();
        void toFile(string str);
        void program();
        void mainFunc();
        void constDeclaration();
        void constDefine();
        int integerCon();
        int integerUnsigned();
        void varDeclaration();
        bool varDefine();
        bool varDefineNonInit(ValueType valueType, int flag);
        void varDefineInit(ValueType valueType, int dim, string name, ItemType itemType, int i, int j);
        void constant(ValueType valueType);
        ValueType constant(int *in);
        void returnFun();
        void nonReturnFun();
        string declarationHead();
        void paraTable();
        void compoundStat();
        void statList(int flag);
        void stat();
        void loopStat();
        void condition();
        int stride();
        ValueType expr(string &tmpName);
        ValueType expr(vector<PostfixItem>& infix);
        ValueType item(vector<PostfixItem> &infix);
        ValueType factor(vector<PostfixItem> &infix);
        void conditionalStat();
        void funStatReturn(string funcName);
        void funStatNonReturn(string funcName);
        void valueParaTable(string funcName);
        void assignStat(ValueType vtl, string funcName);
        void caseStat();
        void caseTable(ValueType vt, string tmpName, string label);
        void caseSubStat(ValueType vt, string tmpName, string label, string nextLabel);
        void defaultStat();
        void scanfStat();
        void printfStat();
        void returnStat();
        void skipto(SymbolType st);
        void addItem(string name, string funcName, ItemType itemType, ValueType valueType, int order);
                void addItem(string name, ItemType itemType, ValueType valueType, int i, int j, int dim);
        void addItem(string name, ItemType itemType, ValueType valueType);
        void addItem(string name, ItemType itemType, ValueType valueType, int value);
        void addItem(string name, ItemType itemType, ValueType valueType, string value);
        void addItem(string name, FuncType funcType);
        bool isDefined(string name);
        bool isNotDefined(string name);
        void updateField(string name, FuncType funcType);

    private:
        LexicalAnalyzer &lexicalAnalyzer;
        Error& error;
        
    // private:

    //     int symbolCount;
};
#endif
