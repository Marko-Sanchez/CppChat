#ifndef SERVER_COMMANDLINEPARSER_H
#define SERVER_COMMANDLINEPARSER_H

namespace utility
{
struct CommandLineArgs
{
    bool showversion {false};
    int port         {8080};
};

CommandLineArgs ParseArgs(int argc, char* argv[]);
}// namespace utility
#endif
