#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include "LexicalAnalyzer.h"
#include "GrammarAnalyzer.h"
#include "error.h"

int main() {
    Error error(1);
    LexicalAnalyzer lexicalAnalyzer(error);
    
    lexicalAnalyzer.readFile();
    GrammarAnalyzer grammaAnalyzer(lexicalAnalyzer, error);
    grammaAnalyzer.program();
}