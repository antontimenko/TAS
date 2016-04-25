#ifndef _PREPROCESSOR_H_
#define _PREPROCESSOR_H_

#include "Global.h"
#include "Token.h"
#include "Math.h"

struct TokenSegment {
    string segName;
    vector<TokenContainer> tokenContainers;
};

vector<TokenContainer>::const_iterator getMathTokenSequence(vector<TokenContainer>::const_iterator begin, vector<TokenContainer>::const_iterator end);
tuple<vector<TokenSegment>, map<string, Integer>> preprocess(const vector<TokenContainer> &tokenContainerVector);

#endif
