#include "TSLexeme.h"

#include "TSUtility.h"
#include "TSException.h"
#include <ctype.h>

vector<TSLexemeContainer> constructLexemeContainerVector(const string &sourceFileContents)
{
    vector<TSLexemeContainer> lexemeContainerVector;

    string currentLexeme;
    size_t row = 1;
    size_t column = 1;
    size_t currentLexemeRow;
    size_t currentLexemeColumn;
    for (size_t i = 0; i < sourceFileContents.size(); ++i)
    {
        if ((i > 0) && ((sourceFileContents[i - 1] == cLF) || ((sourceFileContents[i - 1] == cCR) && (sourceFileContents[i] != cLF))))
        {
            ++row;
            column = 1;
        }

        const char &currentChar = sourceFileContents[i];

        if ((!currentLexeme.empty()) && (isCharQuoteCompatible(currentLexeme[0])))
        {
            currentLexeme += currentChar;
            
            if (currentChar == currentLexeme[0])
            {
                lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
                currentLexeme.erase();
            }
        }
        else
        {
            if (isCharIdentifierCompatible(currentChar))
            {
                if (currentLexeme.empty())
                {
                    currentLexemeRow = row;
                    currentLexemeColumn = column;
                }

                currentLexeme += currentChar;
            }
            else if (isCharSingleCharacterLexemCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
                    currentLexeme.erase();
                }

                currentLexemeRow = row;
                currentLexemeColumn = column;

                lexemeContainerVector.push_back({row, column, string(1, currentChar)});
            }
            else if (isCharLexemDistributorCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
                    currentLexeme.erase();
                }
            }
            else if (isCharQuoteCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
                    currentLexeme.erase();
                }

                currentLexemeRow = row;
                currentLexemeColumn = column;

                currentLexeme += currentChar;
            }
            else
                throw TSCompileError("Unknown character", row, column, 1);

            ++column;
        }
    }

    if (!currentLexeme.empty())
    {
        if ((isCharQuoteCompatible(currentLexeme[0])) && (currentLexeme[0] != currentLexeme[currentLexeme.size() - 1]))
            throw TSCompileError("Unclosed string detected", currentLexemeRow, currentLexemeColumn, 1);
        
        lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
    }

    return lexemeContainerVector;
}

vector<TSLexemeContainer> convertLexemeContainerVectorToUpperCase(vector<TSLexemeContainer> &lexemeContainerVector)
{
    vector<TSLexemeContainer> newLexemeContainerVector;

    for (size_t i = 0; i < lexemeContainerVector.size(); ++i)
    {
        if (!isCharQuoteCompatible(lexemeContainerVector[i].lexeme[0]) && !isCharQuoteCompatible(lexemeContainerVector[i].lexeme[lexemeContainerVector[i].lexeme.size() - 1]))
        {
            string currentLexeme;
            
            for (size_t j = 0; j < lexemeContainerVector[i].lexeme.size(); ++j)
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
