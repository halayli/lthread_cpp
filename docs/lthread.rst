lthread_cpp::Lthread
====================

.. cpp:class:: Lthread

Launches a single lthread in the background.

`#include <lthread_cpp/lthread.h>`

Member Functions
----------------

.. cpp:function:: Lthread()

.. cpp:function:: Lthread(&Method, params,...)

.. cpp:function:: Lthread(&Class::Method, params,...)

Creates new `lthread` object and associates it with an lthread. The constructor copies/moves all arguments args... to an lthread-accessible storage.

.. cpp:function:: Join()

   Joins on a single lthread and blocks until the lthread returns.

.. cpp:function:: Detach()

   Marks the lthread launched as detachable to be freed upon return. This is a direct binding to `lthread_detach`

.. cpp:function:: Id()

   Returns an lthread_t* ptr pointing to the original lthread created by `lthread_create`

.. cpp:function:: Joinable()

   Returns `true` if the lthread can be joined on(i.e., launched)


.. note:: Lthread objects are not copyable

Example
-------

.. code-block:: cpp

	void MyMethod(std::vector<int> my_vec) {}

	void Run()
	{
	  std::vector<int> v{1,2,3,4};
	  Lthread t1{&MyMethod, v};
	  t1.Detach();
	}

        Lthread{&Run};
        Lthread::Run();
::
