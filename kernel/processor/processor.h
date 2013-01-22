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

namespace processor
{
    constexpr uint64_t max_cores = 512;
    constexpr uint64_t max_ioapics = 16;
    
    extern "C" memory::x64::pml4 * get_cr3();
    extern "C" void reload_cr3();
    
    namespace gdt
    {
        struct gdt_entry
        {
            uint64_t limit_low:16;
            uint64_t base_low:24;
            uint64_t accessed:1;
            uint64_t read_write:1;
            uint64_t conforming:1;
            uint64_t code:1;
            uint64_t normal:1;
            uint64_t dpl:2;
            uint64_t present:1;
            uint64_t limit_high:4;
            uint64_t available:1;
            uint64_t long_mode:1;
            uint64_t big:1;
            uint64_t granularity:1;
            uint64_t base_high:8;
        } __attribute__((packed));
        
        struct tss
        {
            uint32_t reserved;
            uint64_t rsp0;
            uint64_t rsp1;
            uint64_t rsp2;
            uint64_t rsp3;
            uint64_t reserved2;
            uint64_t ist1;
            uint64_t ist2;
            uint64_t ist3;
            uint64_t ist4;
            uint64_t ist5;
            uint64_t ist6;
            uint64_t ist7;
            uint64_t reserved3;
            uint16_t reserved4;
            uint16_t iomap;
        } __attribute__((packed));
        
        void initialize();
    }
    
    void initialize();
    
    inline void invlpg(uint64_t addr)
    {
        asm volatile ("invlpg (%0)" :: "r"(addr) : "memory");
    }
}