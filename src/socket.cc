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
    throw SocketException("Failed to connect to: %s:%d", host_or_ip.c_str(), port);
  }

  if (base)
    freeaddrinfo(base);

  return Socket(fd);
}

TcpListener::TcpListener(const std::string& ip, short port)
  : fd_(-1), ip_(ip), port_(port)
{
}

Socket TcpListener::Accept(int timeout_ms)
{
  struct sockaddr peer_addr = {0};
  socklen_t addrlen = sizeof(peer_addr);

  int cli_fd = lthread_accept(fd_, &peer_addr, &addrlen);

  if (!cli_fd)
    throw SocketException("Accept failed: %s", strerror(errno));
  return Socket(cli_fd, &peer_addr, &addrlen);
}

void TcpListener::Listen()
{
  int ret = 0;
  int opt = 1;
  struct sockaddr_in sin;

  fd_ = lthread_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!fd_)
    throw SocketException("Failed to create listening socket: %s", strerror(errno));

  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(int)) == -1)
    perror("Failed to set SO_REUSEADDR on listening socket");

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = PF_INET;
  sin.sin_addr.s_addr = inet_addr(ip_.c_str());
  if (sin.sin_addr.s_addr == INADDR_NONE)
    sin.sin_addr.s_addr = INADDR_LOOPBACK;
  sin.sin_port = htons(port_);

  ret = ::bind(fd_, (struct sockaddr *)&sin, sizeof(sin));
  if (ret == -1) {
    close(fd_);
    throw SocketException("Failed to bind on port: %d (%s)", port_, strerror(errno));
  }

  ret = listen(fd_, 1024);
  if (ret == -1) {
    close(fd_);
    throw SocketException("Failed to listen: %s", strerror(errno));
  }

  fcntl(fd_, F_SETFD, fcntl(fd_, F_GETFD) | FD_CLOEXEC);
}

void TcpListener::Close()
{
  if (fd_) {
    lthread_close(fd_);
    fd_ = -1;
  }
}

Socket::Socket(int fd) : fd_(fd), addrlen_(0) {}

Socket::Socket(int fd, struct sockaddr* addr, socklen_t* addrlen) :
  fd_(fd), addr_(*addr), addrlen_(*addrlen) {}

size_t Socket::Send(const char* buf)
{
  return Send(buf, strlen(buf));
}

size_t Socket::Send(const char* buf, size_t length)
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

SocketProxy::~SocketProxy()
{
  client_.Close();
  server_.Close();
}

void SocketProxy::Run()
{
  client_thread_ = Lthread{&SocketProxy::RecvFromClient, this};
  server_thread_ = Lthread{&SocketProxy::RecvFromServer, this};

  client_thread_.Join();
  server_thread_.Join();
}

void SocketProxy::RecvFromClient()
{
  SendRecv(client_, server_);
}

void SocketProxy::RecvFromServer()
{
  SendRecv(server_, client_);
}

void SocketProxy::SendRecv(Socket& client, Socket& server)
{
  while(keep_running_)
  {
    char buf[1024];
    try {
      size_t sz = client.Recv(buf, 1024, 5000);
      server.Send(buf, sz);
    } catch (SocketTimeout& e) {
      continue;
    } catch (SocketException& e) {
      break;
    }
  }
  keep_running_ = false;
}


