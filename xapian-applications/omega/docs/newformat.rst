=====================================
Add support for a new format to Omega
=====================================

We can add support for a new file format to Omega through an external filter or a library. For this, we must follow a series of steps.

First of all, we need a mime type for the new file format. Omega uses mime types to identify the format of a file and handle it in a proper way. The official registry is at http://www.iana.org/assignments/media-types/ but not all filetypes have a corresponding official mime-type. In that case, a de-facto standard "x-" prefixed mime-type often exists. A good way to look for one is to ask the ``file`` utility to identify a file (Omega uses the same library as file to identify files when it does not recognise the extension)::

  file --mime-type example.fb2

which responses::

  example.fb2: text/xml

Sometimes ``file`` just returns a generic answer (most commonly ``text/plain`` or ``application/octet-stream``) and occasionally it misidentifies a file. If that is the case, we can associate the file format extension with a particular mime type at 'mimemap.tokens'. If multiple extensions are used for a format (such as htm and html for HTML) then add an entry for each.

When indexing a filename which has an extension in upper-case or mixed-case, omindex will check for an exact match for the extension, and if not found, it will force the extension to lower-case and try again, so just add the extension in lower-case unless different cases actually have different meanings.

In this example, ``text/xml`` is too broadly so we can associate ``fb2`` to ``application/x-fictionbook+xml`` which is much more specific.

Then, you have to decide if you will use a library or an external filter.

Using a library:
================

For safety reasons, it is not allowed to directly add libraries to omega source code. It is possible that some libraries contain bugs that affect the correct operation of the program. Because of that, omega isolates external libraries in subprocess using modules ``worker`` and ``assistant``. These modules will take care of the safety measures and use handlers to get access to the libraries.

1. To begin with, we need a library to extract information from the desired format. If many options are available, try to choose a library with an active community, that supports UTF-8 encoding and has a licence compatible with MIT/X license and GNU GPL version 2 and later. If UTF-8 output is not supported, you can convert the encoding using ``convert_to_utf8`` (replacing "ISO-8859-1" with the character set which is produced)
   ::

     convert_to_utf8(text, "ISO-8859-1");

2. Once you have the library and the mimetype, it is time to modify the code. We have to create a new handler, which is the process used by omindex to access to the library. In order to do it, we have to create a file ‘handler_yourlibrary.cc’ that includes ‘handler.h’ and gives a definition to function ``extract`` (there are some examples at xapian-applications/omega such as 'handler_poppler.cc'). For this, inside the function body you will use the library to get the necessary information and store it in the corresponding arguments
   ::

     bool
     extract(const std::string& filename,
             std::string& dump,
             std::string& title,
             std::string& keywords,
             std::string& author,
             std::string& pages)

   You can get more information about this function at 'xapian-applications/omega/handler.h'.

3. After the handler is implemented, the build system must be updated. In particular, it is necessary to modify 'configure.ac' and 'makefile.am'.

   In 'configure.ac', we need to check if the library is available using ``PKG_CHECK_MODULES`` and define ``HAVE_LIBRARY`` if we can use it. It is highly recommended to follow the example of the other handlers.
   Some macros that you might find useful are ``AC_CHECK_HEADERS``, ``AC_DEFINE``, ``AC_COMPILE``, ``AC_LINK_IFELSE``
   ::

     dnl check if libraryname is available.
     PKG_CHECK_MODULES([LIBRARY], [libraryname], [
         AC_DEFINE(HAVE_LIBRARY, 1, [Define HAVE_LIBRARY if the library is available])
         OMINDEX_MODULES="$OMINDEX_MODULES omindex_yournewformat"],[ ])

   In makefile.am,  we should add 'omindex_yourformat' to ``ExtraProg`` and define the variables ``omindex_yourformat_SOURCES``, and  ``omindex_yourformat_LDADD`` and ``omindex_yourformat_CPPFLAGS`` if they are necessary.

   If we want to add 'omindex_pdf' using poppler, we will get
   ::

     dnl check if poppler is available.
     PKG_CHECK_MODULES([POPPLER], [poppler-cpp], [
         AC_DEFINE(HAVE_POPPLER, 1, [Define HAVE_POPPLER if the library is available])
         OMINDEX_MODULES="$OMINDEX_MODULES omindex_pdf"],[ ])

   in 'configure.ac', and
   ::

     EXTRA_PROGRAMS = omindex_pdf

   and
   ::

     omindex_pdf_SOURCES = assistant.cc worker_comms.cc handler_poppler.cc
     omindex_pdf_LDADD = $(POPPLER_LIBS)
     omindex_pdf_CXXFLAGS = $(POPPLER_CFLAGS)

   in 'makefile.am'.

