#include "Rendezvous.hpp"

RendezvousServer::RendezvousServer(const char *const ip_addr_str, const uint16_t port) :
    sockfd (0)
{
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    in_addr_t ip_addr;
    assert(inet_pton(AF_INET, ip_addr_str, &ip_addr) > 0);

    sockaddr_in sock_addr = {
        .sin_family = AF_INET,
        .sin_port = port,
        .sin_addr = ip_addr
    };
    
    assert(bind(sockfd, (sockaddr*)&sock_addr, sizeof(sock_addr)) == 0);
}

RendezvousServer::~RendezvousServer()
{
    assert(close(sockfd) == 0);
}

void RendezvousServer::run() const
{
    const SockAddrWrapper ipA_global = waitClient();
    const SockAddrWrapper ipB_global = waitClient();

    const SockAddrWrapper ipA_local = recvIpFromA(ipA_global);

    const ClientAddr clientA(ipA_local, ipA_global);
    const SockAddrWrapper ipB_local = recvIpFromB(clientA, ipB_global);

    const ClientAddr clientB(ipB_local, ipB_global);
    sendAIpOfB(ipA_global, clientB);
}

SockAddrWrapper RendezvousServer::getLocalIpFrom(const SockAddrWrapper from_global_ip) const
{
    SockAddrWrapper ip_local;
    while (true)
    {
        LocalAddrRequest local_addr_req;
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        recvfrom(
            sockfd,
            &local_addr_req,
            sizeof(local_addr_req),
            MSG_WAITALL,
            (sockaddr*)&client_addr,
            &client_addr_len
        );

        if (local_addr_req.type != RequestType::LOCAL_ADDR || from_global_ip != client_addr)
        {
            continue;
        }

        ip_local = client_addr;
        break;
    }

    return ip_local;
}

SockAddrWrapper RendezvousServer::waitClient() const
{
    SockAddrWrapper client_global_ip;
    while (true)
    {
        BaseRequest open_session_req;
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        recvfrom(
            sockfd,
            &open_session_req,
            sizeof(open_session_req),
            MSG_WAITALL,
            (sockaddr*)&client_addr,
            &client_addr_len
        );

        if (open_session_req.type != RequestType::SYN)
        {
            continue;
        }

        client_global_ip = client_addr;
        break;
    }

    return client_global_ip;
}

SockAddrWrapper RendezvousServer::recvIpFromA(const SockAddrWrapper ipA_global) const
{
    const BaseRequest start_send_local_req(RequestType::GIVE_IP);
    sendto(
        sockfd,
        &start_send_local_req,
        sizeof(start_send_local_req),
        0,
        (const sockaddr*)&ipA_global.addr,
        sizeof(ipA_global.addr)
    );

    return getLocalIpFrom(ipA_global);
}

SockAddrWrapper RendezvousServer::recvIpFromB(const ClientAddr clientA, const SockAddrWrapper ipB_global) const
{
    const LocalGlobalAddrRequest send_ipA_req(clientA);
    sendto(
        sockfd,
        &send_ipA_req,
        sizeof(send_ipA_req),
        0,
        (const sockaddr*)&ipB_global.addr,
        sizeof(ipB_global.addr)
    );

    return getLocalIpFrom(ipB_global);
}

void RendezvousServer::sendAIpOfB(const SockAddrWrapper ipA_global, const ClientAddr clientB) const
{
    const LocalGlobalAddrRequest send_B_addr_req(clientB);
    sendto(
        sockfd,
        &send_B_addr_req,
        sizeof(send_B_addr_req),
        0,
        (const sockaddr*)&ipA_global.addr,
        sizeof(ipA_global.addr)
    );
}

arg_parser::options_description createParser()
{
    arg_parser::options_description desc("Allowed options:");
    desc.add_options()
        ("help", "print help message")
        ("addr", arg_parser::value<std::string>()->required(), "rendezvous server address")
        ("port", arg_parser::value<uint16_t>()->required(), "rendezvous server port");

    return desc;
}

RendezvousServer parseCmd(const int argc, const char **argv, const arg_parser::options_description desc)
{
    arg_parser::variables_map var_map;
    arg_parser::store(arg_parser::parse_command_line(argc, argv, desc), var_map);

    if (var_map.count("help"))
    {
        std::cout << desc << '\n';
        exit(1);
    }

    arg_parser::notify(var_map);

    RendezvousServer server(var_map["addr"].as<std::string>().c_str(), var_map["port"].as<uint16_t>());
    return server;
}

int main(const int argc, const char **argv) {
    const arg_parser::options_description desc = createParser();
    const RendezvousServer server = parseCmd(argc, argv, desc);

    server.run();

    return 0;
}