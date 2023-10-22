###### 代码说明：

C0（类C语言 ）编译器，实现了词法分析，语法分析，符号表构建，错误处理，代码生成，代码优化等过程，先生成自行设计的中间代码四元式，最终将C0编译成mips汇编语言。

C0：语义参照C语言，具有常量、变量、整数、字符、字符串、整型、字符型、一维/二维数组、函数（带参数）、赋值语句、if语句、while语句、for语句、switch-case语句、函数调用、复合语句、读语句、写语句（带字符串）

###### 设计说明：

一、  词法分析。

1、   编码前的设计

（1）结构设计。

Void getSymbol();该函数函数完成文件的输入输出，通过while(!inFile.eof())来循环读取至文件末尾，在该循环利用分支语句对不同类型的单词进行识别分类并按格式输出。

Void set();该函数将单词与类别码建立对应关系，通过单词的识别就能通过map找到对应的类别码。

Main函数调用该函数。

（2）细节实现。

Void getSymbol()；将单词类别分为标识符，字符常量，整型常量，字符串，二元符号，一元符号几类分别进行处理。标识符又在读完整个单词后根据是否是关键字进行不同处理。标识符，整型常量识别完后都需要回退一个字符，进行二元符号的识别时若结果为一元符号也要回退一个字符。

Void set();建立两个string数组，一个存放类别码，一次存放单词，在set函数里通过map建立一一对应的关系。

2、   编码后的设计。

写代码的过程中发现之前的设计过于简单了，是单纯为了过这次测试而写的，扩展性太差。词法分析程序是通过语法分析程序调用的，于是考虑到扩展性，并且了解了一些c++的面向对象后，选择新建GrammarAnalyzer类和LexicalAnalyzer类。

（1）GrammarAnalyzer类。

目前只包含构造方法和grammarAnalyze方法，grammarAnalyze方法简单实现了调用LexicaAnalyzer类的getSymbol方法获得单词及其类别码，并进行输出。

（2）LexicalAnalyzer类。

包含initialLexicalAnalyzer，getSymbol和set方法。initialLexicalAnalyzer方法实现了对于LexicalAnalyzer的初始化设定并返回LexicalAnalyzer类的一个实例，getSymbol方法相对于之前的设计细节上改动不大，只是将读取所有单词改为了每次调用该方法只读取一个单词。

整体相对于之前的设计有了更好的可扩展性，并在细节上做了一些优化处理。

二、  语法分析。

1、编码之前的设计。

（1）结构设计。

删去GrammarAnalyzer类的grammarAnalyze方法（该方法原用于调用词法分析方法，并进行相应输出）。

＜程序＞  ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞

根据以上文法，添加program方法，以program方法为入口，根据文法逐层向下构建下列方法。

​	`   void mainFunc();`

​    `void constDeclaration();`

​    `void constDefine();`

​    `int integerCon();`

​    `void integerUnsigned();`

​    `void varDeclaration();`

​    `bool varDefine();`

​    `bool varDefineNonInit();`

​    `void varDefineInit();`

​    `void constant();`

​    `void returnFun();`

​    `void nonReturnFun();`

​    `string declarationHead();`

​    `void paraTable();`

​    `void compoundStat();`

​    `void statList();`

​    `void stat();`

​    `void loopStat();`

​    `void condition();`

​    `void stride();`

​    `void expr();`

​    `void item();`

​    `void factor();`

​    `void conditionalStat();`

​    `void funStatReturn();`

​    `void funStatNonReturn();`

​    `void valueParaTable();`

​    `void assignStat();`

​    `void caseStat();`

​    `void caseTable();`

​    `void caseSubStat();`

​    `void defaultStat();`

​    `void scanfStat();`

​    `void printfStat();`

​    `void returnStat();`

＜语句＞  ::= ＜循环语句＞｜＜条件语句＞| ＜有返回值函数调用语句＞; |＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜情况语句＞｜＜空＞;|＜返回语句＞; | '{'＜语句列＞'}' 

由以上文法中＜有返回值函数调用语句＞; |＜无返回值函数调用语句＞;｜＜赋值语句＞; 可知需要初步构建符号表，能够判断出标识符是有返回值函数，无返回值函数、还是变量。

初步建立符号表所包含的属性如下：

