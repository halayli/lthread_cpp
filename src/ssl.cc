#include <stdio.h>

#include "lthread_cpp/socket.h"
#include "lthread_cpp/ssl.h"


static SSL_CTX* g_ctx = nullptr;

using namespace lthread_cpp::net;

void SSLSocket::Init(const std::string& pem_file, const std::string& key_file)
{
  SSL_load_error_strings();
  SSLeay_add_ssl_algorithms();
  const SSL_METHOD* meth = SSLv23_server_method();
  g_ctx = SSL_CTX_new (meth);
  if (!g_ctx) {
    fprintf(stderr, "Failed to initialize SSL Context\n");
    exit(2);
  }
}

SSLSocket::SSLSocket(Socket&& s)
{
  ssl_ = SSL_new(g_ctx);
  if (!ssl_)
    throw SocketException("Failed to create ssl object");

  SSL_set_fd (ssl_, s.fd());
  sock_ = std::move(s);
}

size_t SSLSocket::Send(const char* buf, size_t length, int timeout_ms)
{
  while (1)
  {
    int ret = SSL_write(ssl_, buf, length);
    if (ret > 0)
      return ret;
    else if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_WRITE)
      sock_.WaitWrite(timeout_ms);
    else
      throw SocketException("SSL_write failed");
  }
}

int SSLSocket::Accept(int timeout_ms) const
{
  while (1)
  {
    printf("timeout_ms:%d\n", timeout_ms);
    int ret = SSL_accept(ssl_);
    if (ret > 0)
      return ret;
    else if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_READ)
      sock_.WaitRead(timeout_ms);
    else
      throw SocketException("SSL_accept failed");
  }
}

size_t SSLSocket::Send(const char* buf, int timeout_ms)
{
  return Send(buf, strlen(buf), timeout_ms);
}

size_t SSLSocket::Recv(char* buf, size_t length, int timeout_ms)
{
  while (1)
  {
    int ret = SSL_read(ssl_, buf, strlen(buf));
    if (ret > 0)
      return ret;
    else if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_READ)
      sock_.WaitRead(timeout_ms);
    else
      throw SocketException("SSL_read failed");
  }
}

void SSLSocket::Close()
{
  if (ssl_)
    SSL_free (ssl_);
}
