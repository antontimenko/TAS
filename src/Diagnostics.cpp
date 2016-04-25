#include "Diagnostics.h"

#include <iostream>
#include <iomanip>
#include <sstream>

const map<Token::Type, string> tokenTypeDescriptionMap = {
    {Token::Type::USER_IDENTIFIER, "User Identifier"},
    {Token::Type::MEMORY_BRACKET, "Memory Bracket"},
    {Token::Type::MATH_SYMBOL, "Math Symbol"},
    {Token::Type::COMMA, "Comma"},
    {Token::Type::COLON, "Colon"},
    {Token::Type::SEGMENT_DIRECTIVE, "Segment Directive"},
    {Token::Type::INSTRUCTION, "Instruction"},
    {Token::Type::REGISTER, "Register"},
    {Token::Type::SIZE_IDENTIFIER, "Size Identifier"},
    {Token::Type::DATA_IDENTIFIER, "Data Identifier"},
    {Token::Type::CONSTANT_NUMBER, "Constant Number"},
    {Token::Type::CONSTANT_STRING, "Constant String"},
    {Token::Type::CONDITION_DIRECTIVE, "Condition Directive"},
    {Token::Type::CONDITION, "Condition"},
    {Token::Type::SIZE_OPERATOR, "Size Operator"},
    {Token::Type::EQU_DIRECTIVE, "EQU Directive"},
    {Token::Type::END_DIRECTIVE, "END Directive"},
    {Token::Type::ASSUME_DIRECTIVE, "ASSUME Directive"}
};

void printError(string text) {
    cout << Color::BWhite << text << Color::Reset << endl;
}

