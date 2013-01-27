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

#include <memory/vm.h>

namespace processor
{
    class ioapic
    {
    public:
        ioapic() : _is_valid(false), _is_nmi_valid(false) {}
        
        ioapic(uint32_t apic_id, uint32_t base_vector, uint64_t base_address) : _apic_id(apic_id), _base_vector_number(base_vector),
            _base_address(base_address), _is_valid(true), _is_nmi_valid(false)
        {
            memory::vm::map_multiple(memory::vm::ioapic_area + apic_id * 4 * 1024, memory::vm::ioapic_area + (apic_id + 1) * 4 * 1024,
                _base_address);
            
            _base_address = memory::vm::ioapic_area + apic_id * 4 * 1024;
            
            _size = ((_read_register(1) >> 16) & ~(1 << 8)) + 1;
        }
        
        bool set_global_nmi(uint32_t vector, uint32_t flags)
        {
            if (vector >= _base_vector_number && vector < _base_vector_number + _size)
            {
                _global_nmi_vector = vector;
                _global_nmi_flags = flags;
            
                _is_nmi_valid = true;
            }
            
            return _is_nmi_valid;
        }
        
        void initialize()
        {
            for (uint32_t i = 0; i < _size; ++i)
            {
                _write_register(0x10 + 2 * i, _read_register(0x10 + 2 * i) | (1 << 16));
            }
        }
        
    private:
        uint32_t _apic_id;
        uint32_t _base_vector_number;
        uint32_t _size;
        
        uint64_t _base_address;
        
        uint32_t _global_nmi_vector;
        uint32_t _global_nmi_flags;
        
        bool _is_valid;
        bool _is_nmi_valid;
        
        uint32_t _read_register(uint8_t id)
        {
            *(uint32_t *)(_base_address) = id;
            return *(uint32_t *)(_base_address + 0x10);
        }
        
        void _write_register(uint8_t id, uint32_t val)
        {
            *(uint32_t *)(_base_address) = id;
            *(uint32_t *)(_base_address + 0x10) = val;
        }
    };
}