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

/**
 * @brief Manages the transition between threads based on the specified conditions (sleep, block, terminate).
*/
    void jump_to_threads_helper(bool sleep, bool block, bool terminate);


/**
 * @brief Blocks a ready thread with the specified thread ID.
     * Sets the thread's state to BLOCKED and adds it to the blocked threads set.
     * If the thread is in the ready queue, removes it.
*/
    void block_ready_thread(int tid);

/**
 * @brief Searches for the next available thread ID within the maximum allowed thread count.
 *
 * @return The function returns the found ID or a constant indicating that
 * the scheduler is full.
*/
    int find_next_id_available();


/**
 * @brief Terminates the thread with the given thread ID.
     * Removes the thread from the scheduler's data structures (all threads and blocked threads).
 *
 * @return The function returns 0 on success and -1 otherwise.
*/
    int terminate_thread(int tid);


/**
 * @brief Adds a new thread to the scheduler.
     * Sets the thread's state to READY and adds it to the all threads and ready threads data structures.
*/
    void add_thread(sp_thread &thread);


/**
 * @return The function returns the running thread.
*/
    sp_thread& get_running_thread()
    {
        return _running_thread;
    }


/**
 * @brief Searches for a thread with the given thread ID.
 *
 * @return The function returns a shared pointer to the found thread or
 * nullptr if not found.
*/
    sp_thread thread_found(int tid);


/**
 * @brief Resumes a blocked thread with the specified thread ID.
     * If the thread is not sleeping, adds it back to the ready threads queue.
     * Sets the thread's state to READY and removes it from the blocked threads set.
*/
    void resume_thread(int tid);


/**
 * @brief Updates the sleeping threads, decrementing their sleep time.
     * If a thread reaches the end of its sleep period, sets its state to READY and adds it back to the ready threads queue.
     * Removes completed sleep threads from the sleeping threads set.
*/
    void sleeping_threads_update();


/**
 * @brief Increment the scheduler's total quantums.
*/
    void total_quantums_increment()
    { _total_quantums++; }


/**
 * @brief Puts a thread to sleep by adding it to the sleeping threads set with a specified sleep duration.
*/
    void put_to_sleep(int tid, int quantums)
    { _sleeping_threads[tid] = quantums; }


/**
 * @brief Removes a thread from the sleeping threads set.
*/
    void remove_from_sleep(int tid);

/**
 * @brief Updates the scheduler's running thread by dequeuing the front
 * thread from the ready threads and sets its state to running.
 *
 * @return The function returns
*/
    void update_deque();


/**
 * @return The function returns the total quantums.
*/
    int get_total_quantums()
    { return _total_quantums; }


/**
 * @brief Clears all data structures in the scheduler, including ready threads, blocked threads, all threads, and sleeping threads.
*/
    void Clear();
};

#endif //RESOURCES_SCHEDULER_H