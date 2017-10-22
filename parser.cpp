#include "parser.h"
#include "interpreter.h"

#include <sstream>

void Parser::gl() {
    position++;
    if (position >= v.size()) {
        position = v.size();
        v.push_back(scanner.getNextLex());
    }
}

const lex& Parser::l() {
    return v[position];
}

Parser::Parser(lexScanner& s) : scanner(s), position(0), rpnPosition(0) {
}

// блок – это программа, которая заканчивается фигурной скобкой и возвращает последнее значение
// и выводит всегда плюсики, а значение тоже только последнее. а, не он выводит.
// для этой программы создать отдельный парсер (с тем же сканером)


bool Parser::Program(bool blockMode) {
    Interpreter* iBig = nullptr;
    bool first = true;
    scanner.a = (blockMode ? '+' : '>');
    // нужно втолкнуть перед позицией, чтобы после {1 становилось {\n1, а не {1\n
    //v.push_back(LEX_ENDL);
    v.insert(v.begin() + position, LEX_ENDL);
    size_t backup = position;
    size_t rpnSizeBackup = rpn.size();
    if (!blockMode) iBig = new Interpreter(rpn, scanner.os);
    while (l() == LEX_SEMICOLON || l() == LEX_ENDL) {
        if (first) { gl(); first = false; }
        bool flag = true;
        if (l() == LEX_SEMICOLON) gl();
        while (l() == LEX_ENDL) { if (flag) cout << (blockMode ? "+ " : "> "); flag ^= 1; gl(); }
        scanner.os.str("");
        if (l() == (blockMode ? LEX_RBRACE : LEX_END)) return true;
        backup = position;
        rpnSizeBackup = rpn.size();
        scanner.a = '+';
        while (!Expression(!blockMode, blockMode)) {
            // интерактивный интерпретатор должен сразу переводить в RPN и выполнять
            rpn.erase(rpn.begin() + rpnSizeBackup, rpn.end());
            // l() - символ, на котором он сломался? или нет?
            // если он встретил '\n' внутри, то он не будет считывать новые символы?
            if (v[v.size() - 1] == LEX_ENDL) {
                flag = true;
                do {
                    v.erase(v.end() - 1);
                    position = v.size() - 1;
                    if (flag) cout << scanner.os.str();
                    //cout.flush();
                    flag ^= 1;
                    scanner.os.str("");
                    gl();
                } while (l() == LEX_ENDL);
                position = backup;
            }
            else return false; // причина не в этом
            scanner.os.str("");

        }
        scanner.a = (blockMode ? '+' : '>');

        v.erase(v.begin(), v.end() - 1);
        /*for (size_t i = 0; i < rpn.size(); i++)
            cout << rpn[i] << endl;*/

        scanner.os.str("");
        if (!blockMode) {
            iBig->run();
            rpn.clear();
        }
        cout << scanner.os.str();
        scanner.os.str("");
    }
    return l() == (blockMode ? LEX_RBRACE : LEX_END);
    // комментарий с новой строки должен игнорироваться, а после оператора – считаться концом строки...
}

bool Parser::Block() {
    if (l() != LEX_LBRACE) return false;
    gl();

    // нужно записывать в ПОЛИЗ, но не выполнять
    if (!Program(true)) return false;

    if (rpn[rpn.size() - 1] == LEX_SEMICOLON)
        rpn.erase(rpn.end() - 1);

    if (l() != LEX_RBRACE) return false;
    gl();
    return true;
}

// Expression -> Variable <- Expression | Exp1 | Block
// проверяем на блок
// если не блок, то выделяем Variable <- и проверяем на Expression (правая рекурсия)
// если не Variable <-, то проверяем на Exp1
bool Parser::Expression(bool global, bool blockMode) {
    if (l() == LEX_LBRACE) return Block();
    size_t backup = position;
    size_t rpnbackup = rpn.size();
    int i = 0;
    lex op = lex(LEX_ASSIGN);
    while (true) {
        if (!Variable(true)) {
            position = backup;
            rpn.erase(rpn.begin() + rpnbackup, rpn.end());
            break;
        }
        if (l() != LEX_ASSIGN) {
            position = backup;
            rpn.erase(rpn.begin() + rpnbackup, rpn.end());
            break;
        }
        else {
            gl();
            backup = position;
            rpnbackup = rpn.size();
            i++;
        }
    }
    if (i && !Expression(false, false)) return false;
    if (!i && !Exp1()) return false;
    for (int j = 0; j < i; j++) rpn.push_back(op);

    if (global) {
        rpn.push_back(i ? LEX_SEMICOLON : LEX_ENDL);
    }
    else { //
        if (blockMode) rpn.push_back(LEX_SEMICOLON); // ничего не выводить
    }

    return true;
}

