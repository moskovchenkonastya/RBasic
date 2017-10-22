#ifndef __RBasic__parser__
#define __RBasic__parser__

#include <vector>
#include "lex.h"

class Interpreter;

class Parser {
    std::vector<lex> v;
    const lex& l();
    size_t position;
    size_t rpnPosition;
    lexScanner& scanner;
    
    bool Expression(bool global, bool blockMode);
    bool For();
    bool Block();
    bool Exp1();
    bool Exp2();
    bool Exp3();
    bool Exp4();
    bool Exp5();
    bool Exp6();
    bool Exp7();
    bool Variable(bool lvalue);
    bool ArgList();
    bool ArgListElem();
    bool F();
    void gl();
public:
    Parser (lexScanner& s);
    std::vector<lex> rpn;
    bool Program(bool blockMode);
};

#endif /* defined(__RBasic__parser__) */