```
string name; *//**标识符名*

    int order; *//**序号*

   string funcName; *//**所在函数名，全局变量则赋值为“global”*

   ItemType itemType; *//**标识符类型*

    ValueType valueType; *//**标识符值类型*

    FuncType funcType; *//**函数类型*

union Value { //值

      int number;

      string* ch;

      string* str;

    } value;
```

 

 

（2）细节设计。

＜变量说明＞与＜有返回值函数定义＞，＜无返回值函数定义＞与＜主函数＞定义有重合，（如果提取公共部分来处理的话，感觉结构会有点混乱），所以还是采用回溯。

＜变量定义无初始化＞与＜变量定义及初始化＞的重合部分较多，所以考虑全部在＜变量定义无初始化＞中进行处理，若读取到“=”，则跳到＜变量定义及初始化＞中进行处理。

其他的则按部就班根据文法定义一步步来写。

2、编码后的设计。

很多方法需要加参数传递，便于建立符号表如 

```
void constDeclaration(string funcName);

void constDefine(string funcName);
```

传递函数名，便于将该常量加入符号表并确定作用域。

 

另外constant方法进行了重写，便于在不同的方法以不同的参数调用。

```
void constant(ValueType valueType, union SymbolItem::Value value);

void constant(ValueType valueType);
```

 

对于加入符号表的操作，添加了addItem方法。

```
void addItem(string name, string funcName, ItemType itemType, ValueType valueType, union SymbolItem::Value value);

void addItem(string name, string funcName, ItemType itemType, ValueType valueType);

void addItem(string name, string funcName, ItemType itemType, FuncType funcType);
```

 

分别对应常量及已初始化变量，未初始化变量，函数。

 

 

最终方法列表如下：

```
    void program();

    void mainFunc();

    void constDeclaration(string funcName);

    void constDefine(string funcName);

    int integerCon();

    void integerUnsigned();

    void varDeclaration(string funcName);

    bool varDefine(string funcName);

    bool varDefineNonInit(ValueType valueType, int flag, string funcName);

    void varDefineInit(ValueType valueType, int dim, string name, string funcName, ItemType itemType, union SymbolItem::Value value);

    void constant(ValueType valueType, union SymbolItem::Value value);

    void constant(ValueType valueType);

    void returnFun();

    void nonReturnFun();

    string declarationHead();

    void paraTable(string funcName);

    void compoundStat(string funcName);

    void statList();

    void stat();

    void loopStat();

    void condition();

    void stride();

    void expr();

    void item();

    void factor();

    void conditionalStat();

    void funStatReturn();

    void funStatNonReturn();

    void valueParaTable();

    void assignStat();

    void caseStat();

    void caseTable();

    void caseSubStat();

    void defaultStat();

    void scanfStat();

    void printfStat();

    void returnStat();

    void addItem(string name, string funcName, ItemType itemType, ValueType valueType, union SymbolItem::Value value);

    void addItem(string name, string funcName, ItemType itemType, ValueType valueType);

    void addItem(string name, string funcName, ItemType itemType, FuncType funcType);
```

 

在输出过程中，对于需要回溯的部分采用了缓存机制，在判断是否回溯时，先将所有需要输出的部分存入缓存，若需要回溯，则清空缓存；若不需要回溯，则输出所有缓存并清空。

三、  错误处理。

1、编码之前的设计。

因为语法分析只需要简单的判断标识符属于哪种类型，只是很简单的设计了一下符号表，所以要重新构建一个合理的符号表架构。

（1）符号表设计。

采用map<string,map<string, SymbolItem>> symbolTables;这样map嵌套的方式设计符号表，symbolTables的键是函数名，值是函数名对应的局部符号表。

```
map<string, SymbolItem> globalTable;
```

另外，globalTable则用来存放全局的符号，包括全局常量，全局变量以及函数。GlobalTable中的键为标识符名，键为标识符对应的信息（包括标识符类型，函数类型，值类型，维数，标识符名，所在函数名等信息，全部保存在SymbolItem类中）。其他函数对应的局部符号表与globalTable类似，并且都保存进symbolTables中。

添加全局变量map<string, SymbolItem> *symbolTable指向语法分析过程中当前作用域所对应的符号表。

