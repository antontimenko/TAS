#ifndef _TSPREPROCESSOR_H_
#define _TSPREPROCESSOR_H_

#include "TSGlobal.h"
#include "TSToken.h"
#include "TSMath.h"

typedef tuple<string, vector<TSTokenContainer>> TSTokenContainersSegmentContainer;

vector<TSTokenContainer>::const_iterator getMathTokenSequence(vector<TSTokenContainer>::const_iterator begin, vector<TSTokenContainer>::const_iterator end);
vector<TSTokenContainersSegmentContainer> preprocess(const vector<TSTokenContainer> &tokenContainerVector);

#endif
