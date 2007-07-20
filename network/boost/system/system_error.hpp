//  Boost system_error.hpp  --------------------------------------------------//

//  Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SYSTEM_ERROR_HPP
#define BOOST_SYSTEM_ERROR_HPP

#include <string>
#include <stdexcept>
#include <cassert>
#include <boost/system/error_code.hpp>

namespace boost
{
  namespace system
  {
    enum message_action { append_message, no_message };
    
    //  class system_error  --------------------------------------------------//

    class system_error : public std::runtime_error
    {
    public:
      explicit system_error( error_code ec )
        : std::runtime_error(std::string()), m_error_code(ec),
          m_append_message(true) {}

      system_error( error_code ec, const std::string & what_arg,
        message_action ma = append_message )
          : std::runtime_error(what_arg), m_error_code(ec),
            m_append_message(ma==append_message) {}

      system_error( error_code::value_type ev, error_category ecat )
        : std::runtime_error(std::string()), m_error_code(ev,ecat),
          m_append_message(true) {}

      system_error( error_code::value_type ev, error_category ecat,
        const std::string & what_arg, message_action ma = append_message )
          : std::runtime_error(what_arg), m_error_code(ev,ecat),
            m_append_message(ma==append_message) {}

      virtual ~system_error() throw() {}

      const error_code & code() const throw() { return m_error_code; }

      const char * what() const throw()
      // see http://www.boost.org/more/error_handling.html for lazy build rationale
      {
        if ( !m_error_code || !m_append_message ) return runtime_error::what();
        if ( m_what.empty() )
        {
          try
          {
            m_what = runtime_error::what();
            if ( !m_what.empty() ) m_what += ": ";
            m_what += m_error_code.message();
          }
          catch (...) { return runtime_error::what(); }
        }
        return m_what.c_str();
      }


    private:
      error_code           m_error_code;
      mutable std::string  m_what;
      bool                 m_append_message;
    };

  } // namespace system
} // namespace boost

#endif // BOOST_SYSTEM_ERROR_HPP


