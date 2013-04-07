#if !defined(h_2c5af17f_8daf_418f_acb8_5cfce724ec1a)
#define h_2c5af17f_8daf_418f_acb8_5cfce724ec1a

#if defined(LOG4CPP_HAVE_BOOST)
#include <boost/version.hpp>
#if BOOST_VERSION > 103400

#include "Portability.hh"
#include "LayoutAppender.hh"

namespace log4cpp
{
   class LOG4CPP_EXPORT SmptAppender : public LayoutAppender
   {
      public:
         struct mail_params;

         SmptAppender(const std::string& name, const std::string& host, const std::string& from, 
                      const std::string& to, const std::string& subject);
         virtual ~SmptAppender();
         virtual void close() { }
      
      protected:
         virtual void _append(const LoggingEvent& event);

      private:
         mail_params * mail_params_;
   };
}

#endif // BOOST_VERSION >= 103400
#endif // LOG4CPP_HAS_BOOST
#endif // h_2c5af17f_8daf_418f_acb8_5cfce724ec1a
