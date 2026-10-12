#ifndef _MessageQueue_h_
#define _MessageQueue_h_

#include "Message.h"
#include "../util/Export.h"

#include <optional>
#include <mutex>
#include <queue>

/** A thread-safe message queue.  The entire public interface is guarded with mutex locks. */
class FO_COMMON_API MessageQueue
{
public:
    MessageQueue(std::mutex& monitor) :
        m_monitor{monitor}
    {}

    /** Returns true iff the queue is empty. */
    bool Empty() const;

    /** Returns the number of messages in the queue. */
    std::size_t Size() const;

    /** Empties the queue. */
    void Clear();

    /** Adds \a message to the end of the queue. */
    void PushBack(Message message);

    /** Return and remove the first message in the queue. */
    std::optional<Message> PopFront();

private:
    std::queue<Message> m_queue;
    std::mutex&         m_monitor;
};


#endif
