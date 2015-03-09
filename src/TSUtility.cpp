#include "TSUtility.h"

const char cLF = 0xA;
const char cCR = 0xD;

const char identifierChars[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_@?$";
const char numberChars[] = "1234567890";
const char singleCharLexemChars[] = ",:[]()+-";
const char lexemDistributorChars[] = {0x9, 0x20};
const char commentChar = ';';
const char quoteCompatibleChars[] = "\'\"";

bool isCharIdentifierCompatible(char ch)
{
    return string(identifierChars).find(ch) != string::npos;
}

bool isCharNumberCompatible(char ch)
{
    return string(numberChars).find(ch) != string::npos;
}

bool isCharSingleCharacterLexemCompatible(char ch)
{
    return string(singleCharLexemChars).find(ch) != string::npos;
}

bool isCharLexemDistributorCompatible(char ch)
{
    return string(lexemDistributorChars).find(ch) != string::npos;
}

bool isCharCommentCompatible(char ch)
{
    return ch == commentChar;
}

bool isCharQuoteCompatible(char ch)
{
    return string(quoteCompatibleChars).find(ch) != string::npos;
}
