/* $Id$
PStreams - POSIX Process I/O for C++
Copyright (C) 2001,2002 Jonathan Wakely

This file is part of PStreams.

PStreams is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of
the License, or (at your option) any later version.

PStreams is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with PStreams; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
 * @file pstream.h
 * @brief Declares all PStreams classes.
 * @author Jonathan Wakely
 *
 * Defines classes redi::ipstream, redi::opstream, redi::pstream
 * and, conditionally, redi::rpstream.
 */

#ifndef REDI_PSTREAM_H
#define REDI_PSTREAM_H

#include <ios>
#include <streambuf>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cerrno>
#include <sys/types.h>  // for pid_t
#include <sys/wait.h>   // for waitpid()
#include <unistd.h>     // for pipe() fork() exec() and filedes functions
#include <signal.h>     // for kill()


// TODO   add input buffering to pstreambuf

// TODO   abstract process creation and control to a process class.

// TODO   capitalise class names ?
// basic_pstreambuf -> BasicPStreamBuf
// basic_opstream   -> BasicOPStream
// basic_ipstream   -> BasicIPStream
// basic_pstream    -> BasicPStream
// basic_rpstream   -> BasicRPStream



/// The library version.
#define PSTREAMS_VERSION 0x002a   // 0.42


/**
 *  @namespace redi
 *  @brief  All PStreams classes are declared in namespace redi.
 *
 *  Like the standard IOStreams, PStreams is a set of class templates,
 *  taking a character type and traits type. As with the standard streams
 *  they are most likely to be used with @c char and the default
 *  traits type, so typedefs for this most common case are provided.
 *
 *  The @c pstream_common class template is not intended to be used directly,
 *  it is used internally to provide the common functionality for the
 *  other stream classes.
 */
namespace redi
{
  /// Common base class providing constants and typenames.
  struct pstreams
  {
    /// Type used to specify how to connect to the process
    typedef std::ios_base::openmode           pmode;

    static const pmode pstdin  = std::ios_base::out; ///< Write to stdin
    static const pmode pstdout = std::ios_base::in;  ///< Read from stdout
    static const pmode pstderr = std::ios_base::app; ///< Read from stderr

  protected:
    static const size_t bufsz = 32;
    static const size_t pbsz  = 2;
  };

  /// Class template for stream buffer.
  template <typename CharT, typename Traits>
    class basic_pstreambuf
    : public std::basic_streambuf<CharT, Traits>
    , public pstreams
    {
    public:
      // Type definitions for dependent types
      typedef CharT                             char_type;
      typedef Traits                            traits_type;
      typedef typename traits_type::int_type    int_type;
      typedef typename traits_type::off_type    off_type;
      typedef typename traits_type::pos_type    pos_type;
      /// Type used for file descriptors
      typedef int                               fd_t;

      /// Default constructor.
      basic_pstreambuf();

      /// Constructor that initialises the buffer with @a command.
      basic_pstreambuf(const std::string& command, pmode mode);

      /// Constructor that initialises the buffer with @a file and @a argv..
      basic_pstreambuf(const std::string& file, const std::vector<std::string>& argv, pmode mode);

      /// Destructor.
      ~basic_pstreambuf();

      /// Initialise the stream buffer with @a command.
      basic_pstreambuf*
      open(const std::string& command, pmode mode);

      /// Initialise the stream buffer with @a file and @a argv.
      basic_pstreambuf*
      open(const std::string& file, const std::vector<std::string>& argv, pmode mode);

      /// Close the stream buffer and wait for the process to exit.
      basic_pstreambuf*
      close();

      /// Send a signal to the process.
      basic_pstreambuf*
      kill(int signal = SIGTERM);

      /// Close the pipe connected to the process' stdin.
      void
      peof();

      /// Change active input source.
      bool
      read_err(bool readerr = true);
      
      /// Report whether the stream buffer has been initialised.
      bool
      is_open() const;

      /// Report whether the process has exited.
      bool
      exited();

#if REDI_EVISCERATE_PSTREAMS
      /// Obtain FILE pointers for each of the process' standard streams.
      size_t
      fopen(FILE*& in, FILE*& out, FILE*& err);
#endif

      /// Return the exit status of the process.
      int
      status() const;

      /// Return the error number for the most recent failed operation.
      int
      error() const;

    protected:
      /// Transfer characters to the pipe when character buffer overflows.
      int_type
      overflow(int_type c);

      /// Transfer characters from the pipe when the character buffer is empty.
      int_type
      underflow();

      /// Make a character available to be returned by the next extraction.
      int_type
      pbackfail(int_type c = traits_type::eof());

      /// Write any buffered characters to the stream.
      int
      sync();

      std::streamsize
      xsputn(const char_type* s, std::streamsize n);

      /// Insert a character into the pipe.
      bool
      write(char_type c);

      /// Extract a character from the pipe.
      bool
      read(char_type& c);

      /// Insert a sequence of characters into the pipe.
      std::streamsize
      write(char_type* s, std::streamsize n);

      /// Extract a sequence of characters from the pipe.
      std::streamsize
      read(char_type* s, std::streamsize n);

    protected:
      /// Enumerated type to indicate whether stdout or stderr is to be read.
      enum buf_read_src { rsrc_out = 0, rsrc_err = 1 };

      /// Initialise pipes and fork process.
      pid_t
      fork(pmode mode);

      /// Wait for the child process to exit.
      int
      wait(bool nohang = false);

      /// Return the file descriptor for the output pipe;
      fd_t&
      wpipe();

      /// Return the file descriptor for the active input pipe
      fd_t&
      rpipe();

      /// Return the file descriptor for the specified input pipe.
      fd_t&
      rpipe(buf_read_src which);

      void
      create_buffers(pmode mode);

      void
      destroy_buffers(pmode mode);

      /// Writes buffered characters to the process' stdin pipe..
      bool
      empty_buffer();

      bool
      fill_buffer();

      /// Return the active input buffer
      char_type*
      rbuffer();

      buf_read_src
      switch_read_buffer(buf_read_src);

    private:
      basic_pstreambuf(const basic_pstreambuf&);
      basic_pstreambuf& operator=(const basic_pstreambuf&);

      void
      init_rbuffers();

      pid_t         ppid_;        // pid of process
      fd_t          wpipe_;       // pipe used to write to process' stdin
      fd_t          rpipe_[2];    // two pipes to read from, stdout and stderr
      char_type*                wbuffer_;
      char_type*                rbuffer_[2];
      char_type*                rbufstate_[3];
      /// Index into rpipe_[] to indicate active source for read operations
      buf_read_src  rsrc_;
      int           status_;      // hold exit status of child process
      int           error_;       // hold errno if fork() or exec() fails
    };

