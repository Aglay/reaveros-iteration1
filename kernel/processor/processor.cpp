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

#include <processor/processor.h>
#include <memory/memory.h>
#include <processor/interrupt_entry.h>
#include <processor/gdt.h>
#include <processor/idt.h>
#include <acpi/acpi.h>
#include <processor/ioapic.h>
#include <processor/hpet.h>
#include <processor/handlers.h>

namespace
{
    uint64_t _lapic_base;

    processor::core * _cores;
    processor::ioapic * _ioapics;

    uint64_t _num_cores;
    uint64_t _num_ioapics;

    processor::interrupt_entry _sources[128];

    bool _ready = false;
}

bool processor::ready()
{
    return _ready;
}

uint64_t processor::get_lapic_base()
{
    return _lapic_base;
}

uint8_t processor::translate_isa(uint8_t irq)
{
    if (_sources[irq])
    {
        return _sources[irq].vector();
    }

    return irq;
}

void processor::initialize()
{
    _lapic_base = memory::vm::allocate_address_range(4096);

    gdt::initialize();
    idt::initialize();

    initialize_exceptions();

    acpi::initialize();

    acpi::parse_madt(_cores, _num_cores, _ioapics, _num_ioapics, _sources);

    for (uint64_t i = 0; i < _num_ioapics; ++i)
    {
        _ioapics[i].initialize(_sources);
    }

    hpet::initialize();

/*    if (!hpet::ready())
    {
        pit::initialize();
    }

    lapic::initialize();
    smp::boot(_cores + 1, _num_cores - 1);

    memory::drop_bootloader_mapping();

    _ready = true;*/
}
