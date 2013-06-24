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

#include <memory/stack.h>
#include <memory/map.h>

#include <screen/screen.h>

extern memory::pmm::frame_stack _global_stack;

memory::pmm::frame_stack::frame_stack() : _first{}, _last{}, _size{}, _global{ &_global_stack }
{
}

memory::pmm::frame_stack::frame_stack(memory::map_entry * map, uint64_t map_size) : _first{}, _last{}, _size{}, _global{}
{
    for (uint64_t i = 0; i < map_size; ++i)
    {
        if (map[i].type == 1 && (map[i].base >= 1024 * 1024 || map[i].base + map[i].length > 1024 * 1024))
        {
            for (uint64_t frame = (map[i].base < 1024 * 1024) ? (1024 * 1024) : ((map[i].base + 4095) & ~(uint64_t)4095);
                frame < map[i].base + map[i].length; frame += 4096)
            {
                push(frame);
            }
        }
    }
}

uint64_t memory::pmm::frame_stack::pop()
{
    if (!_size)
    {
        PANIC("TODO: _size = 0");

        if (_first)
        {
            // dealocate chunk
        }

        else
        {
            if (_global)
            {
                push_chunk(_global->pop_chunk());
            }

            else
            {
                // free caches etc.
            }
        }
    }

    auto _ = utils::make_unique_lock(_last->lock);

    uint64_t ret = _last->stack[--_last->size];

    if (_last->size == 0)
    {
        if (_first != _last)
        {
            _last = _last->prev;
        }

        else
        {
            PANIC("TODO: frame stack exhausted");
        }
    }

    if (_last != _first && _last->size == frame_stack_chunk::max - 50)
    {
        auto _ = utils::make_unique_lock(_last->lock);
        auto __ = utils::make_unique_lock(_last->next->lock);

        if (_last->next->next)
        {
            // free _last->next->next
        }
    }

    screen::debug("\nframe_stack::pop(): returning ", (void *)ret);

    return ret;
}

memory::pmm::frame_stack_chunk * memory::pmm::frame_stack::pop_chunk()
{
    if (_first == _last)
    {
        return nullptr;
    }

    auto _ = utils::make_unique_lock(_last->lock);
    auto __ = utils::make_unique_lock(_first->lock);

    if (_first->next)
    {
        auto _ = utils::make_unique_lock(_first->next->lock);
    }

    auto ret = _first;
    _first = _first->next;

    return ret;
}

void memory::pmm::frame_stack::push(uint64_t frame)
{
    auto _ = utils::make_unique_lock(_lock);

    if (!_first)
    {
        if (!_first)
        {
            _last = _first = allocate_chained<frame_stack_chunk>();
        }
    }

    if (_last->size == frame_stack_chunk::max - 100 && !_last->next)
    {
        auto __ = utils::make_unique_lock(_last->lock);

        screen::debug("\nLocked lock at ", &_last->lock);

        _last->next = allocate_chained<frame_stack_chunk>();
        _last->next->prev = _last;

        screen::debug("\nframe_stack::push(): allocated another at ", _last->next);
    }

    {
        auto __ = utils::make_unique_lock(_last->lock);

        if (_last->size == frame_stack_chunk::max)
        {
            screen::debug("\nframe_stack::push(): set _last to _next");
            _last = _last->next;
        }
    }

    auto __ = utils::make_unique_lock(_last->lock);

    _last->stack[_last->size++] = frame;
    ++_size;
}

void memory::pmm::frame_stack::push_chunk(memory::pmm::frame_stack_chunk * chunk)
{
    if (chunk->size == frame_stack_chunk::max)
    {
        auto _ = utils::make_unique_lock(_first->lock);

        chunk->prev = nullptr;
        chunk->next = _first;
        _first = chunk;

        return;
    }

    auto _ = utils::make_unique_lock(_last->lock);
    auto __ = utils::make_unique_lock(chunk->lock);

    auto last = _last;

    while (last->next)
    {
        last = last->next;
    }

    auto ___ = utils::make_unique_lock(last->lock);

    last->next = chunk;
    chunk->prev = last;
    chunk->next = nullptr;

    _size += chunk->size;
}
