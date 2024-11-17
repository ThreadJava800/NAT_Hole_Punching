#pragma once

#include "Common.hpp"

class RendezvousServer
{
public:
    explicit RendezvousServer(const char *const ip_addr_str, const uint16_t port);
    ~RendezvousServer();

    void run();

private:
    SockAddrWrapper getLocalIpFrom(const SockAddrWrapper from_global_ip);

    SockAddrWrapper waitClient();
    SockAddrWrapper recvIpFromA(const SockAddrWrapper ipA_global);
    SockAddrWrapper recvIpFromB(const ClientAddr clientA, const SockAddrWrapper ipB_global);
    void sendAIpOfB(const SockAddrWrapper ipA_global, const ClientAddr clientB);

private:
    int sockfd;
    sockaddr_in sock_addr;
};

arg_parser::options_description createParser();
RendezvousServer parseCmd(const int argc, const char **argv, const arg_parser::options_description desc);
