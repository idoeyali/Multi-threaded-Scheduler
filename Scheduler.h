#ifndef RESOURCES_SCHEDULER_H
#define RESOURCES_SCHEDULER_H

#include <bits/stdc++.h>
#include "iostream"
#include "Thread.h"
#define SCHEDULER_IS_FULL -1
#define THREAD_NOT_FOUND -1
#define END_TO_SLEEP 0


typedef std::shared_ptr<Thread> sp_thread;

class Scheduler
{
private:
    int _quantum_usecs;
    int _total_quantums;
    std::map<int, sp_thread> _all_tid;
    std::deque<sp_thread> _ready_threads;
    std::map<int, sp_thread> _blocked_threads;
    std::map<int, int> _sleeping_threads;
    sp_thread _running_thread;


public:
    Scheduler(int quantum_usecs)
    {
        _total_quantums = MAIN_QUANTUMS_VALUE;
        _quantum_usecs = quantum_usecs;
        auto main_thread = std::make_shared<Thread>();
        _running_thread = main_thread;
        _all_tid.insert({main_thread->get_tid(), main_thread});
    }

    void jump_to_threads_helper(bool sleep, bool block, bool terminate);

    void block_ready_thread(int tid);

    int find_next_id_available();

    int terminate_thread(int tid);

    void add_thread(sp_thread &thread);

    sp_thread& get_running_thread()
    {
        return _running_thread;
    }

    sp_thread thread_found(int tid);

    void resume_thread(int tid);

    void sleeping_threads_update();

    void total_quantums_increment()
    { _total_quantums++; }

    void put_to_sleep(int tid, int quantums)
    { _sleeping_threads[tid] = quantums; }

    void remove_from_sleep(int tid);

    void update_deque();

    int get_total_quantums()
    { return _total_quantums; }

    void Clear();
};

#endif //RESOURCES_SCHEDULER_H