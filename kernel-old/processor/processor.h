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

#pragma once

#include <memory/x64paging.h>
#include <processor/current_core.h>

namespace processor
{
    bool ready();

    extern "C" memory::x64::pml4 * get_cr3();
    extern "C" void reload_cr3();

    class ioapic;
    class core;

    void initialize();
    extern "C" void ap_initialize();
    ioapic & get_ioapic(uint8_t);
    uint8_t translate_isa(uint8_t);

    using current_core::ipis;
    using current_core::broadcast_types;

    void ipi(core *, ipis, uint8_t = 0);
    void broadcast(broadcast_types, ipis, uint8_t = 0);

    processor::core * get_core(uint64_t);
}