  /// Class template for common base class.
  template <typename CharT, typename Traits = std::char_traits<CharT> >
    class pstream_common
    : virtual public std::basic_ios<CharT, Traits>
    , public pstreams
    {
    protected:
      typedef basic_pstreambuf<CharT, Traits>       streambuf_type;

    public:
      /// Type used to specify how to connect to the process
      typedef typename streambuf_type::pmode        pmode;

      /// Default constructor.
      pstream_common();

      /// Constructor that initialises the stream by starting a process.
      pstream_common(const std::string& command, pmode mode);

      /// Constructor that initialises the stream by starting a process.
      pstream_common(const std::string& file, const std::vector<std::string>& argv, pmode mode);

      /// Start a process.
      virtual void
      open(const std::string& command, pmode mode);

      /// Start a process.
      virtual void
      open(const std::string& file, const std::vector<std::string>& argv, pmode mode);

      /// Close the pipe.
      void
      close();

      /// Report whether the stream's buffer has been initialised.
      bool
      is_open() const;

      /// Return the command used to initialise the stream.
      const std::string&
      command() const;

      /// Return a pointer to the stream buffer
      streambuf_type*
      rdbuf() const;

#if REDI_EVISCERATE_PSTREAMS
      /// Obtain FILE pointers for each of the process' standard streams.
      size_t
      fopen(FILE*& in, FILE*& out, FILE*& err);
#endif

    protected:
      /// Pure virtual destructor
      virtual
      ~pstream_common() = 0;

      std::string       command_; ///< The command used to start the process.
      streambuf_type    buf_;     ///< The stream buffer.
    };


  /**
   * @class basic_ipstream
   * @brief Class template for Input PStreams.
   *
   * Reading from an ipstream reads the command's standard output and/or
   * standard error (depending on how the ipstream is opened)
   * and the command's standard input is the same as that of the process
   * that created the object, unless altered by the command itself.
   */

  template <typename CharT, typename Traits = std::char_traits<CharT> >
    class basic_ipstream
    : public std::basic_istream<CharT, Traits>
    , public pstream_common<CharT, Traits>
    {
      typedef std::basic_istream<CharT, Traits>     istream_type;
      typedef pstream_common<CharT, Traits>         pbase_type;
      typedef typename pbase_type::streambuf_type   streambuf_type;

    public:
      /// Type used to specify how to connect to the process
      typedef typename pbase_type::pmode            pmode;

      /// Default constructor, creates an uninitialised stream.
      basic_ipstream()
      : istream_type(NULL), pbase_type()
      {}

      /**
       * @brief Constructor that initialises the stream by starting a process.
       *
       * Initialises the stream buffer by calling open() with the supplied
       * arguments.
       *
       * @param command a string containing a shell command.
       * @param mode    the I/O mode to use when opening the pipe.
       * @see   open()
       */
      basic_ipstream(const std::string& command, pmode mode = std::ios_base::in)
      : istream_type(NULL), pbase_type(command, mode)
      {}

      /**
       * @brief Constructor that initialises the stream by starting a process.
       *
       * Initialises the stream buffer by calling open() with the supplied
       * arguments.
       *
       * @param file  a string containing the pathname of a program to execute.
       * @param argv  a vector of argument strings passed to the new program.
       * @param mode  the I/O mode to use when opening the pipe.
       * @see   open()
       */
      basic_ipstream(const std::string& file, const std::vector<std::string>& argv, pmode mode = std::ios_base::in)
      : istream_type(NULL), pbase_type(file, argv, mode)
      {}

      /**
       * @brief Destructor
       *
       * Closes the stream and waits for the child to exit.
       */
      ~basic_ipstream()
      {}

      /**
       * @brief Start a process.
       *
       * Starts a new process by passing @a command to the shell
       * and opens a pipe to the process with the specified @a mode.
       *
       * @param command a string containing a shell command.
       * @param mode    the I/O mode to use when opening the pipe.
       * @see   pstream_common::open()
       */
      void
      open(const std::string& command, pmode mode = std::ios_base::in)
      { pbase_type::open(command, mode); }

      /**
       * @brief Start a process.
       *
       * Starts a new process by executing @a file with the arguments in
       * @a argv and opens pipes to the process as given by @a mode.
       *
       * @param file  a string containing the pathname of a program to execute.
       * @param argv  a vector of argument strings passed to the new program.
       * @param mode  the I/O mode to use when opening the pipe.
       * @see   pstream_common::open()
       */
      void
      open(const std::string& file, const std::vector<std::string>& argv, pmode mode = std::ios_base::in)
      { pbase_type::open(file, argv, mode); }

      /**
       * @brief Set streambuf to read from process' @c stdout.
       * @return  @c *this
       */
      basic_ipstream&
      out()
      {
        buf_.read_err(false);
        return *this;
      }

      /**
       * @brief Set streambuf to read from process' @c stderr.
       * @return  @c *this
       */
      basic_ipstream&
      err()
      {
        buf_.read_err(true);
        return *this;
      }
    };


  /**
   * @class basic_opstream
   * @brief Class template for Output PStreams.
   * 
   * Writing to an open opstream writes to the standard input of the command;
   * the command's standard output is the same as that of the process that
   * created the pstream object, unless altered by the command itself.
   */

