lthread_cpp::net::TcpListener
=============================

.. cpp:namespace:: lthread_cpp

.. cpp:class:: TcpListener

Member Functions
----------------

.. cpp:function:: TcpListener(const std::string& ip, short port)

   Initializes `TcpListener` instance with the ip and port specified.

.. cpp:function:: void Listen()

   Binds IP and port to socket.

   :throws: :cpp:class:`SocketException()` if it fails to bind or listen.

.. cpp:function:: Socket Accept()

    Blocks until a new connection is accepted.

    :return: A new :cpp:class:`Socket` object for the new connection.

    :throws: :cpp:class:`SocketException()` if `lthread_accept()` failed.


.. cpp:function:: void Close()

   Closes listening port.


.. note:: TcpListener objects are not copyable

Example
-------

.. code-block:: cpp

	void Run()
	{
	  TcpListener listener("127.0.0.0", 8090);
	  listener.Listen();
	  while (1) {
	    Socket s = listener.Accept();
	    Lthread t1 {&HandleConnection, std::move(s)};
	    t1.Detach();
	  }
	}
::
