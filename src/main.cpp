#include "TSGlobal.h"
#include "TSCompiler.h"
#include "TSDiagnostics.h"

const char *listingFileType = "lst";

int main(int argc, const char **argv) {
    if (argc < 2)
        printError("Error: no source file specified");
    else {
        string sourceFileName(argv[1]);

        TSCompile(sourceFileName);
    }

    return 0;
}
