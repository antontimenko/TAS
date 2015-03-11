#include "TSUtility.h"

const char cLF = 0xA;
const char cCR = 0xD;

const string identifierChars = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_@?$";
const string numberChars = "1234567890";
const string singleCharLexemChars = ",:[]()+-";
const string lexemDistributorChars = {0x9, 0x20, 0xA, 0xD};
const char commentChar = ';';
const string quoteCompatibleChars = "\'\"";

bool isCharIdentifierCompatible(char ch)
{
    return identifierChars.find(ch) != string::npos;
}

bool isCharNumberCompatible(char ch)
{
    return numberChars.find(ch) != string::npos;
}

bool isCharSingleCharacterLexemCompatible(char ch)
{
    return singleCharLexemChars.find(ch) != string::npos;
}

bool isCharLexemDistributorCompatible(char ch)
{
    return lexemDistributorChars.find(ch) != string::npos;
}

bool isCharCommentCompatible(char ch)
{
    return ch == commentChar;
}

bool isCharQuoteCompatible(char ch)
{
    return quoteCompatibleChars.find(ch) != string::npos;
}
