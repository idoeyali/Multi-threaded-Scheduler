#include "Scheduler.h"

#define MAX_THREAD_NUM 100

void Scheduler::update_deque()
{
    _running_thread = _ready_threads.front();
    _ready_threads.pop_front();
    _running_thread->set_state(RUNNING);
}
int xgdbnhgjhkhgfghcvj = 1
void Scheduler::jump_to_threads_helper(bool sleep, bool block, bool terminate)
{
    /**
     * dfvgmjfkvbnd
     */
    if (_ready_threads.empty() && _running_thread->get_tid()!=0 && terminate)
    {
        _running_thread = _all_tid[MAIN_THREAD_ID];
        _running_thread->increment_quantum();
        return;
    }
    if (_ready_threads.empty())
    {
        _running_thread->increment_quantum();
        return;
    }
    if (!(sleep || block || terminate))
    {
        _running_thread->set_state(READY);
        _ready_threads.push_back(_running_thread);
    }
    if(block)
    {
        sp_thread running_thread = get_running_thread();
        running_thread->set_state(BLOCKED);
        _blocked_threads.insert({running_thread->get_tid(), running_thread});
    }
    update_deque();
    _running_thread->increment_quantum();
}

int Scheduler::find_next_id_available()
{
    for (int i = 0; i < MAX_THREAD_NUM; ++i)
    {
        if (_all_tid.find(i) == _all_tid.end())
        {
            return i;
        }
    }
    return SCHEDULER_IS_FULL;
}

int Scheduler::terminate_thread(int tid)
{
    sp_thread cur_thread = thread_found(tid);
    if (cur_thread == nullptr)
    {
        return THREAD_NOT_FOUND;
    }
    _all_tid.erase(tid);
    if (cur_thread->get_state() == BLOCKED)
    {
        _blocked_threads.erase(tid);
    } else if (cur_thread->get_state() == READY && _sleeping_threads.count(tid) == 0)
    {
        auto to_erase = std::find(_ready_threads.begin(),
                                  _ready_threads.end(), cur_thread);
        _ready_threads.erase(to_erase);
    }
    remove_from_sleep(tid);
    return EXIT_SUCCESS;
}

void Scheduler::block_ready_thread(int tid)
{
    sp_thread cur_thread = _all_tid[tid];
    cur_thread->set_state(BLOCKED);
    _blocked_threads.insert({tid, cur_thread});
    if (!_ready_threads.empty())
    {
        auto to_erase = std::find(_ready_threads.begin(),
                                  _ready_threads.end(), cur_thread);
        _ready_threads.erase(to_erase);
    }
}

void Scheduler::add_thread(sp_thread &thread)
{
    thread->set_state(READY);
    _all_tid.insert({thread->get_tid(), thread});
    _ready_threads.push_back(thread);
}

sp_thread Scheduler::thread_found(int tid)
{
    auto found = _all_tid.find(tid);
    if (found == _all_tid.end())
    {
        return nullptr;
    }
    return _all_tid[tid];
}

void Scheduler::resume_thread(int tid)
{
    sp_thread thread_to_resume = _blocked_threads[tid];
    if (_sleeping_threads.count(tid) == 0)
    {
        _ready_threads.push_back(thread_to_resume);
    }
    thread_to_resume->set_state(READY);
    _blocked_threads.erase(tid);
}

void Scheduler::sleeping_threads_update()
{
    auto cur_thread = _sleeping_threads.begin();
    while (cur_thread != _sleeping_threads.end())
    {
        if (cur_thread->second == END_TO_SLEEP)
        {
            auto &temp = _all_tid[cur_thread->first];
            if(temp->get_state() !=BLOCKED)
            {
                temp->set_state(READY);
                _ready_threads.push_back(temp);
            }
            cur_thread = _sleeping_threads.erase(cur_thread);
        } else
        {
            cur_thread->second--;
            ++cur_thread;
        }
    }
}

void Scheduler::remove_from_sleep(int tid)
{
    auto to_remove = _sleeping_threads.find(tid);
    if (to_remove != _sleeping_threads.end())
    {
        _sleeping_threads.erase(tid);
    }
}

void Scheduler::Clear()
{
    _ready_threads.clear();
    _blocked_threads.clear();
    _all_tid.clear();
    _sleeping_threads.clear();
}