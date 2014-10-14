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
  SendRecv(client_, server_);
}

void SocketProxy::RecvFromServer()
{
  SendRecv(server_, client_);
}

void SocketProxy::SendRecv(Socket* client, Socket* server)
{
  while(!shutdown_ && keep_running_)
  {
    char buf[1024];
    try {
      size_t sz = client->Recv(buf, 1024, 5000);
      server->Send(buf, sz);
    } catch (SocketTimeout& e) {
      continue;
    } catch (SocketException& e) {
      break;
    }
  }
  keep_running_ = false;
}
