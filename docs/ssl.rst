lthread_cpp::net::SSLSocket
---------------------------

.. cpp:class:: SSLSocket

Turns a :cpp:class:`Socket` to :cpp:class:`SSLSocket`

Member Functions
================

.. cpp:function:: SSLSocket(Socket && s)

Initializes/wraps a new SSLSocket from an existing established :cpp:class:`Socket`.

.. cpp:function:: SSLSocket()

Initializes an new SSLSocket ready to connect to peer using :cpp:class:`SSLSocket::Connect()`.

.. cpp:function:: static void Init(const std::string& server_pem_filename, const std::string& server_key_filename, const std::string& ca_cert_filename, const std::string& ca_path)

   Initializes SSL settings. Must be called once before any SSLSocket connection is received or established. Throws `SSLException` if it failed to initialize SSL Ctx with any of the value
s provided.

.. cpp:function:: Accept(int timeout_ms=5000)

   Initiates an SSL Accept with the assumption that the TCP connection was accept(2)-ed and not established via connect(2). Throws `SSLException` if ssl accept failed.

.. cpp:function:: Connect(const std::string& host_or_ip, short port, int timeout_ms)

   Establishes a TCP connection to host/ip:port and initiates an SSL Connect afterwards. Throws `SSLException` if SSL connect failed or `SocketExcpetion` if TCP connect failed.

.. cpp:function:: RequirePeerVerification()

   Will set SSL peer verification flag on.

.. cpp:function:: std::string GetCertCommonName()

   Returns common name in certificate received.

.. cpp:function:: size_t Send(const char* buf, int timeout_ms=5000)

   Sends a C style string over SSL socket. buf must be null-terminated. Throws `SSLException` if it failed.

.. cpp:function:: size_t Send(const char* buf, size_t length, int timeout_ms=5000)
   
   Sends length bytes of buf over SSL socket. Throws `SSLException` if send failed.

.. cpp:function:: size_t Recv(char* buf, size_t length, int timeout_ms=1000)

   Receives up to length bytes and place them into buf. Throws `SSLException` if it failed.

.. cpp:function:: void Close()

   Cleanly closes SSL socket and it's underlying TCP connection.

.. note:: Lthread objects are not copyable

.. code-block:: cpp

	void Proxy::HandleConnection(Socket& tcp_conn)
	{

	  SSLSocket client;
	  std::string common_name;

	  // do an SSL handshake over the new tcp connection we just received and grab
	  // the required customer certificate after it has been verified against
	  // CA certificates provided to SSLSocket::Init
	  try {
	    SSLSocket ssl_socket(std::move(tcp_conn));
	    ssl_socket.RequirePeerVerification();
	    ssl_socket.Accept();
	    common_name = ssl_socket.GetCertCommonName();
	    client = std::move(ssl_socket);
	  } catch (SocketException& e) {
	    LOG(ERROR) << "SSL handshake failed from "
			<< tcp_conn.Desc() << ". (" << e.what() << ")";
	    return;
	  }

	  // At this point, client can send/recv bytes over established SSL
          client.Send("hello world!\n");
	}
::
