Installing Xapian
=================

Introduction
------------

This document is intended to give a quick guide to how to install
Xapian. You can find more detailed instructions in the ``INSTALL`` file
which is in top level directory of each source tree.

Xapian can be built on UNIX systems (including MacOS X), and also
Microsoft Windows systems using GCC with mingw or cygwin, or MSVC.

Packaged binary versions
------------------------

Pre-built Xapian packages are available for a number of platforms,
including most of the popular Linux distributions and BSD variants, and
also Cygwin and MSVC. If you are using such a platform, you'll probably
find it easiest to use pre-built packages - it saves having to compile
by hand and you'll generally get updates automatically.

There are some links on our `download
page <http://xapian.org/download>`_ but it's likely that Xapian packages
are available for platforms we aren't aware of. Feel free to let us know
and we'll add a link.

In some cases, the version packaged may be rather old, in which case you
can either request the packager to update, or build from source. If you
find we're linking to a package which isn't being updated, please let us
know so we can remove the link.

Installing from Source
----------------------

Download
~~~~~~~~

The first step is to obtain a copy of the software from the `Xapian
download page <http://xapian.org/download>`_.

Unpacking
~~~~~~~~~

Use the usual tools to unpack the archives. For example, on a Linux
system::

     tar xf xapian-core-<versionnumber>.tar.xz
     tar xf xapian-omega-<versionnumber>.tar.xz
     tar xf xapian-bindings-<versionnumber>.tar.xz

If tar on your system doesn't support xz decompression, you can instead use::

     xz -dc xapian-core-<versionnumber>.tar.xz|tar xf -
     xz -dc xapian-omega-<versionnumber>.tar.xz|tar xf -
     xz -dc xapian-bindings-<versionnumber>.tar.xz|tar xf -

These commands should unpack the archives into separate subdirectories
(``xapian-core-<versionnumber>``, ``xapian-omega-<versionnumber>`` and
``xapian-bindings-<versionnumber>``).

Configuring and building the Xapian library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For full details of the different options available when configuring and
building, read the file "INSTALL" in the top level directory of your
newly unpacked source tree. But in many cases, the following quick
summary is all you need to know.

Building for MSVC is currently handled using a separately maintained set
of makefiles - you can find a link to these on the `Xapian download
page <http://xapian.org/download>`_.

Each directory contains a ``configure`` script which checks various
features of your system. Assuming this runs successfully, you can then
run ``make`` to build the software, and ``make install`` to actually
install it. By default, the software installs under ``/usr/local``, but
you can change this by passing ``--prefix=/path/to/install`` to
``configure``. So for example, you might use the following series of
commands to build and install xapian-core under ``/opt``::

     cd xapian-core-<version>
     ./configure --prefix=/opt
     make
     sudo make install

If you don't have root access to install Xapian, you can specify a
prefix in your home directory, for example::

     ./configure --prefix=/home/jenny/xapian-install

Configuring and building Omega
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Omega can be built in almost exactly the same way as the core library.
Omega's ``configure`` script will try to locate your Xapian installation
by looking for the ``xapian-config`` script, which is installed as
``<prefix>/bin/xapian-config``. If ``<prefix>/bin/xapian-config`` isn't
on your ``PATH``, or you have multiple installations of Xapian (perhaps
a debug and non-debug build, or two different versions), you can specify
a ``xapian-config`` to use by passing ``XAPIAN_CONFIG`` on the configure
command line, as shown below::

     cd xapian-omega-<version>
     ./configure --prefix=/opt XAPIAN_CONFIG=/opt/bin/xapian-config
     make
     sudo make install

Note that we use GNU libtool, which will set the runtime library search
path if your Xapian installation isn't in the dynamic linker search
path, so there's no need to mess around with setting
``LD_LIBRARY_PATH``.

Configuring and building Xapian-bindings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Xapian-bindings is built much like Omega. One thing to be aware of is
that by default we install the built bindings where they need to go to
work without further intervention, so they may get installed under
``/usr`` even if the prefix is elsewhere. See the ``INSTALL`` file for
xapian-bindings for details of how you can override this, and what steps
you'll need to take to run scripts which use the bindings if you do.

Building from git
~~~~~~~~~~~~~~~~~

If you wish to help develop Xapian, read `how to build from the Xapian
git repository <http://xapian.org/bleeding>`_.
