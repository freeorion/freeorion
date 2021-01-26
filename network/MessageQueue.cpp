#include "MessageQueue.h"

#include "Message.h"

#include <boost/optional/optional.hpp>

MessageQueue::MessageQueue(std::mutex& monitor) :
    m_monitor{monitor}
{}

bool MessageQueue::Empty() const {
    std::scoped_lock lock(m_monitor);
    return m_queue.empty();
}

std::size_t MessageQueue::Size() const {
    std::scoped_lock lock(m_monitor);
    return m_queue.size();
}

void MessageQueue::Clear() {
    std::scoped_lock lock(m_monitor);
    m_queue.clear();
}

void MessageQueue::PushBack(Message message) {
    std::scoped_lock lock(m_monitor);
    m_queue.push_back(std::move(message));
}

boost::optional<Message> MessageQueue::PopFront() {
    std::scoped_lock lock(m_monitor);
    if (m_queue.empty())
        return boost::none;
    Message message;
    swap(message, m_queue.front());
    m_queue.pop_front();
    return message;
}