void printCompileError(string text, const string &sourceFileContents, CodePosition pos) {
    cout << Color::BWhite << flush;

    cout << Color::BRed << "Compile Error" << Color::BWhite << " ("
         << pos.row << ":"
         << pos.column << "): "
         << text << endl;

    size_t i;
    size_t j;

    i = 0;
    size_t currentRow = 1;
    while (currentRow < pos.row) {
        if ((sourceFileContents[i] == cCR) || (sourceFileContents[i] == cLF))
            ++currentRow;
        if ((sourceFileContents[i] == cCR) && (sourceFileContents[i + 1] == cLF))
            ++i;
        ++i;
    }

    size_t lineStartIndex = i;
    
    j = 1;
    while ((sourceFileContents[i] != cCR) && (sourceFileContents[i] != cLF) && (i < sourceFileContents.size())) {
        if (j == pos.column)
            cout << Color::BRed << flush;
        else if (j == pos.column + pos.length)
            cout << Color::BWhite << flush;

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
    while (j < pos.column) {
        if (sourceFileContents[i] == 0x9)
            cout << "    ";
        else
            cout << " ";

        ++i;
        ++j;
    }
    cout << Color::BGreen << "^" << Color::BWhite << endl;

    cout << Color::Reset << flush;
}

string getTokenString(const Token &token) {
    string returnString;

    switch (token.type()) {
    case Token::Type::USER_IDENTIFIER:
        returnString = token.value<string>();
        break;
    case Token::Type::MEMORY_BRACKET:
        returnString = findByValue(Token::memoryBracketMap, token.value<Token::MemoryBracket>())->first;
        break;
    case Token::Type::MATH_SYMBOL:
        returnString = findByValue(Token::mathSymbolMap, token.value<Token::MathSymbol>())->first;
        break;
    case Token::Type::SEGMENT_DIRECTIVE:
        returnString = findByValue(Token::segmentDirectiveMap, token.value<Token::SegmentDirective>())->first;
        break;
    case Token::Type::INSTRUCTION:
        returnString = findByValue(Token::instructionMap, token.value<Token::Instruction>())->first;
        break;
    case Token::Type::REGISTER:
        returnString = findByValue(Token::registerMap, token.value<Token::Register>())->first;
        break;
    case Token::Type::SIZE_IDENTIFIER:
        returnString = findByValue(Token::sizeIdentifierMap, token.value<Token::SizeIdentifier>())->first;
        break;
    case Token::Type::DATA_IDENTIFIER:
        returnString = findByValue(Token::dataIdentifierMap, token.value<Token::DataIdentifier>())->first;
        break;
    case Token::Type::CONSTANT_NUMBER:
        returnString = token.value<Integer>().str();
        break;
    case Token::Type::CONSTANT_STRING:
        returnString = token.value<string>();
        break;
    case Token::Type::CONDITION_DIRECTIVE:
        returnString = findByValue(Token::conditionDirectiveMap, token.value<Token::ConditionDirective>())->first;
        break;
    case Token::Type::CONDITION:
        returnString = findByValue(Token::conditionMap, token.value<Token::Condition>())->first;
        break;
    case Token::Type::COMMA:
        returnString = Token::commaStr;
        break;
    case Token::Type::COLON:
        returnString = Token::colonStr;
        break;
    case Token::Type::SIZE_OPERATOR:
        returnString = Token::sizeOperatorStr;
        break;
    case Token::Type::EQU_DIRECTIVE:
        returnString = Token::equDirectiveStr;
        break;
    case Token::Type::END_DIRECTIVE:
        returnString = Token::endDirectiveStr;
        break;
    case Token::Type::ASSUME_DIRECTIVE:
        returnString = Token::assumeDirectiveStr;
    }

    return returnString;
}

string getTokenDescription(const Token &token) {
    string res = tokenTypeDescriptionMap.find(token.type())->second;

    if (token.type() == Token::Type::REGISTER) {
        Token::Register reg = token.value<Token::Register>();

        if (reg.match(OperandMask::UREG)) {
            if (reg.match(OperandMask::S8))
                res += " 8";
            else if (reg.match(OperandMask::S16))
                res += " 16";
            else
                res += " 32";
        } else
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

size_t getMaxWidth(const vector<string> &strVector) {
    size_t maxWidth = 0;
    
    for (auto it = strVector.begin(); it != strVector.end(); ++it) {
        if (it->size() > maxWidth)
            maxWidth = it->size();
    }

    return maxWidth;
}

void printSpace(size_t ammount) {
    for (size_t i = 0; i < ammount; ++i)
        cout << ' ';
    cout << flush;
}

void printTable(string name, const vector<vector<string>> &data) {
    cout << Color::BWhite << flush;

    vector<size_t> maxWidthVector(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        maxWidthVector[i] = getMaxWidth(data[i]);

    size_t wholeWidth = 0;
    for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it)
        wholeWidth += *it;

    cout << tch3;
    for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it) {
        for (size_t i = 0; i < *it; ++i)
            cout << tch0;
        if (it != maxWidthVector.end() - 1)
            cout << tch0;
    }
    cout << tch2 << endl;

    cout << tch1;
    if (name.size() < (wholeWidth + data.size())) {
        printSpace((wholeWidth + data.size() - 1 - name.size()) / 2);
        cout << name;
        printSpace((wholeWidth + data.size() - 1) - ((wholeWidth + data.size() - 1 - name.size()) / 2) - name.size());
    } else
        cout << string(name.begin(), name.begin() + (wholeWidth + data.size() - 1));
    cout << tch1 << endl;

    cout << tch7;
    for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it) {
        for (size_t i = 0; i < *it; ++i)
            cout << tch0;
        if (it != maxWidthVector.end() - 1)
            cout << tch9;
    }
    cout << tch6 << endl;

    for (size_t i = 0; i < data[0].size(); ++i) {
        cout << tch1;
        for (size_t j = 0; j < data.size(); ++j) {
            if (i < data[j].size()) {
                cout << data[j][i];
                printSpace(maxWidthVector[j] - data[j][i].size());
            } else
                printSpace(maxWidthVector[j]);

            cout << tch1;
        }
        cout << endl;

        if (i != data[0].size() - 1) {
            cout << tch7;
            for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it) {
                for (size_t i = 0; i < *it; ++i)
                    cout << tch0;
                if (it != maxWidthVector.end() - 1)
                    cout << tch10;
            }
            cout << tch6 << endl;
        }
    }

    cout << tch4;
    for (auto it = maxWidthVector.begin(); it != maxWidthVector.end(); ++it) {
        for (size_t i = 0; i < *it; ++i)
            cout << tch0;
        if (it != maxWidthVector.end() - 1)
            cout << tch8;
    }
    cout << tch5 << endl;

    cout << Color::Reset << flush;
}

