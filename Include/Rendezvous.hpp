#pragma once

#include <vector>

#include "Common.hpp"

class RendezvousServer
{
public:
    explicit RendezvousServer(const char *const ip_addr_str, const uint16_t port);
    ~RendezvousServer();

    [[noreturn]] void run();

private:
    void acquireClient();
    [[noreturn]] void runDaemon();

private:
    int sockfd;
    sockaddr_in sock_addr;
    std::vector<ClientAddr> clients;
};

arg_parser::options_description createParser();
RendezvousServer parseCmd(const int argc, const char **argv, const arg_parser::options_description desc);
