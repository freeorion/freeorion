#ifndef _MessageQueue_h_
#define _MessageQueue_h_

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/optional/optional_fwd.hpp>

#include <list>

#include "../util/Export.h"


class Message;

/** A thread-safe message queue.  The entire public interface is guarded with mutex locks. */
class FO_COMMON_API MessageQueue
{
public:
    MessageQueue(boost::mutex& monitor, const bool& rx_connected);

    /** Returns true iff the queue is empty. */
    bool Empty() const;

    /** Returns the number of messages in the queue. */
    std::size_t Size() const;

    /** Empties the queue. */
    void Clear();

    /** Adds \a message to the end of the queue. */
    void PushBack(Message& message);

    /** Return and remove the first message in the queue. */
    boost::optional<Message> PopFront();

    /** Indicates that no more new message will be added to the queue. This causes any pending
        GetFirstSynchronousMessage() to return boost::none.*/
    void RxDisconnected();

    /** Return and remove the first synchronous repsonse message from the queue or return
        boost::none if there is no synchronous message and the queue has stopped growwing. */
    boost::optional<Message> GetFirstSynchronousMessage();

private:
    std::list<Message> m_queue;
    boost::condition   m_have_synchronous_response;
    boost::mutex&      m_monitor;
    const bool&        m_rx_connected;
};


#endif
