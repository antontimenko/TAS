#include "TSDiagnostics.h"

const map<TSToken::Type, string> tokenTypeDescriptionMap = {
    {TSToken::Type::USER_IDENTIFIER, "User Identifier"},
    {TSToken::Type::MEMORY_BRACKET, "Memory Bracket"},
    {TSToken::Type::MATH_SYMBOL, "Math Symbol"},
    {TSToken::Type::COMMA, "Comma"},
    {TSToken::Type::COLON, "Colon"},
    {TSToken::Type::SEGMENT_DIRECTIVE, "Segment Directive"},
    {TSToken::Type::INSTRUCTION, "Instruction"},
    {TSToken::Type::REGISTER, "Register"},
    {TSToken::Type::SIZE_IDENTIFIER, "Size Identifier"},
    {TSToken::Type::DATA_IDENTIFIER, "Data Identifier"},
    {TSToken::Type::CONSTANT_NUMBER, "Constant Number"},
    {TSToken::Type::CONSTANT_STRING, "Constant String"},
    {TSToken::Type::CONDITION_DIRECTIVE, "Condition Directive"},
    {TSToken::Type::CONDITION, "Condition"},
    {TSToken::Type::SIZE_OPERATOR, "Size Operator"},
    {TSToken::Type::EQU_DIRECTIVE, "EQU Directive"},
    {TSToken::Type::END_DIRECTIVE, "END Directive"}
};

void printError(string text)
{
    cout << TSColor::BWhite << text << TSColor::Reset << endl;
}

void printCompileError(string text, const string &sourceFileContents, TSCodePosition pos)
{
    cout << TSColor::BWhite << flush;

    cout << TSColor::BRed << "Compile Error" << TSColor::BWhite << " ("
         << pos.row << ":"
         << pos.column << "): "
         << text << endl;

    size_t i;
    size_t j;

    i = 0;
    size_t currentRow = 1;
    while (currentRow < pos.row)
    {
        if ((sourceFileContents[i] == cCR) || (sourceFileContents[i] == cLF))
            ++currentRow;
        if ((sourceFileContents[i] == cCR) && (sourceFileContents[i + 1] == cLF))
            ++i;
        ++i;
    }

    size_t lineStartIndex = i;
    
    j = 1;
    while ((sourceFileContents[i] != cCR) && (sourceFileContents[i] != cLF) && (i < sourceFileContents.size()))
    {
        if (j == pos.column)
            cout << TSColor::BRed << flush;
        else if (j == pos.column + pos.length)
            cout << TSColor::BWhite << flush;

        if (sourceFileContents[i] == 0x9)
            cout << "    ";
        else
            cout << sourceFileContents[i];

        ++i;
        ++j;
    }
    cout << endl;

    i = lineStartIndex;
    j = 1;
    while (j < pos.column)
    {
        if (sourceFileContents[i] == 0x9)
            cout << "    ";
        else
            cout << " ";

        ++i;
        ++j;
    }
    cout << TSColor::BGreen << "^" << TSColor::BWhite << endl;

    cout << TSColor::Reset << flush;
}

string getTokenString(const TSToken &token)
{
    string returnString;

    switch (token.type())
    {
    case TSToken::Type::USER_IDENTIFIER:
        returnString = token.value<string>();
        break;
    case TSToken::Type::MEMORY_BRACKET:
        returnString = findByValue(TSToken::memoryBracketMap, token.value<TSToken::MemoryBracket>())->first;
        break;
    case TSToken::Type::MATH_SYMBOL:
        returnString = findByValue(TSToken::mathSymbolMap, token.value<TSToken::MathSymbol>())->first;
        break;
    case TSToken::Type::SEGMENT_DIRECTIVE:
        returnString = findByValue(TSToken::segmentDirectiveMap, token.value<TSToken::SegmentDirective>())->first;
        break;
    case TSToken::Type::INSTRUCTION:
        returnString = findByValue(TSToken::instructionMap, token.value<TSToken::Instruction>())->first;
        break;
    case TSToken::Type::REGISTER:
        returnString = findByValue(TSToken::registerMap, token.value<TSToken::Register>())->first;
        break;
    case TSToken::Type::SIZE_IDENTIFIER:
        returnString = findByValue(TSToken::sizeIdentifierMap, token.value<TSToken::SizeIdentifier>())->first;
        break;
    case TSToken::Type::DATA_IDENTIFIER:
        returnString = findByValue(TSToken::dataIdentifierMap, token.value<TSToken::DataIdentifier>())->first;
        break;
    case TSToken::Type::CONSTANT_NUMBER:
        returnString = token.value<TSInteger>().str();
        break;
    case TSToken::Type::CONSTANT_STRING:
        returnString = token.value<string>();
        break;
    case TSToken::Type::CONDITION_DIRECTIVE:
        returnString = findByValue(TSToken::conditionDirectiveMap, token.value<TSToken::ConditionDirective>())->first;
        break;
    case TSToken::Type::CONDITION:
        returnString = findByValue(TSToken::conditionMap, token.value<TSToken::Condition>())->first;
        break;
    case TSToken::Type::COMMA:
        returnString = TSToken::commaStr;
        break;
    case TSToken::Type::COLON:
        returnString = TSToken::colonStr;
        break;
    case TSToken::Type::SIZE_OPERATOR:
        returnString = TSToken::sizeOperatorStr;
        break;
    case TSToken::Type::EQU_DIRECTIVE:
        returnString = TSToken::equDirectiveStr;
        break;
    case TSToken::Type::END_DIRECTIVE:
        returnString = TSToken::endDirectiveStr;
        break;
    }

    return returnString;
}

