#ifndef _TSCODEPOSITION_H_
#define _TSCODEPOSITION_H_

#include "TSGlobal.h"

class TSCodePosition {
public:
    size_t row;
    size_t column;
    size_t length;
    inline bool operator==(const TSCodePosition &pos) const {
        return (row == pos.row) &&
               (column == pos.column) &&
               (length == pos.length);
    }
};

class TSTokenContainer;

template<typename T>
inline TSCodePosition calculatePos(T begin, T end) {
    TSCodePosition pos = begin->pos;

    for (auto it = begin + 1; it != end; ++it)
        pos.length += it->pos.length + (it->pos.column - ((it - 1)->pos.column + (it - 1)->pos.length));

    return pos;
}

#endif
