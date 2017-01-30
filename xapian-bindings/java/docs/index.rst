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
  javac -classpath built/xapian.jar:. docs/examples/SimpleIndex.java
  javac -classpath built/xapian.jar:. docs/examples/SimpleSearch.java

How to run the examples:
########################

To run the examples, you need to give Java a special system-property named
"java.library.path".  The value of this property is the path of the directory
where the libxapian_jni.so (or whatever extension is used on your platform)
JNI library is located.

::

 java -Djava.library.path=built -classpath built/xapian.jar:docs/examples \
      SimpleIndex ./test.db index words like java

 java -Djava.library.path=built -classpath built/xapian.jar:docs/examples \
      SimpleSearch ./test.db index words like java

Alternatively, you can avoid needing the -Djava.library.path setting by
setting the LD_LIBRARY_PATH environment variable, or by installing the JNI
library in the appropriate directory so your JVM finds it automatically
(for example, on Mac OS X you can copy it into /Library/Java/Extensions/).

The java bindings have been tested recently with OpenJDK versions 1.8.0_77,
1.7.0_03, and 1.6.0_38, but they should work with any java toolchain with
suitable JNI support - please report success stories or any problems to the
development mailing list: xapian-devel@lists.xapian.org

Strings and binary data
#######################

The Xapian C++ API is largely agnostic about character encoding, and uses
the `std::string` type as an opaque container for a sequence of bytes.
In places where the bytes represent text (for example, in the
`Stem`, `QueryParser` and `TermGenerator` classes), UTF-8 encoding is used.
In Java, the `String` class uses UTF-16 encoding, and can't hold arbitrary
binary data.

The approach taken to this problem by these bindings (in Xapian 1.4.4 and
later) is to map C++ `std::string` to/from Java byte arrays (`byte[]`) in
places where the data is inherently binary (serialisation functions) or likely
to be binary (document values).

This loses a bit of generality compared to the C++ API - for example, in C++
you can add a term with a binary data value but in Java it has to be a
Unicode string.  But users rarely actually need or want that generality,
and losing it means that you can just work with Java `String`.

Document values work best when the values are compactly encoded, so a binary
encoding is usually appropriate.  However, if you really want to put a text
value in a document value slot you can explicitly convert `String` to/from
a byte array of UTF-8 data like so::

  import java.nio.charset.StandardCharsets;

  //...

  doc.addValue(1, some_string.getBytes(StandardCharsets.UTF_8));

  //...
  String value = new String(doc.getValue(1), StandardCharsets.UTF_8);

As well as terms, document data and user metadata are also required to be
text at the moment when using these bindings.

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
