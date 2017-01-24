#ifndef _Run_Queue_h_
#define _Run_Queue_h_

#include <boost/noncopyable.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <vector>

template <class WorkItem>
class RunQueue;

template <class WorkItem>
struct ThreadQueue : public boost::noncopyable {
    RunQueue<WorkItem>*     global_queue;
    volatile unsigned       running_queue_size;
    volatile unsigned       schedule_queue_size;
    std::vector<WorkItem*>* running_queue;
    std::vector<WorkItem*>* schedule_queue;
    std::vector<WorkItem*>  work_queue_1;
    std::vector<WorkItem*>  work_queue_2;
    boost::thread           thread;

    ThreadQueue(RunQueue<WorkItem>* the_global_queue);
    void operator ()();
};

template <class WorkItem>
class RunQueue : public boost::noncopyable {
public:
    RunQueue(unsigned n_threads);
    ~RunQueue();
    void AddWork(WorkItem* item);
    void Wait(boost::unique_lock<boost::shared_mutex>& lock);

private:
    volatile bool                   m_terminate;
    boost::shared_mutex             m_schedule_mutex;
    boost::condition_variable_any   m_work_available;
    boost::condition_variable_any   m_work_done;
    std::vector<std::shared_ptr<ThreadQueue<WorkItem>>> m_thread_queues;
    std::vector<WorkItem*>          m_transfer_queue;
    volatile unsigned               m_transfer_queue_size;

    friend struct ThreadQueue<WorkItem>;
    void GetTotalWorkload(unsigned& total_workload, unsigned& scheduleable_workload);
    bool Schedule(ThreadQueue<WorkItem>& requested_by);
};

#include "RunQueue.tcc"

#endif // _Run_Queue_h_
