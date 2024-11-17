#pragma once

#include "Common.hpp"

class Client
{
public:
    explicit Client(
        const char *const client_addr_str, 
        const uint16_t client_port,
        const char *const server_addr_str, 
        const uint16_t server_port
    );
    ~Client();

    [[noreturn]] void run();

private:
    ClientAddr acquirePeer();
    [[noreturn]] void runDaemon(const ClientAddr peer);

private:
    sockaddr_in client, rendezvous;
};

arg_parser::options_description createParser();
Client parseCmd(const int argc, const char **argv, const arg_parser::options_description desc);
