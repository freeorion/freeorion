//  boost/system/error_code.hpp  ---------------------------------------------//

//  Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/filesystem

#ifndef BOOST_SYSTEM_ERROR_CODE_HPP
#define BOOST_SYSTEM_ERROR_CODE_HPP

#include <boost/system/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/assert.hpp>
#include <boost/operators.hpp>
#include <boost/detail/identifier.hpp>
#include <string>
#include <stdexcept>

#include <boost/config/abi_prefix.hpp> // must be the last #include

namespace boost
{
  namespace system
  {
# ifndef BOOST_NO_STD_WSTRING  // workaround Cygwin's lack of wstring_t
    typedef std::wstring wstring_t;
# else
    typedef std::basic_string<wchar_t> wstring_t;
# endif

    class error_code;

    // typedefs for registering additional decoders  -------------------------//

    typedef int          (*errno_decoder)( const error_code & );
    typedef std::string  (*message_decoder)( const error_code & );
    typedef wstring_t    (*wmessage_decoder)( const error_code & );

    //  class error_category  ------------------------------------------------//

    class BOOST_SYSTEM_DECL error_category
      : public boost::detail::identifier< uint_least32_t, error_category >
    {
    public:
      error_category()
        : boost::detail::identifier< uint_least32_t, error_category >(0){}
      explicit error_category( value_type v )
        : boost::detail::identifier< uint_least32_t, error_category >(v){}
    };

    //  predefined error categories  -----------------------------------------//

    const error_category  errno_ecat(0);  // unspecified value

# ifdef BOOST_WINDOWS_API
    const error_category  native_ecat(1); // unspecified value
# else
    const error_category  native_ecat(0); // unspecified value
# endif

    //  class error_code  ----------------------------------------------------//

    class BOOST_SYSTEM_DECL error_code
    {
    public:
      typedef boost::int_least32_t  value_type;

      // constructors:
      error_code()
        : m_value(0), m_category(errno_ecat) {}
      error_code( value_type val, error_category cat )
        : m_value(val), m_category(cat) {}


      // observers:
      value_type      value() const         { return m_value; }
      error_category  category() const      { return m_category; }
      int             to_errno() const;  // name chosen to limit surprises
                                         // see Kohlhoff Jun 28 '06
      std::string     message() const;
      wstring_t       wmessage() const;

      void assign( value_type val, const error_category & cat )
      { 
        m_value = val;
        m_category = cat;
      }

      // relationals:
      bool operator==( const error_code & rhs ) const
      {
        return value() == rhs.value() && category() == rhs.category();
      }
      bool operator!=( const error_code & rhs ) const
      {
        return !(*this == rhs);
      }
      bool operator<( const error_code & rhs ) const
      {
        return category() < rhs.category() 
          || ( category() == rhs.category() && value() < rhs.value() );
      }
      bool operator<=( const error_code & rhs ) const { return *this == rhs || *this < rhs; }
      bool operator> ( const error_code & rhs ) const { return !(*this <= rhs); }
      bool operator>=( const error_code & rhs ) const { return !(*this < rhs); }

      typedef void (*unspecified_bool_type)();
      static void unspecified_bool_true() {}

      operator unspecified_bool_type() const  // true if error
      { 
        return m_value == value_type() ? 0 : unspecified_bool_true;
      }

      bool operator!() const  // true if no error
      {
        return m_value == value_type();
      }

      // statics:
      static error_category new_category( errno_decoder ed = 0,
        message_decoder md = 0, wmessage_decoder wmd = 0 );
      static bool get_decoders( error_category cat, errno_decoder & ed,
        message_decoder & md,  wmessage_decoder & wmd );

    private:
      value_type      m_value;
      error_category  m_category;
    };

    //  non-member functions  ------------------------------------------------//

    inline std::size_t hash_value( const error_code & ec )
    {
      return static_cast<std::size_t>(ec.value())
        + (static_cast<std::size_t>(ec.category().value()) << 16 );
    }

  } // namespace system
} // namespace boost

#include <boost/config/abi_suffix.hpp> // pops abi_prefix.hpp pragmas

#endif // BOOST_SYSTEM_ERROR_CODE_HPP


