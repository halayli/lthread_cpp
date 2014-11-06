Introduction
============

`lthread_cpp <https://github.com/halayli/lthread_cpp>`_ is a C++11 binding to C's `lthread <https://github.com/halayli/lthread>`_ version.

Currently, lthread is supported on FreeBSD, OS X, and Linux (x86 & 64bit arch).

Installation & Linking
----------------------

.. code-block:: Shell

    https://github.com/halayli/lthread_cpp.git
    cd lthread_cpp
    cmake .
    sudo make install

Linking
^^^^^^^

Pass `-llthread_cpp` to your compiler to use lthread_cpp in your program.

