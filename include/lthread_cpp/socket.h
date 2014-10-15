#ifndef LTHREAD_CPP_SOCKET_H
#define LTHREAD_CPP_SOCKET_H

#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>

#include <exception>
#include <stdexcept>
#include <string>
#include <cstdarg>

#include "lthread_cpp/lthread.h"

namespace lthread_cpp {
namespace net {

class TcpListener;
class Socket;

Socket TcpConnect(const std::string& host_or_ip,
                  short port,
                  int timeout_ms=1000);


class Socket {
 public:
  virtual size_t Send(const char* buf, int timeout_ms=1000);
  virtual size_t Send(const char* buf, size_t length, int timeout_ms=1000);
  virtual size_t Recv(char* buf, size_t length, int timeout_ms=1000);
  virtual void Close();
  size_t Writev(struct iovec* v, int iovcnt);
  size_t RecvExact(char* buf, size_t length, int timeout_ms=1000);
  void WaitWrite(int timeout_ms=1000) const;
  void WaitRead(int timeout_ms=1000) const;

  ~Socket();
  Socket() : fd_(-1) {}

  inline Socket(Socket&& new_c) : fd_(-1)
  {
    fd_ = new_c.fd_;
    addr_ = new_c.addr_;
    addrlen_ = new_c.addrlen_;
    new_c.fd_ = -1;
  }

  inline Socket& operator=(Socket&& rr_c)
  {
    // close current connection if we have one, new connection assigned will
    // take over. Same thing for above.
    if (fd_ != -1 && fd_ != rr_c.fd_)
      Close();
    fd_ = rr_c.fd_;
    rr_c.fd_ = -1;
    addr_ = rr_c.addr_;
    addrlen_ = rr_c.addrlen_;

    return *this;
  }

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  bool IsConnected() const { return fd_ > -1; }
  int fd() const { return fd_; }
  std::string Ip() const;
  std::string Desc() const;

 private:
  Socket(int fd);
  Socket(int fd, struct sockaddr* addr, socklen_t* addrlen);
  int fd_;
  struct sockaddr addr_;
  socklen_t addrlen_;

  friend Socket TcpConnect(const std::string& host_or_ip,
                           short port,
                           int timeout_ms);
  friend class TcpListener;
};

class SocketException : public std::exception {
 public:
  explicit SocketException(const std::string& message): msg_(message) {}
  explicit SocketException(const char* fmt...)
  {
    char buf[1024];
    va_list arglist;
    va_start( arglist, fmt);
    vsnprintf(buf, 1024, fmt, arglist);
    va_end(arglist);
    msg_ = buf;
  }

  virtual const char* what() const noexcept
  {
    return msg_.c_str();
  }
  virtual ~SocketException() throw () {}

 protected:
  std::string msg_;
};

class SocketTimeout : public SocketException {
 public:
  virtual ~SocketTimeout() throw () {}
  explicit SocketTimeout(const char* message): SocketException(message) {}
  explicit SocketTimeout(): SocketException("") {}
};

}
}

#endif
