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

#include <screen/bootterm.h>
#include <memory/map.h>

namespace screen
{
    boot_terminal terminal;
}

uint64_t _find_backbuffer(memory::map_entry * map, uint64_t map_size)
{
    for (uint64_t i = 0; i < map_size; ++i)
    {
        if (map[i].type == 4)
        {
            return map[i].base;
        }
    }
    
    return 0;
}

screen::boot_terminal::boot_terminal(screen::mode * mode, memory::map_entry * map, uint64_t map_size)
    : _mode(mode), _backbuffer(_find_backbuffer(map, map_size))
{
    *(volatile uint64_t *)0 = _backbuffer;
//    _clear();
}

screen::boot_terminal::~boot_terminal()
{
}
