#include <cstdio>
#include <print>

#include <boost/system.hpp>

#include "cppchat.h"
#include "commandlineparser.h"
#include "server.hpp"

int main (int argc, char *argv[])
{
    utility::CommandLineArgs args = utility::ParseArgs(argc, argv);
    if (args.showversion)
    {
        std::println("cppchat-server {} ({})", CPPCHAT_VERSION, CPPCHAT_GIT_HASH);
        return 0;
    }

    try
    {
        server::core::AsyncTCPServer server;
        server.Run(args.port, 2);
    }
    catch(boost::system::system_error &ec)
    {
        std::println(stderr, "Error occured accepting request! Message: {}", ec.what());
    }

    return 0;
}
