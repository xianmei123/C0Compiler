#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include <string>
#include "type.h"
using namespace std;
class SymbolItem {
    private:
        string name; //标识符名
        int order; //序号
        string funcName; //所在函数名，全局变量则赋值为“global”
        ItemType itemType; //标识符类型
        ValueType valueType; //标识符值类型
        FuncType funcType; //函数类型
        int dimx;
        int dimy;
        int dim;
        int size;
    public:
        struct Value {
            int number;
            char ch;
            string str;
        } value;
        SymbolItem(string name1, string funcName1);
        int getOrder() {
            return order;
        }
        void setOrder(int order1) {
            order = order1;
        }
        string getName() {
            return name;
        }
        void setName(string name1) {
            name = name1;
        }
        string getFuncName() {
            return funcName;
        }
        void setFuncName(string funcName1) {
            funcName = funcName1;
        }
        ItemType getItemType() {
            return itemType;
        }
        void setItemType(ItemType itemType1) {
            itemType = itemType1;
        } 
        ValueType getValueType() {
            return valueType;
        }
        void setValueType(ValueType valueType1) {
            valueType = valueType1;
        } 
        FuncType getFuncType() {
            return funcType;
        }
        void setFuncType(FuncType funcType1) {
            funcType = funcType1;
        }
        int getSize() {
            return size;
        }
        void setSize(int size1) {
            size = size1;
        }
        int getDimx() {
            return dimx;
        }
        void setDimx(int dimx1) {
            dimx = dimx1;
        }
        int getDimy() {
            return dimy;
        }
        void setDimy(int dimy1) {
            dimy = dimy1;
        }
        int getDim() {
            return dim;
        }
        void setDim(int dim1) {
            dim = dim1;
        }
};
#endif