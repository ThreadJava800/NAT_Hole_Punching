#include "Client.hpp"

Client::Client(
        const ClientType type_,
        const char *const client_addr_str, 
        const uint16_t client_port,
        const char *const server_addr_str, 
        const uint16_t server_port
    ) :
    sockfd      (0),
    rendezvous  (),
    this_addr   (),
    type        (type_)
{
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    in_addr_t client_ip_addr;
    assert(inet_pton(AF_INET, client_addr_str, &client_ip_addr) > 0);

    this_addr.addr = {
        .sin_family = AF_INET,
        .sin_port = client_port,
        .sin_addr = client_ip_addr
    };
    assert(bind(sockfd, (sockaddr*)&this_addr, sizeof(this_addr)) == 0);

    in_addr_t rendezvous_ip_addr;
    assert(inet_pton(AF_INET, server_addr_str, &rendezvous_ip_addr) > 0);

    rendezvous.addr.sin_addr.s_addr = rendezvous_ip_addr;
    rendezvous.addr.sin_port        = server_port;
}

Client::~Client()
{
    assert(close(sockfd) == 0);
}

void Client::setUdpTimeout(const __time_t sec, const __time_t usec) const
{
    const timeval timeout = {
        .tv_sec  = sec,
        .tv_usec = usec
    };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

ClientAddr Client::recvPeerIpPair() const
{
    LocalGlobalAddrRequest get_ip_pair_req;
    while (true)
    {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        recvfrom(
            sockfd,
            &get_ip_pair_req,
            sizeof(get_ip_pair_req),
            MSG_WAITALL,
            (sockaddr*)&client_addr,
            &client_addr_len
        );

        if (get_ip_pair_req.type == RequestType::LOCAL_GLOBAL_ADDR && !(rendezvous != client_addr))
        {
            break;
        }
    }
    return get_ip_pair_req.client_addr;
}

void Client::run() const
{
    connectToRendezvous();

    ClientAddr peer_addr_pair;
    SockAddrWrapper peer_real_ip;
    switch (type)
    {
    case ClientType::A:
        communicateRendezvousA();
        peer_addr_pair = recvPeerIpPair();

        peer_real_ip = findValidIp(peer_addr_pair);
        respondToPeerIpSearch();

        createChatA(peer_real_ip);
        break;
    case ClientType::B:
        peer_addr_pair = communicateRendezvousB();

        respondToPeerIpSearch();
        peer_real_ip = findValidIp(peer_addr_pair);

        createChatB(peer_real_ip);
        break;
    case ClientType::DEFAULT:
    default:
        assert(false && "Invalid client type in runner");
        break;
    }
}

void Client::connectToRendezvous() const
{
    const BaseRequest start_send_local_req(RequestType::SYN);
    sendto(
        sockfd,
        &start_send_local_req,
        sizeof(start_send_local_req),
        0,
        (const sockaddr*)&rendezvous.addr,
        sizeof(rendezvous.addr)
    );
}

void Client::communicateRendezvousA() const
{
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

        if (open_session_req.type == RequestType::GIVE_IP && !(rendezvous != client_addr))
        {
            break;
        }
    }

    const LocalAddrRequest send_local_addr_req(this_addr);
    sendto(
        sockfd,
        &send_local_addr_req,
        sizeof(send_local_addr_req),
        0,
        (const sockaddr*)&rendezvous.addr,
        sizeof(rendezvous.addr)
    );
}

ClientAddr Client::communicateRendezvousB() const
{
    const ClientAddr a_ip_pair = recvPeerIpPair();

    const LocalAddrRequest send_local_addr_req(this_addr);
    sendto(
        sockfd,
        &send_local_addr_req,
        sizeof(send_local_addr_req),
        0,
        (const sockaddr*)&rendezvous.addr,
        sizeof(rendezvous.addr)
    );

    return a_ip_pair;
}

bool Client::isConnectionEstablished(const SockAddrWrapper ip_addr) const
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

    return !(ip_addr != client_addr) && open_session_req.type == RequestType::SYN;
}