对于函数调用语句，要根据函数名去找对应的符号表中对应的参数有点麻烦，所以还添加了全局变量map<string, vector<SymbolItem>> paras;用来保存函数对应的参数，paras的键是函数名，值是函数对应的所有参数。

（2）符号表建立过程。

symbolTable初始化指向globalTable，每当识别一个函数时，就新建map<string, SymbolItem>类型的符号表，使symbolTable指向它，并将其加入symbolTables。所有需要存进符号表的标识符都存进symbolTable指向的符号表。

（3）错误处理设计。

新增Error类，包括方法void errorLog(ErrorCode ec, int lineNumber);（根据传入的错误码和行数进行输出）和构造函数。

在main函数中实例化Error，并将其传入GrammarAnalyzer类和LexicalAnalyzer类的构造函数中共用。

a类错误：

在LexicalAnalyzer类中的getSymbol()函数中进行判断字符和字符串中有无非法符号，或无任何符号。

b类错误：

在GrammarAnalyzer类中新增isDefined()方法，在当前作用域中查找是否有重名定义，已定义则放回true；

c类错误：

新增isNotDefined()方法，在当前作用域和全局符号表中查找标识符是否定义，未定义则返回true；

d，e类错误：

在识别出标识符为函数时，将值参数表与对应的参数表对比。

f类错误：

条件判断的左右表达式都通过表达式分析函数来处理，并返回表达式类型，判断是否为整型。

g，h类错误：

根据当前作用域函数的类型判断该return语句返回值类型是否正确，或者有无返回值语句。

i类错误：

根据表达式类型判断是不是字符型。

j类错误：

在给标识符赋值的情况下，判断是否标识符是常量。

k，l，m类错误：

在该出现分号，右小括号，右中括号的地方，判断是否出现，未出现则报错，出现则继续读下一个符号。

n类错误：

在数组初始化中判断维数是否匹配。

o类错误：

在变量定义初始化及switch语句中判断变量类型与值是否一致。

p类错误：

类似无return语句的判断。

3、   编码之后的修改。

（1）符号表修改。

在写代码的过程中发现symbolTables这个变量没有什么存在的必要，因为所有关于标识符的判断只需要当前作用域及全局作用域即可。

所以修改后的符号表设计使用了

```
map<string, SymbolItem> funcTable;

map<string, SymbolItem> globalTable;

map<string, vector<SymbolItem>> paras;
```

三个变量，分别保存当前作用域的标识符，全局标识符，以及函数对应的参数。

每当分析到一个函数时，将funcTable清空，如果该函数不是重定义，就将其加入globalTable中，并将对应的参数表加入paras中，否则不加入。另外，在该函数中读取到的标识符都存入funcTable中。

（2）错误处理修改。

K，l，m类错误：

其中分号的处理

A=b

A=c；

这样的应该报错在A=b行，但当发现错误时已经读取到了A=c;中的A了，所以新增变量lineNum来保存读取b（即分号的前一个符号）时的行号，当发现缺少分号时，即报错在该行号。

另外，for循环中也要考虑c类（名字未定义）和j类（给常量赋值）错误。

无return语句和无default语句要报错在}所在行号。

四、代码生成。

1、编码之前的设计。

（1）四元式定义（没有给出的部分）：

读语句 read char/int x

写语句 print string/char/int/ident x

函数定义int foo()

参数定义 para int a

变量定义 var int a

（2）四元式代码设计。

定义结构体，包含midCodeType（四元式类型），target，left，op，right（如赋值语句a=b+c，与前四个变量一一对应），valueType（如赋值语句target的值类型），leftValue（如果left为常数，就用leftValue存储left的值），rightValue（同leftValue），targetValue（如果target的值可计算，则用targetValue存储结算结果），funcType（如函数定义，函数调用，存储函数的类型，返回int或char或无返回值），x、y（用来存储数组变量两个维度的大小），arrayName（存储数组名）。

MidCodeType如下：

```
enum MidCodeType

{

  VarDef,

  ValueParamDeliver, // 值参传递

  FunctionCall,

  AssignArray,

  AssignByArray,

  AssignState,

  Label,

  FunctionDef,

  ParamDef,

  Goto, // 无条件跳转

  BZ, // 等于

  BNZ, // 不等于

  LSSm, // 小于

  LEQm, // 小于等于

  GREm, // 大于

  GEQm, // 大于等于

  EQLm,

  NEQm,

  ReadChar,

  ReadInt,

  PrintStr,

  PrintBeforeExpr,

  PrintChar,

  PrintInt,

  PrintIdent,

  ReturnInt,

  ReturnChar,

  ReturnEmpty,

  ReturnIdent,

  Finish

};
```

