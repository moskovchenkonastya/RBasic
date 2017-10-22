#include "ident.h"

size_t indexed_address::length() {
    if (filtered)
        return indices.size();
    return tid[id_num].val->length();
}

void indexed_address::print(ostream& stream) {
    value* val = tid[id_num].val;
    switch (val->mode()) {
        case value::ID_BOOL:
            if (!filtered) {
                for (size_t i = 0; i < ((logical*)val)->val.size(); i++)
                    stream << ((logical*)val)->val.at(indices[i]) << ' ';
                break;
            }
            if (!indices.size()) { stream << "logical(0)"; break; }
            for (size_t i = 0; i < this->indices.size(); i++)
                stream << (((logical*)val)->val.at(indices[i])) << ' ';
            break;
            
        case value::ID_NULL: stream << "TODO: NULL"; break;
        case value::ID_NUM:
            if (!filtered) {
                for (size_t i = 0; i < ((numeric*)val)->val.size(); i++)
                    stream << ((numeric*)val)->val.at(indices[i]) << ' ';
                break;
            }
            if (!indices.size()) { stream << "numeric(0)"; break; }
            for (size_t i = 0; i < indices.size(); i++)
                stream << ((numeric*)val)->val.at(indices[i]) << ' ';
            break;
            
        case value::ID_STRING:
            if (!filtered) {
                for (size_t i = 0; i < ((character*)val)->val.size(); i++)
                    stream << ((character*)val)->val.at(indices[i]) << ' ';
                break;
            }
            if (!indices.size()) { stream << "character(0)"; break; }
            for (size_t i = 0; i < indices.size(); i++)
                stream << ((character*)val)->val.at(indices[i]) << ' ';
            break;
        default: cerr << "error: indexed address on stack"; break;
    }
}