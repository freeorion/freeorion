// -*- C++ -*-
#ifndef _MessageQueue_h_
#define _MessageQueue_h_

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#include <list>


class Message;

/** A thread-safe message queue.  The entire public interface is guarded with mutex locks. */
class MessageQueue
{
public:
    MessageQueue(boost::mutex& monitor);

    /** Returns true iff the queue is empty. */
    bool Empty() const;

    /** Returns the number of messages in the queue. */
    std::size_t Size() const;

    /** Empties the queue. */
    void Clear();

    /** Adds \a message to the end of the queue. */
    void PushBack(Message& message);

    /** Returns the front message in the queue. */
    void PopFront(Message& message);

    /** Returns the first synchronous repsonse message in the queue.  If no such message is found, this function blocks
        the calling thread until a synchronous response element is added. */
    void EraseFirstSynchronousResponse(Message& message);

private:
    std::list<Message> m_queue;
    boost::condition   m_have_synchronous_response;
    boost::mutex&      m_monitor;
};


#endif
