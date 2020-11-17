#ifndef LEXICAL_ANALYSIS__H
#define LEXICAL_ANALYSIS__H
#include <fstream>
#include "type.h"
#include "error.h"
using namespace std;
#define  CLEAR_AND_GET token.word = &str; str.clear();

class LexicalAnalyzer {
    public:
        LexicalAnalyzer(Error& error);
        //static LexicalAnalyzer &initialLexicalAnalyzer();
        void readFile();
        char getChar();
        void getSymbol();
        void set();
        SymbolType symbolType;
    private:
        Error& error;
        string content;
        long fileLength;
        int lineNum;
        long index;
        long oldIndex;
        char c;
        union Token {
            int number;
            string *word;
        } token;
        
        friend class GrammarAnalyzer;
};

ostream &operator<< (ostream &ofs, SymbolType st);
#endif