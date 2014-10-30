lthread_cpp::net::Socket
------------------------

.. cpp:class:: Socket

A wrapper around lthread's socket calls. `Socket` instance is returned by [TcpConnect](socket.md#tcp-connect) and [TcpListener::Accept](listener.md) and cannot be constructed on its own.

Member Functions
================

These functions reflect their lthread equivalent and are to be called from inside lthreads.

.. cpp:function:: size_t Send(const char* buf)

   Sends a C-style string over a socket. This is a convenience function to avoid calling strlen() on `buf`.

.. cpp:function:: size_t Send(const char* buf, size_t length)

   Sends `length` bytes over a socket.

.. cpp:function:: size_t Recv(char* buf, size_t length, int timeout_ms=1000)

   Receives upto `length` bytes over a socket. Throws a `SocketTimeout` exception if a timeout occured. `timeout_ms=0` waits indefinitely.

.. cpp:function:: void Close()

   Closes the network socket.

.. cpp:function:: Writev(struct iovec* v, int iovcnt)

   Sends an iovec over a socket

.. cpp:function:: RecvExact(char* buf, size_t length, int timeout_ms=1000)

   Receives `length` bytes or throws `SocketTimeout` exception if it timed out before receiving the full number of bytes.

.. cpp:function:: WaitWrite(int timeout_ms=1000) const

   Waits until the socket is writable. throws `SocketTimeout` exception if timeout occured.

.. cpp:function:: WaitRead(int timeout_ms=1000) const

   Waits until the socket is readable. throws `SocketTimeout` exception if timeout occured.

.. cpp:function:: bool IsConnected() const

   Returns true if socket is connected.

.. cpp:function:: int fd() const

   Returns the fd wrapped in the `Socket` instance

.. cpp:function:: std::string Ip() const

   Returns the remote IP Address as a string

.. cpp:function:: std::string Desc() const

   Returns remote_ip:ephemeral_port as a string

.. cpp:function:: Socket& operator=(Socket&& rr_c)

   Moves a socket from one instance to another.

.. note:: Socket objects are not copyable

.. code-block:: cpp
	void Run()
	{
	  Socket s = TcpConnect("127.0.0.1", 80);
	  s.Send("GET / HTTP/1.1\r\n\r\n");

	  char response[1024];
	  s.Recv(response, 1024);
	  // s closes as it goes out of scope
	}
::
