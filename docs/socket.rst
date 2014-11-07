Socket
======

.. code-block:: cpp

    #include <lthread_cpp/socket.h>
    using namespace lthread::net;

.. cpp:class:: Socket

A wrapper around lthread's socket calls. `Socket` instance is returned by :cpp:func:`TcpConnect` and :cpp:class:`TcpListener` and cannot be constructed on its own.

Member Functions
----------------

These functions reflect their lthread equivalent and must be called inside lthreads.

.. cpp:function:: size_t Send(const char* buf)

    Sends a C-style string over a socket.

    :param const char* buf: NULL-terminated buffer.

    :return: Number of bytes sent.
    :throws: :cpp:class:`SocketException` on socket failure.

.. cpp:function:: size_t Send(const char* buf, size_t length)

    Sends `length` bytes over a socket.

    :param const char\* buf: Ptr to buffer containing data to send.
    :param size_t length: Number of bytes to send from `buf`.

    :return: Number of bytes sent.
    :throws: :cpp:class:`SocketException` on socket failure.

.. cpp:function:: size_t Recv(char* buf, size_t length, int timeout_ms=1000)

    Receives up to `length` bytes over a socket.

    :param char* buf: Buffer to read data into.
    :param size_t length: Buffer size to fill.
    :param timeout_ms(optional, default=1000): Milliseconds to wait before timing out.

    :throws: :cpp:class:`SocketException` on socket failure.
    :throws: :cpp:class:`SocketTimeout` if a timeout occured. `timeout_ms=0` waits indefinitely.

.. cpp:function:: void Close()

    Closes the network socket.

.. cpp:function:: size_t Writev(struct iovec* v, int iovcnt)

    Sends an iovec over a socket.

    :param struct iovec* v: iovec pointing to one or more ptr/size entries.
    :param int iovcnt: Number of entries in the iovec.

    :throws: :cpp:class:`SocketException` on socket failure.

.. cpp:function:: size_t RecvExact(char* buf, size_t length, int timeout_ms=1000)

    Receives exactly `length` bytes into buf.

    :param char* buf: Buffer to read data into.
    :param size_t length: Buffer size to fill.
    :param timeout_ms(optional, default=1000): Milliseconds to wait before timing out.

    :throws: :cpp:class:`SocketException` on socket failure.
    :throws: :cpp:class:`SocketTimeout` if it timed out before receiving the full number of bytes.

.. cpp:function:: void WaitWrite(int timeout_ms=1000) const

    Waits until the socket is writable.

    :param timeout_ms(optional, default=1000): Milliseconds to wait before timing out.

    :throws: :cpp:class:`SocketException` on socket failure.
    :throws: :cpp:class:`SocketTimeout` if timeout occured.

.. cpp:function:: void WaitRead(int timeout_ms=1000) const

    Waits until the socket is readable.

    :param timeout_ms(optional, default=1000): Milliseconds to wait before timing out.

    :throws: :cpp:class:`SocketException` on socket failure.
    :throws: :cpp:class:`SocketTimeout` if timeout occured.

.. cpp:function:: bool IsConnected() const

    Returns true if socket is connected.

.. cpp:function:: int fd() const

    Returns the fd wrapped in the :cpp:class:`Socket` instance.

.. cpp:function:: std::string Ip() const

    Returns the remote IP Address as a string.

    :return: string containing IP address.

.. cpp:function:: std::string Desc() const

    Returns remote_ip:ephemeral_port as a string

.. cpp:function:: Socket& operator=(Socket&& rr_c)

    Moves a socket from one instance to another.


.. note:: Socket objects are movable but not copyable.

Exceptions
----------

SocketTimeout
^^^^^^^^^^^^^
.. cpp:class:: SocketTimeout

    Empty class raised on socket timeout operations.

SocketException
^^^^^^^^^^^^^^^
.. cpp:class:: SocketException

    Inherits std::exception(), raised on socket errors.

Examples
--------

.. code-block:: cpp

    using namespace lthread;
    using namespace lthread::net;

    void Run()
    {
     Socket s = TcpConnect("127.0.0.1", 80);
     s.Send("GET / HTTP/1.1\r\n\r\n");

     char response[1024];
     s.Recv(response, 1024);
     // s closes as it goes out of scope
    }