string getTokenDescription(const TSToken &token)
{
    string res = tokenTypeDescriptionMap.find(token.type())->second;

    if (token.type() == TSToken::Type::REGISTER)
    {
        TSToken::Register reg = token.value<TSToken::Register>();

        if (reg.match(TSOperandMask::UREG))
        {
            if (reg.match(TSOperandMask::S8))
                res += " 8";
            else if (reg.match(TSOperandMask::S16))
                res += " 16";
            else
                res += " 32";
        }
        else
            res += " Segment";
    }

    return res;
}

constexpr auto tch0     = "\u2500"; // ─
constexpr auto tch1     = "\u2502"; // |
constexpr auto tch2     = "\u2510"; // ┐
constexpr auto tch3     = "\u250c"; // ┌
constexpr auto tch4     = "\u2514"; // └
constexpr auto tch5     = "\u2518"; // ┘
constexpr auto tch6     = "\u2524"; // ┤
constexpr auto tch7     = "\u251c"; // ├
constexpr auto tch8     = "\u2534"; // ┴
constexpr auto tch9     = "\u252c"; // ┬
constexpr auto tch10    = "\u253c"; // ┼

size_t getMaxWidth(const vector<string> &strVector)
{
    size_t maxWidth = 0;
    
    for (auto it = strVector.begin(); it != strVector.end(); ++it)
    {
        if (it->size() > maxWidth)
            maxWidth = it->size();
    }

    return maxWidth;
}

void printSpace(size_t ammount)
{
    for (size_t i = 0; i < ammount; ++i)
        cout << ' ';
    cout << flush;
}

void printTable(string name, const vector<vector<string>> &data)
{
    cout << TSColor::BWhite << flush;

    vector<size_t> maxWidthVector(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        maxWidthVector[i] = getMaxWidth(data[i]);

    size_t wholeWidth = 0;
    for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it)
        wholeWidth += *it;

    cout << tch3;
    for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it)
    {
        for (size_t i = 0; i < *it; ++i)
            cout << tch0;
        if (it != maxWidthVector.end() - 1)
            cout << tch0;
    }
    cout << tch2 << endl;

    cout << tch1;
    if (name.size() < (wholeWidth + data.size()))
    {
        printSpace((wholeWidth + data.size() - 1 - name.size()) / 2);
        cout << name;
        printSpace((wholeWidth + data.size() - 1) - ((wholeWidth + data.size() - 1 - name.size()) / 2) - name.size());
    }
    else
        cout << string(name.begin(), name.begin() + (wholeWidth + data.size() - 1));
    cout << tch1 << endl;

    cout << tch7;
    for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it)
    {
        for (size_t i = 0; i < *it; ++i)
            cout << tch0;
        if (it != maxWidthVector.end() - 1)
            cout << tch9;
    }
    cout << tch6 << endl;

    for (size_t i = 0; i < data[0].size(); ++i)
    {
        cout << tch1;
        for (size_t j = 0; j < data.size(); ++j)
        {
            if (i < data[j].size())
            {
                cout << data[j][i];
                printSpace(maxWidthVector[j] - data[j][i].size());
            }
            else
                printSpace(maxWidthVector[j]);

            cout << tch1;
        }
        cout << endl;

        if (i != data[0].size() - 1)
        {
            cout << tch7;
            for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it)
            {
                for (size_t i = 0; i < *it; ++i)
                    cout << tch0;
                if (it != maxWidthVector.end() - 1)
                    cout << tch10;
            }
            cout << tch6 << endl;
        }
    }

    cout << tch4;
    for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it)
    {
        for (size_t i = 0; i < *it; ++i)
            cout << tch0;
        if (it != maxWidthVector.end() - 1)
            cout << tch8;
    }
    cout << tch5 << endl;

    cout << TSColor::Reset << flush;
}

