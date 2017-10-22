#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <ctype.h>
#include "ident.h"
#include "lex.h"

using namespace std;


identTable tid;
numTable nid;
strTable sid;

ostream& operator<< (ostream& stream, const lex& l) {
    stream << "<" << lexTypeNames[l.type()] << "," << l.value();
    switch (l.type()) {
        case LEX_ID:
            stream << "," << tid[l.value()].name;
            break;

        case LEX_NUM:
            stream << "," << nid[l.value()];
            break;

        case LEX_STRING:
            stream << "," << sid[l.value()];

        default:
            break;
    }
    stream << ">";
    return stream;
}


lex lexScanner::getNextLex() {
    static bool possibleUnary = true;
    while (true) {
        switch (state) {
            case BEGIN:
                if (isalpha(c)) {
                    state = ID;
                    buffer.clear();
                    buffer += c;
                    getCharacter();
                    break;
                }
                if (isdigit(c) || c == '.') {
                    // FIXME: с точки может начинаться как идентификатор, так и число
                    state = NUMBER;
                    buffer.clear();
                    buffer += c;
                    getCharacter();
                    break;
                }
                if (c == '#') {
                    state = COMMENT;
                    break;
                }
                if (c == '"') {
                    buffer.clear();
                    getCharacter();
                    state = STRING;
                    break;
                }
                if (string(",&|:;+*/{([])}\n").find(c) != string::npos) {
#ifndef RBS
                    if (c == '\n') {
                        os << a << ' ';
                    }
#endif
                    
                    buffer.clear();
                    buffer += c;
                    getCharacter();
                    
                    if (td[buffer] == LEX_MINUS || td[buffer] == LEX_PLUS) {
                        possibleUnary = false;
                        return lex(td[buffer]);
                    }
                    else {
                        switch (td[buffer]) {
                            case LEX_COMMA:
                            case LEX_COLON:
                            case LEX_SEMICOLON:
                            case LEX_LBRACE:
                            case LEX_LBRACKET:
                            case LEX_LPAR:
                                possibleUnary = true;
                                break;
                            default:
                                possibleUnary = false;
                        }
                        possibleUnary = false;
                        return lex(td[buffer]);
                    }
                }
               
                   
                   
                if (c == '-') {
                    buffer.clear();
                    buffer += c;
                    getCharacter();
                    if (isdigit(c) || c == '.') {
                        state = NUMBER;
                        buffer += c;
                        getCharacter();
                        break;
                    }
                    else {
                        possibleUnary = false;
                        return LEX_MINUS;
                    }
                }
                if (c == '>') {
                    state = GREATER;
                    buffer.clear();
                    buffer += c;
                    getCharacter();
                    break;
                }
                if (c == '<') {
                    state = LESS;
                    buffer.clear();
                    buffer += c;
                    getCharacter();
                    break;
                }
                if (c == '!') {
                    state = LESS;
                    buffer.clear();
                    buffer += c;
                    getCharacter();
                    break;
             
                if (c == '=') {
                    state = EQUAL;
                    buffer.clear();
                    buffer += c;
                    getCharacter();
                    break;
                }
                if (c == -1) {
                    state = END;
                    break;
                }
                if (isspace(c)) {
                    getCharacter();
                    break;
                }
                cerr << "invalid character";
                getCharacter();
                break;

            case END:
                possibleUnary = false;
                return lex(LEX_END);

            case ID:
                if (isalnum(c) || c == '.') {
                    buffer += c;
                    getCharacter();
                    break;
                }
                else {
                    lexType t = tw[buffer];
                    if (t != LEX_NULL) {
                        state = BEGIN;
                        possibleUnary = false;
                        return lex(t);
                    }
                    else {
                        // identifier
                        state = BEGIN;
                        possibleUnary = false;
                        return lex(LEX_ID, tid.add(buffer));
                    }
                }

            case NUMBER:
                // FIXME: в числе может содержаться только одна точка!
                if (isdigit(c) || c == '.') {
                    buffer += c;
                    getCharacter();
                    break;
                }
                else {
                    state = BEGIN;
                    possibleUnary = false;
                    return lex(LEX_NUM, nid.add(stod(buffer)));
                }

            case COMMENT:
                if (c == '\n') {
                    getCharacter();
                    state = BEGIN;
                    possibleUnary = false; // FIXME: или в этом случае не нужно? это же может быть разделитель
                    return lex(LEX_ENDL);
                }
                else if (c == EOF) // comment can be the last line
                    state = END;
                else getCharacter();
                break;

            case STRING:
                if (c != '"') {
                    buffer += c;
                    getCharacter();
                    break;
                }
                else {
                    getCharacter();
                    state = BEGIN;
                    possibleUnary = false;
                    return lex(LEX_STRING, sid.add(buffer));
                }

            case GREATER:
            case EXCLAMATION:
            case EQUAL:
                if (c == '=') {
                    buffer += c;
                    getCharacter();
                }
                state = BEGIN;
                if (td[buffer] == LEX_E || td[buffer] == LEX_G || td[buffer] == LEX_EQ || td[buffer] == LEX_GE || td[buffer] == LEX_NE)
                    possibleUnary = true;
                else possibleUnary = false;
                return lex(td[buffer]);

            case LESS:
                if (c == '=' || c == '-') {
                    buffer += c;
                    getCharacter();
                }
                state = BEGIN;
                possibleUnary = true;
                return lex(td[buffer]);
                break;
                    
            /*case PROBEL:
                    buffer.clear();
                    buffer += c;
                    getCharacter();*/
                    
            default:
                break;
        }
     }
    }


};


