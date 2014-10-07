#include "lthread_cpp/listener.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

using namespace lthread_cpp::net;
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