  template <typename CharT, typename Traits = std::char_traits<CharT> >
    class basic_opstream
    : public std::basic_ostream<CharT, Traits>
    , public pstream_common<CharT, Traits>
    {
      typedef std::basic_ostream<CharT, Traits>     ostream_type;
      typedef pstream_common<CharT, Traits>         pbase_type;
      typedef typename pbase_type::streambuf_type   streambuf_type;

    public:
      /// Type used to specify how to connect to the process
      typedef typename pbase_type::pmode            pmode;

      /// Default constructor, creates an uninitialised stream.
      basic_opstream()
      : ostream_type(NULL), pbase_type()
      {}

      /**
       * @brief Constructor that initialises the stream by starting a process.
       *
       * Initialises the stream buffer by calling open() with the supplied
       * arguments.
       *
       * @param command a string containing a shell command.
       * @param mode    the I/O mode to use when opening the pipe.
       * @see   open()
       */
      basic_opstream(const std::string& command, pmode mode = std::ios_base::out)
      : ostream_type(NULL), pbase_type(command, mode)
      {}

      /**
       * @brief Constructor that initialises the stream by starting a process.
       *
       * Initialises the stream buffer by calling open() with the supplied
       * arguments.
       *
       * @param file  a string containing the pathname of a program to execute.
       * @param argv  a vector of argument strings passed to the new program.
       * @param mode  the I/O mode to use when opening the pipe.
       * @see   open()
       */
      basic_opstream(const std::string& file, const std::vector<std::string>& argv, pmode mode = std::ios_base::out)
      : ostream_type(NULL), pbase_type(file, argv, mode)
      {}

      /**
       * @brief Destructor
       *
       * Closes the stream and waits for the child to exit.
       */
      ~basic_opstream() { }

      /**
       * @brief Start a process.
       * @param command a string containing a shell command.
       * @param mode    the I/O mode to use when opening the pipe.
       * @see   pstream_common::open(const std::string&, pmode)
       */
      void
      open(const std::string& command, pmode mode = std::ios_base::out)
      { pbase_type::open(command, mode); }

      /**
       * @brief Start a process.
       * @param file  a string containing the pathname of a program to execute.
       * @param argv  a vector of argument strings passed to the new program.
       * @param mode  the I/O mode to use when opening the pipe.
       * @see   pstream_common::open(const std::string&, const std::vector<std::string>&, pmode)
       */
      void
      open(const std::string& file, const std::vector<std::string>& argv, pmode mode = std::ios_base::out)
      { pbase_type::open(file, argv, mode); }
    };


  /**
   * @class basic_pstream
   * @brief Class template for Bidirectional PStreams.
   *
   * Writing to a pstream opened with @c pmode @c pstdin writes to the
   * standard input of the command.
   * Reading from a pstream opened with @c pmode @c pstdout and/or @c pstderr
   * reads the command's standard output and/or standard error.
   * Any of the process' @c stdin, @c stdout or @c stderr that is not
   * connected to the pstream (as specified by the @c pmode)
   * will be the same as the process that created the pstream object,
   * unless altered by the command itself.
   */
  template <typename CharT, typename Traits = std::char_traits<CharT> >
    class basic_pstream
    : public std::basic_iostream<CharT, Traits>
    , public pstream_common<CharT, Traits>
    {
      typedef std::basic_iostream<CharT, Traits>    iostream_type;
      typedef pstream_common<CharT, Traits>         pbase_type;
      typedef typename pbase_type::streambuf_type   streambuf_type;

    public:
      /// Type used to specify how to connect to the process
      typedef typename pbase_type::pmode            pmode;

      /// Default constructor, creates an uninitialised stream.
      basic_pstream()
      : iostream_type(NULL), pbase_type()
      {}

      /**
       * @brief Constructor that initialises the stream by starting a process.
       *
       * Initialises the stream buffer by calling open() with the supplied
       * arguments.
       *
       * @param command a string containing a shell command.
       * @param mode    the I/O mode to use when opening the pipe.
       * @see   open(const std::string&, pmode)
       */
      basic_pstream(const std::string& command, pmode mode = std::ios_base::in|std::ios_base::out)
      : iostream_type(NULL), pbase_type(command, mode)
      {}

      /**
       * @brief Constructor that initialises the stream by starting a process.
       *
       * Initialises the stream buffer by calling open() with the supplied
       * arguments.
       *
       * @param file  a string containing the pathname of a program to execute.
       * @param argv  a vector of argument strings passed to the new program.
       * @param mode  the I/O mode to use when opening the pipe.
       * @see   open(const std::string&, const std::vector<std::string>&, pmode)
       */
      basic_pstream(const std::string& file, const std::vector<std::string>& argv, pmode mode = std::ios_base::in|std::ios_base::out)
      : iostream_type(NULL), pbase_type(file, argv, mode)
      {}

      /**
       * @brief Destructor
       *
       * Closes the stream and waits for the child to exit.
       */
      ~basic_pstream() { }

      /**
       * @brief Start a process.
       * @param command a string containing a shell command.
       * @param mode    the I/O mode to use when opening the pipe.
       * @see   pstream_common::open(const std::string&, pmode)
       */
      void
      open(const std::string& command, pmode mode = std::ios_base::in|std::ios_base::out)
      { pbase_type::open(command, mode); }

      /**
       * @brief Start a process.
       * @param file  a string containing the pathname of a program to execute.
       * @param argv  a vector of argument strings passed to the new program.
       * @param mode  the I/O mode to use when opening the pipe.
       * @see   pstream_common::open(const std::string&, const std::vector<std::string>&, pmode)
       */
      void
      open(const std::string& file, const std::vector<std::string>& argv, pmode mode = std::ios_base::in|std::ios_base::out)
      { pbase_type::open(file, argv, mode); }

      /**
       * @brief Set streambuf to read from process' @c stdout.
       * @return  @c *this
       */
      basic_pstream&
      out()
      {
        buf_.read_err(false);
        return *this;
      }

      /**
       * @brief Set streambuf to read from process' @c stderr.
       * @return  @c *this
       */
      basic_pstream&
      err()
      {
        buf_.read_err(true);
        return *this;
      }
    };


  /// Type definition for common template specialisation.
  typedef basic_pstreambuf<char, std::char_traits<char> > pstreambuf;
  /// Type definition for common template specialisation.
  typedef basic_ipstream<char> ipstream;
  /// Type definition for common template specialisation.
  typedef basic_opstream<char> opstream;
  /// Type definition for common template specialisation.
  typedef basic_pstream<char> pstream;


  template <typename C, typename T>
    std::basic_ostream<C,T>&
    peof(std::basic_ostream<C,T>& s);

