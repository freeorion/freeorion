#include "MessageQueue.h"

#include "Message.h"

#include <boost/optional/optional.hpp>

namespace {
    struct SynchronousResponseMessage {
        bool operator()(const Message& message) const
        { return message.SynchronousResponse(); }
    };
}

MessageQueue::MessageQueue(boost::mutex& monitor) :
    m_monitor(monitor), m_stopped_growing(false)
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
    if (m_queue.back().SynchronousResponse())
        m_have_synchronous_response.notify_one();
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

void MessageQueue::StopGrowing() {
    boost::mutex::scoped_lock lock(m_monitor);
    m_stopped_growing = true;
    m_have_synchronous_response.notify_one();
}

boost::optional<Message> MessageQueue::GetFirstSynchronousMessage() {
    boost::mutex::scoped_lock lock(m_monitor);
    std::list<Message>::iterator it = std::find_if(m_queue.begin(), m_queue.end(), SynchronousResponseMessage());
    while (it == m_queue.end()) {
        if (m_stopped_growing)
            return boost::none;
        m_have_synchronous_response.wait(lock);
        it = std::find_if(m_queue.begin(), m_queue.end(), SynchronousResponseMessage());
    }
    Message message;
    swap(message, *it);
    m_queue.erase(it);
    return message;
}
