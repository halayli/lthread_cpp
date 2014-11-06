lthread_cpp::net
================

.. cpp:namespace:: lthread_cpp::net

Functions & Exceptions
----------------------

TcpConnect
^^^^^^^^^^
.. cpp:function:: Socket TcpConnect(const std::string& host_or_ip, short port, int timeout_ms=1000)

    Connects to a remote host.


    :param const std::string& host_or_ip: Host to connect to.
    :param short port: Tcp port.
    :param int timeout_ms(1000): Milliseconds to wait connecting.

    :return: :cpp:class:`Socket()` for the new connection

    :throws: :cpp:class:`SocketException()` on socket failure.