  /**
   * When inserted into an ouput pstream the manipulator calls 
   * basic_pstreambuf<C,T>::peof() to close the output pipe,
   * causing the child process to receive the End Of File indicator
   * on subsequent reads from its @c stdin stream.
   * 
   * @brief  Manipulator to close the pipe connected to the process' stdin.
   * @param   s  An output PStream class.
   * @return  The stream object the manipulator was invoked on.
   * @warning The effect of this manipulator is undefined if it is used
   *          with a stream object for which @c std::basic_ios<C,T>::rdbuf()
   *          does not return a pointer to a pstreambuf. If unsure do not
   *          use this manipulator and call basic_pstreambuf<C,T>::peof()
   *          directly.
   * @see     basic_pstreambuf<C,T>::peof(), basic_pstream<C,T>::rdbuf(),
   *          basic_ipstream<C,T>::rdbuf(), basic_opstream<C,T>::rdbuf().
   */
  template <typename C, typename T>
    std::basic_ostream<C,T>&
    peof(std::basic_ostream<C,T>& s)
    {
      static_cast<basic_pstreambuf<C,T>*>(s.rdbuf())->peof();
      //dynamic_cast<basic_pstreambuf<C,T>*>(s.rdbuf())->peof();
      return s;
    }


  /*
   * member definitions for pstreambuf
   */


  /**
   * @class basic_pstreambuf
   * Provides underlying streambuf functionality for the PStreams classes.
   */

  /** Creates an uninitialised stream buffer. */
  template <typename C, typename T>
    inline
    basic_pstreambuf<C,T>::basic_pstreambuf()
    : ppid_(0)
    , wpipe_(-1)
    , wbuffer_(0)
    , rsrc_(rsrc_out)
    , status_(-1)
    , error_(0)
    {
      init_rbuffers();
    }


  /**
   * Initialises the stream buffer by calling open() with the supplied
   * arguments.
   *
   * @param command a string containing a shell command.
   * @param mode    the I/O mode to use when opening the pipe.
   * @see   open()
   */
  template <typename C, typename T>
    inline
    basic_pstreambuf<C,T>::basic_pstreambuf(const std::string& command, pmode mode)
    : ppid_(0)
    , wpipe_(-1)
    , wbuffer_(0)
    , rsrc_(rsrc_out)
    , status_(-1)
    , error_(0)
    {
      init_rbuffers();
      open(command, mode);
    }

  /**
   * Initialises the stream buffer by calling open() with the supplied
   * arguments.
   *
   * @param file  a string containing the name of a program to execute.
   * @param argv  a vector of argument strings passsed to the new program.
   * @param mode  the I/O mode to use when opening the pipe.
   * @see   open()
   */
  template <typename C, typename T>
    inline
    basic_pstreambuf<C,T>::basic_pstreambuf(const std::string& file, const std::vector<std::string>& argv, pmode mode)
    : ppid_(0)
    , wpipe_(-1)
    , wbuffer_(0)
    , rsrc_(rsrc_out)
    , status_(-1)
    , error_(0)
    {
      init_rbuffers();
      open(file, argv, mode);
    }

  /**
   * Closes the stream by calling close().
   * @see close()
   */
  template <typename C, typename T>
    inline
    basic_pstreambuf<C,T>::~basic_pstreambuf()
    {
      close();
    }

  /**
   * Starts a new process by passing @a command to the shell
   * and opens pipes to the process with the specified @a mode.
   * There is no way to tell whether the shell command succeeded, this
   * function will always succeed unless resource limits (such as
   * memory usage, or number of processes or open files) are exceeded.
   *
   * @param   command  a string containing a shell command.
   * @param   mode     a bitwise OR of one or more of @c out, @c in, @c err.
   * @return  NULL if the shell could not be started or the
   *          pipes could not be opened, @c this otherwise.
   */
  template <typename C, typename T>
    inline basic_pstreambuf<C,T>*
    basic_pstreambuf<C,T>::open(const std::string& command, pmode mode)
    {
      basic_pstreambuf<C,T>* ret = NULL;

      if (!is_open())
      {
        switch(fork(mode))
        {
          case 0 :
          {
            // this is the new process, exec command
            ::execlp("sh", "sh", "-c", command.c_str(), 0);

            // can only reach this point if exec() failed

            // parent can get exit code from waitpid()
            std::exit(errno);
          }
          case -1 :
          {
            // couldn't fork, error already handled in pstreambuf::fork()
            break;
          }
          default :
          {
            // this is the parent process
#if 0
            // check process not exited already
            // very unlikely, since not enough time for shell to parse cmd!

            switch (wait(true))
            {
              case 0 :
                // activate buffers
                create_buffers(mode);
                ret = this;
                break;
              case 1:
                // child exited already
                sync();
                close_fd_array(&wpipe_, 1);
                close_fd_array(rpipe_, 2);
                break;
              default :
                break;
            }
#else
            // activate buffers
            create_buffers(mode);
            ret = this;
#endif
          }
        }
      }
      return ret;
    }


  /**
   * Starts a new process by executing @a file with the arguments in
   * @a argv and opens pipes to the process with the specified @a mode.
   * By convention argv[0] should be the file name of the file being executed.
   * Will duplicate the actions of  the  shell  in searching for an
   * executable file if the specified file name does not contain a slash (/)
   * character.
   *
   * @param   file  a string containing the pathname of a program to execute.
   * @param   argv  a vector of argument strings passed to the new program.
   * @param   mode  a bitwise OR of one or more of @c out, @c in and @c err.
   * @return  NULL if a pipe could not be opened or if the program could
   *          not be executed, @c this otherwise.
   * @see execvp()
   */
  template <typename C, typename T>
    inline basic_pstreambuf<C,T>*
    basic_pstreambuf<C,T>::open(const std::string& file, const std::vector<std::string>& argv, pmode mode)
    {
      basic_pstreambuf<C,T>* ret = NULL;

      if (!is_open())
      {
        switch(fork(mode))
        {
          case 0 :
          {
            // this is the new process, exec command

            char** arg_v = new char*[argv.size()+1];
            for (size_t i = 0; i < argv.size(); ++i)
            {
              const std::string& src = argv[i];
              char*& dest = arg_v[i];
              dest = new char[src.size()+1];
              dest[ src.copy(dest, src.size()) ] = 0;
            }
            arg_v[argv.size()] = 0;

            ::execvp(file.c_str(), arg_v);

            // can only reach this point if exec() failed

            // parent can get exit code from waitpid()
            std::exit(errno);
          }
          case -1 :
          {
            // couldn't fork, error already handled in pstreambuf::fork()
            break;
          }
          default :
          {
            // this is the parent process
#if 0
            // check process not exited already
            // very unlikely, since not enough time for shell to parse cmd!

            switch (wait(true))
            {
              case 0 :
                // activate buffers
                create_buffers(mode);
                ret = this;
                break;
              case 1:
                // child exited already
                sync();
                close_fd_array(&wpipe_, 1);
                close_fd_array(rpipe_, 2);
                break;
              default :
                break;
            }
#else
            // activate buffers
            create_buffers(mode);
            ret = this;
#endif
          }
        }
      }
      return ret;
    }