void printTokenTable(const vector<TSTokenContainer> &tokenContainerVector)
{
    vector<string> strIndexVector{"Index"};
    vector<string> strCoordsVector{"Coords"};
    vector<string> strTokenVector{"Name"};
    vector<string> strTokenDescriptionVector{"Description"};

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it)
    {
        strIndexVector.push_back(std::to_string(it - tokenContainerVector.begin()));
        strCoordsVector.push_back(string("(") + std::to_string(it->pos.row) + "," + std::to_string(it->pos.column) + ")");
        strTokenVector.push_back(getTokenString(it->token));
        strTokenDescriptionVector.push_back(getTokenDescription(it->token));
    }

    printTable("Lexeme Table", {strIndexVector, strCoordsVector, strTokenVector, strTokenDescriptionVector});
}

void printTokenTable(const vector<TSTokenContainer> &tokenContainerVector, const vector<TSLexemeContainer> &lexemeContainerVector)
{
    vector<string> strIndexVector{"Index"};
    vector<string> strCoordsVector{"Coords"};
    vector<string> strTokenVector{"Name"};
    vector<string> strTokenDescriptionVector{"Description"};

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it)
    {
        strIndexVector.push_back(std::to_string(it - tokenContainerVector.begin()));
        strCoordsVector.push_back(string("(") + std::to_string(it->pos.row) + "," + std::to_string(it->pos.column) + ")");
        strTokenVector.push_back(lexemeContainerVector[it - tokenContainerVector.begin()].lexeme);
        strTokenDescriptionVector.push_back(getTokenDescription(it->token));
    }

    printTable("Native Lexeme Table", {strIndexVector, strCoordsVector, strTokenVector, strTokenDescriptionVector});
}

void printEquTable(const map<string, TSInteger> &equMap)
{
    vector<string> strNameVector{"Name"};
    vector<string> strValueVector{"Value"};

    for (auto it = equMap.begin(); it != equMap.end(); ++it)
    {
        strNameVector.push_back(it->first);
        strValueVector.push_back(it->second.str());
    }

    printTable("EQU Table", {strNameVector, strValueVector});
}

void printPseudoLabelTable(const map<string, TSLabelParamType> &labelMap)
{
    vector<string> strNameVector{"Name"};
    vector<string> strTypeVector{"Type"};
    vector<string> strIndexVector{"Index"};
    vector<string> strSegmentVector{"Segment"};

    for (auto it = labelMap.begin(); it != labelMap.end(); ++it)
    {
        strNameVector.push_back(it->first);
        strTypeVector.push_back(get<0>(it->second) == TSLabelType::LABEL ? "Label" : findByValue(TSToken::dataIdentifierMap, get<1>(it->second))->first);
        strIndexVector.push_back(std::to_string(get<2>(it->second)));
        strSegmentVector.push_back(get<3>(it->second));
    }

    printTable("Pseudo Label Table", {strNameVector, strTypeVector, strIndexVector, strSegmentVector});
}

void printPseudoSentenceTable(const vector<TSPseudoSentencesSegmentContainer> &segmentPseudoSentenceVector)
{
    size_t maxOpsAmmount = 0;
    for (auto segIt = segmentPseudoSentenceVector.begin(); segIt != segmentPseudoSentenceVector.end(); ++segIt)
    {
        for (auto it = get<1>(*segIt).begin(); it != get<1>(*segIt).end(); ++it)
        {
            if (it->operandsTokenContainerVector.size() > maxOpsAmmount)
                maxOpsAmmount = it->operandsTokenContainerVector.size();
        }
    }

    vector<vector<string>> strOperandsVector;

    for (size_t i = 0; i < maxOpsAmmount; ++i)
    {
        vector<string> strOperandVector(1, string("Op ") + std::to_string(i + 1));
        strOperandsVector.push_back(strOperandVector);
    }

    vector<string> strIndexVector{"Index"};
    vector<string> strNameVector{"Name"};
    vector<string> strSegmentVector{"Segment"};

    for (auto segIt = segmentPseudoSentenceVector.begin(); segIt != segmentPseudoSentenceVector.end(); ++segIt)
    {
        for (auto it = get<1>(*segIt).begin(); it != get<1>(*segIt).end(); ++it)
        {
            strIndexVector.push_back(std::to_string(it - get<1>(*segIt).begin()));
            strNameVector.push_back(getTokenString(it->baseTokenContainer.token));
            strSegmentVector.push_back(get<0>(*segIt));

            size_t i;
            for (i = 0; i < it->operandsTokenContainerVector.size(); ++i)
            {
                string opStr;
                for (size_t j = 0; j < it->operandsTokenContainerVector[i].size(); ++j)
                    opStr += getTokenString(it->operandsTokenContainerVector[i][j].token) + ' ';

                strOperandsVector[i].push_back(opStr);
            }

            for (; i < strOperandsVector.size(); ++i)
                strOperandsVector[i].push_back("");
        }
    }

    vector<vector<string>> strTableVectors{strIndexVector, strNameVector, strSegmentVector};
    strTableVectors.insert(strTableVectors.end(), strOperandsVector.begin(), strOperandsVector.end());

    printTable("Pseudo Sentence Table", strTableVectors);
}

