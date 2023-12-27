//
// Created by user on 20/04/2023.
//
#include "uthreads.h"
#include "Scheduler.h"
#include <csignal>
#include "sys/time.h"

#define FAILURE -1
#define QUANTUM_ERROR "The quantum should be un-positive number."
#define SIGACTION_ERROR "Sigaction error."
#define SET_TIMER_ERROR "Setitimer error."
#define LIBRARY_FULL_ERROR "There is no free space in the library."
#define INVALID_INPUT "The input is invalid."
#define INVALID_ID "The thread's ID is not existed in the library."
#define BLOCK_MAIN_ERROR "You can't blocked the main thread."
#define SLEEP_MAIN_ERROR "You can't sleep the main thread."
#define SIGPROCMASK_ERROR "Sigprocmask error."
#define SIGADDSET_ERROR "Sigaddset error."
struct sigaction sa = {};
struct itimerval timer;
sigset_t blocked_signal;
std::shared_ptr<Scheduler> scheduler;

int block_signals()
{
    if (sigprocmask(SIG_BLOCK, &blocked_signal, nullptr))
    {
        fprintf(stderr, "system error: "  SIGPROCMASK_ERROR "\n");
        return FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
unblock_signals - helper function to unblock signals
@return EXIT_SUCCESS if the signals were successfully unblocked, FAILURE otherwise
*/
int unblock_signals()
{
        if (sigprocmask(SIG_UNBLOCK, &blocked_signal, nullptr))
        {
            fprintf(stderr, "system error: "  SIGPROCMASK_ERROR "\n");
            return FAILURE;
        }
        return EXIT_SUCCESS;
}

void jump_to_thread(bool sleep, bool block, bool terminate)
{
    scheduler->sleeping_threads_update();
    scheduler->total_quantums_increment();
    scheduler->jump_to_threads_helper(sleep, block, terminate);
    if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) < 0)
    {
        fprintf(stderr, "system error: " SET_TIMER_ERROR "\n");
    }
    unblock_signals();
    siglongjmp(scheduler->get_running_thread()->_envp, 1);
}

void timer_handler(int signal)
{
    int ret_val = sigsetjmp(scheduler->get_running_thread()->_envp, 1);
    if (ret_val == 0)
    {
        jump_to_thread(false, false, false);
    }
}

int uthread_init(int quantum_usecs)
{
    if (quantum_usecs <= INITIAL_QUANTUMS_VALUE)
    {
        fprintf(stderr, "thread library error: " QUANTUM_ERROR "\n");
        return FAILURE;
    }
    scheduler = std::make_shared<Scheduler>(quantum_usecs);
    sa.sa_handler = &timer_handler;
    if (sigaction(SIGVTALRM, &sa, nullptr) < 0)
    {
        fprintf(stderr, "system error: " SIGACTION_ERROR "\n");
        return FAILURE;
    }
    if (sigaddset(&blocked_signal, SIGVTALRM) < 0)
    {
        fprintf(stderr, "system error: " SIGADDSET_ERROR "\n");
        return FAILURE;
    }
    // Configure the timer to expire after 1 sec... */
    timer.it_value.tv_sec = 0;        // first time interval, seconds part
    timer.it_value.tv_usec = quantum_usecs;        // first time interval, microseconds part
    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = 0;    // following timeincr intervals, seconds part
    timer.it_interval.tv_usec = quantum_usecs;
    if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) < 0)
    {
        fprintf(stderr, "system error: " SET_TIMER_ERROR "\n");
        return FAILURE;
    }
    return EXIT_SUCCESS;
}

int uthread_spawn(thread_entry_point entry_point)
{
    block_signals();
    if (!entry_point)
    {
        fprintf(stderr, "thread library error: " INVALID_INPUT "\n");
        unblock_signals();
        return FAILURE;
    }
    int new_id = scheduler->find_next_id_available();
    if (new_id == FAILURE)
    {
        fprintf(stderr, "thread library error: " LIBRARY_FULL_ERROR "\n");
        unblock_signals();
        return FAILURE;
    }
    auto new_thread = std::make_shared<Thread>(new_id, entry_point);
    scheduler->add_thread(new_thread);
    unblock_signals();
    return new_id;
}

int uthread_terminate(int tid)
{
    block_signals();
    if (tid == MAIN_THREAD_ID)
    {
        unblock_signals();
        scheduler->Clear();
        exit(0);
    }
    if (scheduler->terminate_thread(tid) == FAILURE)
    {
        fprintf(stderr, "thread library error: " INVALID_ID "\n");
        unblock_signals();
        return FAILURE;
    }
    if (uthread_get_tid() == tid)
    {
        unblock_signals();
        jump_to_thread(false, false, true);
    }
    else
    {
        unblock_signals();
    }
    return EXIT_SUCCESS;
}

int uthread_block(int tid)
{
    if (tid == MAIN_THREAD_ID)
    {
        fprintf(stderr, "thread library error: " BLOCK_MAIN_ERROR "\n");
        return FAILURE;
    }
    block_signals();
    sp_thread cur_thread = scheduler->thread_found(tid);
    if (cur_thread == nullptr)
    {
        fprintf(stderr, "thread library error: " INVALID_ID "\n");
        unblock_signals();
        return FAILURE;
    }
    if (cur_thread->get_state() == BLOCKED)
    {
        unblock_signals();
        return EXIT_SUCCESS; }
    if (uthread_get_tid() == tid)
    {
        unblock_signals();
        int ret_val = sigsetjmp(cur_thread->_envp, 1);
        if (ret_val == 0)
        {
            jump_to_thread(false, true, false);
        }
        return EXIT_SUCCESS;
    }
    else
    {
        unblock_signals();
    }
    scheduler->block_ready_thread(tid);
    return EXIT_SUCCESS;
}

int uthread_resume(int tid)
{
    block_signals();
    sp_thread thread_to_resume = scheduler->thread_found(tid);
    if (thread_to_resume == nullptr)
    {
        fprintf(stderr, "thread library error: " INVALID_ID "\n");
        unblock_signals();
        return FAILURE;
    }
    if (thread_to_resume->get_state() == BLOCKED)
    {
        scheduler->resume_thread(tid);
    }
    unblock_signals();
    return EXIT_SUCCESS;
}

int uthread_sleep(int num_quantums)
{
    block_signals();
    int running_tid = uthread_get_tid();
    if (running_tid == MAIN_THREAD_ID)
    {
        fprintf(stderr, "thread library error: " SLEEP_MAIN_ERROR "\n");
        unblock_signals();
        return FAILURE;
    }
    scheduler->put_to_sleep(running_tid, num_quantums);
    unblock_signals();
    int ret_val = sigsetjmp(scheduler->get_running_thread()->_envp, 1);
    if (ret_val == 0)
    {
        jump_to_thread(true, false, false);
    }
    return EXIT_SUCCESS;
}

int uthread_get_tid()
{
    return scheduler->get_running_thread()->get_tid();
}

int uthread_get_total_quantums()
{
    return scheduler->get_total_quantums();
}

int uthread_get_quantums(int tid)
{
    block_signals();
    sp_thread thread = scheduler->thread_found(tid);
    if (thread == nullptr)
    {
        fprintf(stderr, "thread library error: " INVALID_ID "\n");
        return FAILURE;
    }
    unblock_signals();
    return thread->get_quantums();
}
