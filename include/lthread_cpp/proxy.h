#ifndef LTHREAD_CPP_PROXY_H
#define LTHREAD_CPP_PROXY_H

#include "lthread_cpp/socket.h"

#include <functional>
#include <vector>

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
  void Run(std::function<bool()>shutdown);

  using OnDataCallback = std::function<void(const char*, size_t)>;

  void OnClientSend(OnDataCallback);
  void OnClientRecv(OnDataCallback);

  void OnServerSend(OnDataCallback);
  void OnServerRecv(OnDataCallback);

 private:

  void RecvFromClient();
  void RecvFromServer();
  void SendRecv(Socket* from,
                Socket* to,
                const std::vector<OnDataCallback>& recv_callbacks,
                const std::vector<OnDataCallback>& send_callbacks);

  Lthread server_thread_;
  Lthread client_thread_;

  Socket* client_;
  Socket* server_;

  std::vector<OnDataCallback> on_client_recv_;
  std::vector<OnDataCallback> on_client_send_;
  std::vector<OnDataCallback> on_server_recv_;
  std::vector<OnDataCallback> on_server_send_;

  bool keep_running_;
  std::function<bool()> shutdown_;
};

}
}

#endif
