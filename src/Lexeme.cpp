#include "Lexeme.h"

#include "Exception.h"
#include <ctype.h>

const char cLF = 0xA;
const char cCR = 0xD;

const string identifierChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_@?$";
const string numberChars = "1234567890";
const string singleCharLexemChars = ",:[]()+-*/";
const string lexemDistributorChars = {0x9, 0x20, cLF, cCR};
const char commentChar = ';';
const string quoteCompatibleChars = "\'\"";

bool isCharIdentifierCompatible(char ch) {
    return (identifierChars.find(ch) != string::npos) || isCharNumberCompatible(ch);
}

bool isCharNumberCompatible(char ch) {
    return numberChars.find(ch) != string::npos;
}

bool isCharSingleCharacterLexemeCompatible(char ch) {
    return singleCharLexemChars.find(ch) != string::npos;
}

bool isCharLexemeDistributorCompatible(char ch) {
    return lexemDistributorChars.find(ch) != string::npos;
}

bool isCharQuoteCompatible(char ch) {
    return quoteCompatibleChars.find(ch) != string::npos;
}

vector<LexemeContainer> constructLexemeContainerVector(const string &sourceFileContents) {
    vector<LexemeContainer> lexemeContainerVector;

    string currentLexeme;
    size_t row = 1;
    size_t column = 1;
    size_t currentLexemeRow;
    size_t currentLexemeColumn;
    bool isCommentStarted = false;
    for (size_t i = 0; i < sourceFileContents.size(); ++i) {
        if ((i > 0) && ((sourceFileContents[i - 1] == cLF) || ((sourceFileContents[i - 1] == cCR) && (sourceFileContents[i] != cLF)))) {
            ++row;
            column = 1;
        }

        const char &currentChar = sourceFileContents[i];

        if (isCommentStarted) {
            if ((currentChar == cLF) || (currentChar == cCR))
                isCommentStarted = false;
        } else {
            if ((!currentLexeme.empty()) && (isCharQuoteCompatible(currentLexeme[0]))) {
                currentLexeme += currentChar;
                
                if (currentChar == currentLexeme[0]) {
                    lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
                    currentLexeme.erase();
                }
            } else {
                if (isCharIdentifierCompatible(currentChar)) {
                    if (currentLexeme.empty()) {
                        currentLexemeRow = row;
                        currentLexemeColumn = column;
                    }

                    currentLexeme += currentChar;
                } else if (isCharSingleCharacterLexemeCompatible(currentChar)) {
                    if (!currentLexeme.empty()) {
                        lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
                        currentLexeme.erase();
                    }

                    currentLexemeRow = row;
                    currentLexemeColumn = column;

                    lexemeContainerVector.push_back({row, column, string(1, currentChar)});
                } else if (isCharLexemeDistributorCompatible(currentChar)) {
                    if (!currentLexeme.empty()) {
                        lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
                        currentLexeme.erase();
                    }
                } else if (currentChar == commentChar) {
                    if (!currentLexeme.empty()) {
                        lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
                        currentLexeme.erase();
                    }

                    isCommentStarted = true;
                } else if (isCharQuoteCompatible(currentChar)) {
                    if (!currentLexeme.empty()) {
                        lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
                        currentLexeme.erase();
                    }

                    currentLexemeRow = row;
                    currentLexemeColumn = column;

                    currentLexeme += currentChar;
                } else
                    throw CompileError("Unknown character", {row, column, 1});
            }

            ++column;
        }
    }

    if (!currentLexeme.empty()) {
        if ((isCharQuoteCompatible(currentLexeme[0])) && (currentLexeme[0] != currentLexeme[currentLexeme.size() - 1]))
            throw CompileError("Unclosed string detected", {currentLexemeRow, currentLexemeColumn, 1});
        
        lexemeContainerVector.push_back({currentLexemeRow, currentLexemeColumn, currentLexeme});
    }

    return lexemeContainerVector;
}

vector<LexemeContainer> convertLexemeContainerVectorToUpperCase(vector<LexemeContainer> &lexemeContainerVector) {
    vector<LexemeContainer> newLexemeContainerVector;

    for (auto it = lexemeContainerVector.begin(); it != lexemeContainerVector.end(); ++it) {
        if (!isCharQuoteCompatible(it->lexeme[0]) && !isCharQuoteCompatible(it->lexeme[it->lexeme.size() - 1])) {
            string currentLexeme;
            
            for (size_t j = 0; j < it->lexeme.size(); ++j)
                currentLexeme += (char)toupper(it->lexeme[j]);

            newLexemeContainerVector.push_back({it->row, it->column, currentLexeme});
        } else
            newLexemeContainerVector.push_back(*it);
    }

    return newLexemeContainerVector;
}
