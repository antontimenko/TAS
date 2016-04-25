#include "Global.h"
#include "Compiler.h"
#include "Diagnostics.h"

const char *listingFileType = "lst";

int main(int argc, const char **argv) {
    if (argc < 2)
        printError("Error: no source file specified");
    else {
        string sourceFileName(argv[1]);

        Compile(sourceFileName);
    }

    return 0;
}
