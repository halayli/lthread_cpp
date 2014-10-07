#include <stdio.h>

#include "lthread_cpp/socket.h"
#include "lthread_cpp/ssl.h"


static SSL_CTX* g_ctx = nullptr;

using namespace lthread_cpp::net;

void SSLSocket::Init(const std::string& pem_f, const std::string& key_f)
{
  SSL_load_error_strings();
  SSLeay_add_ssl_algorithms();
  const SSL_METHOD* meth = SSLv23_server_method();
  g_ctx = SSL_CTX_new (meth);
  if (!g_ctx)
    throw SSLException("Failed to initialize SSL context")

  if (SSL_CTX_use_certificate_file(g_ctx, pem_f.c_str(), SSL_FILETYPE_PEM) <= 0)
    throw SSLException("Failed to use pem file");

  if (SSL_CTX_use_PrivateKey_file(g_ctx, key_f.c_str(), SSL_FILETYPE_PEM) <= 0)
    throw SSLException("Failed to use private key file");

  if (!SSL_CTX_check_private_key(g_ctx))
    throw SSLException("Private key does not match the certificate public key");
}

void SSLSocket::Accept(int timeout_ms)
{
  while (1)
  {
    int ret = SSL_accept(ssl_);
    if (ret == 1)
      return;
    if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_READ)
      sock_.WaitRead(timeout_ms);
    else if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_WRITE)
      sock_.WaitWrite(timeout_ms);
    else
      throw SSLException("Accept failed");
  }
}

SSLSocket::SSLSocket(Socket&& s)
{
  ssl_ = SSL_new(g_ctx);
  if (!ssl_)
    throw SSLException("Failed to create ssl object");

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
      throw SSLException("SSL_write failed");
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
    int ret = SSL_read(ssl_, buf, length);
    if (ret > 0)
      return ret;
    if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_READ)
      sock_.WaitRead(timeout_ms);
    else if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_WRITE)
      sock_.WaitWrite(timeout_ms);
    else
      throw SSLException("SSL_read failed");
  }
}

void SSLSocket::Close()
{
  if (ssl_)
    SSL_free(ssl_);
}