void printTokenTable(const vector<TokenContainer> &tokenContainerVector) {
    vector<string> strIndexVector{"Index"};
    vector<string> strCoordsVector{"Coords"};
    vector<string> strTokenVector{"Name"};
    vector<string> strTokenDescriptionVector{"Description"};

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it) {
        strIndexVector.push_back(std::to_string(it - tokenContainerVector.begin()));
        strCoordsVector.push_back(string("(") + std::to_string(it->pos.row) + "," + std::to_string(it->pos.column) + ")");
        strTokenVector.push_back(getTokenString(it->token));
        strTokenDescriptionVector.push_back(getTokenDescription(it->token));
    }

    printTable("Lexeme Table", {strIndexVector, strCoordsVector, strTokenVector, strTokenDescriptionVector});
}

void printTokenTable(const vector<TokenContainer> &tokenContainerVector, const vector<LexemeContainer> &lexemeContainerVector)
{
    vector<string> strIndexVector{"Index"};
    vector<string> strCoordsVector{"Coords"};
    vector<string> strTokenVector{"Name"};
    vector<string> strTokenDescriptionVector{"Description"};

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it) {
        strIndexVector.push_back(std::to_string(it - tokenContainerVector.begin()));
        strCoordsVector.push_back(string("(") + std::to_string(it->pos.row) + "," + std::to_string(it->pos.column) + ")");
        strTokenVector.push_back(lexemeContainerVector[it - tokenContainerVector.begin()].lexeme);
        strTokenDescriptionVector.push_back(getTokenDescription(it->token));
    }

    printTable("Native Lexeme Table", {strIndexVector, strCoordsVector, strTokenVector, strTokenDescriptionVector});
}

void printEquTable(const map<string, Integer> &equMap) {
    vector<string> strNameVector{"Name"};
    vector<string> strValueVector{"Value"};

    for (auto it = equMap.begin(); it != equMap.end(); ++it) {
        strNameVector.push_back(it->first);
        strValueVector.push_back(it->second.str());
    }

    printTable("EQU Table", {strNameVector, strValueVector});
}

void printPseudoLabelTable(const map<string, Label> &labelMap) {
    vector<string> strNameVector{"Name"};
    vector<string> strTypeVector{"Type"};
    vector<string> strIndexVector{"Index"};
    vector<string> strSegmentVector{"Segment"};

    for (auto it = labelMap.begin(); it != labelMap.end(); ++it) {
        strNameVector.push_back(it->first);
        strTypeVector.push_back(it->second.dataIdentifier ? findByValue(Token::dataIdentifierMap, *it->second.dataIdentifier)->first : "LABEL");
        strIndexVector.push_back(std::to_string(it->second.ptr));
        strSegmentVector.push_back(it->second.segName);
    }

    printTable("Pseudo Label Table", {strNameVector, strTypeVector, strIndexVector, strSegmentVector});
}

