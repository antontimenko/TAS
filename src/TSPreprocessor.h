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

vector<TSSegmentTokenContainer> preprocess(const vector<TSTokenContainer> &tokenContainerVector);

#endif
