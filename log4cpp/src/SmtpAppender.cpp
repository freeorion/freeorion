#if defined(LOG4CPP_HAVE_BOOST)
#include <boost/version.hpp>
#if BOOST_VERSION >= 103500

#define LOG4CPP_HAVE_INT64_T
#include <log4cpp/SmtpAppender.hh>
#include <log4cpp/FactoryParams.hh>
#include <log4cpp/HierarchyMaintainer.hh>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/noncopyable.hpp>
#include <memory>

namespace log4cpp
{
   void sender_shutdown();

   struct SmptAppender::mail_params
   {
      mail_params(const std::string& host,
                  const std::string& from,
                  const std::string& to,
                  const std::string& subject) : host_(host), from_(from), to_(to), subject_(subject)
      {
      }

      std::string host_;
      std::string from_;
      std::string to_;
      std::string subject_;
   };

   namespace
   {
      struct sender : public boost::noncopyable
      {
         sender();
         static sender& instance();
         void shutdown();
         static sender* instance_;
         void send(const SmptAppender::mail_params& mp, const LoggingEvent& event);
         void operator()();
         typedef std::list<std::pair<SmptAppender::mail_params, LoggingEvent> > mails_t;

         boost::mutex mails_mutex_;
         boost::condition data_condition_;
         mails_t mails_;
         std::auto_ptr<boost::thread> sender_thread_;
         volatile bool should_exit_;
      };
      
      sender* sender::instance_ = 0;
      boost::once_flag sender_once = BOOST_ONCE_INIT;
      
      sender::sender() : should_exit_(false)
      {
         sender_thread_.reset(new boost::thread(boost::ref(*this)));   
         HierarchyMaintainer::getDefaultMaintainer().register_shutdown_handler(&sender_shutdown);
      }

      void create_sender()
      {
         sender::instance_ = new sender;
      }

      sender& sender::instance()
      {
         boost::call_once(&create_sender, sender_once);
         return *instance_;
      }

      void sender::shutdown()
      {
         {boost::mutex::scoped_lock lk(mails_mutex_);
            should_exit_ = true;
            data_condition_.notify_all();
         }

         sender_thread_->join();
         delete instance_;
      }

      void sender::send(const SmptAppender::mail_params& mp, const LoggingEvent& event)
      {
         boost::mutex::scoped_lock lk(mails_mutex_);
         mails_.push_back(mails_t::value_type(mp, event));
         data_condition_.notify_all();
      }
      
      void sender::operator()()
      {
         while(!should_exit_)
         {
            mails_t::iterator i;
            
            {boost::mutex::scoped_lock lk(mails_mutex_);
               if (should_exit_ && mails_.empty())
                  return;               

               if (mails_.empty())
               {
                  data_condition_.wait(lk);
                  if (should_exit_ && mails_.empty())
                     return;
               }

               i = mails_.begin();
            }
            
            try
            {
               boost::asio::ip::tcp::iostream s;
               s.exceptions(std::ios::failbit | std::ios::eofbit | std::ios::badbit);
               s.connect(i->first.host_, "25");
               std::string buf;
               std::getline(s, buf);
               s << "HELO test\xd\xa" << std::flush;
               std::getline(s, buf);
               s << "MAIL FROM:" << i->first.from_<< "\xd\xa" << std::flush;
               std::getline(s, buf);
               s << "RCPT TO:" << i->first.to_ << "\xd\xa" << std::flush;
               std::getline(s, buf);
               s << "DATA\xd\xa" << std::flush;
               std::getline(s, buf);
               s << "Subject: " << i->first.subject_ << "\xd\xa""Content-Transfer-Encoding: 8bit\xd\xa\xd\xa"
                  << i->second.message << "\xd\xa" ".\xd\xa" << std::flush;
               std::getline(s, buf);
               s << "QUIT\xd\xa" << std::flush;
               std::getline(s, buf);
            }
            catch(const std::exception& e)
            {
               std::cout << e.what();
            }
            
            {boost::mutex::scoped_lock lk(mails_mutex_);
               mails_.erase(i);
            }
         }
      }
   }

   void sender_shutdown()
   {
      sender::instance().shutdown();
   }

   SmptAppender::SmptAppender(const std::string& name, 
                              const std::string& host, 
                              const std::string& from,
                              const std::string& to, 
                              const std::string& subject) : LayoutAppender(name), 
                                                            mail_params_(new mail_params(host, from, to, subject))
                                                            
   {
   }

   void SmptAppender::_append(const LoggingEvent& event)
   {
      sender::instance().send(*mail_params_, event);
   }
   
   SmptAppender::~SmptAppender()
   {
      delete mail_params_;
   }

   std::auto_ptr<Appender> create_smtp_appender(const FactoryParams& params)
   {
      std::string name, host, from, to, subject;
      params.get_for("SMTP appender").required("name", name)("host", host)("from", from)
                                              ("to", to)("subject", subject);
      return std::auto_ptr<Appender>(new SmptAppender(name, host, from, to, subject));
   }
}
#endif // BOOST_VERSION >= 103500
#endif // LOG4CPP_HAS_BOOST
