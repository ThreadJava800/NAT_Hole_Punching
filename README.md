# NAT Hole Punching

This simple program shows an example of NAT Hole Punching between 2 peers over UDP. Repository contains implementations of client side and server (Rendezvous) side. In the beginning of communication clients send their local ips to Rendezvous. Then Rendezvous server, sends pairs <local_peer_addr, global_peer_addr> back to peers. After clients received their peer address pair, they try to find out by which address the peer would respond. After one ip is confirmed, connection could be established.

## Usage
Compiling (you must have [boost_program_options](https://packages.debian.org/ru/sid/libboost-program-options-dev) installed):
```bash
make server  # build Rendezvous
make client  # build client 
```

Then start Rendezvous server and clients (in different terminal windows) **(order is important!)**.

Start Rendezvous server:
```bash
./Server.o --addr [rendezvous ip addr] --port [rendezvous port]
```

Start first client (type A):
```bash
./Client.o --type A --c-addr [local_machine ip addr] --c-port [local_machine port] --s-addr [rendezvous ip addr] --s-port [rendezvous port]
```

Start second client (type B):
```bash
./Client.o --type B --c-addr [local_machine ip addr] --c-port [local_machine port] --s-addr [rendezvous ip addr] --s-port [rendezvous port]
```

## Links
It is an educational task for [computer networks course at MIPT](https://cn-mipt.rerand0m.ru/).
