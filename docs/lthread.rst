Lthread
=======

.. code-block:: cpp

    #include <lthread_cpp/lthread.h>
    using namespace lthread;

.. cpp:class:: Lthread

Launches a single lthread in the background.

Member Functions
----------------

.. cpp:function:: Lthread()

.. cpp:function:: Lthread(&Method, params,...)

.. cpp:function:: Lthread(&Class::Method, params,...)

Creates new `lthread` object and associates it with an lthread. The constructor copies/moves all arguments args... to an lthread-accessible storage.

.. cpp:function:: Join(uint64_t timeout_ms)

   Joins on a single lthread and blocks until the lthread returns.

   :param timeout_ms(optional, default=0): Milliseconds to wait joining on another lthread.
   :throws: :cpp:class:`LthreadTimeout()` on timeout.

.. cpp:function:: void Detach()

   Marks the lthread launched as detachable to be freed upon return. This is a direct binding to `lthread_detach`

.. cpp:function:: lthread_t* Id()

   :return: an lthread_t* ptr pointing to the original lthread created by `lthread_create`

.. cpp:function:: bool Joinable()

   :return: `true` if the lthread can be joined on(i.e., launched)


.. note:: Lthread objects are movable but not copyable.

Exceptions
----------

LthreadTimeout
^^^^^^^^^^^^^^
.. cpp:class:: LthreadTimeout

    Empty class raised on Lthread `join` timeout call.

Example
-------

.. code-block:: cpp

    #include <lthread_cpp/lthread.h>

    using namespace lthread;

    void MyMethod(std::vector<int> my_vec) {}

    void Run()
    {
  	     std::vector<int> v{1,2,3,4};
  	     Lthread t1{&MyMethod, v};
  	     t1.Detach();
    }

    int main()
    {
        Lthread{&Run};
        Lthread::Run();
    }
