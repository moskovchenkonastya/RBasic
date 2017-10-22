#ifndef __RBasic__types__
#define __RBasic__types__

#include <iostream>
#include <string>
#include <cmath>
#include <limits>
#include <sstream>
using namespace std;

enum NullValue { Null = 0 };
enum BoolValue { Nool = -1, False = 0, True = 1 };

class LongDouble;


class Bool {
    BoolValue val;
public:
    Bool(NullValue n = Null) : val(Nool) { }
    Bool(bool b) : val(b ? True : False) { }
    bool operator== (BoolValue bv) const { return val == bv; } // bool или Bool?
    bool operator== (NullValue n) const { return val == Nool; }
    Bool operator && (const Bool& n) const { // FALSE & NULL должно быть равно FALSE
        if (val == Nool || n.val == Nool) return Null;
        else return val && n.val;
    }
    Bool operator || (const Bool& n) const { // FIXME: TRUE | NULL должно быть равно TRUE
        if (val == Nool || n.val == Nool) return Null;
        else return val || n.val;
    }
    Bool operator! () const {
        if (val == Nool) return Null;
        return !val;
    }
    Bool operator < (const Bool& n) const {
        if (val == Nool || n.val == Nool) return Null;
        else return val < n.val;
    }
    Bool operator > (const Bool& n) const {
        if (val == Nool || n.val == Nool) return Null;
        else return val > n.val;
    }
    Bool operator <= (const Bool& n) const {
        if (val == Nool || n.val == Nool) return Null;
        else return val <= n.val;
    }
    Bool operator >= (const Bool& n) const {
        if (val == Nool || n.val == Nool) return Null;
        else return val >= n.val;
    }
    Bool operator == (const Bool& n) const {
        if (val == Nool || n.val == Nool) return Null;
        else return val == n.val;
    }
    Bool operator != (const Bool& n) const {
        if (val == Nool || n.val == Nool) return Null;
        else return val != n.val;
    }
    
    friend ostream& operator<< (ostream& stream, Bool& b) {
        switch (b.val) {
            case Nool: stream << "NULL"; break;
            case False: stream << "FALSE"; break;
            case True: stream << "TRUE"; break;
        }
        return stream;
    }
};

class LongDouble {
    friend class String;
    long double val;
public:
    bool isNull () const { return isnan(val) || isinf(val); }
    LongDouble(long double ld) : val(ld) { }
    LongDouble(NullValue n = Null) : val(std::numeric_limits<long double>::quiet_NaN()) { }
    LongDouble(Bool b) : val((b == Null) ? std::numeric_limits<long double>::quiet_NaN() : ((b == True) ? 1.0 : 0.0)) {
        if (b == Null) val = std::numeric_limits<long double>::quiet_NaN();
        if (b == True) val = 1.0;
        if (b == False) val = 0.0;
    }
    operator Bool() {
        if (isNull()) return Nool;
        return (val != 0.0) ? True : False;
    }
    friend ostream& operator<< (ostream& stream, const LongDouble& ld) {
        if (ld.isNull()) stream << "NULL";
        else stream << ld.val; // FIXME: неэкспоненциальная форма!
        return stream;
    }
    explicit operator size_t() {
        return (size_t)val;
    }
    Bool operator== (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val == ld.val;
    }
    Bool operator!= (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val != ld.val;
    }
    Bool operator< (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val < ld.val;
    }
    Bool operator> (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val > ld.val;
    }
    Bool operator<= (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val <= ld.val;
    }
    Bool operator>= (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val >= ld.val;
    }
    LongDouble operator+ (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val + ld.val;
    }
    LongDouble operator- (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val - ld.val;
    }
    LongDouble operator* (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val * ld.val;
    }
    LongDouble operator/ (const LongDouble& ld) const {
        if (isNull() || ld.isNull()) return Null;
        else return val / ld.val;
    }
};

class String {
    string val;
    bool isNull;
public:
    String (NullValue n = Null) : isNull(true) { }
    String (const Bool& b) : isNull(b == Null), val((b == True) ? "TRUE" : ((b == False) ? "FALSE" : "")) { } // а не "NULL"
    String (const string& s) : val(s), isNull(false) { }
    String (const LongDouble& ld) {
        isNull = ld.isNull();
        if (!isNull) {
            stringstream ss;
            ss << ld.val;
            ss >> val;
        }
    }
    friend ostream& operator<< (ostream& stream, const String& s) {
        if (s.isNull) stream << "\"NULL\""; else stream << '"' << s.val << '"'; return stream;
    }
    Bool operator== (const String& ld) const {
        if (isNull || ld.isNull) return Null;
        else return val == ld.val;
    }
    Bool operator!= (const String& ld) const {
        if (isNull || ld.isNull) return Null;
        else return val != ld.val;
    }
    Bool operator< (const String& ld) const {
        if (isNull || ld.isNull) return Null;
        else return val < ld.val;
    }
    Bool operator> (const String& ld) const {
        if (isNull || ld.isNull) return Null;
        else return val > ld.val;
    }
    Bool operator<= (const String& ld) const {
        if (isNull || ld.isNull) return Null;
        else return val <= ld.val;
    }
    Bool operator>= (const String& ld) const {
        if (isNull || ld.isNull) return Null;
        else return val >= ld.val;
    }

};




#endif /* defined(__RBasic__types__) */
