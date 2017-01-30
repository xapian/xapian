Xapian Java Bindings
********************

The current API should be regarded as experimental - we plan to review it,
possibly make some changes and then declare it stable early in the 1.4.x
release series.

How to build the bindings:
##########################

Running "make" and then "make install" will "install" a JNI glue shared library
into a "built" subdirectory of the java build directory.  The jar file is built
into the "built" subdirectory too.

You can copy these two files into your java installation, or just use them
in-place.

How to compile the examples:
############################

::

  cd java
  javac -classpath built/xapian.jar:. org/xapian/examples/SimpleIndex.java
  javac -classpath built/xapian.jar:. org/xapian/examples/SimpleSearch.java

How to run the examples:
########################

To run the examples, you need to give Java a special system-property named
"java.library.path".  The value of this property is the path of the directory
where the libxapian_jni.so (or whatever extension is used on your platform)
JNI library is located.

::

 java -Djava.library.path=built -classpath built/xapian.jar:. \
      org.xapian.examples.SimpleIndex ./test.db index words like java

 java -Djava.library.path=built -classpath built/xapian.jar:. \
      org.xapian.examples.SimpleSearch ./test.db index words like java

Alternatively, you can avoid needing the -Djava.library.path setting by
setting the LD_LIBRARY_PATH environment variable, or by installing the JNI
library in the appropriate directory so your JVM finds it automatically
(for example, on Mac OS X you can copy it into /Library/Java/Extensions/).

The java bindings have been tested recently with OpenJDK versions 1.8.0_77,
1.7.0_03, and 1.6.0_38, but they should work with any java toolchain with
suitable JNI support - please report success stories or any problems to the
development mailing list: xapian-devel@lists.xapian.org

Naming of wrapped methods:
##########################

Methods are renamed to match Java's naming conventions.  So get_mset becomes
getMSet, etc.  Also get_description is wrapped as toString.

MatchAll and MatchNothing
#########################

In Xapian 1.3.0 and later, these are wrapped as static constants
``Query.MatchAll`` and ``Query.MatchNothing``.

If you want to be compatible with earlier versions, you can continue to use
``new Query("")`` instead of ``Query.MatchAll`` and ``new Query()`` instead of
``Query.MatchNothing``.

TODO list:
##########

* Write SimpleExpand.java.

* Fix string passing to be zero-byte clean:
  https://trac.xapian.org/ticket/46

* These were missing in the JNI bindings - it would be good to add them to
  SmokeTest.java:

    - optional parameter "parameter" for Query ctor.

    - new QueryParser API.

    - changes to Enquire sorting API.

    - new method ESet::back().

    - Third (optional) argument to Document::add_posting().

    - Xapian::Weight and standard subclasses.
