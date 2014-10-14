#ifndef LTHREAD_CPP_PROXY_H
#define LTHREAD_CPP_PROXY_H

#include "lthread_cpp/socket.h"

namespace lthread_cpp {
namespace net {

class SocketProxy {
 public:
  SocketProxy(Socket* client, Socket* server)
  {
    client_ = client;
    server_ = server;
    keep_running_ = true;
  }
  ~SocketProxy();

  // starting point after a tcp connection is accepted
  void Run(const bool* shutdown);

 private:
  void RecvFromClient();
  void RecvFromServer();
  void SendRecv(Socket* from, Socket* to);

  Lthread server_thread_;
  Lthread client_thread_;

  Socket* client_;
  Socket* server_;

  bool keep_running_;
  const bool* shutdown_;
};

}
}

#endif
