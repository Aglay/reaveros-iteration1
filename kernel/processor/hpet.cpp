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

#include <processor/hpet.h>
#include <acpi/acpi.h>
#include <screen/screen.h>

namespace
{
    processor::hpet::timer * _timers = nullptr;
    uint64_t _num_timers = 0;

    enum _hpet_registers
    {
        _general_capabilities = 0x0,
        _general_configuration = 0x10,
        _interrupt_status = 0x20,
        _main_counter = 0xF0
    };

    uint64_t _timer_configuration(uint8_t i)
    {
        return 0x100 + 0x20 * i;
    }

    uint64_t _timer_comparator(uint8_t i)
    {
        return 0x108 + 0x20 * i;
    }

    uint64_t _fsb_route(uint8_t i)
    {
        return 0x110 + 0x20 * i;
    }
}

void processor::hpet::initialize()
{
    acpi::parse_hpet(_timers, _num_timers);

    if (_num_timers)
    {
        processor::set_high_precision_timer(_timers);
    }
}

bool processor::hpet::ready()
{
    return _num_timers;
}

processor::hpet::timer::timer(uint8_t number, pci_vendor_t pci_vendor, uint64_t address, uint8_t counter_size,
    uint8_t comparators, uint16_t minimum_tick, uint8_t page_protection) : _number{ number }, _pci_vendor{ pci_vendor.vendor },
    _size{ (uint8_t)(32 + 32 * counter_size) }, _comparator_count{ comparators }, _minimum_tick{ minimum_tick },
    _page_protection{ page_protection }, _register{ address }
{
    _register(_general_configuration, _register(_general_configuration) | 1);

    for (uint8_t i = 0; i < _comparator_count; ++i)
    {
        new (_comparators + i) comparator{ this, i };
    }

    uint64_t period = _register(_general_capabilities) >> 32;
    _frequency = 1000000000000000ull / period;

    screen::debug("\nDetected HPET counter period: ", period);
    screen::debug("\nDetected HPET counter frequency: ", _frequency);
}

processor::timer_event_handle processor::hpet::timer::one_shot(uint64_t time, processor::timer_handler handler, uint64_t param)
{
    uint64_t min = _comparators[0].get_usage();
    uint64_t min_idx = 0;

    for (uint8_t i = 1; i < _comparator_count; ++i)
    {
        if (_comparators[i].valid() && _comparators[i].get_usage() < min)
        {
            min_idx = i;
            min = _comparators[i].get_usage();
        }
    }

    return _comparators[min_idx].periodic(time, handler, param);
}

processor::timer_event_handle processor::hpet::timer::periodic(uint64_t period, processor::timer_handler handler, uint64_t param)
{
    uint64_t min = _comparators[0].get_usage();
    uint64_t min_idx = 0;

    for (uint8_t i = 1; i < _comparator_count; ++i)
    {
        if (_comparators[i].valid() &&_comparators[i].get_usage() < min)
        {
            min_idx = i;
            min = _comparators[i].get_usage();
        }
    }

    return _comparators[min_idx].periodic(period, handler, param);
}

void processor::hpet::timer::cancel(uint64_t)
{
    NEVER;
}

processor::hpet::comparator::comparator() : _parent{}
{
}

processor::hpet::comparator::comparator(processor::timer * parent, uint8_t index) : _parent{ parent }, _index{ index },
    _active_timers{}, _free_descriptors{}
{
    TODO;
}

processor::timer_event_handle processor::hpet::comparator::one_shot(uint64_t time, processor::timer_handler handler, uint64_t param)
{
    utils::interrupt_lock _;
    auto __ = utils::make_unique_lock(_lock);

    ++_usage;

    TODO;

    return {};
}

processor::timer_event_handle processor::hpet::comparator::periodic(uint64_t period, processor::timer_handler handler, uint64_t param)
{
    utils::interrupt_lock _;
    auto __ = utils::make_unique_lock(_lock);

    _usage += 100;

    TODO;

    return {};
}

void processor::hpet::comparator::cancel(uint64_t )
{
    TODO;
}