4. The last step is adding a new worker for the mime type to omindex. We can do it on the function ``add_default_libreries`` at 'index_file.cc'. Here we will need a compilation variable HAVE_LIBRARY, which was defined at 'configure.ac'.

   Following with the example of poppler
   ::

     add_default_libreries() {
     #if defined HAVE_POPPLER
         Worker* omindex_pdf = new Worker("omindex_pdf");
         index_library("application/pdf", omindex_pdf);
     #endif

Finally, we can compile our program to be sure that everything is okay. If the modifications are correct, we will find a new executable omindex_yourformat in the working directory.

Using a filter:
===============

To add a new filter to omega we have to follow a series of steps:

1. The first job is to find a good external filter. Some formats have several filters to choose from. The attributes which interest us are reliably extracting the text with word breaks in the right places, and supporting Unicode (ideally as UTF-8). If you have several choices, try them on some sample files.

   The ideal (and simplest) case is that you have a filter which can produce an UTF-8 output in plain text. It may requiere special command line options to do so, in which case work out what they are from the documentation or source code, and check that the output is indeed as documented.

   It is most efficient if the filter program can write to stdout, but output to a temporary file works too.

   For example, if we want to use ``python2text`` for handling ``text/x-python``, we should use ``python2text --utf8 --stdout``.

2. Then, we need to add the filter to Omega. Omega has the ability to specify additional external filters on the command line using ``--filter=M[,[T][,C]]:CMD``, which process files with MIME Content-Type M through command CMD and produces output (on stdout or in a temporary file) with format T (Content-Type or file extension; currently txt (default), html or svg) in character encoding C (default: UTF-8). For example
   ::

     --filter=text/x-foo,text/html,utf-16:'foo2utf16 --content %f %t'

   In this case, we are going to handle ``text/x-foo`` files with ``foo2utf16`` that is going to produce html with UTF-16 encoding on a temporary file. Note that %f will be replaced with the filename and %t with a temporary output file (that is going to be created by omindex at runtime). This tells omindex to index files with content-type ``text/x-foo`` by running
   ::

     foo2utf16 --content path/to/file path/to/temporary/file

   If we want to add the filter permanently, we can add a new entry in ``index_add_default_filters`` at 'index_file.cc'. Following with the example
   ::

     index_command(“text/x-foo”, Filter(“foo2utf16 --content %f %t”, "text/html", “utf-16”))

   There are more options that we can use for Filter (see 'index_file.h').

3. In some cases, you will have to run several programs for each file or make some extra work so you will either need to put together a script which fits what omindex supports, or else modify the source code in ‘index_file.cc’ by adding a test for the new mime-type to the long if/else-if chain inside ``index_mimetype`` function. New formats should generally go at the end, unless they are very common
   ::

     } else if (mimetype == "text/x-foo") {

   The filename of the file is in ``file``. The code you add should at least extract the "body" text of the document into the C++ variable ``dump``. Optionally, you can also set ``title`` (the document's title), ``keywords`` (additional text to index, but not to show the user), ``sample`` (if set, this is used to generate the static document "snippet" which is stored; if not set, this is generated from dump) and ``topic``
   ::

     string tmpfile = get_tmpfile("tmp.html");
     if (tmpfile.empty())
       return;
     string safetmp = shell_protect(tmpfile);
     string cmd = "foo2utf16 --content " + shell_protect(file) + " " + safetmp;
     try {
       (void)stdout_to_string(cmd);
       dump = file_to_string(tmpfile);
       convert_to_utf8(dump, "UTF-16");
       unlink(tmpfile.c_str());
     } catch (ReadError) {
       cout << "\"" << cmd << "\" failed - skipping\n";
       unlink(tmpfile.c_str());
       return;
     } catch (...) {
       unlink(tmpfile.c_str());
     }

   The ``shell_protect`` function escapes shell meta-characters in the filename. The ``stdout_to_string`` function runs a command and captures its output as a C++ std::string. If the command is not installed on PATH, omindex detects this automatically and disables support for the mimetype in the current run, so it will only try the first file of each such type.

   If UTF-8 output is not supported, pick the best (or only!) supported encoding and then convert the output to UTF-8 - to do this, once you have dump, convert it like so (replacing "UTF-16" with the character set which is produced)
   ::

     convert_to_utf8(string, "UTF-16");

If you find a reliable external filter or library and think it might be useful to other people, please let us know about it.

Submitting a patch:
===================

Once you are happy with how your handler/filter works, please submit a patch so we can include it in future releases (creating a new trac ticket and attaching the patch is best). Before doing so, please also update docs/overview.rst by:

- Adding the format and extensions recognised for it to the list.
- Adding the mime-type to 'mimemap.tokens'.

It would be really useful if you are able to supply some sample files with a licence which allows redistribution so we can test the filters on it. Ideally ones with non-ASCII characters so that we know Unicode support works.

You can read more about how to contribute to Xapian `here <https://xapian-developer-guide.readthedocs.io/en/latest/contributing/index.html>`_.

If you have problems you can ask for help by the `irc channel <https://webchat.freenode.net/?channels=%23xapian>`_ or the `mailing list <https://xapian.org/lists>`_.
