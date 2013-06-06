/**
 * Reaver Project OS, Rose License
 *
 * Copyright (C) 2011-2013 Reaver Project Team:
 * 1. Michał "Griwes" Dominiak
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation is required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Michał "Griwes" Dominiak
 *
 **/

#include <scheduler/scheduler.h>
#include <processor/current_core.h>
#include <scheduler/thread.h>
#include <scheduler/process.h>

namespace
{
    volatile bool _initialize_aps = false;
    volatile uint64_t _in_init = 0;
    volatile uint64_t _bsp = 0;

    memory::index_stack _global_pcb_stack;
    memory::index_stack _global_tcb_stack;

    scheduler::thread_scheduler _global_scheduler;

    scheduler::thread * _create_current_thread()
    {
        return nullptr;
    }
}

void scheduler::initialize()
{
    // TODO:
    // 1. initialize PCB global and local stacks - DONE
    // 2. initialize TCB global and local stacks - DONE
    // 3. initialize thread queues for each priority
    // 4. initialize thread queues waiting on each interrupt or exception
    // 5. initialize kernel process and current thread
    // 6. initialize IPC subsystem
    // 7. set _initialize_aps - DONE
    // 8. wait for rest of the cores to finish scheduler initialization - DONE
    // 9. schedule current thread

    new ((void *)&_global_pcb_stack) memory::index_stack{memory::vm::global_pcb_stack_area, 0, 64 * 1024, max_processes};
    new ((void *)&_global_tcb_stack) memory::index_stack{memory::vm::global_tcb_stack_area, 0, 64 * 1024, max_threads};
    new ((void *)&_global_scheduler) scheduler::thread_scheduler{};

    _bsp = processor::current_core::id();
    _initialize_aps = true;

    ap_initialize(); // this looks funny here, on BSP

    while (_in_init)
    {
        processor::current_core::sleep(20000000);
    }

    _global_scheduler.add(_create_current_thread());
}

void scheduler::ap_initialize()
{
    ++_in_init;

    while (!_initialize_aps)
    {
        processor::current_core::sleep(20000000);
    }

    processor::core * current = processor::get_core(processor::current_core::id());

    new ((void *)&current->pcb_stack()) memory::index_stack{memory::vm::pcb_stack_area + processor::current_core::id()
        * memory::core_index_stack_size, &_global_pcb_stack};
    new ((void *)&current->tcb_stack()) memory::index_stack{memory::vm::tcb_stack_area + processor::current_core::id()
        * memory::core_index_stack_size, &_global_tcb_stack};

    for (uint64_t i = 0; i < 1024; ++i)
    {
        current->pcb_stack().push(_global_pcb_stack.pop());
        current->tcb_stack().push(_global_tcb_stack.pop());
        current->tcb_stack().push(_global_tcb_stack.pop());
    }

    new ((void *)&current->scheduler()) scheduler::thread_scheduler{&_global_scheduler};

    --_in_init;

    processor::current_core::stop_timer();

    if (processor::current_core::id() != _bsp)
    {
        for (;;)
        {
            asm volatile ("hlt");
        }
    }
}