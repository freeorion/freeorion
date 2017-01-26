
#include "RunQueue.h"
#include <boost/ref.hpp>
#include <boost/thread/thread.hpp>
#include <algorithm>

template <class WorkItem>
ThreadQueue<WorkItem>::ThreadQueue(RunQueue<WorkItem>* the_global_queue) : work_queue_1(), work_queue_2() {
    global_queue         = the_global_queue;
    running_queue_size   = 0U;
    schedule_queue_size  = 0U;
    running_queue  = &work_queue_1;
    schedule_queue = &work_queue_2;
    thread = boost::thread(boost::ref(*this));
}

template <class WorkItem>
void ThreadQueue<WorkItem>::operator ()() {
    while (true) {
        while (running_queue_size) {
            WorkItem* current_item = (*running_queue)[running_queue_size - 1];
            (*current_item)();
            delete current_item;
            --running_queue_size;
        }
        {
            boost::shared_lock<boost::shared_mutex> schedule_lock(global_queue->m_schedule_mutex);

            while (schedule_queue_size == 0U) {
                schedule_lock.unlock();
                if (global_queue->Schedule(*this)) return;
                schedule_lock.lock();
            }

            unsigned const n_fetched_jobs = (std::min)(1000U, ((unsigned) schedule_queue_size + 1U)/2U);
            typename std::vector<WorkItem*>::iterator const schedule_queue_end = schedule_queue->begin() + schedule_queue_size;

            if (n_fetched_jobs > running_queue->size())
                running_queue->resize(n_fetched_jobs);

            std::copy(schedule_queue_end - n_fetched_jobs, schedule_queue_end, running_queue->begin());
            schedule_queue_size -= n_fetched_jobs;
            running_queue_size = n_fetched_jobs;
        }
    }
}

template <class WorkItem>
RunQueue<WorkItem>::RunQueue(unsigned n_threads) :
    m_terminate (false),
    m_schedule_mutex(),
    m_work_available(),
    m_work_done(),
    m_thread_queues(),
    m_transfer_queue(),
    m_transfer_queue_size(0U)
{
    boost::unique_lock<boost::shared_mutex> schedule_lock(m_schedule_mutex);

    for (unsigned i = 0U; i < n_threads; ++i) {
        m_thread_queues.push_back(std::make_shared<ThreadQueue<WorkItem>>(this));
    }
}

template <class WorkItem>
RunQueue<WorkItem>::~RunQueue() {
    {
        boost::shared_lock<boost::shared_mutex> schedule_lock(m_schedule_mutex);
        m_terminate = true;
    }
    m_work_available.notify_all();
    for (std::shared_ptr<ThreadQueue<WorkItem>> thread_queue : m_thread_queues)
        thread_queue->thread.join();
}

template <class WorkItem>
void RunQueue<WorkItem>::AddWork(WorkItem* item) {
    boost::shared_lock<boost::shared_mutex> schedule_lock(m_schedule_mutex);
    const unsigned old_transfer_queue_size = m_transfer_queue_size++;

    if (m_transfer_queue.size() < m_transfer_queue_size)
        m_transfer_queue.resize(m_transfer_queue_size);
    m_transfer_queue[old_transfer_queue_size] = item;
    m_work_available.notify_one(); 
}

namespace {
    template <class Lockable> class scoped_unlock : public boost::noncopyable {
    private:
        Lockable& m_lockable;
    public:
        scoped_unlock(Lockable& lockable) : m_lockable(lockable) { m_lockable.unlock(); }
        ~scoped_unlock() { m_lockable.lock(); }
    };
}

template <class WorkItem>
void RunQueue<WorkItem>::Wait(boost::unique_lock<boost::shared_mutex>& lock) {
    scoped_unlock< boost::unique_lock<boost::shared_mutex> > wait_unlock(lock); // create before schedule_lock, destroy after schedule_lock
    boost::unique_lock<boost::shared_mutex> schedule_lock(m_schedule_mutex); // create after wait_unlock, destroy before wait_unlock

    while (true) {
        unsigned total_workload,  scheduleable_workload;

        GetTotalWorkload(total_workload, scheduleable_workload);
        if (total_workload == 0) break;

        if (scheduleable_workload > 0) m_work_available.notify_one();
        m_work_done.wait(schedule_lock); // m_schedule_mutex is unlocked by wait() while the thread waits
    }
}

