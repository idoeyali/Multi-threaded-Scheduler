
#ifndef RESOURCES_THREAD_H
#define RESOURCES_THREAD_H


#include "string"
#include "uthreads.h"
#include <cstdio>
#include <csetjmp>
#include <csignal>
#include <unistd.h>
#include <ctime>
#define MAIN_QUANTUMS_VALUE 1
#define INITIAL_QUANTUMS_VALUE 0
#define MAIN_THREAD_ID 0

enum State
{
    READY,
    BLOCKED,
    RUNNING
};

class Thread
{
private:
    int _tid;
    State _state;
    char *_stack;
    int _quantum;
    thread_entry_point _entry_point;

public:
    sigjmp_buf _envp;
    Thread();

    Thread(int tid, thread_entry_point entry_point);
    int get_quantums()
    {
        return _quantum;
    }
    int get_tid() const
    {
        return _tid;
    }
    State get_state(){return _state;}

    void set_state(State state){_state = state;}

    void increment_quantum(){_quantum++;}
    ~Thread();
};

#endif //RESOURCES_THREAD_H