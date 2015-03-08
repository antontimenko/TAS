#include "TSLexicalAnalyze.h"

#include "TSUtility.h"

vector<string> constructLexemeVector(const string &sourceFileContents)
{
    vector<string> lexemeVector;

    string currentLexeme;
    for (uint i = 0; i < sourceFileContents.size(); ++i)
    {
        //cout << sourceFileContents[i] << ": " << (uint)sourceFileContents[i] << endl;

        const char &currentChar = sourceFileContents[i];

        if ((!currentLexeme.empty()) && (isCharQuoteCompatible(currentLexeme[0])))
        {
            currentLexeme += currentChar;
            if (currentChar == currentLexeme[0])
            {
                lexemeVector.push_back(currentLexeme);
                currentLexeme.erase();
            }
        }
        else
        {
            if (isCharIdentifierCompatible(currentChar))
                currentLexeme += currentChar;
            else if (isCharSingleCharacterLexemCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeVector.push_back(currentLexeme);
                    currentLexeme.erase();
                }

                lexemeVector.push_back(string(1, currentChar));
            }
            else if (isCharLexemDistributorCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeVector.push_back(currentLexeme);
                    currentLexeme.erase();
                }
            }
            else if (isCharQuoteCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeVector.push_back(currentLexeme);
                    currentLexeme.erase();
                }

                currentLexeme += currentChar;
            }
            else if (isCharLineTerminatorCompatible(currentChar))
            {
                if (!currentLexeme.empty())
                {
                    lexemeVector.push_back(currentLexeme);
                    currentLexeme.erase();
                }
            }
            else
            {
                cout << "Error: undefined character" << endl;
                lexemeVector.clear();
                currentLexeme.erase();
                break;
            }
        }
    }

    if (!currentLexeme.empty())
        lexemeVector.push_back(currentLexeme);

    return lexemeVector;
}
