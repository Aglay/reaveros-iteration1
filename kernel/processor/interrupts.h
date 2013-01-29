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

namespace processor
{
    namespace idt
    {
        struct exc_context
        {
            uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
            uint64_t rdi, rsi, rdx, rcx, rbx, rax;
            uint64_t rip, cs, rflags, rsp, ss;
        } __attribute__((packed));
        
        struct exc_context_error
        {
            uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
            uint64_t rdi, rsi, rdx, rcx, rbx, rax;
            uint64_t code;
            uint64_t rip, cs, rflags, rsp, ss;
        } __attribute__((packed));
        
        struct irq_context
        {
            uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
            uint64_t rdi, rsi, rdx, rcx, rbx, rax;
            uint64_t number;
            uint64_t rip, cs, rflags, rsp, ss;
        } __attribute__((packed)); // this is the same as previous, but is conceptually different, so let's make it distinct
                                   // type
        
        void initialize();
    }
    
    namespace irq
    {
        void initialize();
        void add_handler(uint64_t, void (*)(uint64_t));
    }
}