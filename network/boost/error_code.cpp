//  error_code support implementation file  ----------------------------------//

//  Copyright Beman Dawes 2002, 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/system

//----------------------------------------------------------------------------//

//  VC++ 8.0 warns on usage of certain Standard Library and API functions that
//  can be cause buffer overruns or other possible security issues if misused.
//  See http://msdn.microsoft.com/msdnmag/issues/05/05/SafeCandC/default.aspx
//  But the wording of the warning is misleading and unsettling, there are no
//  portable alternative functions, and VC++ 8.0's own libraries use the
//  functions in question. So turn off the warnings.
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE

// define BOOST_SYSTEM_SOURCE so that <boost/system/config.hpp> knows
// the library is being built (possibly exporting rather than importing code)
#define BOOST_SYSTEM_SOURCE 

#include <boost/system/config.hpp>
#include <boost/system/error_code.hpp>
#include <boost/cerrno.hpp>
#include <vector>

using namespace boost::system;

#include <cstring> // for strerror/strerror_r

# ifdef BOOST_NO_STDC_NAMESPACE
    namespace std { using ::strerror; }
# endif

# if defined( BOOST_WINDOWS_API )
#   include "windows.h"
#   ifndef ERROR_INCORRECT_SIZE
#    define ERROR_INCORRECT_SIZE ERROR_BAD_ARGUMENTS
#   endif
# endif

//----------------------------------------------------------------------------//

namespace
{

#ifdef BOOST_WINDOWS_API
  struct native_to_errno_t
  { 
    boost::int32_t native_value;
    int to_errno;
  };

  const native_to_errno_t native_to_errno[] = 
  {
    // see WinError.h comments for descriptions of errors
    
    // most common errors first to speed sequential search
    { ERROR_FILE_NOT_FOUND, ENOENT },
    { ERROR_PATH_NOT_FOUND, ENOENT },

    // rest are alphabetical for easy maintenance
    { 0, 0 }, // no error 
    { ERROR_ACCESS_DENIED, EACCES },
    { ERROR_ALREADY_EXISTS, EEXIST },
    { ERROR_BAD_UNIT, ENODEV },
    { ERROR_BUFFER_OVERFLOW, ENAMETOOLONG },
    { ERROR_BUSY, EBUSY },
    { ERROR_BUSY_DRIVE, EBUSY },
    { ERROR_CANNOT_MAKE, EACCES },
    { ERROR_CANTOPEN, EIO },
    { ERROR_CANTREAD, EIO },
    { ERROR_CANTWRITE, EIO },
    { ERROR_CURRENT_DIRECTORY, EACCES },
    { ERROR_DEV_NOT_EXIST, ENODEV },
    { ERROR_DEVICE_IN_USE, EBUSY },
    { ERROR_DIR_NOT_EMPTY, ENOTEMPTY },
    { ERROR_DIRECTORY, EINVAL }, // WinError.h: "The directory name is invalid"
    { ERROR_DISK_FULL, ENOSPC },
    { ERROR_FILE_EXISTS, EEXIST },
    { ERROR_HANDLE_DISK_FULL, ENOSPC },
    { ERROR_INVALID_ACCESS, EACCES },
    { ERROR_INVALID_DRIVE, ENODEV },
    { ERROR_INVALID_FUNCTION, ENOSYS },
    { ERROR_INVALID_HANDLE, EBADHANDLE },
    { ERROR_INVALID_NAME, EINVAL },
    { ERROR_LOCK_VIOLATION, EACCES },
    { ERROR_LOCKED, EACCES },
    { ERROR_NEGATIVE_SEEK, EINVAL },
    { ERROR_NOACCESS, EACCES },
    { ERROR_NOT_ENOUGH_MEMORY, ENOMEM },
    { ERROR_NOT_READY, EAGAIN },
    { ERROR_NOT_SAME_DEVICE, EXDEV },
    { ERROR_OPEN_FAILED, EIO },
    { ERROR_OPEN_FILES, EBUSY },
    { ERROR_OUTOFMEMORY, ENOMEM },
    { ERROR_READ_FAULT, EIO },
    { ERROR_SEEK, EIO },
    { ERROR_SHARING_VIOLATION, EACCES },
    { ERROR_TOO_MANY_OPEN_FILES, ENFILE },
    { ERROR_WRITE_FAULT, EIO },
    { ERROR_WRITE_PROTECT, EROFS }
  };

  int windows_ed( const error_code & ec )
  {
    const native_to_errno_t * cur = native_to_errno;
    do
    {
      if ( ec.value() == cur->native_value ) return cur->to_errno;
      ++cur;
    } while ( cur != native_to_errno + sizeof(native_to_errno)/sizeof(native_to_errno_t) );
    return EOTHER;
  }

// TODO:
  
//Some quick notes on the implementation (sorry for the noise if
//someone has already mentioned them):
//
//- The ::LocalFree() usage isn't exception safe.
//
//See:
//
//<http://boost.cvs.sourceforge.net/boost/boost/boost/asio/system_exception.hpp?revision=1.1&view=markup>
//
//in the implementation of what() for an example.
//
//Cheers,
//Chris

