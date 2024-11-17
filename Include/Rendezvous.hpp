#pragma once

#include "Common.hpp"

class RendezvousServer
{
public:
    explicit RendezvousServer(const char *const ip_addr_str, const uint16_t port);
    ~RendezvousServer();

    void run() const;

private:
    SockAddrWrapper getLocalIpFrom(const SockAddrWrapper from_global_ip) const;

    SockAddrWrapper waitClient() const;
    SockAddrWrapper recvIpFromA(const SockAddrWrapper ipA_global) const;
    SockAddrWrapper recvIpFromB(const ClientAddr clientA, const SockAddrWrapper ipB_global) const;
    void sendAIpOfB(const SockAddrWrapper ipA_global, const ClientAddr clientB) const;

private:
    int sockfd;
};

arg_parser::options_description createParser();
RendezvousServer parseCmd(const int argc, const char **argv, const arg_parser::options_description desc);
