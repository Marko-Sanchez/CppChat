#include "commandlineparser.h"

#include <cctype>
#include <string>

namespace utility
{
CommandLineArgs ParseArgs(int argc, char* argv[])
{
    CommandLineArgs args;
    for (int i {1}; i < argc; ++i)
    {
        std::string arg {argv[i]};

        if (arg == "--version")
        {
            args.showversion = true;
        }
        else if (std::isdigit(static_cast<unsigned char>(arg[0])))
        {
            try
            {
                args.port = std::stoi(std::string(arg));
            }
            catch (...)
            {
                args.port = 8080;// use default
            }
        }
    }

    return args;
}
}// namespace utility
