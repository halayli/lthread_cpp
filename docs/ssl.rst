lthread_cpp::net::SSLSocket
===========================

.. cpp:namespace:: lthread_cpp

.. cpp:class:: SSLSocket

Turns a :cpp:class:`Socket()` to :cpp:class:`SSLSocket()`

Member Functions
----------------

.. cpp:function:: static void Init(const std::string& server_pem_filename, const std::string& server_key_filename, const std::string& ca_cert_filename, const std::string& ca_path)

    Initializes SSL settings. Must be called once before any SSLSocket connection is received or established.

    :throws: :cpp:class:`SSLException()` if it failed to initialize SSL context with any of the values provided.

.. cpp:function:: SSLSocket(Socket && s)

    Initializes/wraps a new SSLSocket from an existing established :cpp:class:`Socket`.

.. cpp:function:: SSLSocket()

    Initializes an new SSLSocket ready to connect to peer using :cpp:class:`SSLSocket::Connect()`.

.. cpp:function:: void Accept(int timeout_ms=5000)

    Initiates an SSL Accept with the assumption that the TCP connection was accept(2)-ed and not established via connect(2).

    :throws: :cpp:class:`SSLException()` if ssl accept failed.

.. cpp:function:: void Connect(const std::string& host_or_ip, short port, int timeout_ms)

    Establishes a TCP connection to host/ip:port and initiates an SSL Connect afterwards.

    :throws: :cpp:class:`SSLException()` if SSL connect failed
    :throws: :cpp:class:`SocketException()` on socket failure.

.. cpp:function:: void RequirePeerVerification()

    Will set SSL peer verification flag on.

.. cpp:function:: std::string GetCertCommonName()

    Returns common name in certificate received.

.. cpp:function:: size_t Send(const char* buf, int timeout_ms=5000)

    Sends a C style string over SSL socket.

    :param const char* buf: NULL-terminated buffer.

    :throws: :cpp:class:`SSLException()` on socket failure.

.. cpp:function:: size_t Send(const char* buf, size_t length, int timeout_ms=5000)

    Sends length bytes of buf over SSL socket.

    :param const char\* buf: Ptr to buffer containing data to send.
    :param size_t length: Number of bytes to send from `buf`.
    :param timeout_ms(optional, default=5000): Milliseconds to wait before timing out.

    :throws: :cpp:class:`SSLException()` on socket failure.

.. cpp:function:: size_t Recv(char* buf, size_t length, int timeout_ms=5000)

    Receives up to length bytes and place them into buf.

    :param char* buf: Buffer to read data into.
    :param size_t length: Buffer size to fill.
    :param timeout_ms(optional, default=5000): Milliseconds to wait before timing out.

    :throws: :cpp:class:`SSLException()` on socket failure.

.. cpp:function:: void Close()

    Cleanly closes SSL socket and its underlying TCP connection.

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
