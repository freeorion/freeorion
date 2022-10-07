#ifndef _MessageQueue_h_
#define _MessageQueue_h_

#include <boost/thread/condition.hpp>
#include <boost/optional/optional_fwd.hpp>

#include <queue>
#include <mutex>

#include "../util/Export.h"
#include "Message.h"

/** A thread-safe message queue.  The entire public interface is guarded with mutex locks. */
class FO_COMMON_API MessageQueue
{
public:
    MessageQueue(std::mutex& monitor);

    /** Returns true iff the queue is empty. */
    bool Empty() const;

    /** Returns the number of messages in the queue. */
    std::size_t Size() const;

    /** Empties the queue. */
    void Clear();

    /** Adds \a message to the end of the queue. */
    void PushBack(Message message);

    /** Return and remove the first message in the queue. */
    boost::optional<Message> PopFront();

private:
    std::queue<Message> m_queue;
    std::mutex&         m_monitor;
};


#endif
