#ifndef _TSPREPROCESSOR_H_
#define _TSPREPROCESSOR_H_

#include "TSGlobal.h"
#include "TSToken.h"

struct TSSegmentContainer
{
    string name;
    vector<TSTokenContainer> tokenContainerVector;
};

vector<TSSegmentContainer> preprocess(const vector<TSTokenContainer> &tokenContainerVector);

#endif