void printRawSentenceTable(const vector<TSRawSentencesSegmentContainer> &rawSentencesSegmentContainerVector)
{
    size_t maxOpsAmmount = 0;
    for (auto segIt = rawSentencesSegmentContainerVector.begin(); segIt != rawSentencesSegmentContainerVector.end(); ++segIt)
    {
        for (auto it = get<1>(*segIt).begin(); it != get<1>(*segIt).end(); ++it)
        {
            auto present = (*it)->present();
            const vector<string> &operandStrVector = get<1>(present);

            if (operandStrVector.size() > maxOpsAmmount)
                maxOpsAmmount = operandStrVector.size();
        }
    }

    vector<vector<string>> strOperandsVector;

    for (size_t i = 0; i < maxOpsAmmount; ++i)
    {
        vector<string> strOperandVector(1, string("Op ") + std::to_string(i + 1));
        strOperandsVector.push_back(strOperandVector);
    }

    vector<string> strIndexVector{"Index"};
    vector<string> strNameVector{"Name"};
    vector<string> strSegmentVector{"Segment"};

    for (auto segIt = rawSentencesSegmentContainerVector.begin(); segIt != rawSentencesSegmentContainerVector.end(); ++segIt)
    {
        for (auto it = get<1>(*segIt).begin(); it != get<1>(*segIt).end(); ++it)
        {
            auto present = (*it)->present();
            const vector<string> &operandStrVector = get<1>(present);

            strIndexVector.push_back(std::to_string(it - get<1>(*segIt).begin()));
            strNameVector.push_back(get<0>(present));
            strSegmentVector.push_back(get<0>(*segIt));

            size_t i;
            for (i = 0; i < operandStrVector.size(); ++i)
            {
                strOperandsVector[i].push_back(operandStrVector[i]);
            }

            for (; i < strOperandsVector.size(); ++i)
                strOperandsVector[i].push_back("");
        }
    }

    vector<vector<string>> strTableVectors{strIndexVector, strNameVector, strSegmentVector};
    strTableVectors.insert(strTableVectors.end(), strOperandsVector.begin(), strOperandsVector.end());

    printTable("Raw Sentence Table", strTableVectors);
}

void printSentenceTable(const vector<TSSentencesSegmentContainer> &sentencesSegmentContainerVector)
{
    size_t maxOpsAmmount = 0;
    for (auto segIt = sentencesSegmentContainerVector.begin(); segIt != sentencesSegmentContainerVector.end(); ++segIt)
    {
        for (auto it = get<1>(*segIt).begin(); it != get<1>(*segIt).end(); ++it)
        {
            auto present = (*it)->present();
            const vector<string> &operandStrVector = get<1>(present);

            if (operandStrVector.size() > maxOpsAmmount)
                maxOpsAmmount = operandStrVector.size();
        }
    }

    vector<vector<string>> strOperandsVector;

    for (size_t i = 0; i < maxOpsAmmount; ++i)
    {
        vector<string> strOperandVector(1, string("Op ") + std::to_string(i + 1));
        strOperandsVector.push_back(strOperandVector);
    }

    vector<string> strIndexVector{"Index"};
    vector<string> strNameVector{"Name"};
    vector<string> strSegmentVector{"Segment"};

    for (auto segIt = sentencesSegmentContainerVector.begin(); segIt != sentencesSegmentContainerVector.end(); ++segIt)
    {
        for (auto it = get<1>(*segIt).begin(); it != get<1>(*segIt).end(); ++it)
        {
            auto present = (*it)->present();
            const vector<string> &operandStrVector = get<1>(present);

            strIndexVector.push_back(std::to_string(it - get<1>(*segIt).begin()));
            strNameVector.push_back(get<0>(present));
            strSegmentVector.push_back(get<0>(*segIt));

            size_t i;
            for (i = 0; i < operandStrVector.size(); ++i)
            {
                strOperandsVector[i].push_back(operandStrVector[i]);
            }

            for (; i < strOperandsVector.size(); ++i)
                strOperandsVector[i].push_back("");
        }
    }

    vector<vector<string>> strTableVectors{strIndexVector, strNameVector, strSegmentVector};
    strTableVectors.insert(strTableVectors.end(), strOperandsVector.begin(), strOperandsVector.end());

    printTable("Sentence Table", strTableVectors);
}
