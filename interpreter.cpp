#include "lex.h"
#include "parser.h"
#include "ident.h"
#include <stack>
#include <fstream>
#include <stdexcept>
#include "interpreter.h"

// нужно сделать, чтобы после блока оператор снятия со стека исчезал

namespace stdfunc {

    value* c(const vector<value*>& v) {
        // типы: если там есть строка, то строка
        // иначе если есть число, то числоь
        // иначе если есть булева, то булева
        // иначе нуль
        bool hasString = false, hasNumber = false, hasBoolean = false, hasNull = true;
        for (size_t i = 0; i < v.size(); i++) {
            switch (v[i]->mode()) {
                case value::ID_STRING: hasString = true; break;
                case value::ID_NUM: hasNumber = true; break;
                case value::ID_BOOL: hasBoolean = true; break;
                case value::ID_NULL: hasNull = true; break;
                default: break;
            }
        }
        if (hasString) {
            Vector<String> w;
            for (ssize_t i = v.size() - 1; i >= 0; i--) {
                Vector<String> y = *v[i];
                for (size_t j = 0; j < y.size(); j++)
                    w.push_back(y[j]);
            }
            return new character(w);
        }
        if (hasNumber) {
            Vector<LongDouble> w;
            for (ssize_t i = v.size() - 1; i >= 0; i--) {
                Vector<LongDouble> y = *v[i];
                for (size_t j = 0; j < y.size(); j++)
                    w.push_back(y[j]);
            }
            return new numeric(w);
        }
        if (hasBoolean) {
            Vector<Bool> w;
            for (ssize_t i = v.size() - 1; i >= 0; i--) {
                Vector<Bool> y = *v[i];
                for (size_t j = 0; j < y.size(); j++)
                    w.push_back(y[j]);
            }
            return new logical(w);
        }
        return new value();
    }

    value* mode (const vector<value*>& v) {
        character* res = new character;
        for (ssize_t i = v.size() - 1; i >= 0; i--) {
            switch (v[i]->mode()) {
                case value::ID_BOOL: res->val.push_back(string("\"logical\"")); break;
                case value::ID_STRING: res->val.push_back(string("\"character\"")); break;
                case value::ID_NUM: res->val.push_back(string("\"numeric\"")); break;
                case value::ID_NULL: res->val.push_back(string("\"NULL\"")); break;
                default: res->val.push_back(string("\"error\"")); break;
            }
        }
        return res;
    }

    value* length (const vector<value*>& v) {
        // ой, у меня length принимает сколько угодно аргументовы
        numeric* res = new numeric;
        for (ssize_t i = v.size() - 1; i >= 0; i--)
            res->val.push_back(v[i]->length());
        return res;
    }

    typedef value* (*fptr)(const vector<value*>&);
    static map<string, fptr> func({
        {"c", c},
        {"mode", mode},
        {"length", length}
    });
}

