#ifndef RBasic_interpreter_h
#define RBasic_interpreter_h

#include <stack>
#include "ident.h"

using namespace std;

class Interpreter {
    vector<lex>& rpn;
    ostream& os;
public:
    Interpreter(vector<lex>& v, ostream& o) : rpn(v), os(o) { }
    stack<value*> st; 
    // нужно класть и значения, и идентификаторы.
    // а, нужно класть значения адресного типа.
    void run();
};

#endif
