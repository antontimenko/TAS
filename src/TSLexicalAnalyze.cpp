#include "TSLexicalAnalyze.h"

#include "TSUtility.h"
#include "TSException.h"
#include <ctype.h>

vector<TSLexemeContainer> constructLexemeContainerVector(const string &sourceFileContents)
{
    vector<TSLexemeContainer> lexemeContainerVector;

    string currentLexeme;
    uint row = 1;
    uint column = 1;
    for (uint i = 0; i < sourceFileContents.size(); ++i)
    {
        const char &currentChar = sourceFileContents[i];

        if ((!currentLexeme.empty()) && (isCharQuoteCompatible(currentLexeme[0])))
        {
            currentLexeme += currentChar;
            if (currentChar == currentLexeme[0])
            {
                lexemeContainerVector.push_back({row, column - currentLexeme.size(), currentLexeme});
                currentLexeme.erase();
            }

            ++column;
        }
        else
        {
            if (isCharIdentifierCompatible(currentChar))
            {
                currentLexeme += currentChar;

                ++column;
            }
            else if (isCharSingleCharacterLexemCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeContainerVector.push_back({row, column - currentLexeme.size(), currentLexeme});
                    currentLexeme.erase();
                }

                lexemeContainerVector.push_back({row, column, string(1, currentChar)});

                ++column;
            }
            else if (isCharLexemDistributorCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeContainerVector.push_back({row, column - currentLexeme.size(), currentLexeme});
                    currentLexeme.erase();
                }

                ++column;
            }
            else if (isCharQuoteCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeContainerVector.push_back({row, column - currentLexeme.size(), currentLexeme});
                    currentLexeme.erase();
                }

                currentLexeme += currentChar;

                ++column;
            }
            else if ((sourceFileContents[i] == cCR) || (sourceFileContents[i] == cLF))
            {
                if ((sourceFileContents[i] == cCR) || 
                     ((sourceFileContents[i] == cLF) && ((i == 0) || (sourceFileContents[i - 1] != cCR))))
                {
                    if (!currentLexeme.empty())
                    {
                        lexemeContainerVector.push_back({row, column - currentLexeme.size(), currentLexeme});
                        currentLexeme.erase();
                    }

                    ++row;
                    column = 1;
                }
            }
            else
                throw TSCompileError(string("Unknown character \"") + currentChar + "\"", &sourceFileContents, row, column, 1);
        }
    }

    if (!currentLexeme.empty())
        lexemeContainerVector.push_back({row, column - currentLexeme.size(), currentLexeme});

    return lexemeContainerVector;
}

vector<TSLexemeContainer> convertLexemeContainerVectorToUpperCase(vector<TSLexemeContainer> &lexemeContainerVector)
{
    vector<TSLexemeContainer> newLexemeContainerVector;

    for (uint i = 0; i < lexemeContainerVector.size(); ++i)
    {
        if (!isCharQuoteCompatible(lexemeContainerVector[i].lexeme[0]) && !isCharQuoteCompatible(lexemeContainerVector[i].lexeme[lexemeContainerVector[i].lexeme.size() - 1]))
        {
            string currentLexeme;
            
            for (uint j = 0; j < lexemeContainerVector[i].lexeme.size(); ++j)
            {
                currentLexeme += (char)toupper(lexemeContainerVector[i].lexeme[j]);
            }

            newLexemeContainerVector.push_back({lexemeContainerVector[i].row, lexemeContainerVector[i].column, currentLexeme});
        }
        else
        {
            newLexemeContainerVector.push_back(lexemeContainerVector[i]);
        }
    }

    return newLexemeContainerVector;
}