void Interpreter::run() {
    // TODO: удалять снимаемые со стека значения!
    st = stack<value*>();
    for (size_t i = 0; i < rpn.size(); i++) {
        value *v, *a, *b;
        switch (rpn[i].type()) {
            case RPN_GOTO:
                i = rpn[i].value() - 1; // будет i++;
                continue;
                
            case LEX_ID:
                // положить значение
                st.push(tid[rpn[i].value()].val->copy());
                break;

            case RPN_FNAME:
            case RPN_ARGC:
                // хранится в виде адреса, но не является им
                st.push(new address(rpn[i].value()));
                break;

            case RPN_LVAL:
                // положить адрес
                st.push(new indexed_address(rpn[i].value()));
                break;

            case LEX_STRING:
                st.push(new character(sid[rpn[i].value()]));
                break;

            case LEX_NUM:
                st.push(new numeric(nid[rpn[i].value()]));
                break;

            case LEX_RNULL:
                st.push(new value);
                break;

            case LEX_TRUE:
            case LEX_FALSE:
                st.push(new logical(rpn[i].type()));
                break;

            case LEX_AND:
            case LEX_OR: {
                b = st.top(); st.pop();
                a = st.top(); st.pop();
                logical* r = new logical;
                Vector<Bool> va = *a, vb = *b;
                for (size_t j = 0; j < max(va.size(), vb.size()); j++)
                    if (rpn[i].type() == LEX_AND) r->val.push_back(va[i%va.size()] && vb[i%vb.size()]);
                    else r->val.push_back(va[i%va.size()] || vb[i%vb.size()]);
                st.push(r);
            }
                break;

            case LEX_NOT:{
                a = st.top(); st.pop();
                logical* r = new logical;
                Vector<Bool> va = *a;
                delete a;
                for (size_t i = 0; i < va.size(); i++)
                    r->val.push_back(!va[i%va.size()]);
                st.push(r);
            }
                break;

            case LEX_G:
            case LEX_GE:
            case LEX_L:
            case LEX_LE:
            case LEX_E:
            case LEX_NE: {
                b = st.top(); st.pop();
                a = st.top(); st.pop();
                logical *res = new logical;
                if (a->mode() == value::ID_STRING || b->mode() == value::ID_STRING) {
                    Vector<String> va = *a, vb = *b;
                    for (size_t j = 0; j < max(va.size(), vb.size()); j++)
                        switch (rpn[i].type()) {
                            case LEX_NE: res->val.push_back(va[j%va.size()] != vb[j%vb.size()]); break;
                            case LEX_E: res->val.push_back(va[j%va.size()] == vb[j%vb.size()]); break;
                            case LEX_G: res->val.push_back(va[j%va.size()] > vb[j%vb.size()]); break;
                            case LEX_GE: res->val.push_back(va[j%va.size()] >= vb[j%vb.size()]); break;
                            case LEX_L: res->val.push_back(va[j%va.size()] < vb[j%vb.size()]); break;
                            case LEX_LE: res->val.push_back(va[j%va.size()] <= vb[j%vb.size()]); default: break;
                        }
                }
                else if (a->mode() == value::ID_NUM || b->mode() == value::ID_NUM) {
                    Vector<LongDouble> va = *a, vb = *b;
                    for (size_t j = 0; j < max(va.size(), vb.size()); j++)
                        switch (rpn[i].type()) {
                            case LEX_NE: res->val.push_back(va[j%va.size()] != vb[j%vb.size()]); break;
                            case LEX_E: res->val.push_back(va[j%va.size()] == vb[j%vb.size()]); break;
                            case LEX_G: res->val.push_back(va[j%va.size()] > vb[j%vb.size()]); break;
                            case LEX_GE: res->val.push_back(va[j%va.size()] >= vb[j%vb.size()]); break;
                            case LEX_L: res->val.push_back(va[j%va.size()] < vb[j%vb.size()]); break;
                            case LEX_LE: res->val.push_back(va[j%va.size()] <= vb[j%vb.size()]); default: break;
                        }
                }
                else {
                    Vector<Bool> va = *a, vb = *b;
                    for (size_t j = 0; j < max(va.size(), vb.size()); j++)
                        switch (rpn[i].type()) {
                            case LEX_NE: res->val.push_back(va[j%va.size()] != vb[j%vb.size()]); break;
                            case LEX_E: res->val.push_back(va[j%va.size()] == vb[j%vb.size()]); break;
                            case LEX_G: res->val.push_back(va[j%va.size()] > vb[j%vb.size()]); break;
                            case LEX_GE: res->val.push_back(va[j%va.size()] >= vb[j%vb.size()]); break;
                            case LEX_L: res->val.push_back(va[j%va.size()] < vb[j%vb.size()]); break;
                            case LEX_LE: res->val.push_back(va[j%va.size()] <= vb[j%vb.size()]); default: break;                       }
                }
                delete a; delete b;
                st.push(res);
            }
                break;

            case LEX_UNARY_MINUS:
            case LEX_UNARY_PLUS:
                break;

            case LEX_PLUS:
            case LEX_MINUS:
            case LEX_MUL:
            case LEX_DIV: {
                b = st.top(); st.pop();
                a = st.top(); st.pop();
                Vector<LongDouble> va = *a, vb = *b;
                delete a; delete b;
                numeric* r = new numeric;
                for (size_t j = 0; j < max(va.size(), vb.size()); j++)
                    switch (rpn[i].type()) {
                        case LEX_PLUS: r->val.push_back(va[j%va.size()] + vb[j%vb.size()]); break;
                        case LEX_MINUS: r->val.push_back(va[j%va.size()] - vb[j%vb.size()]); break;
                        case LEX_MUL: r->val.push_back(va[j%va.size()] * vb[j%vb.size()]); break;
                        case LEX_DIV: r->val.push_back(va[j%va.size()] / vb[j%vb.size()]); default: break;
                    }
                st.push(r);
            }
                break;

            case LEX_RBRACKET:
            {
                b = st.top(); st.pop(); // Vector<long double> или Vector<BOOL>
                a = st.top(); st.pop(); // почему-то впихнулось число, а не индекс. а, это нормально, здесь нет присваивания
                // нужно уметь индексировать и адреса, и числа


                Vector<LongDouble> indices;
                switch (b->mode()) {
                    case value::ID_BOOL: {
                        // должен циклически применяться
                        logical* bv = (logical*)b;
                        for (size_t i = 0; i < max(a->length(), bv->val.size()); i++)
                            if (bv->val[i%bv->val.size()] == True) indices.push_back(i);
                    }
                        break;
                    case value::ID_NUM: {
                        numeric* bv = (numeric*)b;
                        Vector<LongDouble> bvv = *bv;
                        for (size_t i = 0; i < bvv.size(); i++)
                            indices.push_back(bvv[i]-1); // нумерация с единицы
                    }
                        break;
                    case value::ID_STRING:
                        // ошибка
                        break;
                    case value::ID_NULL:
                        break;
                    default:
                        break;
                }
                if (a->mode() == value::RPN_ADDRESS) {
                    indexed_address* addr = (indexed_address*) a;
                    st.push(new indexed_address(*addr, indices));
                }
                else {
                    switch (a->mode()) {
                        case value::ID_BOOL: {
                            logical* va = (logical*)a;
                            Vector<Bool> res;
                            for (size_t i = 0; i < indices.size(); i++)
                                res.push_back(va->val[indices[i]]);
                            st.push(new logical(res));
                        }
                            break;

                        case value::ID_STRING: {
                            character* va = (character*)a;
                            Vector<String> res;
                            for (size_t i = 0; i < indices.size(); i++)
                                res.push_back(va->val[indices[i]]);
                            st.push(new character(res));
                        }
                            break;

                        case value::ID_NUM: {
                            numeric* va = (numeric*)a;
                            Vector<LongDouble> res;
                            for (size_t i = 0; i < indices.size(); i++)
                                res.push_back(va->val[indices[i]]);
                            st.push(new numeric(res));
                        }
                            break;
                        case value::ID_NULL:
                            st.push(new value);
                        default: break;
                    }
                }
                // v содержит номер в таблице идентификаторов
                // нужно взять элемент(ы) вектора и положить вектор из их адресов в таблицу

            }
                break;

            case LEX_ENDL:
                v = st.top();
                v->print(os);
                // DO NOT BREAK

            case LEX_SEMICOLON:
                v = st.top(); delete v; st.pop();
                break;

            case LEX_ASSIGN: {
                // присваивание по вектору адресов!
                b = st.top(); st.pop();
                a = st.top(); st.pop();
                indexed_address* addr = (indexed_address*)a; // почему он оказался address? а, потому что там не было квадратных скобок. тогда нужно класть всегда indexed_address, а не address, только индексы не заполнять (выставить флаг "без фильтрации")
                size_t id_num = *addr;
                if (!addr->filtered) {
                    delete tid[id_num].val;
                    tid[id_num].val = b->copy();
                }
                // достаем вектор, приводя его к типу
                // присваиваем
                // кладем обратно
                else {
                    value* val = tid[id_num].val;
                    // в этот вектор нужно записать значения

                    switch (val->mode()) {
                        case value::ID_BOOL:
                            // в бул добавляем
                            switch (b->mode()) {
                                case value::ID_BOOL:
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        ((logical*)val)->val.at(addr->indices[i]) = ((logical*)b)->val[i];
                                    break;

                                case value::ID_NUM: {
                                    Vector<LongDouble> res = *val;
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        res.at(addr->indices[i]) = ((numeric*)b)->val[i];
                                    delete tid[id_num].val;
                                    tid[id_num].val = new numeric(res);
                                }
                                    break;

                                case value::ID_NULL:
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        ((logical*)val)->val.at(addr->indices[i]) = Null;
                                    break;

                                case value::ID_STRING: {
                                    // TODO: исправить лишнее копирование
                                    Vector<String> res = *val;
                                    Vector<String> bv = *b;
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        res.at(addr->indices[i]) = bv[i];
                                    delete tid[id_num].val;
                                    tid[id_num].val = new character(res);
                                }
                                default: break;
                            }
                            break;

                        case value::ID_NUM: // в числовой вектор добавляем
                            switch (b->mode()) {
                                case value::ID_NUM:
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        ((numeric*)val)->val.at(addr->indices[i]) = ((numeric*)b)->val[i];
                                    break;

                                case value::ID_BOOL:
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        ((numeric*)val)->val.at(addr->indices[i]) = ((logical*)b)->val[i];
                                    break;

                                case value::ID_NULL:
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        ((numeric*)val)->val.at(addr->indices[i]) = Null;
                                    break;

                                case value::ID_STRING: {
                                    character *res = new character(*(numeric*)val);
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        res->val.at(addr->indices[i]) = ((character*)b)->val[i];
                                    delete tid[id_num].val;
                                    tid[id_num].val = res;
                                }
                                default: break;
                            }
                            break;

                        case value::ID_NULL: {
                            delete tid[id_num].val;
                            switch (b->mode()) {
                                case value::ID_NUM: {
                                    Vector<LongDouble> res;
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        res.at(addr->indices[i]) = ((numeric*)b)->val[i];
                                    tid[id_num].val = new numeric(res);
                                }
                                    break;

                                case value::ID_BOOL: {
                                    Vector<Bool> res;
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        res.at(addr->indices[i]) = ((logical*)b)->val[i];
                                    tid[id_num].val = new logical(res);
                                }
                                    break;

                                case value::ID_NULL:
                                    // FIXME: а нужно ли что-нибудь делать вообще?
                                    //tid[id_num].val = new value;
                                    break;

                                case value::ID_STRING: {
                                    Vector<String> res;
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        res.at(addr->indices[i]) = ((character*)b)->val[i];
                                    tid[id_num].val = new character(res);
                                }
                                    break;
                                default: break;
                            }
                            break;
                        }
                        case value::ID_STRING:
                            // все приводится к строке
                            switch (b->mode()) {
                                case value::ID_STRING:
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        ((character*)val)->val.at(addr->indices[i]) = ((character*)b)->val[i];
                                    break;

                                case value::ID_NUM: {
                                    Vector<String> bv = *b;
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        ((character*)val)->val.at(addr->indices[i]) = ((numeric*)b)->val[i];
                                }
                                    break;
                                    
                                case value::ID_BOOL: {
                                    Vector<String> bv = *b;
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        ((character*)val)->val.at(addr->indices[i]) = ((logical*)b)->val[i];
                                }
                                    break;

                                case value::ID_NULL:
                                    for (size_t i = 0; i < addr->indices.size(); i++)
                                        ((character*)val)->val.at(addr->indices[i]) = Null;
                                    break;

                                default: break;
                            }

                            break;
                        default:
                            break;
                    }

                }

                st.push(b); // что втолкнуть? да, именно правую часть
                delete addr;
            }
                break;

            case LEX_EQ:
                break;

            case RPN_CALL: {
                // имя аргументы аргц вызов
                //value* fname = st.top();
                address* argc = (address*)st.top(); st.pop();
                vector<value*> argv;
                size_t argcv = *argc;
                for (size_t i = 0; i < argcv; i++) {
                    // аргументы функции передаются в обратном порядке
                    argv.push_back(st.top()->copy()); delete st.top(); st.pop();
                }
                address* fname = (address*)st.top(); st.pop();
                //fname->print(cout);
                st.push(stdfunc::func[tid[fname->val].name](argv));
                delete argc;
                delete fname;
            }
                break;


            case LEX_COLON: {
                b = st.top(); st.pop();
                a = st.top(); st.pop();
                Vector<LongDouble> va = *a, vb = *b;
                delete a; delete b;
                if (va.size() != 1 || vb.size() != 1)
                    cerr << "wrong size";
                numeric* res = new numeric;
                LongDouble da = va[0], db = vb[0];
                if (da.isNull() || db.isNull()) res->val.push_back(Null);
                else {
                    for (LongDouble i = da; (i <= db) == True; i = i + 1)
                        res->val.push_back(i);
                    if ((da != db) == True)
                        for (LongDouble i = da; (i >= db) == True; i= i - 1)
                            res->val.push_back(i);
                }
                st.push(res);
            }
                break;

            case LEX_COMMA:
            case LEX_LPAR:
            case LEX_RPAR:
            case LEX_LBRACKET:
            case LEX_LBRACE:
            case LEX_RBRACE:
            case LEX_END:
            case LEX_NULL:
            case LEX_IN:
            case LEX_FOR:
                throw invalid_argument("unexpected lex in RPN");
        }
    }
}
#ifdef LEX
int main(int argc, const char *argv[]) {
    lexScanner s;
    s.getCharacter();
    lex l;
    while ((l = s.getNextLex()).type() != LEX_END) {
        os << l;
    }
    return 0;
}
#else
int main(int argc, const char * argv[]) {
        stringstream ss;
    // нужно вводить не посимвольно, а построчно, и отправлять эти строки парсеру, который их будет отправлять лексическому анализатору
#ifdef RBS
    //ifstream in(argv[1]);
    lexScanner s(cin, ss);
#else
    lexScanner s(cin, ss);
    cout << "> ";
#endif
    s.getCharacter();
    Parser p(s);
    if(!p.Program(false)) cerr << "syntax error" << endl;
    return 0;
}
#endif
