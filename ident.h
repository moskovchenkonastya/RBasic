#ifndef __RBasic__ident__
#define __RBasic__ident__

#include <string>
#include <sstream>
#include <vector>
#include "lex.h"
#include "types.h"

using namespace std;

template <class T>
class Vector : public vector<T> {
public:
    Vector() : vector<T>() { }
    Vector(initializer_list<typename vector<T>::value_type> __il) : vector<T>(__il) { }
    typename vector<T>::reference at (LongDouble i) {
        for (LongDouble j = vector<T>::size(); (j <= i) == True; j = j + 1) vector<T>::push_back(Null);
        return vector<T>::at((size_t)i);
    }
    const typename vector<T>::value_type operator[](LongDouble i) {
        if ((i < vector<T>::size()) == True) return vector<T>::at((size_t)i);
        else return Null;
    }
};


class value {
public:
    virtual value* copy () { return new value(*this); }
    virtual size_t length () { return 0; }
    enum identMode { ID_NULL, ID_STRING, ID_NUM, ID_BOOL, RPN_ADDRESS };
    virtual identMode mode() { return ID_NULL; }
    virtual operator Vector<LongDouble>() { return {Null}; }
    virtual operator Vector<String>() { return {Null}; }
    virtual operator Vector<Bool>() { return {Null}; }
    virtual void print(ostream& stream) { stream << "NULL" << endl; }
    virtual ~value() { }
};

class address : public value { // переименовать в argc – это одно число
public:
    size_t val;
    virtual class value* copy () { return new address(*this); }
    virtual identMode mode() { return RPN_ADDRESS; }
    address(size_t i) : val(i) { }
    operator size_t() { return val; }
    virtual void print(ostream& stream) { stream << "error: address is on the stack" << endl; }
   
};

class indexed_address : public value {
public:
    size_t id_num;
    Vector<LongDouble> indices;
    bool filtered;
    virtual size_t length ();
    virtual class value* copy() { return new indexed_address(*this); }
    virtual identMode mode() { return RPN_ADDRESS; /* FIXME */ }
    indexed_address(size_t n, const Vector<LongDouble>& v) : id_num(n), indices(v), filtered(true) { }
    indexed_address(size_t n) : id_num(n), filtered(false) { }
    operator size_t() { return id_num; }
    operator Vector<LongDouble>() { return indices; }
    virtual void print(ostream& stream);
};
// a=2.5
// b=1.41
// 9*(a+b)/60*3.14

// 3.91*9

// FIXME
// присваивание по индексу не работает вообще!
// 1,2,NULL,4 & TRUE = NULL, NULL, NULL, NULL
class numeric : public value {
public:
    Vector<LongDouble> val;
    virtual class value* copy () { return new numeric(*this); }
    numeric (const Vector<LongDouble>& v) : val(v) {}
    numeric (LongDouble f) : val({f}) { }
    numeric () { }
    virtual size_t length() { return val.size(); }
    virtual identMode mode() { return ID_NUM; }
    virtual operator Vector<LongDouble>() { return val; } // копирование!
    virtual void print (ostream& stream) {
        if (!val.size()) stream << "numeric(0)";
        for (size_t i = 0; i < val.size(); i++)
            stream << val[i] << ' ';
        stream << endl;
    }
    virtual operator Vector<String>() {
        Vector<String> res;
        stringstream ss;
        string s;
        for (size_t i = 0; i < val.size(); i++) {
            ss.clear();
            ss << val[i];
            ss >> s;
            res.push_back(s);
        }
        return res;
    }
    virtual operator Vector<Bool>() {
        Vector<Bool> res;
        for (size_t i = 0; i < val.size(); i++)
            res.push_back(val.at(i)); // TODO: проверить семантику
        return res;
    }
};

class character : public value {
public:
    Vector<String> val;
    character(const Vector<String>& v) : val(v) { }
    character(const String& s) : val({s}) { }
    character() {}
    virtual class value* copy () { return new character(*this); }
    virtual size_t length () { return val.size(); }
    virtual identMode mode() { return ID_STRING; }
    virtual operator Vector<String>() { return val; } // копирование!
    virtual operator Vector<LongDouble>() { cerr << "character to numeric" << endl; return {Null}; }
    virtual operator Vector<Bool>() { cerr << "character to logical" << endl; return {Null}; }
    virtual void print (ostream& stream) {
        if (!val.size()) stream << "character(0)";
        for (size_t i = 0; i < val.size(); i++)
            stream << val[i] << ' ';
        stream << endl;
    }
};

class logical : public value {
public:
    Vector<Bool> val;
    virtual class value* copy () { return new logical(*this); }
    logical() { }
    logical(const Vector<Bool>& v) : val(v) { }
    logical(lexType t) : val({t == LEX_TRUE}) { }
    virtual identMode mode() { return ID_BOOL; }
    virtual size_t length () { return val.size(); }
    virtual operator Vector<Bool>() { return val; } // копирование!
    virtual operator Vector<LongDouble>() {
        Vector<LongDouble> res;
        for (size_t i = 0; i < val.size(); i++)
            res.push_back(val.at(i));
        return res;
    }
    virtual operator Vector<String>() {
        Vector<String> res;
        for (size_t i = 0; i < val.size(); i++)
            res.push_back(val.at(i));
        return res;
    }
    virtual void print (ostream& stream) {
        if (!val.size()) stream << "logical(0)";
        for (size_t i = 0; i < val.size(); i++)
            stream << val.at(i) << ' ';
        stream << endl;
    }
};

// класс, который нужен для опредления индетификатор лексем 
class ident {
public:
    std::string      name;
    value* val;
public:
    value::identMode mode() { return val->mode(); }
    ident()     : val(new class value)     { }
    ident(const std::string& s) : name(s), val(new class value) { }
};


//класс table_ident
// создает таблицу индефикаторов(будем использовать в executure для определения адреса или значения переменной)
class identTable : public vector<ident> { // NULL value not handled
public:
    size_t add (const std::string& name) {
        for (size_t i = 0; i < size(); i++)
            if (at(i).name == name)
                return i;
        push_back(ident(name));
        return size() - 1;
    }
};

extern identTable tid;


#endif /* defined(__RBasic__ident__) */
