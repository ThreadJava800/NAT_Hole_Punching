#pragma once

#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>

namespace arg_parser = boost::program_options;

struct SockAddrWrapper
{
public:
    explicit SockAddrWrapper() :
        addr({})
    {}

    SockAddrWrapper(const sockaddr_in addr_) :
        addr(addr_)
    {}

    friend bool operator==(const SockAddrWrapper &lhs, const SockAddrWrapper &rhs)
    {
        return lhs.addr.sin_addr.s_addr == rhs.addr.sin_addr.s_addr &&
               lhs.addr.sin_port        == rhs.addr.sin_port;
    }

    friend bool operator!=(const SockAddrWrapper &lhs, const sockaddr_in &rhs)
    {
        return lhs.addr.sin_addr.s_addr != rhs.sin_addr.s_addr ||
               lhs.addr.sin_port        != rhs.sin_port;
    }

public:
    sockaddr_in addr;
};

struct ClientAddr
{
public:
    explicit ClientAddr() :
        local  (),
        global ()
    {}

    explicit ClientAddr(const SockAddrWrapper local_, const SockAddrWrapper global_) :
        local  (local_),
        global (global_)
    {}

public:
    SockAddrWrapper local, global;
};

enum class RequestType
{
    DEFAULT,
    LOCAL_ADDR,             // only local addr
    LOCAL_GLOBAL_ADDR,      // local addr + global addr
    GET_PEER,               // request peer ip
    SYN,                    // create session
    GIVE_IP                 // event is sent to A client signalizing he can start sending local addr
};

struct BaseRequest
{
public:
    explicit BaseRequest() :
        type (RequestType::DEFAULT)
    {}

    explicit BaseRequest(const RequestType type_) :
        type (type_)
    {}

public:
    RequestType type;
};

struct LocalAddrRequest : public BaseRequest
{
public:
    explicit LocalAddrRequest() :
        BaseRequest (),
        local       ()
    {}

    explicit LocalAddrRequest(sockaddr_in local_) :
        BaseRequest (RequestType::LOCAL_ADDR),
        local       (local_)
    {}

public:
    sockaddr_in local;
};

struct LocalGlobalAddrRequest : public BaseRequest
{
public:
    explicit LocalGlobalAddrRequest() :
        BaseRequest (),
        client_addr ()
    {}

    explicit LocalGlobalAddrRequest(const ClientAddr client_addr_) :
        BaseRequest (RequestType::LOCAL_GLOBAL_ADDR),
        client_addr (client_addr_)
    {}

public:
    ClientAddr client_addr;
};