  /**
   * @brief  Helper function to close an array of file descriptors.
   * Inspects each of the @a count file descriptors in the array @a filedes
   * and calls @c close() if they have a non-negative value.
   * @param filedes an array of file descriptors
   * @param count size of the array.
   */
  inline void
  close_fd_array(int* filedes, size_t count)
  {
    for (size_t i = 0; i < count; ++i)
      if (filedes[i] >= 0)
        if (::close(filedes[i]) == 0)
          filedes[i] = -1;
  }

  /**
   * Creates pipes as specified by @a mode and calls @c fork() to create
   * a new process. If the fork is successful the parent process stores
   * the child's PID and the opened pipes and the child process replaces
   * its standard streams with the opened pipes.
   * 
   * If an error occurs the error code will be set to one of the possile
   * errors for @c pipe() or @c fork().
   * See your system's documentation for these error codes.
   *
   * @param   mode  an OR of pmodes specifying which of the child's
   *                standard streams to connect to.
   * @return  On success the PID of the child is returned in the parent's
   *          context and zero is returned in the child's context.
   *          On error -1 is returned and the error code is set appropriately.
   */
  template <typename C, typename T>
    pid_t
    basic_pstreambuf<C,T>::fork(pmode mode)
    {
      pid_t pid = -1;

      // three pairs of file descriptors, for pipes connected to the
      // process' stdin, stdout and stderr
      // (stored in a single array so close_fd_array() can close all at once)
      fd_t fd[6] =  {-1, -1, -1, -1, -1, -1};
      fd_t* pin = fd;
      fd_t* pout = fd+2;
      fd_t* perr = fd+4;

      // constants for read/write ends of pipe
      const int RD = 0;
      const int WR = 1;

      // N.B.
      // For the pstreambuf pin is an output stream and
      // pout and perr are input streams.

      if (!error_ && mode&pstdin && ::pipe(pin))
        error_ = errno;

      if (!error_ && mode&pstdout && ::pipe(pout))
        error_ = errno;

      if (!error_ && mode&pstderr && ::pipe(perr))
        error_ = errno;

      if (!error_)
      {
        pid = ::fork();
        switch (pid)
        {
          case 0 :
          {
            // this is the new process

            // for each open pipe close one end and redirect the
            // respective standard stream to the other end

            if (*pin >= 0)
            {
              ::close(pin[WR]);
              ::dup2(pin[RD], STDIN_FILENO);
              ::close(pin[RD]);
            }
            if (*pout >= 0)
            {
              ::close(pout[RD]);
              ::dup2(pout[WR], STDOUT_FILENO);
              ::close(pout[WR]);
            }
            if (*perr >= 0)
            {
              ::close(perr[RD]);
              ::dup2(perr[WR], STDERR_FILENO);
              ::close(perr[WR]);
            }
            break;
          }
          case -1 :
          {
            // couldn't fork for some reason
            error_ = errno;
            // close any open pipes
            close_fd_array(fd, 6);
            break;
          }
          default :
          {
            // this is the parent process, store process' pid
            ppid_ = pid;

            // store one end of open pipes and close other end
            if (*pin >= 0)
            {
              wpipe_ = pin[WR];
              ::close(pin[RD]);
            }
            if (*pout >= 0)
            {
              rpipe_[rsrc_out] = pout[RD];
              ::close(pout[WR]);
            }
            if (*perr >= 0)
            {
              rpipe_[rsrc_err] = perr[RD];
              ::close(perr[WR]);
            }

            if (rpipe_[rsrc_out] == -1 && rpipe_[rsrc_err] >= 0)
            {
              // reading stderr but not stdout, so use stderr for all reads
              read_err(true);
            }
          }
        }
      }
      else
      {
        // close any pipes we opened before failure
        close_fd_array(fd, 6);
      }
      return pid;
    }


  /**
   * Closes all pipes and waits for the associated process to finish.
   * If an error occurs the error code will be set to one of the possible errors
   * for @c waitpid(). See your system's documentation for these errors.
   *
   * @return  @c this on successful close or @c NULL if there is no
   *          process to close or if an error occurs.
   */
  template <typename C, typename T>
    inline basic_pstreambuf<C,T>*
    basic_pstreambuf<C,T>::close()
    {
      basic_pstreambuf<C,T>* ret = NULL;
      if (is_open())
      {
        sync();
        destroy_buffers(pstdin|pstdout|pstderr);

        close_fd_array(&wpipe_, 1);
        close_fd_array(rpipe_, 2);

        if (wait() == 1)
        {
          ret = this;
        }
      }
      return ret;
    }


  /**
   *  Called on construction to initialise the arrays used for reading.
   */
  template <typename C, typename T>
    inline void
    basic_pstreambuf<C,T>::init_rbuffers()
    {
      rpipe_[rsrc_out] = rpipe_[rsrc_err] = -1;
      rbuffer_[rsrc_out] = rbuffer_[rsrc_err] = 0;
      rbufstate_[0] = rbufstate_[1] = rbufstate_[2] = 0;
    }


  template <typename C, typename T>
    void
    basic_pstreambuf<C,T>::create_buffers(pmode mode)
    {
      if (mode & pstdin)
      {
        delete[] wbuffer_;
        wbuffer_ = new char_type[bufsz];
        setp(wbuffer_, wbuffer_ + bufsz);
      }
      if (mode & pstdout)
      {
        delete[] rbuffer_[rsrc_out];
        rbuffer_[rsrc_out] = new char_type[bufsz];
        if (rsrc_ == rsrc_out)
          setg(rbuffer_[rsrc_out] + pbsz, rbuffer_[rsrc_out] + pbsz,
              rbuffer_[rsrc_out] + pbsz);
      }
      if (mode & pstderr)
      {
        delete[] rbuffer_[rsrc_err];
        rbuffer_[rsrc_err] = new char_type[bufsz];
        if (rsrc_ == rsrc_err)
          setg(rbuffer_[rsrc_err] + pbsz, rbuffer_[rsrc_err] + pbsz,
              rbuffer_[rsrc_err] + pbsz);
      }
    }