（3）表达式处理。

先在语法分析expr时获得中缀表达式，再转化为后缀表达式，再生成四元式。

（4）常量、变量处理。

常量当作常数处理，直接以常数形式存储进四元式中。设一变量名为x，为全局变量，则在四元式中变量名改为G_x；若变量x在函数init中定义，则在四元式中变量名改为L_init_x。这样做为了保证所有变量在四元式中变量名唯一。中间变量中间变量名生成函数string generateTmpName(int num)生成，形式为T_num。

（5）数组处理。

数组定义时，用四元式的x、y存储数组变量两个维度的大小，target存储数组名，valueType存储数组类型。

数组被赋值时，用四元式的leftValue，rightValue存储数组的下标，target存储要赋给数组的变量名或值，arrayName存储数组名。

用数组成员给其他变量赋值时，用四元式的leftValue，rightValue存储数组的下标，target存储被赋值的变量名，arrayName存储数组名。

（6）函数处理。

将函数声明与参数添加进四元式，将main当作无返回值无参数的函数处理。若函数最后没有返回语句，则添加return empty的四元式。

（1）字符串。

在生成中间代码过程中，保存所有要输出的字符串，生成mips时，将这些字符串均转换为mips代码"str0:  .asciiz  \"\\n\""，并计算所有字符串所用的空间。

（2）变量。

全局变量：在生成中间代码时，计算出全局变量相对于基地址的偏移，如第一个全局变量为int类型，偏移为0，第二个全局变量偏移就为4。基地址由0x10010000加上字符串所占用内存大小得出。

局部变量：在生成中间代码时，计算出每个函数内的局部变量（把参数也当成局部变量）相对于$fp（$fp的值在函数部分说明）的偏移，如init函数中第一个局部变量为int类型，则其偏移-12（偏移-4存储了上一个栈帧的$fp，偏移-8存储了$ra），第二个局部变量偏移为-16。

（3）函数。

用$fp到的$sp内存空间保存当前函数的变量，每次调用一个函数，将$fp存进$sp-4的位置，将$ra存进$sp-8的位置，然后将$sp的值存进$fp，并将$sp减去预先算好的对应函数的内存空间大小。就得到了新的$fp和$sp，对应新的函数的内存空间。

（4）数组。

数组当作一串连续的变量来存储，与变量存储方式无本质差别，读取的时候根据索引来计算偏移就可以。

（5）中间变量。

中间变量用t寄存器来存储，循环分配。

2、编码之后的设计。

编码前后的设计改动不大，可能是开始写之间就想好了很多细节，基本上都是按照编码之前的设计完成的。Mips的中间变量部分原本是想循环分配寄存器，原本是考虑不会存在某段语句会同时需要9个中间变量，但实际上有时候会出问题。所以暂时也将中间变量保存在了栈中，类似局部变量，并将其保存空间放在了局部变量保存空间的后面。

五、  代码优化。

大致实现了局部公共子表达式删除，也写了活跃变量分析，临时变量寄存器的分配以及常量传播。

（1） 局部公共子表达式删除。

按照书上的方法先进行分块，新建Block类，包含vector<int> prev(前驱块号)，vector<int> post（后继块号），vector<string> def，vector<string> use，vector<string> in， vector<string> out（到达定义数据流分析用）。

然后采用理论课上介绍的dag图方法，新建DagNode结构，包括op（运算符），lIndex，rIndex（左右子结点对应的结点编号）和name（结点名）。vector<DagNode> nodes存储所有Dag图上的结点，map<string, int> str2Node存储变量名到结点的映射，最后采用该书上的方法进行编程。

（2） 临时变量寄存器分配。

记录块内临时变量的定义位置和最后的使用位置，有此判断临时变量间是否冲突，用graph[m][m]存储冲突图的邻接矩阵，再用图着色算法分配好t寄存器。

（3） 常量传播。

记录值为常量的变量，删去这条中间代码，并在之后用到此变量的中间代码都用常量替换，如果这个变量以后会用到，则不删除这条中间代码。