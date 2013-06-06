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

#include <processor/processor.h>
#include <processor/interrupts.h>
#include <memory/memory.h>
#include <acpi/acpi.h>
#include <processor/core.h>
#include <processor/ioapic.h>
#include <screen/screen.h>
#include <processor/current_core.h>
#include <processor/pit.h>
#include <processor/interrupt_entry.h>
#include <processor/smp.h>
#include <scheduler/scheduler.h>

namespace
{
    processor::core _cores[processor::max_cores];
    processor::ioapic _ioapics[processor::max_ioapics];

    uint64_t _num_cores = 0;
    uint64_t _num_ioapics = 0;

    processor::interrupt_entry _sources[128];

    bool _ready = false;
}

bool processor::ready()
{
    return _ready;
}

extern "C" void _load_gdt();
extern "C" void _load_idt();
extern "C" void _load_gdt_from(uint64_t);
extern "C" void _load_idt_from(uint64_t);
extern "C" processor::gdt::gdt_entry _gdt_start[];

void processor::initialize()
{
    screen::transaction();

    gdt::initialize();
    idt::initialize();

    acpi::initialize();
    acpi::parse_madt(_cores, _num_cores, _ioapics, _num_ioapics, _sources);

    for (uint64_t i = 0; i < _num_ioapics; ++i)
    {
        _ioapics[i].initialize(_sources);
    }

    // TODO: HPET

/*    hpet::initialize();

    if (!hpet::present())
    {*/
        pit::initialize();
    /*}*/

    current_core::initialize();

    screen::console._set_owner(current_core::id());

    uint64_t num_ap_cores = _num_cores - 1;
    smp::boot(_cores + 1, num_ap_cores);
    _num_cores = num_ap_cores + 1;

    memory::drop_bootloader_mapping();

    _ready = true;
}

void processor::ap_initialize()
{
    uint64_t stack = memory::stack_manager::allocate();
    memory::stack_manager::set(stack);

    current_core::initialize();
    core * current = get_core(current_core::id());

    processor::gdt::gdt_entry * _core_gdt = current->_gdt;
    processor::gdt::tss * _core_tss = &current->_tss;
    processor::gdt::gdtr * _core_gdtr = &current->_gdtr;

    gdt::ap_initialize(_core_gdtr, _core_gdt, _core_tss);
    idt::ap_initialize();

    scheduler::ap_initialize();

    memory::drop_bootloader_mapping(false);

    for (;;) ;
}

processor::ioapic & processor::get_ioapic(uint8_t irq)
{
    for (uint64_t i = 0; i < _num_ioapics; ++i)
    {
        if (irq >= _ioapics[i].begin() && irq < _ioapics[i].end())
        {
            return _ioapics[i];
        }
    }

    PANIC("Too high IRQ number requested");

    return *(ioapic *)nullptr;
}

uint8_t processor::translate_isa(uint8_t irq)
{
    if (_sources[irq])
    {
        return _sources[irq].vector();
    }

    return irq;
}

namespace
{
    processor::gdt::tss _tss;

    void _setup_gdte(uint64_t id, bool code, bool user, processor::gdt::gdt_entry * start = _gdt_start)
    {
        start[id].normal = 1;

        start[id].code = code;
        start[id].dpl = user * 3;
        start[id].long_mode = 1;
        start[id].present = 1;
        start[id].read_write = 1;
    }

    void _setup_tss(uint64_t id, processor::gdt::gdt_entry * start = _gdt_start, processor::gdt::tss * tss = &_tss)
    {
        memory::zero(&tss);

        tss->iomap = sizeof(processor::gdt::tss);

        start[id].base_low = (uint64_t)tss & 0xFFFFFF;
        start[id].base_high = ((uint64_t)tss >> 24) & 0xFF;
        *(uint32_t *)&start[id + 1] = ((uint64_t)tss >> 32) & 0xFFFFFFFF;

        start[id].limit_low = (sizeof(processor::gdt::tss) & 0xFFFF) - 1;
        start[id].limit_high = sizeof(processor::gdt::tss) >> 16;

        start[id].accessed = 1;
        start[id].code = 1;
        start[id].present = 1;
        start[id].dpl = 3;

        tss->ist1 = memory::stack_manager::allocate();
        tss->ist2 = memory::stack_manager::allocate();
        tss->ist3 = memory::stack_manager::allocate();
    }
}

void processor::gdt::initialize()
{
    memory::zero(_gdt_start, 7);

    _setup_gdte(1, true, false);
    _setup_gdte(2, false, false);
    _setup_gdte(3, true, true);
    _setup_gdte(4, false, true);
    _setup_tss(5);

    _load_gdt();
}

void processor::gdt::ap_initialize(processor::gdt::gdtr * gdtr, processor::gdt::gdt_entry * core_gdt, processor::gdt::tss * core_tss)
{
    memory::zero(core_gdt, 7);

    _setup_gdte(1, true, false, core_gdt);
    _setup_gdte(2, false, false, core_gdt);
    _setup_gdte(3, true, true, core_gdt);
    _setup_gdte(4, false, true, core_gdt);
    _setup_tss(5, core_gdt, core_tss);

    gdtr->limit = 7 * sizeof(gdt_entry) - 1;
    gdtr->address = (uint64_t)core_gdt;
    _load_gdt_from((uint64_t)gdtr);
}

void processor::ipi(processor::core * core, processor::ipis ipi, uint8_t vector)
{
    processor::current_core::ipi(core->apic_id(), ipi, vector);
}

void processor::broadcast(processor::broadcast_types target, processor::current_core::ipis ipi, uint8_t vector)
{
    processor::current_core::broadcast(target, ipi, vector);
}

processor::core * processor::get_core(uint64_t id)
{
    for (uint64_t i = 0; i < _num_cores; ++i)
    {
        if (_cores[i].apic_id() == id)
        {
            return _cores + i;
        }
    }

    PANIC("Tried to get core with nonexistent APIC ID");

    return nullptr;
}