  template <typename C, typename T>
    void
    basic_pstreambuf<C,T>::destroy_buffers(pmode mode)
    {
      if (mode & pstdin)
      {
        setp(0, 0);
        delete[] wbuffer_;
        wbuffer_ = 0;
      }
      if (mode & pstdout)
      {
        if (rsrc_ == rsrc_out)
          setg(0, 0, 0);
        delete[] rbuffer_[rsrc_out];
        rbuffer_[rsrc_out] = 0;
      }
      if (mode & pstderr)
      {
        if (rsrc_ == rsrc_err)
          setg(0, 0, 0);
        delete[] rbuffer_[rsrc_err];
        rbuffer_[rsrc_err] = 0;
      }
    }

  template <typename C, typename T>
    typename basic_pstreambuf<C,T>::buf_read_src
    basic_pstreambuf<C,T>::switch_read_buffer(buf_read_src src)
    {
      if (rsrc_ != src)
      {
        char_type* tmpbufstate[3];
        tmpbufstate[0] = eback();
        tmpbufstate[1] = gptr();
        tmpbufstate[2] = egptr();
        setg(rbufstate_[0], rbufstate_[1], rbufstate_[2]);
        for (size_t i = 0; i < 3; ++i)
          rbufstate_[i] = tmpbufstate[i];
      }
      return rsrc_;
    }


  /**
   * Suspends execution and waits for the associated process to exit, or
   * until a signal is delivered whose action is to terminate the current
   * process or to call a signal handling function. If the process has
   * already exited wait() returns immediately.
   *
   * @param   nohang  true to return immediately if the process has not exited.
   * @return  1 if the process has exited, 0 if @a nohang is true and the
   *          process has not exited yet, -1 if an error occurs, in which
   *          case the error can be found using error().
   */
  template <typename C, typename T>
    inline int
    basic_pstreambuf<C,T>::wait(bool nohang)
    {
      int exited = -1;
      if (is_open())
      {
        int status;
        switch(::waitpid(ppid_, &status, nohang ? WNOHANG : 0))
        {
          case 0 :
            // nohang was true and process has not exited
            exited = 0;
            break;
          case -1 :
            error_ = errno;
            break;
          default :
            // process has exited
            ppid_ = 0;
            status_ = status;
            exited = 1;
            // TODO  close pipes with close_fd_array() ?
            break;
        }
      }
      return exited;
    }


  /**
   * Sends the specified signal to the process.  A signal can be used to
   * terminate a child process that would not exit otherwise.
   * 
   * If an error occurs the error code will be set to one of the possible
   * errors for @c kill().  See your system's documentation for these errors.
   *
   * @param   signal  A signal to send to the child process.
   * @return  @c this or @c NULL if @c kill() fails.
   */
  template <typename C, typename T>
    inline basic_pstreambuf<C,T>*
    basic_pstreambuf<C,T>::kill(int signal)
    {
      basic_pstreambuf<C,T>* ret = NULL;
      if (is_open())
      {
        if (::kill(ppid_, signal))
          error_ = errno;
        else
          ret = this;
      }
      return ret;
    }


  /**
   *  return  True if the associated process has exited, false otherwise.
   *  see     basic_pstreambuf<C,T>::close()
   */
  template <typename C, typename T>
    inline bool
    basic_pstreambuf<C,T>::exited()
    {
      // TODO  should close() if is_open() and has exited
      return wait(true)==1;
    }


  /**
   *  return  The exit status of the child process, or -1 if close()
   *          has not yet been called to wait for the child to exit.
   *  see     basic_pstreambuf<C,T>::close()
   */
  template <typename C, typename T>
    inline int
    basic_pstreambuf<C,T>::status() const
    {
      return status_;
    }

  /**
   *  return  The error code of the most recently failed operation, or zero.
   */
  template <typename C, typename T>
    inline int
    basic_pstreambuf<C,T>::error() const
    {
      return error_;
    }

  /**
   * closes the output pipe, causing the child process to receive the
   * End Of File indicator on subsequent reads from its @c stdin stream.
   *
   * @return  peof() returns no value.
   */
  template <typename C, typename T>
    inline void
    basic_pstreambuf<C,T>::peof()
    {
      sync();
      destroy_buffers(pstdin);
      close_fd_array(&wpipe_, 1);
    }

  /**
   * @return  true if a previous call to open() succeeded and close()
   *          has not been called successfully, false otherwise.
   * @warning This function can not be used to determine whether the
   *          command used to initialise the buffer was successfully
   *          executed or not. If the shell command failed this function
   *          will still return true.
   */
  template <typename C, typename T>
    inline bool
    basic_pstreambuf<C,T>::is_open() const
    {
      return bool(ppid_>0);
    }

  /**
   * Toggle the stream used for reading. If @a readerr is @c true then the
   * process' @c stderr output will be used for subsequent extractions, if
   * @a readerr is false the the process' stdout will be used.
   * @param readerr @c true to read @c stderr, @c false to read @c stdout.
   * @return @c true if the requested stream is open and will be used for
   * subsequent extractions, @c false otherwise.
   */
  template <typename C, typename T>
    inline bool
    basic_pstreambuf<C,T>::read_err(bool readerr)
    {
      buf_read_src src = readerr ? rsrc_err : rsrc_out;
      if (rpipe_[src]>=0)
      {
        rsrc_ = src;
        return true;
      }
      return false;
    }

  /**
   * Called when the internal character buffer is not present or is full,
   * to transfer the buffer contents to the pipe. For unbuffered streams
   * this is called for every insertion.
   *
   * @param c a character to be written to the pipe
   * @return @c traits_type::not_eof(c) if @a c is equal to @c
   * traits_type::eof(). Otherwise returns @a c if @a c can be written
   * to the pipe, or @c traits_type::eof() if not.
   */
  template <typename C, typename T>
    typename basic_pstreambuf<C,T>::int_type
    basic_pstreambuf<C,T>::overflow(int_type c)
    {
      if (!empty_buffer())
        return traits_type::eof();
      else if (!traits_type::eq_int_type(c, traits_type::eof()))
        return sputc(c);
      else
        return traits_type::not_eof(c);
    }


  template <typename C, typename T>
    int
    basic_pstreambuf<C,T>::sync()
    {
      return (empty_buffer() ? 0 : -1);
    }


