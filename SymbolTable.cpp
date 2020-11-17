#include <map>
#include "SymbolTable.h"
using namespace std;
map<string, map<string, SymbolItem>> symbolTables;
SymbolItem::SymbolItem(string name1, string funcName1) {
    name = name1;
    funcName = funcName1;
    dimx = 0;
    dimy = 0;
}