void printPseudoSentenceTable(const vector<PseudoSentencesSegment> &segmentPseudoSentenceVector, bool printAssumes) {
    size_t maxOpsAmmount = 0;
    for (auto segIt = segmentPseudoSentenceVector.begin(); segIt != segmentPseudoSentenceVector.end(); ++segIt) {
        for (auto it = segIt->pseudoSentences.begin(); it != segIt->pseudoSentences.end(); ++it) {
            if (it->operandsTokenContainerVector.size() > maxOpsAmmount)
                maxOpsAmmount = it->operandsTokenContainerVector.size();
        }
    }

    vector<vector<string>> strOperandsVector;

    for (size_t i = 0; i < maxOpsAmmount; ++i) {
        vector<string> strOperandVector(1, string("Op ") + std::to_string(i + 1));
        strOperandsVector.push_back(strOperandVector);
    }

    vector<string> strIndexVector{"Index"};
    vector<string> strNameVector{"Name"};
    vector<string> strSegmentVector{"Segment"};
    vector<string> strAssumesVector{"Assumes"};

    for (auto segIt = segmentPseudoSentenceVector.begin(); segIt != segmentPseudoSentenceVector.end(); ++segIt) {
        for (auto it = segIt->pseudoSentences.begin(); it != segIt->pseudoSentences.end(); ++it) {
            strIndexVector.push_back(std::to_string(it - segIt->pseudoSentences.begin()));
            strNameVector.push_back(getTokenString(it->baseTokenContainer.token));
            strSegmentVector.push_back(segIt->segName);

            auto assumeMap = it->assume.getMap();
            string assumeStr;
            for (auto jt = assumeMap.begin(); jt != assumeMap.end(); ++jt) {
                if (!assumeStr.empty())
                    assumeStr += ", ";
                assumeStr += jt->first;
                assumeStr += ":";
                assumeStr += findByValue(Token::registerMap, jt->second)->first;
            }

            strAssumesVector.push_back(assumeStr);

            size_t i;
            for (i = 0; i < it->operandsTokenContainerVector.size(); ++i) {
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
    if (printAssumes)
        strTableVectors.push_back(strAssumesVector);

    printTable("Pseudo Sentence Table", strTableVectors);
}

void printRawSentenceTable(const vector<RawSentencesSegment> &rawSentencesSegmentContainerVector, const map<string, Label> &labelMap, bool printAssumes) {
    size_t maxOpsAmmount = 0;
    for (auto segIt = rawSentencesSegmentContainerVector.begin(); segIt != rawSentencesSegmentContainerVector.end(); ++segIt) {
        for (auto it = segIt->rawSentences.begin(); it != segIt->rawSentences.end(); ++it) {
            auto present = (*it)->present(labelMap);
            const vector<string> &operandStrVector = get<1>(present);

            if (operandStrVector.size() > maxOpsAmmount)
                maxOpsAmmount = operandStrVector.size();
        }
    }

    vector<vector<string>> strOperandsVector;

    for (size_t i = 0; i < maxOpsAmmount; ++i) {
        vector<string> strOperandVector(1, string("Op ") + std::to_string(i + 1));
        strOperandsVector.push_back(strOperandVector);
    }

    vector<string> strIndexVector{"Index"};
    vector<string> strNameVector{"Name"};
    vector<string> strSegmentVector{"Segment"};
    vector<string> strAssumesVector{"Assumes"};

    for (auto segIt = rawSentencesSegmentContainerVector.begin(); segIt != rawSentencesSegmentContainerVector.end(); ++segIt) {
        for (auto it = segIt->rawSentences.begin(); it != segIt->rawSentences.end(); ++it) {
            auto present = (*it)->present(labelMap);
            const vector<string> &operandStrVector = get<1>(present);

            strIndexVector.push_back(std::to_string(it - segIt->rawSentences.begin()));
            strNameVector.push_back(get<0>(present));
            strSegmentVector.push_back(segIt->segName);

            auto assumeMap = (*it)->assume().getMap();
            string assumeStr;
            for (auto jt = assumeMap.begin(); jt != assumeMap.end(); ++jt) {
                if (!assumeStr.empty())
                    assumeStr += ", ";
                assumeStr += jt->first;
                assumeStr += ":";
                assumeStr += findByValue(Token::registerMap, jt->second)->first;
            }

            strAssumesVector.push_back(assumeStr);

            size_t i;
            for (i = 0; i < operandStrVector.size(); ++i)
                strOperandsVector[i].push_back(operandStrVector[i]);

            for (; i < strOperandsVector.size(); ++i)
                strOperandsVector[i].push_back("");
        }
    }

    vector<vector<string>> strTableVectors{strIndexVector, strNameVector, strSegmentVector};
    strTableVectors.insert(strTableVectors.end(), strOperandsVector.begin(), strOperandsVector.end());
    if (printAssumes)
        strTableVectors.push_back(strAssumesVector);

    printTable("Raw Sentence Table", strTableVectors);
}

void printSentenceTable(const vector<SentencesSegment> &sentencesSegmentContainerVector, bool printAssumes)
{
    size_t maxOpsAmmount = 0;
    for (auto segIt = sentencesSegmentContainerVector.begin(); segIt != sentencesSegmentContainerVector.end(); ++segIt) {
        for (auto it = segIt->sentences.begin(); it != segIt->sentences.end(); ++it) {
            auto present = (*it)->present();
            const vector<string> &operandStrVector = get<1>(present);

            if (operandStrVector.size() > maxOpsAmmount)
                maxOpsAmmount = operandStrVector.size();
        }
    }

    vector<vector<string>> strOperandsVector;

    for (size_t i = 0; i < maxOpsAmmount; ++i) {
        vector<string> strOperandVector(1, string("Op ") + std::to_string(i + 1));
        strOperandsVector.push_back(strOperandVector);
    }

    vector<string> strIndexVector{"Index"};
    vector<string> strNameVector{"Name"};
    vector<string> strSegmentVector{"Segment"};
    vector<string> strAssumesVector{"Assumes"};

    for (auto segIt = sentencesSegmentContainerVector.begin(); segIt != sentencesSegmentContainerVector.end(); ++segIt) {
        for (auto it = segIt->sentences.begin(); it != segIt->sentences.end(); ++it) {
            auto present = (*it)->present();
            const vector<string> &operandStrVector = get<1>(present);

            strIndexVector.push_back(std::to_string(it - segIt->sentences.begin()));
            strNameVector.push_back(get<0>(present));
            strSegmentVector.push_back(segIt->segName);

            auto assumeMap = (*it)->assume.getMap();
            string assumeStr;
            for (auto jt = assumeMap.begin(); jt != assumeMap.end(); ++jt) {
                if (!assumeStr.empty())
                    assumeStr += ", ";
                assumeStr += jt->first;
                assumeStr += ":";
                assumeStr += findByValue(Token::registerMap, jt->second)->first;
            }

            strAssumesVector.push_back(assumeStr);

            size_t i;
            for (i = 0; i < operandStrVector.size(); ++i)
                strOperandsVector[i].push_back(operandStrVector[i]);

            for (; i < strOperandsVector.size(); ++i)
                strOperandsVector[i].push_back("");
        }
    }

    vector<vector<string>> strTableVectors{strIndexVector, strNameVector, strSegmentVector};
    strTableVectors.insert(strTableVectors.end(), strOperandsVector.begin(), strOperandsVector.end());
    if (printAssumes)
        strTableVectors.push_back(strAssumesVector);

    printTable("Sentence Table", strTableVectors);
}

string hexStringFromSentenceBytePresentation(const vector<vector<uchar>> &sentenceBytePresentation) {
    std::stringstream strStream;
    strStream << std::hex << std::uppercase << std::setfill('0');
    
    for (auto it = sentenceBytePresentation.begin(); it != sentenceBytePresentation.end(); ++it) {
        auto jt = it->end();
        do {
            --jt;

            strStream << std::setw(2) << (unsigned int)*jt;
        } while (jt != it->begin());

        if (it != sentenceBytePresentation.end() - 1)
            strStream << ' ';
    }

    return strStream.str();
}

void printListing(const vector<SentencesSegment> &sentencesSegmentContainerVector) {
    for (auto segIt = sentencesSegmentContainerVector.begin(); segIt != sentencesSegmentContainerVector.end(); ++segIt) {
        cout << segIt->segName << " SEGMENT" << endl << endl;

        size_t disp = 0;
        cout << std::setfill('0') << std::uppercase;
        
        for (auto it = segIt->sentences.begin(); it != segIt->sentences.end(); ++it) {
            cout << std::hex << std::setw(4) << disp << std::dec << "  ";
            
            auto computeRes = (*it)->compute();
            string sentenceByteCodeStr = hexStringFromSentenceBytePresentation(computeRes);
            for (size_t i = 0; i < sentenceByteCodeStr.size(); ++i) {
                cout << sentenceByteCodeStr[i];
                if (((i % 29) == 0) && (i != 0))
                    cout << endl << "      ";
            }
            printSpace(32 - sentenceByteCodeStr.size() % 30);
            
            auto sentencePresent = (*it)->present();
            cout << get<0>(sentencePresent);
            printSpace(10 - get<0>(sentencePresent).size());
            for (auto jt = get<1>(sentencePresent).begin(); jt != get<1>(sentencePresent).end(); ++jt) {
                if (jt != get<1>(sentencePresent).begin())
                    cout << ',';
                cout << *jt;
            }

            cout << endl;

            disp += getInstructionBytePresentSize(computeRes);
        }

        cout << std::hex << std::setw(4) << disp << std::dec << "  " << endl;

        cout << std::setfill(' ');

        cout << endl << segIt->segName << " ENDS" << endl << endl;;
    }
}

void printListing(const vector<SentencesSegment> &sentencesSegmentContainerVector, const tuple<vector<PseudoSentencesSegment>, map<string, Label>> &pseudoSentenceSplit) {
    const vector<PseudoSentencesSegment> &pseudoSentencesSegmentContainerVector = get<0>(pseudoSentenceSplit);
    const map<string, Label> &labelMap = get<1>(pseudoSentenceSplit);
    
    for (auto segIt = sentencesSegmentContainerVector.begin(); segIt != sentencesSegmentContainerVector.end(); ++segIt) {
        const vector<PseudoSentence> &pseudoSentenceVector = (pseudoSentencesSegmentContainerVector[segIt - sentencesSegmentContainerVector.begin()]).pseudoSentences;

        cout << segIt->segName << " SEGMENT" << endl << endl;

        size_t disp = 0;
        cout << std::setfill('0') << std::uppercase;
        
        for (auto it = segIt->sentences.begin(); it != segIt->sentences.end(); ++it) {
            for (auto jt = labelMap.begin(); jt != labelMap.end(); ++jt) {
                if ((segIt->segName == jt->second.segName) && ((size_t)(it - segIt->sentences.begin()) == jt->second.ptr)) {
                    printSpace(6);
                    cout << jt->first << ':' << endl;
                    break;
                }
            }

            const PseudoSentence &pseudoSentence = pseudoSentenceVector[it - segIt->sentences.begin()];

            cout << std::hex << std::setw(4) << disp << std::dec << "  ";
            
            auto computeRes = (*it)->compute();
            string sentenceByteCodeStr = hexStringFromSentenceBytePresentation(computeRes);
            for (size_t i = 0; i < sentenceByteCodeStr.size(); ++i) {
                cout << sentenceByteCodeStr[i];
                if (((i % 29) == 0) && (i != 0))
                    cout << endl << "      ";
            }
            printSpace(32 - sentenceByteCodeStr.size() % 30);
            
            auto sentencePresent = (*it)->present();
            cout << get<0>(sentencePresent);
            printSpace(10 - get<0>(sentencePresent).size());

            for (auto jt = pseudoSentence.operandsTokenContainerVector.begin(); jt != pseudoSentence.operandsTokenContainerVector.end(); ++jt) {
                bool isPreviousSingleChar = true;
                for (auto kt = jt->begin(); kt != jt->end(); ++kt) {
                    string tokenStr = getTokenString(kt->token);
                    bool isSingleChar = (tokenStr.size() == 1) && (isCharSingleCharacterLexemeCompatible(tokenStr[0]));

                    if ((!isPreviousSingleChar) && (!isSingleChar))
                        cout << ' ';

                    cout << tokenStr;

                    isPreviousSingleChar = isSingleChar;
                }

                if (jt != pseudoSentence.operandsTokenContainerVector.end() - 1)
                    cout << ',';
            }

            cout << endl;

            disp += getInstructionBytePresentSize(computeRes);
        }

        for (auto jt = labelMap.begin(); jt != labelMap.end(); ++jt) {
            if ((segIt->segName == jt->second.segName) && (segIt->sentences.size() == jt->second.ptr)) {
                printSpace(6);
                cout << jt->first << ':' << endl;
                break;
            }
        }

        cout << std::hex << std::setw(4) << disp << std::dec << "  " << endl;

        cout << std::setfill(' ');

        cout << endl << segIt->segName << " ENDS" << endl << endl;;
    }
}