  template <typename C, typename T>
    std::streamsize
    basic_pstreambuf<C,T>::xsputn(const char_type* s, std::streamsize n)
    {
      if (n < epptr() - pptr())
      {
        memcpy(pptr(), s, n * sizeof(char_type));
        pbump(n);
        return n;
      }
      else
      {
        for (std::streamsize i = 0; i < n; ++i)
        {
          if (traits_type::eq_int_type(sputc(s[i]), traits_type::eof()))
            return i;
        }
        return n;
      }
    }


  /**
   * @return  true if the buffer was emptied, false otherwise.
   */
  template <typename C, typename T>
    bool
    basic_pstreambuf<C,T>::empty_buffer()
    {
      int count = pptr() - pbase();
      std::streamsize written = write(wbuffer_, count);
      if (count > 0 && written == count)
      {
        pbump(-written);
        return true;
      }
      return false;
    }


  /**
   * Called when the internal character buffer is is empty, to re-fill it
   * from the pipe.
   *
   * @return The first available character in the buffer,
   * or @c traits_type::eof() in case of failure.
   * @see uflow()
   */
  template <typename C, typename T>
    typename basic_pstreambuf<C,T>::int_type
    basic_pstreambuf<C,T>::underflow()
    {
      if (gptr() < egptr() || fill_buffer())
        return traits_type::to_int_type(*gptr());
      else
        return traits_type::eof();
    }


  /**
   * Attempts to make @a c available as the next character to be read by
   * @c sgetc().
   *
   * @param   c   a character to make available for extraction.
   * @return  @a c if the character can be made available,
   *          @c traits_type::eof() otherwise.
   */
  template <typename C, typename T>
    typename basic_pstreambuf<C,T>::int_type
    basic_pstreambuf<C,T>::pbackfail(int_type c)
    {
      if (gptr() != eback())
      {
        gbump(-1);
        if (!traits_type::eq_int_type(c, traits_type::eof()))
          *gptr() = traits_type::to_char_type(c);
        return traits_type::not_eof(c);
      }
      else
         return traits_type::eof();
    }

  /**
   * @return  true if the buffer was filled, false otherwise.
   */
  template <typename C, typename T>
    bool
    basic_pstreambuf<C,T>::fill_buffer()
    {
      int npb = std::min(gptr()-eback(), static_cast<int>(pbsz));

      std::memmove(rbuffer()+pbsz-npb, gptr()-npb, npb*sizeof(char_type));

      std::streamsize rc = read(rbuffer() + pbsz, bufsz - pbsz);

      if (rc > 0)
      {
        setg(rbuffer()+pbsz-npb, rbuffer()+pbsz, rbuffer()+pbsz+rc);
        return true;
      }
      else
      {
        setg(0, 0, 0);
        return false;
      }
    }


  /**
   * Attempts to insert @a c into the pipe. Used by overflow().
   *
   * @param c a character to insert.
   * @return true if the character could be inserted, false otherwise.
   * @see write(char_type* s, std::streamsize n)
   */
  template <typename C, typename T>
    inline bool
    basic_pstreambuf<C,T>::write(char_type c)
    {
      return (write(&c, 1) == 1);
    }

  /**
   * Attempts to extract a character from the pipe and store it in @a c.
   * Used by underflow().
   *
   * @param c a reference to hold the extracted character.
   * @return true if a character could be extracted, false otherwise.
   * @see read(char_type* s, std::streamsize n)
   */
  template <typename C, typename T>
    inline bool
    basic_pstreambuf<C,T>::read(char_type& c)
    {
      return (read(&c, 1) == 1);
    }

  /**
   * Writes up to @a n characters to the pipe from the buffer @a s.
   * This currently only works for fixed width character encodings where
   * each character uses sizeof(char_type) bytes.
   *
   * @param s character buffer.
   * @param n buffer length.
   * @return the number of characters written.
   */
  template <typename C, typename T>
    inline std::streamsize
    basic_pstreambuf<C,T>::write(char_type* s, std::streamsize n)
    {
      return (wpipe() >= 0 ? ::write(wpipe(), s, n * sizeof(char_type)) : 0);
    }

  /**
   * Reads up to @a n characters from the pipe to the buffer @a s.
   * This currently only works for fixed width character encodings where
   * each character uses sizeof(char_type) bytes.
   *
   * @param s character buffer.
   * @param n buffer length.
   * @return the number of characters read.
   */
  template <typename C, typename T>
    inline std::streamsize
    basic_pstreambuf<C,T>::read(char_type* s, std::streamsize n)
    {
      return (rpipe() >= 0 ? ::read(rpipe(), s, n * sizeof(char_type)) : 0);
    }


  /** @return a reference to the output file descriptor */
  template <typename C, typename T>
    inline typename basic_pstreambuf<C,T>::fd_t&
    basic_pstreambuf<C,T>::wpipe()
    {
      return wpipe_;
    }

  /** @return a reference to the active input file descriptor */
  template <typename C, typename T>
    inline typename basic_pstreambuf<C,T>::fd_t&
    basic_pstreambuf<C,T>::rpipe()
    {
      return rpipe_[rsrc_];
    }

  /** @return a reference to the specified input file descriptor */
  template <typename C, typename T>
    inline typename basic_pstreambuf<C,T>::fd_t&
    basic_pstreambuf<C,T>::rpipe(buf_read_src which)
    {
      return rpipe_[which];
    }

  /** @return a pointer to the start of the active input buffer area. */
  template <typename C, typename T>
    inline typename basic_pstreambuf<C,T>::char_type*
    basic_pstreambuf<C,T>::rbuffer()
    {
      return rbuffer_[rsrc_];
    }


  /*
   * member definitions for pstream_common
   */

  /**
   * @class pstream_common
   * Abstract Base Class providing common functionality for basic_ipstream,
   * basic_opstream and basic_pstream.
   * pstream_common manages the basic_pstreambuf stream buffer that is used
   * by the derived classes to initialise an IOStream class.
   */

  /** Creates an uninitialised stream. */
  template <typename C, typename T>
    inline
    pstream_common<C,T>::pstream_common()
    : std::basic_ios<C,T>(NULL)
    , command_()
    , buf_()
    {
      init(&buf_);
    }

  /**
   * Initialises the stream buffer by calling open() with the supplied
   * arguments.
   *
   * @param command a string containing a shell command.
   * @param mode    the I/O mode to use when opening the pipe.
   * @see   open()
   */
  template <typename C, typename T>
    inline
    pstream_common<C,T>::pstream_common(const std::string& command, pmode mode)
    : std::basic_ios<C,T>(NULL)
    , command_(command)
    , buf_()
    {
      init(&buf_);
      open(command, mode);
    }
   
