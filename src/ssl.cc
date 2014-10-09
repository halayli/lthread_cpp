#include <stdio.h>

#include "lthread_cpp/socket.h"
#include "lthread_cpp/ssl.h"


static SSL_CTX* g_ctx = nullptr;

using namespace lthread_cpp::net;

SSLException::SSLException(const char* message)
  : SocketException("")
{
 long ret = ERR_get_error();
 char* tmp = ERR_error_string(ret, nullptr);

 msg_ = message;
 msg_ += ": ";
 msg_ += tmp;
}

int pass_through_verify(int preverify_ok, X509_STORE_CTX * ctx)
{
  return preverify_ok;
}

void SSLSocket::Init(const std::string& server_pem_filename,
                     const std::string& server_key_filename,
                     const std::string& ca_cert_filename,
                     const std::string& ca_path)
{
  SSL_load_error_strings();
  SSLeay_add_ssl_algorithms();
  const SSL_METHOD* meth = SSLv23_server_method();
  g_ctx = SSL_CTX_new (meth);
  if (!g_ctx)
    throw SSLException("Failed to initialize SSL context");

  if (SSL_CTX_use_certificate_file(g_ctx,
        server_pem_filename.c_str(), SSL_FILETYPE_PEM) <= 0)
    throw SSLException("Failed to use pem file");

  if (SSL_CTX_use_PrivateKey_file(g_ctx,
        server_key_filename.c_str(), SSL_FILETYPE_PEM) <= 0)
    throw SSLException("Failed to use private key file");

  if (!SSL_CTX_check_private_key(g_ctx))
    throw SSLException("Private key does not match the certificate public key");

  if (!SSL_CTX_load_verify_locations(g_ctx, ca_cert_filename.c_str(),
          ca_path.length() ? ca_path.c_str() : nullptr))
    throw SSLException("Failed to load CA certificate");
}

void SSLSocket::RequirePeerVerification()
{
  SSL_set_verify(ssl_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                 pass_through_verify);
  peer_verification_ = true;
}

void SSLSocket::Connect(const std::string& host, short port, int timeout_ms)
{

  sock_ = TcpConnect(host, port, timeout_ms);
  int ret = SSL_connect(ssl_);
  while (1) {
    if (ret == 1)
      break;
    if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_READ)
      sock_.WaitRead(timeout_ms);
    else if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_WRITE)
      sock_.WaitWrite(timeout_ms);
    else
      throw SSLException("SSL_connect failed");
  }
}

void SSLSocket::Accept(int timeout_ms)
{
  while (1)
  {
    int ret = SSL_accept(ssl_);
    if (ret == 1)
      break;
    if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_READ)
      sock_.WaitRead(timeout_ms);
    else if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_WRITE)
      sock_.WaitWrite(timeout_ms);
    else
      throw SSLException("SSL_accept failed");
  }

  if (peer_verification_)
    cert_ = SSL_get_peer_certificate(ssl_);
}

std::string SSLSocket::GetCertCommonName()
{
  if (!cert_)
    return "";

  return GetCertEntryFromSubject("CN");
}

std::string SSLSocket::GetCertEntryFromSubject(const std::string& entry_name)
{
  X509_NAME *subject_name = X509_get_subject_name(cert_);
  if (subject_name) {
    int nid = OBJ_txt2nid(entry_name.c_str());
    int index = X509_NAME_get_index_by_NID(subject_name, nid, -1);
    X509_NAME_ENTRY *entry = X509_NAME_get_entry(subject_name, index);
    if (entry) {
      ASN1_STRING *entry_asn1 = X509_NAME_ENTRY_get_data(entry);
      if (entry_asn1)
        return std::string((char*)ASN1_STRING_data(entry_asn1));
    }
  }

  return "";

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

    if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_WRITE)
      sock_.WaitWrite(timeout_ms);
    else if (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_READ)
      sock_.WaitRead(timeout_ms);
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
  if (ssl_) {
    SSL_shutdown(ssl_);
    SSL_free(ssl_);
    if (cert_)
      X509_free(cert_);
    cert_ = nullptr;
    ssl_ = nullptr;
  }
}
