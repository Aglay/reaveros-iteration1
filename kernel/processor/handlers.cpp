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

#include <processor/handlers.h>

// TODO: proper handling of errors, better screen of death with printing faulting address, registers and stuff

void processor::exceptions::reserved(processor::idt::irq_context ctx)
{
    if (ctx.cs & 3)
    {
        // resolve problem by - probably - killing a process
        
        return;
    }
    
    PANIC("Reserved exception fired");
    
    return;
}

void processor::exceptions::divide_error(processor::idt::irq_context ctx)
{
    if (ctx.cs & 3)
    {
        // resolve problem by killing a process
        
        return;
    }
    
    PANIC("Divide by zero exception in kernel code");
    
    return;
}

void processor::exceptions::bound_range(processor::idt::irq_context ctx)
{
    if (ctx.cs & 3)
    {
        // signal the process with rose::signal::bound_exceeded
        
        return;
    }
    
    PANIC("Bound range exceeded in kernel code");
    
    return;
}

void processor::exceptions::invalid_opcode(processor::idt::irq_context ctx)
{
    if (ctx.cs & 3)
    {
        // kill the process
        
        return;
    }
    
    PANIC("Invalid opcode in kernel code");
    
    return;
}

void processor::exceptions::no_coprocessor(processor::idt::irq_context)
{
    PANIC("FPU not found");
    
    return;
}

void processor::exceptions::invalid_tss(processor::idt::irq_context_error)
{
    PANIC("Invalid TSS exception");
    
    return;
}

void processor::exceptions::segment_not_present(processor::idt::irq_context_error)
{
    PANIC("Segment not present exception");
    
    return;
}

void processor::exceptions::stack_fault(processor::idt::irq_context_error ctx)
{
    if (ctx.cs & 3)
    {
        // kill the process
        
        return;
    }
    
    PANIC("Stack segment fault in kernel code");
    
    return;
}

void processor::exceptions::protection_fault(processor::idt::irq_context_error ctx)
{
    if (ctx.cs & 3)
    {
        // kill the process
        
        return;
    }
    
    PANIC("General protection fault in kernel code");
    
    return;
}

void processor::exceptions::page_fault(processor::idt::irq_context_error ctx)
{
    if (ctx.cs & 3)
    {
        // inform VMM
        
        return;
    }
    
    // TODO: #PF logic for sparse arrays
    
    PANIC("Page fault in kernel code");
}

void processor::exceptions::fpu_error(processor::idt::irq_context ctx)
{
    if (ctx.cs & 3)
    {
        // kill the process
        
        return;
    }
    
    PANIC("FPU error in kernel code");
    
    return;
}

void processor::exceptions::alignment_check(processor::idt::irq_context)
{
    PANIC("Unexpected alignment check exception");
    
    return;
}

void processor::exceptions::simd_exception(processor::idt::irq_context ctx)
{
    if (ctx.cs & 3)
    {
        // kill the process
        
        return;
    }
    
    PANIC("SIMD exception in kernel code");
    
    return;
}

void processor::exceptions::breakpoint(processor::idt::irq_context)
{
    PANIC("Unexpected breakpoint exception");
    
    return;
}

void processor::exceptions::overflow(processor::idt::irq_context ctx)
{
    if (ctx.cs & 3)
    {
        // kill the process
        
        return;
    }
    
    PANIC("Overflow in kernel code");
    
    return;
}

void processor::exceptions::double_fault(processor::idt::irq_context_error)
{
    PANIC("Double fault");
    
    return;
}

void processor::exceptions::machine_check(processor::idt::irq_context)
{
    PANIC("Machine check exception");
    
    return;
}

void processor::exceptions::non_maskable(processor::idt::irq_context)
{
    PANIC("Non maskable interrupt");
}