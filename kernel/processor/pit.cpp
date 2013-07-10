/**
 * Reaver Project OS, Rose License
 *
 * Copyright (C) 2013 Reaver Project Team:
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

#include <processor/pit.h>
#include <processor/handlers.h>
#include <screen/screen.h>

namespace
{
    processor::pit::timer * _pit = nullptr;
}

void processor::_detail::_pit_handler(processor::idt::isr_context isr, uint64_t context)
{
    ((processor::pit::timer *)context)->_handle(isr);
}

void processor::pit::initialize()
{
    new (_pit) processor::pit::timer();

    processor::set_high_precision_timer(_pit);
}

bool processor::pit::ready()
{
    return _pit;
}

processor::pit::timer::timer() : real_timer{ capabilities::dynamic, 200000 }, _int_vector{}
{
    _int_vector = allocate_isr(0);
    register_handler(_int_vector, _init_handler, (uint64_t)this);

    _one_shot(200000);
    set_isa_irq_int_vector(0, _int_vector);

    STI;
    HLT;
    CLI;

    unregister_handler(_int_vector);
    register_handler(_int_vector, _detail::_pit_handler, (uint64_t)this);

    screen::debug("\nPIT handler installed successfully, PIT state clean");
}

void processor::pit::timer::_init_handler(processor::idt::isr_context , uint64_t context)
{
}

void processor::pit::timer::_one_shot(uint64_t time)
{
    uint64_t hz = 1000000000 / time;
    uint16_t divisor = 1193180 / hz;

    screen::debug("\nSetting PIT to one-shot; time = ", time, ", freq = ", hz, ", divisor = ", divisor);

    outb(0x43, 0x30);

    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));

    uint8_t tmp = inb(0x61) & 0xFE;
    outb(0x61, tmp);
    outb(0x61, tmp | 1);
}

void processor::pit::timer::_periodic(uint64_t period)
{
    TODO;
}
