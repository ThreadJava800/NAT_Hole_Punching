#pragma once

#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

namespace arg_parser = boost::program_options;

struct ClientAddr
{
public:
    explicit ClientAddr() :
        local  (),
        global ()
    {}

    explicit ClientAddr(std::pair<in_addr_t, in_port_t> local_, std::pair<in_addr_t, in_port_t> global_) :
        local  (local_),
        global (global_)
    {}

public:
    std::pair<in_addr_t, in_port_t> local, global;
};

enum class RequestType
{
    DEFAULT,
    LOCAL_ADDR,             // only local addr
    LOCAL_GLOBAL_ADDR,      // local addr + global addr
    GET_PEER                // request peer ip
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

    explicit LocalAddrRequest(std::pair<in_addr_t, in_port_t> local_) :
        BaseRequest (RequestType::LOCAL_ADDR),
        local       (local_)
    {}

public:
    std::pair<in_addr_t, in_port_t> local;
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
