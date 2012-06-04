/**
 * ReaverOS
 * kernel/memory/memory.h
 * Main memory subsystem header.
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

#ifndef _rose_kernel_memory_memory_h_
#define _rose_kernel_memory_memory_h_

#include "../types.h"

namespace Lib
{
    class Stack;
}

namespace Paging
{
    class PML4;
    class PageDirectory;
}

namespace Memory
{
    class MemoryMapEntry;
    class MemoryMap;
    class Heap;
    
    void PreInitialize(void *);
    void Initialize(Memory::MemoryMapEntry *, uint32);

    void RemapKernel();

    void AlignPlacementToPage();
    void * AlignToNextPage(uint64);

    template<typename T>
    void Zero(T * ptr, uint32 iCount = 1)
    {
        uint32 iSize = sizeof(T) * iCount;
        uint8 * p = (uint8 *)ptr;

        bool flag = false;
        
        if (p == (void *)0xffffffff81419000)
        {
            flag = true;
        }

        for (uint64 i = 0; i < iSize; i += 4)
        {
            if (flag && iSize - i < 10)
            {
                asm volatile ("mov %0, %%rax" :: "r"(p + i) : "%rax");
                dbg;
            }
            
            *(uint32 *)(p + i) = 0;
        }

        p += iSize - iSize % 4;

        for (uint64 i = 0; i < iSize % 4; i++)
        {
            p[i] = 0;
        }
    }

    template<typename T>
    void Copy(T * src, T * dest, uint32 iCount = 1)
    {
        uint32 iSize = sizeof(T) * iCount;
        
        uint8 * ps = (uint8 *)src;
        uint8 * pd = (uint8 *)dest;
        
        for (uint64 i = 0; i < iSize; i += 4)
        {
            *(uint32 *)(pd + i) = *(uint32 *)(ps + i);
        }
        
        ps += iSize - iSize % 4;
        pd += iSize - iSize % 4;
        
        for (uint64 i = 0; i < iSize % 4; i++)
        {
            pd[i] = ps[i];
        }
    }

    extern void * pPlacementAddress;
    extern Memory::MemoryMap * pMemoryMap;
    extern Memory::Heap * KernelHeap;
    extern Paging::PML4 * KernelPML4;
    extern Paging::PageDirectory * KernelSpace[2];
    extern uint64 StackStart;
    extern Lib::Stack * Pages;
}

#endif