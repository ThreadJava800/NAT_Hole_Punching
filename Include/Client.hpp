#pragma once

#include "Common.hpp"

#define MAX_MESSAGE_CNT 50
#define MAX_MESSAGE_LEN 0x100
#define UDP_DEFAULT_TIMEOUT_SEC 600

enum class ClientType
{
    DEFAULT,
    A,
    B
};

class Client
{
public:
    explicit Client(
        const ClientType type,
        const char *const client_addr_str, 
        const uint16_t client_port,
        const char *const server_addr_str, 
        const uint16_t server_port
    );
    ~Client();

    void run() const;

private:
    void setUdpTimeout(const __time_t sec = 0, const __time_t usec = 0) const;
    ClientAddr recvPeerIpPair() const;
    void connectToRendezvous() const;
    void communicateRendezvousA() const;
    ClientAddr communicateRendezvousB() const;
    bool isConnectionEstablished(const SockAddrWrapper ip_addr) const;
    SockAddrWrapper findValidIp(const ClientAddr peer) const;
    void respondToPeerIpSearch() const;
    bool sendMessageToPeer(const SockAddrWrapper peer) const;
    void recvMessageFromPeer() const;
    void createChatA(const SockAddrWrapper peer) const;
    void createChatB(const SockAddrWrapper peer) const;

private:
    int sockfd;
    SockAddrWrapper rendezvous, this_addr;
    ClientType type;
};

arg_parser::options_description createParser();
Client parseCmd(const int argc, const char **argv, const arg_parser::options_description desc);
