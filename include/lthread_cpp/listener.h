#ifndef LTHREAD_CPP_LISTENER_H
#define LTHREAD_CPP_LISTENER_H

#include "lthread_cpp/socket.h"

namespace lthread_cpp {
namespace net{

class TcpListener {
 public:
  TcpListener(const std::string& ip, short port);
  ~TcpListener() { Close(); }
  Socket Accept(int timeout_ms=1000);
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
