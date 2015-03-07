#include "TSGlobal.h"
#include "TSCompiler.h"

const char *listingFileType = "lst";

int main(int argc, const char **argv)
{
    if (argc < 2)
        cout << "Error: no source file specified" << endl;
    else
    {
        string sourceFileName(argv[1]);

        int dotPos = sourceFileName.rfind('.');
        string resultFileName;
        if ((size_t)dotPos == string::npos)
            resultFileName = sourceFileName + '.' + listingFileType;
        else
            resultFileName = string(sourceFileName.substr(0, dotPos + 1) + listingFileType);

        TSCompile(sourceFileName, resultFileName);
    }

    return 0;
}