SockAddrWrapper Client::findValidIp(const ClientAddr peer) const
{
    setUdpTimeout(1);

    // try global
    const BaseRequest start_send_local_req(RequestType::SYN);
    sendto(
        sockfd,
        &start_send_local_req,
        sizeof(start_send_local_req),
        0,
        (const sockaddr*)&peer.global.addr,
        sizeof(peer.global.addr)
    );
    if (isConnectionEstablished(peer.global.addr))
    {
        // reset timeout
        setUdpTimeout(UDP_DEFAULT_TIMEOUT_SEC);
        return peer.global;
    }

    sendto(
        sockfd,
        &start_send_local_req,
        sizeof(start_send_local_req),
        0,
        (const sockaddr*)&peer.local.addr,
        sizeof(peer.local.addr)
    );
    assert(isConnectionEstablished(peer.local.addr));

    // reset timeout
    setUdpTimeout(UDP_DEFAULT_TIMEOUT_SEC);
    return peer.local;
}

void Client::respondToPeerIpSearch() const
{
    sockaddr_in client_addr;
    while (true)
    {
        BaseRequest open_session_req;
        socklen_t client_addr_len = sizeof(client_addr);

        recvfrom(
            sockfd,
            &open_session_req,
            sizeof(open_session_req),
            MSG_WAITALL,
            (sockaddr*)&client_addr,
            &client_addr_len
        );

        if (open_session_req.type == RequestType::SYN)
        {
            break;
        }
    }

    const BaseRequest start_session_req(RequestType::SYN);
    sendto(
        sockfd,
        &start_session_req,
        sizeof(start_session_req),
        0,
        (const sockaddr*)&client_addr,
        sizeof(client_addr)
    );
}

bool Client::sendMessageToPeer(const SockAddrWrapper peer) const
{
    std::string message;
    std::cout << "Enter message: ";
    std::getline(std::cin, message);

    if (message.size() > MAX_MESSAGE_LEN)
    {
        std::cout << "\nToo long message!\n";
        return false;
    }

    const char *msg_cstr = message.c_str();
    sendto(
        sockfd,
        msg_cstr,
        message.size(),
        0,
        (const sockaddr*)&peer.addr,
        sizeof(peer.addr)
    );
    return true;
}

void Client::recvMessageFromPeer() const
{
    char peer_answer[MAX_MESSAGE_LEN] = {'\0'};
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    recvfrom(
        sockfd,
        &peer_answer,
        MAX_MESSAGE_LEN,
        MSG_WAITALL,
        (sockaddr*)&client_addr,
        &client_addr_len
    );

    std::cout << "Peer answers: " << peer_answer << '\n';
}

void Client::createChatA(const SockAddrWrapper peer) const
{
    int msg_cnt = 0;
    for (; msg_cnt < MAX_MESSAGE_CNT; ++msg_cnt)
    {
        while (!sendMessageToPeer(peer)) {}
        recvMessageFromPeer();
    }
}

void Client::createChatB(const SockAddrWrapper peer) const
{
    int msg_cnt = 0;
    for (; msg_cnt < MAX_MESSAGE_CNT; ++msg_cnt)
    {
        recvMessageFromPeer();
        while (!sendMessageToPeer(peer)) {}
    }
}

arg_parser::options_description createParser()
{
    arg_parser::options_description desc("Allowed options:");
    desc.add_options()
        ("help", "print help message")
        ("type", arg_parser::value<char>()->required(), "client type (can be A or B)")
        ("c-addr", arg_parser::value<std::string>()->required(), "client ip address")
        ("c-port", arg_parser::value<uint16_t>()->required(), "client port")
        ("s-addr", arg_parser::value<std::string>()->required(), "rendezvous server ip address")
        ("s-port", arg_parser::value<uint16_t>()->required(), "rendezvous server port");

    return desc;
}

Client parseCmd(const int argc, const char **argv, const arg_parser::options_description desc)
{
    arg_parser::variables_map var_map;
    arg_parser::store(arg_parser::parse_command_line(argc, argv, desc), var_map);

    if (var_map.count("help"))
    {
        std::cout << desc << '\n';
        exit(1);
    }

    arg_parser::notify(var_map);

    const char client_type_char = var_map["type"].as<char>();
    ClientType client_type = ClientType::DEFAULT;
    switch (client_type_char)
    {
    case 'A':
        client_type = ClientType::A;
        break;
    case 'B':
        client_type = ClientType::B;
        break;
    default:
        assert(false && "Please provide type 'A' or type'B'");
        break;
    }

    Client client(
        client_type, 
        var_map["c-addr"].as<std::string>().c_str(), 
        var_map["c-port"].as<uint16_t>(),
        var_map["s-addr"].as<std::string>().c_str(),
        var_map["s-port"].as<uint16_t>()
    );
    return client;
}

int main(const int argc, const char **argv) {
    const arg_parser::options_description desc = createParser();
    const Client client = parseCmd(argc, argv, desc);

    client.run();

    return 0;
}