TcpConnect
==========

.. code-block:: cpp

    #include <lthread_cpp/socket.h>

.. cpp:function:: Socket TcpConnect(const std::string& host_or_ip, short port, int timeout_ms=1000)

    Connects to a remote host.

    :param const std\:\:string& host_or_ip: Host to connect to.
    :param short port: Tcp port.
    :param int timeout_ms(1000): Milliseconds to wait connecting.

    :return: :cpp:class:`Socket` for the new connection

    :throws: :cpp:class:`SocketException` on socket failure.


Examples
--------

.. code-block:: cpp

    #include <lthread_cpp/lthread.h>
    #include <lthread_cpp/socket.h>

    using namespace lthread_cpp;
    using namespace lthread_cpp::net;

    void Run()
    {
     Socket s = TcpConnect("127.0.0.1", 80);
     s.Send("GET / HTTP/1.1\r\n\r\n");

     char response[1024];
     s.Recv(response, 1024, 0);
     s.Send("Cool!");
     // s closes as it goes out of scope
    }

    int main()
    {
      Lthread t{&Run};
      t.Detach();
      Lthread::Run();
    }

.. code-block:: shell

    cc -std=c++11 test.cc -o test -llthread_cpp -llthread -lpthread -lstdc++ && ./test
