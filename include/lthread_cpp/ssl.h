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
  size_t Recv(char* buf, size_t length, int timeout_ms=5000);
  void Close();

  static void Init(const std::string& server_pem_filename,
                   const std::string& server_key_filename,
                   const std::string& ca_cert_filename,
                   const std::string& ca_path);

  SSLSocket() : ssl_(nullptr), cert_(nullptr), peer_verification_(false) {}

  inline SSLSocket(SSLSocket&& old_s)
  {
    sock_ = std::move(old_s.sock_);
    ssl_ = old_s.ssl_;
    cert_ = old_s.cert_;
    peer_verification_ = old_s.peer_verification_;

    old_s.peer_verification_ = false;
    old_s.cert_ = nullptr;
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
    cert_ = rr_c.cert_;
    peer_verification_ = rr_c.peer_verification_;

    rr_c.peer_verification_ = false;
    rr_c.cert_ = nullptr;
    rr_c.ssl_ = nullptr;

    return *this;
  }

  void Accept(int timeout_ms=5000);
  void Connect(const std::string& host_or_ip, short port, int timeout_ms);

  void RequirePeerVerification();
  std::string GetCertCommonName();

 private:
  std::string GetCertEntryFromSubject(const std::string& entry_name);
  SSLSocket(const SSLSocket&);
  SSLSocket& operator=(const SSLSocket&);

  Socket sock_;
  SSL* ssl_;
  X509* cert_;
  bool peer_verification_;
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
