#ifndef Rct_h
#define Rct_h

#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <rct/String.h>
#include <rct/Path.h>
#include <rct/List.h>

namespace Rct {
String shortOptions(const option *longOptions);
int readLine(FILE *f, char *buf = 0, int max = -1);
inline int fileSize(FILE *f)
{
    assert(f);
    const int pos = ftell(f);
    fseek(f, 0, SEEK_END);
    const int ret = ftell(f);
    fseek(f, pos, SEEK_SET);
    return ret;
}
String filterPreprocessor(const Path &path);
void removeDirectory(const Path &path);
int canonicalizePath(char *path, int len);
String unescape(String command);
bool startProcess(const Path &dotexe, const List<String> &dollarArgs);
void findExecutablePath(const char *argv0);
Path executablePath();
String backtrace(int maxFrames = -1);
}

#define eintrwrap(VAR, BLOCK)                   \
    do {                                        \
        VAR = BLOCK;                            \
    } while (VAR == -1 && errno == EINTR);

#endif
