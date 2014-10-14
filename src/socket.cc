#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <lthread.h>

#include <sstream>

#include "lthread_cpp/socket.h"
#include "lthread_cpp/ssl.h"

using namespace lthread_cpp::net;

Socket lthread_cpp::net::TcpConnect(const std::string& host_or_ip,
                                    short port,
                                    int timeout_ms)
{
  std::stringstream str_port;
  str_port << port;
  int fd = lthread_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!fd)
    throw SocketException("Failed to create socket for new connection: %s",
                          strerror(errno));

  struct addrinfo hints, *base = NULL;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE;

  int ret;
  if ((ret = getaddrinfo(host_or_ip.c_str(),
                         str_port.str().c_str(),
                         &hints,
                         &base)))
    throw SocketException("Failed to resolve host: %s to establish connection",
                          gai_strerror(ret));

  if (lthread_connect(fd, base->ai_addr, base->ai_addrlen, timeout_ms) < 0) {
    close(fd);
    freeaddrinfo(base);
    throw SocketException("Failed to connect to: %s:%d", host_or_ip.c_str(), port);
  }

  freeaddrinfo(base);

  return Socket(fd);
}

Socket::Socket(int fd) : fd_(fd), addrlen_(0) {}

Socket::Socket(int fd, struct sockaddr* addr, socklen_t* addrlen) :
  fd_(fd), addr_(*addr), addrlen_(*addrlen) {}

size_t Socket::Send(const char* buf, int timeout_ms)
{
  return Send(buf, strlen(buf), timeout_ms);
}

std::string Socket::Desc() const
{
  auto addr_info = (const struct sockaddr_in*)(&addr_);
  char* tmp = inet_ntoa(addr_info->sin_addr);
  unsigned short port = htons(addr_info->sin_port);

  std::stringstream s;
  s << tmp << ":" << port;
  return s.str();
}

size_t Socket::Send(const char* buf, size_t length, int timeout_ms)
{
  ssize_t r = lthread_write(fd_, buf, length);
  if (r == -1)
    throw SocketException("Error in send operation: %s", strerror(errno));

  return (size_t)r;
}

size_t Socket::Writev(struct iovec* iov, int iovcnt)
{
  ssize_t ret = lthread_writev(fd_, iov, iovcnt);
  if (ret == -1)
    throw SocketException("Error in writev operation: %s", strerror(errno));

  return (size_t)ret;
}

size_t Socket::RecvExact(char* buf, size_t length, int timeout_ms)
{
  ssize_t r = lthread_recv_exact(fd_, buf, length, 0, timeout_ms);
  if (r == -1)
    throw SocketException("Error while receiving data: %s", strerror(errno));
  if (r == -2)
    throw SocketTimeout("Timeout occured in recvexact");

  return (size_t)r;

}

size_t Socket::Recv(char* buf, size_t length, int timeout_ms)
{
  ssize_t r = lthread_recv(fd_, buf, length, 0, timeout_ms);
  if (r == -1)
    throw SocketException("Error while receiving data: %s", strerror(errno));
  if (r == -2)
    throw SocketTimeout("Timeout occured in recv");
  if (r == 0)
    throw SocketException("Peer closed: %s", strerror(errno));

  return (size_t)r;
}

void Socket::WaitRead(int timeout_ms) const
{
  int ret = lthread_wait_read(fd_, timeout_ms);
  if (ret == -2)
    throw SocketTimeout();
  if (ret == -1)
    throw SocketException("Peer closed: %s", strerror(errno));
}

void Socket::WaitWrite(int timeout_ms) const
{
  int ret = lthread_wait_write(fd_, timeout_ms);
  if (ret == -2)
    throw SocketTimeout();
  if (ret == -1)
    throw SocketException("Peer closed: %s", strerror(errno));
}

void Socket::Close()
{
  if (fd_ != -1) {
    lthread_close(fd_);
    fd_ = -1;
  }
}

Socket::~Socket()
{
  Close();
}

