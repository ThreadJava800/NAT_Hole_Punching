#include "Rendezvous.hpp"

RendezvousServer::RendezvousServer(const char *const ip_addr_str, const uint16_t port) :
    sockfd    (0),
    sock_addr ({}),
    clients   ()
{
    in_addr_t ip_addr;
    assert(inet_pton(AF_INET, ip_addr_str, &ip_addr) > 0);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sock_addr = {
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

[[noreturn]] void RendezvousServer::run()
{
    // server is able to work only with 2 clients.
    acquireClient();
    acquireClient();

    assert(clients.size() >= 2);

    // send ips to clients and wait for confirmation
    runDaemon();
}

void RendezvousServer::acquireClient()
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

    clients.emplace_back(local_addr_req.local, std::pair<in_addr_t, in_port_t>(client_addr.sin_addr.s_addr, client_addr.sin_port));
}

[[noreturn]] void RendezvousServer::runDaemon()
{
    while (true)
    {
        BaseRequest get_peer_request;
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        recvfrom(
            sockfd,
            &get_peer_request,
            sizeof(get_peer_request),
            MSG_WAITALL,
            (sockaddr*)&client_addr,
            &client_addr_len
        );

        const auto found_pos = std::find_if(
                                    clients.begin(), 
                                    clients.end(), 
                                    [&client_addr](const ClientAddr& val) {
                                        return val.local == std::pair<in_addr_t, in_port_t>(client_addr.sin_addr.s_addr, client_addr.sin_port);
                                    }
                                );
        if (found_pos != clients.end())
        {
            for (auto it = clients.begin(); it != clients.end(); ++it)
            {
                if (it != found_pos)
                {
                    LocalGlobalAddrRequest local_global_addr_req(*it);
                    sendto(
                        sockfd,
                        &local_global_addr_req,
                        sizeof(local_global_addr_req),
                        0,
                        (sockaddr*)&client_addr,
                        client_addr_len
                    );
                }
            }
        }
    }
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
    RendezvousServer server = parseCmd(argc, argv, desc);

    server.run();

    return 0;
}