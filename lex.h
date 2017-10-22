#ifndef RBasic_lex_h
#define RBasic_lex_h

#include <utility>
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

// Представление лексем
enum lexType {
    LEX_NULL = 0,   // 0
    LEX_TRUE, LEX_FALSE, LEX_RNULL, // 3
    LEX_END, // 4
    LEX_AND, LEX_OR, LEX_NOT, LEX_G, LEX_L, LEX_GE, LEX_LE, LEX_E, LEX_NE, LEX_ASSIGN, LEX_COLON, LEX_SEMICOLON, LEX_COMMA, // 17
    LEX_PLUS, LEX_MINUS, LEX_MUL, LEX_DIV, LEX_LPAR, LEX_RPAR, LEX_LBRACKET, LEX_RBRACKET, LEX_EQ, LEX_ENDL, // 27
    LEX_NUM, // 28
    LEX_STRING, // 29
    LEX_ID, // 30
    RPN_CALL, // 31
    RPN_ARGC, // 32
    RPN_LVAL, // 33
    RPN_FNAME, // 34
    LEX_LBRACE,
    LEX_RBRACE,
    LEX_UNARY_MINUS,
    LEX_UNARY_PLUS,
    LEX_FOR,
    LEX_IN,
    LEX_BREAK,
    RPN_GOTO,
    LEX_TAB,
    LEX_SLESH
};

static const string lexTypeNames[] = {
    "LEX_NULL",   // 0
    "LEX_TRUE", "LEX_FALSE", "LEX_RNULL", // 3
    "LEX_END", // 4
    "LEX_AND", "LEX_OR", "LEX_NOT", "LEX_G", "LEX_L", "LEX_GE", "LEX_LE", "LEX_E", "LEX_NE", "LEX_ASSIGN", "LEX_COLON", "LEX_SEMICOLON", "LEX_COMMA", // 17
    "LEX_PLUS", "LEX_MINUS", "LEX_MUL", "LEX_DIV", "LEX_LPAR", "LEX_RPAR", "LEX_LBRACKET", "LEX_RBRACKET", "LEX_EQ", "LEX_ENDL", // 27
    "LEX_NUM", // 28
    "LEX_STRING", // 29
    "LEX_ID", // 30
    "RPN_CALL", // 31
    "RPN_ARGC", // 32
    "RPN_LVAL", // 33
    "RPN_FNAME", // 34
    "LEX_LBRACE",
    "LEX_RBRACE",
    "LEX_UNARY_MINUS",
    "LEX_UNARY_PLUS",
    "LEX_FOR",
    "LEX_IN",
    "LEX_BREAK",
    "RPN_GOTO",
    
    "LEX_TAB",
    "LEX_SLESH"
};




// класс lex
// значение – это индекс в таблице идентификаторов или констант
// еще число аргументов или адрес в RPN
// Преобразовать цепочку символов, представляющих лексему, в пару <тип_лексемы, указатель_на_информацию_о_ней>

class lex : private pair<lexType, size_t> {
    // значение – это индекс в таблице идентификаторов или констант
    // еще число аргументов или адрес в RPN
public:
    lex (lexType type = LEX_NULL, size_t value = 0) : pair<lexType, size_t>(type, value) { }
    lexType type() const { return pair<lexType, size_t>::first; }
    size_t value() const { return pair<lexType, size_t>::second; }
    friend ostream& operator<< (ostream& stream, const lex& l);
    bool operator== (lexType t) const { return type() == t; }
    bool operator!= (lexType t) const { return type() != t; }
};


// таблица для лексем с чиселами с плавающей точкой (numeric)
// фиксируем в специальных таблицах для хранения лексем (numeric) чисел с плавающей точкой
// факт появления этой лексемы в анализируемом тексте

class numTable : public vector<long double> {
public:
    size_t add (long double d) {
        for (size_t i = 0; i < size(); i++)
            if (at(i) == d)
                return i;
        push_back(d);
        return size() - 1;
    }
};

// таблица для строковых символов (character)
// фиксируем в специальных таблицах для хранения строковыe лексемы (character)
// факт появления этой лексемы в анализируемом тексте
class strTable : public vector<string> {
public:
    size_t add (const string& s) {
        for (size_t i = 0; i < size(); i++)
            if (at(i) == s)
                return i;
        push_back(s);
        return size() - 1;
    }
};


// при чтении лексемы ENDL выводить +

class lexScanner {
public:
    char a;
private:
    istream& stream;
public:
    stringstream& os;
public:
    enum stateId {
        BEGIN, ID, NUMBER, PROBEL, COMMENT, STRING, DELIMITER, GREATER, LESS, EXCLAMATION, EQUAL, END
    };
    // static
    map<string, lexType> tw = {
        { "",       LEX_NULL    },
        { "TRUE",   LEX_TRUE    },
        { "FALSE",  LEX_FALSE   },
        { "NULL",   LEX_RNULL   },
        { "for",    LEX_FOR     },
        { "in",     LEX_IN      },
        { "break",  LEX_BREAK   }
    };
    map<string, lexType> td = {
        { "",   LEX_NULL        },
        { "&",  LEX_AND         },
        { "|",  LEX_OR          },
        { "!",  LEX_NOT         },
        { ">",  LEX_G           },
        { "<",  LEX_L           },
        { ">=", LEX_GE          },
        { "<=", LEX_LE          },
        { "==", LEX_E           },
        { "!=", LEX_NE          },
        { "<-", LEX_ASSIGN      },
        { ":",  LEX_COLON       },
        { ";",  LEX_SEMICOLON   },
        { ",",  LEX_COMMA       },
        { "+",  LEX_PLUS        },
        { "-",  LEX_MINUS       },
        { "*",  LEX_MUL         },
        { "/",  LEX_DIV         },
        { "(",  LEX_LPAR        },
        { ")",  LEX_RPAR        },
        { "[",  LEX_LBRACKET    },
        { "]",  LEX_RBRACKET    },
        { "=",  LEX_EQ          },
        { "\n", LEX_ENDL        },
        { "{",  LEX_LBRACE      },
        { "}",  LEX_RBRACE      },
        
        { "\t",   LEX_TAB       },
        { "\"",   LEX_SLESH     }
    };
    stateId state;
    string buffer;
    int c;
    void getCharacter () {
        static bool newlineCount = false;
        c = stream.peek();
        // нужно удвоить newline
        // если символ не newline, то считать его
        // если newlineCount == true, то тоже считать (это именно newline), но еще сбросить newlineCount
        
        if (c != '\n' || newlineCount == true)
            c = stream.get();
        if (c == '\n') newlineCount ^= true;
    }
public:
    lexScanner (istream& s, stringstream& o) : state(BEGIN), stream(s), os(o), a('>') { }
    lex getNextLex ();
    
private:
    lexScanner (const lexScanner&);
    lexScanner& operator= (const lexScanner&);
};

extern numTable nid;
extern strTable sid;

#endif
