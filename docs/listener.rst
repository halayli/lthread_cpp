TcpListener
===========

.. code-block:: cpp

    #include <lthread_cpp/listener.h>

    using namespace lthread_cpp::net;

.. cpp:class:: TcpListener

Member Functions
----------------

.. cpp:function:: TcpListener(const std::string& ip, short port)

   Initializes `TcpListener` instance with the ip and port specified.

.. cpp:function:: void Listen()

   Binds IP and port to socket.

   :throws: :cpp:class:`SocketException` if it fails to bind or listen.

.. cpp:function:: Socket Accept()

    Blocks until a new connection is accepted.

    :return: A new :cpp:class:`Socket` object for the new connection.

    :throws: :cpp:class:`SocketException` if `lthread_accept()` failed.


.. cpp:function:: void Close()

   Closes listening port.

Examples
--------
.. code-block:: cpp

    #include <lthread_cpp/lthread.h>
    #include <lthread_cpp/socket.h>
    #include <lthread_cpp/listener.h>

    using namespace lthread_cpp;
    using namespace lthread_cpp::net;

    void HandleConnection(Socket& s)
    {
      s.Send("Hi");
    }

    void Run()
    {
      TcpListener listener("127.0.0.1", 8090);
      listener.Listen();
      while (1) {
        Socket s = listener.Accept();
        Lthread t1 {&HandleConnection, std::move(s)};
        t1.Detach();
      }
    }

    int main()
    {
        Lthread t{&Run};
        t.Detach();
        Lthread::Run();
    }

.. code-block:: shell

    cc -std=c++11 test.cc -o test -llthread_cpp -llthread -lpthread -lstdc++ && ./test
