#ifndef _CODEPOSITION_H_
#define _CODEPOSITION_H_

#include "Global.h"

class CodePosition {
public:
    size_t row;
    size_t column;
    size_t length;
    inline bool operator==(const CodePosition &pos) const {
        return (row == pos.row) &&
               (column == pos.column) &&
               (length == pos.length);
    }
};

class TokenContainer;

template<typename T>
inline CodePosition calculatePos(T begin, T end) {
    CodePosition pos = begin->pos;

    for (auto it = begin + 1; it != end; ++it)
        pos.length += it->pos.length + (it->pos.column - ((it - 1)->pos.column + (it - 1)->pos.length));

    return pos;
}

#endif
