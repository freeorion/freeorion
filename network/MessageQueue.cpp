#include "MessageQueue.h"

#include "Message.h"

#include <boost/optional/optional.hpp>

MessageQueue::MessageQueue(boost::mutex& monitor) :
    m_monitor{monitor}
{}

bool MessageQueue::Empty() const {
    boost::mutex::scoped_lock lock(m_monitor);
    return m_queue.empty();
}

std::size_t MessageQueue::Size() const {
    boost::mutex::scoped_lock lock(m_monitor);
    return m_queue.size();
}

void MessageQueue::Clear() {
    boost::mutex::scoped_lock lock(m_monitor);
    m_queue.clear();
}

void MessageQueue::PushBack(Message& message) {
    boost::mutex::scoped_lock lock(m_monitor);
    m_queue.push_back(Message());
    swap(m_queue.back(), message);
}

boost::optional<Message> MessageQueue::PopFront() {
    boost::mutex::scoped_lock lock(m_monitor);
    if (m_queue.empty())
        return boost::none;
    Message message;
    swap(message, m_queue.front());
    m_queue.pop_front();
    return message;
}

