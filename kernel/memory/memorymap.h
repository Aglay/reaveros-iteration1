/**
 * ReaverOS
 * kernel/memory/memorymap.h
 * Memory map class.
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

#ifndef __rose_kernel_memory_memorymap_h__
#define __rose_kernel_memory_memorymap_h__

#include "../types.h"
#include "../lib/string.h"

namespace Memory
{
    class MemoryMapEntry
    {
    public:
        MemoryMapEntry() {}
        MemoryMapEntry(uint64 b, uint64 l, uint32 t)
                : m_iBase(b), m_iLength(l), m_iType(t)
        {
        }

        MemoryMapEntry & operator=(const MemoryMapEntry & rhs)
        {
            this->m_iBase = rhs.m_iBase;
            this->m_iLength = rhs.m_iLength;
            this->m_iType = rhs.m_iType;

            return *this;
        }

        String TypeDescription();
        inline uint32 & Type()
        {
            return (uint32 &)this->m_iType;
        }
        inline uint64 & Base()
        {
            return (uint64 &)this->m_iBase;
        }
        inline uint64 & Length()
        {
            return (uint64 &)this->m_iLength;
        }
        inline uint64 End()
        {
            return this->m_iBase + this->m_iLength;
        }

    private:
        uint64 m_iBase;
        uint64 m_iLength;
        uint32 m_iType;
        uint32 ACPI30;
    } __attribute__((packed));

    class MemoryMap
    {
    public:
        MemoryMap(MemoryMapEntry *, uint32);

        void PrintMemoryMap();
        
        uint64 CountUsableMemory();
        uint32 GetMemoryType(uint64);
        String GetMemoryTypeDescription(uint64);
        
        MemoryMapEntry * GetEntries();
        uint32 GetNumberOfEntries();

    private:
        MemoryMapEntry * m_pEntries;
        uint32 m_iSize;
    };
}

#endif