  std::string windows_md( const error_code & ec )
  {
    LPVOID lpMsgBuf;
    ::FormatMessageA( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ec.value(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPSTR) &lpMsgBuf,
        0,
        NULL 
    );
    std::string str( static_cast<LPCSTR>(lpMsgBuf) );
    ::LocalFree( lpMsgBuf ); // free the buffer
    while ( str.size()
      && (str[str.size()-1] == '\n' || str[str.size()-1] == '\r') )
        str.erase( str.size()-1 );
    return str;
  }

  wstring_t windows_wmd( const error_code & ec )
  {
    LPVOID lpMsgBuf;
    ::FormatMessageW( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ec.value(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPWSTR) &lpMsgBuf,
        0,
        NULL 
    );
    wstring_t str( static_cast<LPCWSTR>(lpMsgBuf) );
    ::LocalFree( lpMsgBuf ); // free the buffer
    while ( str.size()
      && (str[str.size()-1] == L'\n' || str[str.size()-1] == L'\r') )
        str.erase( str.size()-1 );
    return str;
  }

#endif

  int errno_ed( const error_code & ec ) { return ec.value(); }

  std::string errno_md( const error_code & ec )
  {
# if defined(BOOST_WINDOWS_API) || defined(__hpux) || (defined(__linux) && !defined(__USE_XOPEN2K))
    const char * c_str = std::strerror( ec.value() );
    return std::string( c_str ? c_str : "EINVAL" );
# else
    char buf[64];
    char * bp = buf;
    std::size_t sz = sizeof(buf);
#  if defined(__CYGWIN__) || defined(__USE_GNU)
    // Oddball version of strerror_r
    const char * c_str = strerror_r( ec.value(), bp, sz );
    return std::string( c_str ? c_str : "EINVAL" );
#  else
    // POSIX version of strerror_r
    int result;
    for (;;)
    {
      if ( (result = strerror_r( ec.value(), bp, sz )) != 0 )
      {
#  if defined(__linux)
        result = errno;
#  endif
        if ( result !=  ERANGE ) break;
      }
      if ( sz > sizeof(buf) ) std::free( bp );
      sz *= 2;
      if ( (bp = static_cast<char*>(std::malloc( sz ))) == 0 )
        return std::string( "ENOMEM" );
    }
    std::string msg( ( result == EINVAL ) ? "EINVAL" : bp );
    if ( sz > sizeof(buf) ) std::free( bp );
    return msg;
#  endif
# endif
  }

  wstring_t errno_wmd( const error_code & ec )
  {
    // TODO: Implement this:
    assert( 0 && "sorry, not implemented yet" );
    wstring_t str;
    return str;
  }

  struct decoder_element
  {
    errno_decoder ed;
    message_decoder md;
    wmessage_decoder wmd;

    decoder_element( errno_decoder ed_,
      message_decoder md_, wmessage_decoder wmd_ )
      : ed(ed_), md(md_), wmd(wmd_) {}
 
    decoder_element() : ed(0), md(0), wmd(0) {}
  };

  const decoder_element init_decoders[] =
#ifdef BOOST_WINDOWS_API
  { decoder_element( errno_ed, errno_md, errno_wmd ),
    decoder_element( windows_ed, windows_md, windows_wmd) };
#else
  { decoder_element( errno_ed, errno_md, errno_wmd ) };
#endif

  typedef std::vector< decoder_element > decoder_vec_type;

  decoder_vec_type & decoder_vec()
  {
    static decoder_vec_type dv( init_decoders,
      init_decoders + sizeof(init_decoders)/sizeof(decoder_element));
    return dv;
  }
} // unnamed namespace

namespace boost
{
  namespace system
  {
    error_category error_code::new_category( 
      errno_decoder ed, message_decoder md, wmessage_decoder wmd )
    {
      decoder_vec().push_back( decoder_element( ed, md, wmd ) );
      return error_category( static_cast<value_type>(decoder_vec().size()) - 1 );
    }

    bool error_code::get_decoders( error_category cat,
      errno_decoder & ed, message_decoder & md,  wmessage_decoder & wmd )                       
    {
      if ( cat.value() < decoder_vec().size() )
      {
        ed = decoder_vec()[cat.value()].ed;
        md = decoder_vec()[cat.value()].md;
        wmd = decoder_vec()[cat.value()].wmd;
        return true;
      }
      return false;
    }

    int error_code::to_errno() const
    {
      return (m_category.value() < decoder_vec().size()
        && decoder_vec()[m_category.value()].ed)
          ? decoder_vec()[m_category.value()].ed( *this )
          : EOTHER;
    }

    std::string error_code::message() const
    {
      return (m_category.value() < decoder_vec().size()
        && decoder_vec()[m_category.value()].md)
          ? decoder_vec()[m_category.value()].md( *this )
          : std::string( "API error" );
    }

    wstring_t error_code::wmessage() const
    {
      return (m_category.value() < decoder_vec().size()
        && decoder_vec()[m_category.value()].wmd)
          ? decoder_vec()[m_category.value()].wmd( *this )
          : wstring_t( L"API error" );
    }

  } // namespace system
} // namespace boost
