/**
 * ReaverOS
 * kernel/memory/paging.h
 * Paging structures definition.
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

#ifndef __rose_kernel_memory_paging_h__
#define __rose_kernel_memory_paging_h__

#include "../types.h"

namespace Paging
{
    struct PageTableEntry
    {
        uint64 Present:1;
        uint64 ReadWrite:1;
        uint64 User:1;
        uint64 WriteThrough:1;
        uint64 CacheDisable:1;
        uint64 Accessed:1;
        uint64 Dirty:1;
        uint64 PAT:1;
        uint64 Global:1;
        uint64 Ignored:3;
        uint64 PageAddress:40;
        uint64 Ignored2:11;
        uint64 Reserved:1;
    };
    
    struct PageTable
    {
        PageTableEntry Entries[512];
    };
    
    struct PageDirectoryEntry
    {
        uint64 Present:1;
        uint64 ReadWrite:1;
        uint64 User:1;
        uint64 WriteThrough:1;
        uint64 CacheDisable:1;
        uint64 Accessed:1;
        uint64 Ignored:1;
        uint64 Reserved:1;
        uint64 Ignored2:4;
        uint64 PageTableAddress:40;
        uint64 Ignored3:11;
        uint64 Reserved2:1;
    };
    
    struct PageDirectory
    {
        PageDirectoryEntry Entries[512];
        PageTable * PageTables[512];
    };
    
    struct PageDirectoryPointerTableEntry
    {
        uint64 Present:1;
        uint64 ReadWrite:1;
        uint64 User:1;
        uint64 WriteThrough:1;
        uint64 CacheDisable:1;
        uint64 Accessed:1;
        uint64 Ignored:1;
        uint64 Reserved:1;
        uint64 Ignored2:4;
        uint64 PageDirectoryAddress:40;
        uint64 Ignored3:11;
        uint64 Reserved2:1;
    };
    
    struct PageDirectoryPointerTable
    {
        PageDirectoryPointerTableEntry Entries[512];
        PageDirectory * PageDirectories[512];
    };
    
    struct PML4Entry
    {
        uint64 Present:1;
        uint64 ReadWrite:1;
        uint64 User:1;
        uint64 WriteThrough:1;
        uint64 CacheDisable:1;
        uint64 Accessed:1;
        uint64 Ignored:1;
        uint64 Reserved:1;
        uint64 Ignored2:4;
        uint64 PDPTAddress:40;
        uint64 Ignored3:11;
        uint64 Reserved2:1;
    };
    
    struct PML4
    {
        PML4Entry Entries[512];
        PageDirectoryPointerTable * PointerTables[512];

        PML4();
        
        inline uint64 GetPhysicalAddress(uint64 pAddr)
        {
            return this->PointerTables[(pAddr >> 39) & 511]->PageDirectories[(pAddr >> 30) & 511]->
            PageTables[(pAddr >> 21) & 511]->Entries[(pAddr >> 12) & 511].PageAddress
            << 12 + pAddr % 4096;
        }

        void Map(uint64, uint64, uint64, bool = false, bool = true, bool = false, bool = false, PML4 * = 0);
    };
}

#endif