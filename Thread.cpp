
#include "Thread.h"
#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
            : "=g" (ret)
            : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}
#endif
Thread::Thread()
{
/**
 * Default Constructor
 */
    _tid = MAIN_THREAD_ID;
    _state = RUNNING;
    _stack = new char[STACK_SIZE];
    sigsetjmp(_envp, 1);
    _quantum = MAIN_QUANTUMS_VALUE;
    sigemptyset(&_envp->__saved_mask);
}

Thread::Thread(int tid, thread_entry_point entry_point)
{
/**
* Constructor
*/
    _tid = tid;
    _entry_point = entry_point;
    _quantum = INITIAL_QUANTUMS_VALUE;
    _stack = new char[STACK_SIZE];
    _state = READY;
    address_t sp = (address_t) _stack + STACK_SIZE - sizeof(address_t);
    address_t pc = (address_t) _entry_point;
    sigsetjmp(_envp, 1);
    (_envp->__jmpbuf)[JB_SP] = translate_address(sp);
    (_envp->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&_envp->__saved_mask);
}

Thread::~Thread()
{
    /**
     * Distructor
     */
    delete[] _stack;
}