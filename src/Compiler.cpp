#include "Compiler.h"

#include "Lexeme.h"
#include "Exception.h"
#include "Token.h"
#include "Diagnostics.h"
#include "Preprocessor.h"
#include "PseudoSentence.h"
#include "RawSentence.h"
#include "Sentence.h"
#include <fstream>

const Compiler::Arch Compiler::arch = Arch::X86_16;

Compiler::Compiler() {
}

void Compiler::compile(const string &sourceFilePath) const {
    try {
        std::ifstream sourceFile(sourceFilePath);
        if (!sourceFile.is_open())
            throw Exception(string("File \'") + sourceFilePath + "\' not found, or permission denied");

        string sourceFileContents((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());

        try {
            auto phase1 = constructLexemeContainerVector(sourceFileContents);
            auto phase2 = convertLexemeContainerVectorToUpperCase(phase1);
            auto phase3 = constructTokenContainerVector(phase2);
            //printTokenTable(phase3, phase2); //LEXICAL ANALYZER
            auto phase4 = preprocess(phase3);
            auto phase5 = splitPseudoSentences(get<0>(phase4));
            auto phase6 = constructRawSentences(get<0>(phase5), get<1>(phase5));
            //printRawSentenceTable(phase6, get<1>(phase5), true); //SYNTATICAL ANALYZER
            auto phase7 = constructSentences(phase6);
            printListing(phase7, phase5); //LISTING
        } catch (CompileError &e) {
            printCompileError(e.what(), sourceFileContents, e.pos());
        }
    } catch (std::exception &e) {
        printError(e.what());
    }
}

Compiler &Compiler::instance() {
    static Compiler compiler;
    return compiler;
}

void Compile(const string &sourceFilePath) {
    Compiler::instance().compile(sourceFilePath);
}
