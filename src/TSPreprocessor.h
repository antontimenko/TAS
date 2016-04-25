#ifndef _TSPREPROCESSOR_H_
#define _TSPREPROCESSOR_H_

#include "TSGlobal.h"
#include "TSToken.h"
#include "TSMath.h"

struct TSTokenSegment {
    string segName;
    vector<TSTokenContainer> tokenContainers;
};

vector<TSTokenContainer>::const_iterator getMathTokenSequence(vector<TSTokenContainer>::const_iterator begin, vector<TSTokenContainer>::const_iterator end);
tuple<vector<TSTokenSegment>, map<string, TSInteger>> preprocess(const vector<TSTokenContainer> &tokenContainerVector);

#endif
