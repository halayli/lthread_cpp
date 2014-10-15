#include "lthread_cpp/proxy.h"


using namespace lthread_cpp::net;
SocketProxy::~SocketProxy()
{
  client_->Close();
  server_->Close();
}

void SocketProxy::Run(const bool* shutdown)
{
  shutdown_ = shutdown;
  client_thread_ = Lthread{&SocketProxy::RecvFromClient, this};
  server_thread_ = Lthread{&SocketProxy::RecvFromServer, this};

  client_thread_.Join();
  server_thread_.Join();
  shutdown_ = nullptr;
}

void SocketProxy::RecvFromClient()
{
  SendRecv(client_, server_, on_client_recv_, on_server_send_);
}

void SocketProxy::RecvFromServer()
{
  SendRecv(server_, client_, on_server_recv_, on_client_send_);
}

void SocketProxy::SendRecv(Socket* client,
                           Socket* server,
                           const std::vector<OnDataCallback>& recv_callbacks,
                           const std::vector<OnDataCallback>& send_callbacks)
{
  while(!shutdown_ && keep_running_)
  {
    char buf[1024];
    try {
      size_t sz = client->Recv(buf, 1024, 5000);
      // run callbacks interested in what was recved
      for (auto f : recv_callbacks)
        f(buf, sz);
      server->Send(buf, sz);
      // run callbacks interested in what was sent
      for (auto f : send_callbacks)
        f(buf, sz);
    } catch (SocketTimeout& e) {
      continue;
    } catch (SocketException& e) {
      break;
    }
  }
  keep_running_ = false;
}

void SocketProxy::OnClientSend(OnDataCallback callback)
{
  on_client_send_.push_back(callback);
}

void SocketProxy::OnClientRecv(OnDataCallback callback)
{
  on_client_recv_.push_back(callback);
}

void SocketProxy::OnServerSend(OnDataCallback callback)
{
  on_server_send_.push_back(callback);
}

void SocketProxy::OnServerRecv(OnDataCallback callback)
{
  on_server_recv_.push_back(callback);
}
