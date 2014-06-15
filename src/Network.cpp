/*
 * Network.cpp
 *
 *  Created on: Jun 14, 2014
 *      Author: Pimenta
 */

// this
#include "Network.hpp"

// standard
#include <cstdio>

// lib
#include <SDL_net.h>
#ifdef _WIN32
  #include <ws2tcpip.h>
#else
  #include <netinet/in.h>
  #include <unistd.h>
#endif

using namespace std;

namespace network {

// =============================================================================
// struct Address;
// =============================================================================

Address::Address() : ip(0), port(0) {
  
}

Address::Address(uint32_t ip, uint16_t port) : ip(ip), port(port) {
  
}

Address::Address(const string& ip, uint16_t port) {
  IPaddress tmp;
  SDLNet_ResolveHost(&tmp, ip.c_str(), port);
  this->ip = tmp.host;
  this->port = tmp.port;
}

string Address::toString() const {
  char tmp[30];
  if (port != 0) {
    sprintf(tmp, "%d.%d.%d.%d:%d",
      int(((uint8_t*)&ip)[0]),
      int(((uint8_t*)&ip)[1]),
      int(((uint8_t*)&ip)[2]),
      int(((uint8_t*)&ip)[3]),
      ntohs(port)
    );
  }
  else {
    sprintf(tmp, "%d.%d.%d.%d",
      int(((uint8_t*)&ip)[0]),
      int(((uint8_t*)&ip)[1]),
      int(((uint8_t*)&ip)[2]),
      int(((uint8_t*)&ip)[3])
    );
  }
  return string(tmp);
}

Address Address::local() {
  IPaddress addr;
#ifdef _WIN32
  SDLNet_ResolveHost(&addr, nullptr, 0);
  SDLNet_ResolveHost(&addr, SDLNet_ResolveIP(&addr), 0);
#else
  IPaddress addrs[100];
  int total = SDLNet_GetLocalAddresses(addrs, 100);
  int i;
  for (i = 0; i < total && addrs[i].host == 0x0100007F; ++i);
  SDLNet_ResolveHost(&addr, SDLNet_ResolveIP(&addrs[i]), 0);
#endif
  return Address(addr.host, 0);
}

uint32_t Address::ntohl(uint32_t ip) {
  return SDLNet_Read32(&ip);
}

uint32_t Address::htonl(uint32_t ip) {
  return SDLNet_Read32(&ip);
}

uint16_t Address::ntohs(uint16_t port) {
  return SDLNet_Read16(&port);
}

uint16_t Address::htons(uint16_t port) {
  return SDLNet_Read16(&port);
}

// =============================================================================
// class UDPSocket;
// =============================================================================

UDPSocket::UDPSocket(uint16_t port) {
#ifdef _WIN32
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)TRUE, sizeof(BOOL));
  SOCKADDR_IN addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  bind(sd, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
#else
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(int));
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sd, (sockaddr*)&addr, sizeof(sockaddr_in));
#endif
}

UDPSocket::UDPSocket(const Address& address, uint32_t group) {
#ifdef _WIN32
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  int tmp = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(ULONG));
  ip_mreq mreq;
  mreq.imr_multiaddr.S_un.S_addr = group;
  mreq.imr_interface.S_un.S_addr = address.ip;
  setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq));
  SOCKADDR_IN addr;
  addr.sin_family = AF_INET;
  addr.sin_port = address.port;
  addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  bind(sd, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
#else
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(int));
  ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = group;
  mreq.imr_interface.s_addr = address.ip;
  setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq));
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = address.port;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sd, (sockaddr*)&addr, sizeof(sockaddr_in));
#endif
}

UDPSocket::~UDPSocket() {
#ifdef _WIN32
  closesocket(sd);
#else
  close(sd);
#endif
}

void UDPSocket::send(const Address& address, const vector<char>& data) {
#ifdef _WIN32
  SOCKADDR_IN servaddr;
  memset((void*)&servaddr, 0, sizeof(SOCKADDR_IN));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = address.ip;
  servaddr.sin_port = address.port;
  sendto(sd, &data[0], data.size(), 0, (SOCKADDR*)&servaddr, sizeof(SOCKADDR_IN));
#else
  sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(sockaddr_ir));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = address.ip;
  servaddr.sin_port = address.port;
  sendto(sd, &data[0], data.size(), 0, &servaddr, sizeof(sockaddr_in));
#endif
}

vector<char> UDPSocket::recv(Address& address) {
  vector<char> data;
#ifdef _WIN32
  fd_set fds;
  timeval timeout;
  FD_ZERO(&fds);
  FD_SET(sd, &fds);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  if (select(0, &fds, nullptr, nullptr, &timeout) > 0) {
    char buf[0x1000];
    SOCKADDR_IN addr;
    int addrsize = sizeof(SOCKADDR_IN);
    int total = recvfrom(sd, buf, 0x1000, 0, (SOCKADDR*)&addr, &addrsize);
    data.assign(buf, buf + total);
    address = Address(addr.sin_addr.S_un.S_addr, addr.sin_port);
  }
#else
  fd_set fds;
  timeval timeout;
  FD_ZERO(&fds);
  FD_SET(sd, &fds);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  if (select(sd + 1, &fds, nullptr, nullptr, &timeout) > 0) {
    char buf[0x1000];
    sockaddr_in addr;
    socklen_t addrsize = sizeof(sockaddr_in);
    int total = recvfrom(sd, buf, 0x1000, 0, (sockaddr*)&addr, &addrsize);
    data.assign(buf, buf + total);
    address = Address(addr.sin_addr.s_addr, addr.sin_port);
  }
#endif
  return data;
}

} // namespace network