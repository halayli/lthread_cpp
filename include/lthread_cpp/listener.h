#ifndef LTHREAD_CPP_LISTENER_H
#define LTHREAD_CPP_LISTENER_H

#include <unistd.h>

#include "lthread_cpp/socket.h"

namespace lthread_cpp {
namespace net{

class TcpListener {
 public:
  TcpListener() : fd_(-1) {}
  TcpListener(const std::string& ip, short port);
  ~TcpListener()
  {
    Close();
    if (fd_ != -1)
      close(fd_);
  }
  Socket Accept();
  void Listen();
  void Close();

 private:
  int fd_;
  std::string ip_;
  short port_;
};


}
}
#endif