bool Parser::Exp1() {
    if (!Exp2()) return false;
    while (l() == LEX_AND || l() == LEX_OR) {
        lex op = l();
        gl();
        if (!Exp2()) return false;
        rpn.push_back(op);
    }
    return true;
}

bool Parser::Exp2() {
    // Exp3 не может начинаться с восклицательного знака
    lex op(LEX_NULL);
    if (l() == LEX_NOT) {
        op = l();
        gl();
    }
    bool res = Exp3();
    if (!res) return false;
    if (op != LEX_NULL) rpn.push_back(op);
    return true;
}

bool Parser::Exp3() {
    if (!Exp4()) return false;
    lex op = l();
    switch (l().type()) {
        case LEX_G:
        case LEX_L:
        case LEX_GE:
        case LEX_LE:
        case LEX_E:
        case LEX_NE:
            gl();
            if (!Exp4()) return false;
            rpn.push_back(op);

        default:
            return true;
    }
}

bool Parser::Exp4() {
    if (!Exp5()) return false;
    while (l() == LEX_PLUS || l() == LEX_MINUS) {
        lex op = l();
        gl();
        if (!Exp5()) return false;
        rpn.push_back(op);
    } 
    return true;
}

bool Parser::Exp5() {
    if (!Exp6()) return false;
    while (l() == LEX_MUL || l() == LEX_DIV) {
        lex op = l();
        gl();
        if (!Exp6()) return false;
        rpn.push_back(op);
    }
    return true;
}


bool Parser::Exp6() {
    if (!Exp7()) return false;
    while (l() == LEX_COLON) {
        lex op = l();
        gl();
        if (!Exp7()) return false;
        rpn.push_back(op);
    }
    return true;
}



bool Parser::Exp7() {
    lex arg = l();
    switch (l().type()) {
        case LEX_ID:
            gl();
            rpn.push_back(arg); // добавился идентификатор. если дальше будет вызов функции, то этот идентификатор нужно переместить в конец
            if (!F()) return false;
            return true;

        case LEX_LPAR:
            gl();
            if (!Expression(false, false)) return false;
            if (l() != LEX_RPAR) return false;
            gl();
            return true;

        case LEX_NUM:
        case LEX_STRING:
        case LEX_TRUE:
        case LEX_FALSE:
        case LEX_RNULL:
            gl();
            rpn.push_back(arg);
            return true;

        default:
            return false;
    }
}

bool Parser::F() {
    lex op, fname;
    size_t ret_addr;
    if (l() == LEX_LPAR) {
        gl();
        ret_addr = rpn.size();
        fname = rpn[rpn.size()-1]; // имя функции
        rpn.erase(rpn.begin() + (rpn.size() - 1));
        rpn.push_back(lex(RPN_FNAME, fname.value()));
        if (!ArgList()) return false; // аргументы функции
        if (l() != LEX_RPAR) return false;
        gl();
        //rpn.push_back(fname);
        rpn.push_back(lex(RPN_CALL, rpn.size() + 1)); // size - это адрес CALL
        return true;
    }
    while (l() == LEX_LBRACKET) {
        gl();
        if (!Expression(false, false)) return false;
        op = l();
        if (l() != LEX_RBRACKET) return false;
        gl();
        rpn.push_back(op);
    }
    return true;
}

bool Parser::Variable(bool lvalue) {
    if (l() != LEX_ID) return false;
    lex id = l();
    if (lvalue) id = lex(RPN_LVAL, id.value());
    rpn.push_back(id);
    gl();
    if (l() == LEX_LBRACKET) {
        gl();
        if (!Expression(false, false)) return false;
        if (l() != LEX_RBRACKET) return false;
        rpn.push_back(l());
        gl();
        return true;
    }
    return true;
}


// ArgList → ε | ArgListElem {, ArgListElem }
// ArgListElem → Expression | Identifier = Expression // тут нужно по обоим путям идти

bool Parser::ArgList() {
    size_t backup = position;
    size_t arg_count = 0;
    if (!ArgListElem()) {
        position = backup;
        rpn.push_back(lex(RPN_ARGC, arg_count));
        return true;
    }
    arg_count++;
    while (l() == LEX_COMMA) {
        gl();
        if (!ArgListElem()) return false;
        arg_count++;
    }
    // после аргументов идет число – количество аргументов
    rpn.push_back(lex(RPN_ARGC, arg_count));
    return true;
}




bool Parser::ArgListElem() {
    size_t backup = position;
    if (l() == LEX_ID) {
        gl();
        if (l() != LEX_EQ) {
            position = backup;
            return Expression(false, false);
        }
        gl();
    }
    return Expression(false, false);
}
