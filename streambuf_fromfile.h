#ifndef STREAM_FROMFILE_H
#define STREAM_FROMFILE_H

#if defined(STREAM_GCC)
	//libstdc++ is required
	#include <ext/stdio_filebuf.h>
	#define streambuf_fromfile_out(name,f) __gnu_cxx::stdio_filebuf<char> name(f,std::ios_base::out)
	#define streambuf_fromfile_in(name,f) __gnu_cxx::stdio_filebuf<char> name(f,std::ios_base::in)
#elif defined(STREAM_BOOST)
	//libboost_iostreams is required
	#include <boost/iostreams/device/file_descriptor.hpp>
	#include <boost/iostreams/stream.hpp>
	#define streambuf_fromfile_out(name,f) boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_sink> name(fileno(f),boost::iostreams::never_close_handle)
	#define streambuf_fromfile_in(name,f) boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_source> name(fileno(f),boost::iostreams::never_close_handle)
#elif defined(STREAM_PORTABLE)
	#define streambuf_fromfile_out(name,f) boost::fdoutbuf name(fileno(f))
	#define streambuf_fromfile_in(name,f) boost::fdinbuf name(fileno(f))

/* The following code declares classes to read from and write to
 * file descriptore or file handles.
 *
 * See
 *      http://www.josuttis.com/cppcode
 * for details and the latest version.
 *
 * - open:
 * 	- integrating BUFSIZ on some systems?
 * 	- optimized reading of multiple characters
 * 	- stream for reading AND writing
 * 	- i18n
 *
 * (C) Copyright Nicolai M. Josuttis 2001.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 *
 * Aug 05, 2001
 */

// for EOF:
#include <cstdio>
// for memcpy():
#include <cstring>
#include <unistd.h>

// BEGIN namespace BOOST
namespace boost {

/************************************************************
 * fdostream
 * - a stream that writes on a file descriptor
 ************************************************************/

class fdoutbuf : public std::streambuf {
  protected:
    int fd;    // file descriptor
  public:
    // constructor
    fdoutbuf (int _fd) : fd(_fd) {
    }
  protected:
    // write one character
    virtual int_type overflow (int_type c) {
        if (c != EOF) {
            char z = c;
            if (write (fd, &z, 1) != 1) {
                return EOF;
            }
        }
        return c;
    }
    // write multiple characters
    virtual
    std::streamsize xsputn (const char* s,
                            std::streamsize num) {
        return write(fd,s,num);
    }
};

class fdostream : public std::ostream {
  protected:
    fdoutbuf buf;
  public:
    fdostream (int fd) : std::ostream(0), buf(fd) {
        rdbuf(&buf);
    }
};

/************************************************************
 * fdistream
 * - a stream that writes on a file descriptor
 ************************************************************/

class fdinbuf : public std::streambuf {
  protected:
    int fd;    // file descriptor
  protected:
    /* data buffer:
     * - at most, pbSize characters in putback area plus
     * - at most, bufSize characters in ordinary read buffer
     */
    static const int pbSize = 4;        // size of putback area
    static const int bufSize = 1024;    // size of the data buffer
    char buffer[bufSize+pbSize];        // data buffer

  public:
    /* constructor
     * - initialize file descriptor
     * - initialize empty data buffer
     * - no putback area
     * => force underflow()
     */
    fdinbuf (int _fd) : fd(_fd) {
        setg (buffer+pbSize,     // beginning of putback area
              buffer+pbSize,     // read position
              buffer+pbSize);    // end position
    }

  protected:
    // insert new characters into the buffer
    virtual int_type underflow () {
#ifndef _MSC_VER
using std::memcpy;
#endif

        // is read position before end of buffer?
        if (gptr() < egptr()) {
            return *gptr();
        }

        /* process size of putback area
         * - use number of characters read
         * - but at most size of putback area
         */
        int numPutback;
        numPutback = gptr() - eback();
        if (numPutback > pbSize) {
            numPutback = pbSize;
        }

        /* copy up to pbSize characters previously read into
         * the putback area
         */
        memcpy (buffer+(pbSize-numPutback), gptr()-numPutback,
                numPutback);

        // read at most bufSize new characters
        int num;
        num = read (fd, buffer+pbSize, bufSize);
        if (num <= 0) {
            // ERROR or EOF
            return EOF;
        }

        // reset buffer pointers
        setg (buffer+(pbSize-numPutback),   // beginning of putback area
              buffer+pbSize,                // read position
              buffer+pbSize+num);           // end of buffer

        // return next character
        return *gptr();
    }
};

class fdistream : public std::istream {
  protected:
    fdinbuf buf;
  public:
    fdistream (int fd) : std::istream(0), buf(fd) {
        rdbuf(&buf);
    }
};

} // END namespace boost
#else
	#error define one of: STREAM_GCC, STREAM_BOOST, STREAM_PORTABLE
#endif

#endif //included.