  /**
   * Initialises the stream buffer by calling open() with the supplied
   * arguments.
   *
   * @param file  a string containing the pathname of a program to execute.
   * @param argv  a vector of argument strings passed to the new program.
   * @param mode  the I/O mode to use when opening the pipe.
   * @see open()
   */
  template <typename C, typename T>
    inline
    pstream_common<C,T>::pstream_common(const std::string& file, const std::vector<std::string>& argv, pmode mode)
    : std::basic_ios<C,T>(NULL)
    , command_(file)
    , buf_()
    {
      init(&buf_);
      open(file, argv, mode);
    }

  /**
   * This is a pure virtual function to make @c pstream_common abstract.
   * Because it is the destructor it will be called by derived classes
   * and so must be defined.  It is also protected, to discourage use of
   * the PStreams classes through pointers or references to the base class.
   *
   * @sa If defining a pure virtual seems odd you should read
   * http://www.gotw.ca/gotw/031.htm (and the rest of the site as well!)
   */
  template <typename C, typename T>
    inline
    pstream_common<C,T>::~pstream_common()
    {
    }

  /**
   * Starts a new process by passing @a command to the shell and
   * opens pipes to the process as given by @a mode.
   *
   * @param command a string containing a shell command.
   * @param mode    the I/O mode to use when opening the pipe.
   * @see   basic_pstreambuf::open()
   */
  template <typename C, typename T>
    inline void
    pstream_common<C,T>::open(const std::string& command, pmode mode)
    {
      if (!buf_.open((command_=command), mode))
        setstate(std::ios_base::failbit);
    }

  /**
   * Starts a new process by executing @a file with the arguments in
   * @a argv and opens pipes to the process as given by @a mode.
   *
   * @param file  a string containing the pathname of a program to execute.
   * @param argv  a vector of argument strings passed to the new program.
   * @param mode  the I/O mode to use when opening the pipe.
   * @see   basic_pstreambuf::open()
   */
  template <typename C, typename T>
    inline void
    pstream_common<C,T>::open(const std::string& file, const std::vector<std::string>& argv, pmode mode)
    {
      if (!buf_.open((command_=file), argv, mode))
        setstate(std::ios_base::failbit);
    }

  /** Waits for the associated process to finish and closes the pipe. */
  template <typename C, typename T>
    inline void
    pstream_common<C,T>::close()
    {
      if (!buf_.close())
        setstate(std::ios_base::failbit);
    }

  /**
   * @return  true if open() has been successfully called, false otherwise.
   * @see     basic_pstreambuf::open()
   */
  template <typename C, typename T>
    inline bool
    pstream_common<C,T>::is_open() const
    {
      return buf_.is_open();
    }

  /** @return a string containing the command used to initialise the stream. */
  template <typename C, typename T>
    inline const std::string&
    pstream_common<C,T>::command() const
    {
      return command_;
    }

  /** @return a pointer to the private stream buffer member. */
  // TODO  document behaviour if buffer replaced.
  template <typename C, typename T>
    inline typename pstream_common<C,T>::streambuf_type*
    pstream_common<C,T>::rdbuf() const
    {
      return const_cast<streambuf_type*>(&buf_);
    }

#if REDI_EVISCERATE_PSTREAMS
  /**
   * @def REDI_EVISCERATE_PSTREAMS
   * If this macro has a non-zero value then certain internals of the
   * @c basic_pstreambuf template class are exposed. In general this is
   * a Bad Thing, as the internal implementation is largely undocumented
   * and may be subject to change at any time, so this feature is only
   * provided because it might make PStreams useful in situations where
   * it is necessary to do Bad Things.
   */

  /**
   * @warning  This function exposes the internals of the stream buffer and
   *           should be used with caution. It is the caller's responsibility
   *           to flush streams etc. in order to clear any buffered data.
   *           The POSIX.1 function @c fdopen(3) is used to obtain the
   *           @c FILE pointers from the streambuf's private file descriptor
   *           members so consult your system's documentation for @c fdopen().
   *
   * @param   in    A FILE* that will refer to the process' stdin.
   * @param   out   A FILE* that will refer to the process' stdout.
   * @param   err   A FILE* that will refer to the process' stderr.
   * @return  An OR of zero or more of @c pstdin, @c pstdout, @c pstderr.
   *
   * For each open stream shared with the child process a @c FILE* is
   * obtained and assigned to the corresponding parameter. For closed
   * streams @c NULL is assigned to the parameter.
   * The return value can be tested to see which parameters should be
   * @c !NULL by masking with the corresponding @c pmode value.
   */
  template <typename C, typename T>
    inline size_t
    basic_pstreambuf<C,T>::fopen(FILE*& in, FILE*& out, FILE*& err)
    {
      in = out = err = NULL;
      size_t open_files = 0;
      if (wpipe() > -1)
      {
        if (in = ::fdopen(wpipe(), "w"))
        {
            open_files |= pstdin;
        }
      }
      if (rpipe(rsrc_out) > -1) 
      {
        if (out = ::fdopen(rpipe(rsrc_out), "r"))
        {
            open_files |= pstdout;
        }
      }
      if (rpipe(rsrc_err) > -1)
      {
        if (err = ::fdopen(rpipe(rsrc_err), "r"))
        {
            open_files |= pstderr;
        }
      }
      return open_files;
    }

  /**
   *  @warning This function exposes the internals of the stream buffer and
   *  should be used with caution.
   *
   *  @param  in   A FILE* that will refer to the process' stdin.
   *  @param  out  A FILE* that will refer to the process' stdout.
   *  @param  err  A FILE* that will refer to the process' stderr.
   *  @return A bitwise-or of zero or more of @c pstdin, @c pstdout, @c pstderr.
   *  @see    basic_pstreambuf::fopen()
   */
  template <typename C, typename T>
    inline size_t
    pstream_common<C,T>::fopen(FILE*& in, FILE*& out, FILE*& err)
    {
      return buf_.fopen(in, out, err);
    }

#endif // REDI_EVISCERATE_PSTREAMS


} // namespace redi

/**
 * @mainpage PStreams Reference
 * @htmlinclude mainpage.html
 */

#endif  // REDI_PSTREAM_H

// vim: ts=2 sw=2 expandtab
