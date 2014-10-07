#ifndef LTHREAD_CPP_SSL_H
#define LTHREAD_CPP_SSL_H

#include <openssl/rsa.h>       /* SSLeay stuff */
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace lthread_cpp {
namespace net {

class Socket;
class SocketException;
class SSLSocket : public Socket {
 public:
  SSLSocket(Socket&& s);
  size_t Send(const char* buf, int timeout_ms=5000);
  size_t Send(const char* buf, size_t length, int timeout_ms=5000);
  size_t Recv(char* buf, size_t length, int timeout_ms=1000);
  void Close();

  static void Init(const std::string& pem_file, const std::string& key_file);

  SSLSocket() : ssl_(nullptr) {}
  inline SSLSocket(SSLSocket&& old_s)
  {
    sock_ = std::move(old_s.sock_);
    ssl_ = old_s.ssl_;

    old_s.ssl_ = nullptr;
  }

  int fd() const { return sock_.fd(); }
  inline SSLSocket& operator=(SSLSocket&& rr_c)
  {
    // close current connection if we have one, new connection assigned will
    // take over. Same thing for above.
    if (sock_.fd() != -1 && sock_.fd() != rr_c.fd())
      Close();
    sock_ = std::move(rr_c.sock_);
    ssl_ = rr_c.ssl_;
    rr_c.ssl_ = nullptr;

    return *this;
  }

  void Accept(int timeout_ms=5000);

  Socket   sock_;
 private:
  SSLSocket(const SSLSocket&);
  SSLSocket& operator=(const SSLSocket&);
  SSL*     ssl_;
};

class SSLException : public SocketException {
 public:
  virtual ~SSLException() throw () {}
  explicit SSLException(const char* message);
  explicit SSLException(): SocketException("") {}
};

}
}

#endif
