/**
 * ReaverOS
 * kernel/processor/synchronization.h
 * Basic synchronization structures.
 */

/**
 * Reaver Project OS, Rose License
 *
 * Copyright (C) 2011-2012 Reaver Project Team:
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

#ifndef __rose_kernel_processor_synchronization_h__
#define __rose_kernel_processor_synchronization_h__

#include "../types.h"
#include "processor.h"

namespace Processor
{
    class LockPrimitive
    {
    public:
        LockPrimitive() {}
        virtual ~LockPrimitive() {}
        
        virtual void Lock() = 0;
        virtual void Unlock() = 0;
    };
    
    typedef LockPrimitive Lock;
    
    class Spinlock : public LockPrimitive
    {
    public:
        Spinlock();
        virtual ~Spinlock();
        
        virtual void Lock();
        virtual void Unlock();
    private:
        uint8 m_iLock;
    };
    
    class Corelock : public LockPrimitive
    {
    public:
        Corelock();
        virtual ~Corelock();
        
        virtual void Lock();
        virtual void Unlock();
        
        Processor::SMP::Core * GetOwner();
    private:
        Spinlock m_internal;
        Processor::SMP::Core * m_pCore;
        uint64 m_iCount;
    };
}

#endif