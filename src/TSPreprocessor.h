#ifndef _TSPREPROCESSOR_H_
#define _TSPREPROCESSOR_H_

#include "TSGlobal.h"
#include "TSToken.h"
#include "TSMath.h"

struct TSSegmentTokenContainer
{
    string name;
    vector<TSTokenContainer> tokenContainerVector;
};

longlong computeMath(const vector<TSTokenContainer> &tokenContainerVector, const map<string, longlong> equMap = map<string, longlong>());
vector<TSTokenContainer>::const_iterator getMathTokenSequence(vector<TSTokenContainer>::const_iterator begin, vector<TSTokenContainer>::const_iterator end);
vector<TSSegmentTokenContainer> preprocess(const vector<TSTokenContainer> &tokenContainerVector);

#endif