template <class WorkItem>
bool RunQueue<WorkItem>::Schedule(ThreadQueue<WorkItem>& requested_by) {
    boost::unique_lock<boost::shared_mutex> schedule_lock(m_schedule_mutex);
    unsigned total_workload;

    while (true) {
        bool new_work_scheduled;

        while (true) {
            unsigned scheduleable_workload;

            // did we get work while waiting for the lock? -> early out
            if (requested_by.schedule_queue_size > 0U)
                return m_terminate;

            GetTotalWorkload(total_workload, scheduleable_workload);
            // no point scheduling when there isn't at least one scheduleable work item
            if (scheduleable_workload > 0) 
                break;
            if (total_workload == 0)
                m_work_done.notify_all();

            // all done here, but never wait for work when we shall terminate
            if (m_terminate)
                return true;

            m_work_available.wait(schedule_lock); // m_schedule_mutex is unlocked by wait() while the thread waits
        }

        new_work_scheduled = false;
        for (unsigned balancing_threshold = total_workload; true; balancing_threshold += m_transfer_queue_size) {
            unsigned threads_to_be_scheduled = m_thread_queues.size();

            for (std::shared_ptr<ThreadQueue<WorkItem>> thread_queue : m_thread_queues) {
                const unsigned         old_schedule_queue_size = thread_queue->schedule_queue_size;
                const unsigned         running_queue_size      = thread_queue->running_queue_size; // running_queue_size might be accessed concurrently
                const unsigned         old_transfer_queue_size = m_transfer_queue_size;
                const unsigned         target_thread_workload  = (balancing_threshold + threads_to_be_scheduled - 1) / threads_to_be_scheduled; // round up in order to avoid infinite loop when the last thread cant take more work
                const unsigned         new_schedule_queue_size = std::min(std::max(running_queue_size, target_thread_workload) - running_queue_size, old_schedule_queue_size + old_transfer_queue_size);
                std::vector<WorkItem*>& schedule_queue         = *(thread_queue->schedule_queue);
                typename std::vector<WorkItem*>::iterator schedule_queue_begin = schedule_queue.begin();
                typename std::vector<WorkItem*>::iterator transfer_queue_begin = m_transfer_queue.begin();

                if (new_schedule_queue_size < old_schedule_queue_size) {
                    m_transfer_queue_size += (old_schedule_queue_size - new_schedule_queue_size);
                    if (m_transfer_queue_size > m_transfer_queue.size())
                        m_transfer_queue.resize(m_transfer_queue_size);
                    schedule_queue_begin = schedule_queue.begin();
                    transfer_queue_begin = m_transfer_queue.begin();

                    std::copy(schedule_queue_begin + new_schedule_queue_size, schedule_queue_begin + old_schedule_queue_size, transfer_queue_begin + old_transfer_queue_size);
                } else if (new_schedule_queue_size > old_schedule_queue_size) {
                    new_work_scheduled = true;

                    m_transfer_queue_size -= (new_schedule_queue_size - old_schedule_queue_size);
                    if (new_schedule_queue_size > schedule_queue.size())
                        schedule_queue.resize(new_schedule_queue_size);
                    schedule_queue_begin = schedule_queue.begin();
                    transfer_queue_begin = m_transfer_queue.begin();

                    std::copy(transfer_queue_begin + m_transfer_queue_size, transfer_queue_begin + old_transfer_queue_size, schedule_queue_begin + old_schedule_queue_size);
                }
                thread_queue->schedule_queue_size = new_schedule_queue_size;
                --threads_to_be_scheduled;
            }
            if (m_transfer_queue_size == 0U) break;
        }
        // did anay thread receive new work? -> notify it by notifying all
        if (new_work_scheduled)
            m_work_available.notify_all();

        // did we manage to schedule work for ourselves? -> done here
        if (requested_by.schedule_queue_size > 0U)
            return m_terminate;

        // nothing more we can do here, but never wait for work when we shall terminate
        if (m_terminate)
            return true;

        m_work_available.wait(schedule_lock); // m_schedule_mutex is unlocked by wait() while the thread waits
    }   // new work -> start over and try if we got or can get some of it

    return false; // should be unreachable
}

template <class WorkItem>
void RunQueue<WorkItem>::GetTotalWorkload(unsigned& total_workload, unsigned& scheduleable_workload) {
    total_workload = scheduleable_workload = m_transfer_queue_size;

    for (std::shared_ptr<ThreadQueue<WorkItem>> thread_queue : m_thread_queues) {
        scheduleable_workload += thread_queue->schedule_queue_size;
        total_workload += thread_queue->schedule_queue_size + thread_queue->running_queue_size; // running_queue_size might be accessed concurrently
    }
}
