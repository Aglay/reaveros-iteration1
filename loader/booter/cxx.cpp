/**
 * Reaver Project OS, Rose License
 *
 * Copyright © 2011-2012 Michał "Griwes" Dominiak
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
 **/

#include <cstdint>
#include <cstddef>

#include <screen/screen.h>
#include <memory/memory.h>
#include <memory/manager.h>

void * operator new(uint32_t size) noexcept
{
    return memory::default_allocator.allocate(size);
}

void operator delete(void * ptr) noexcept
{
    return memory::default_allocator.deallocate(ptr);
}

void * operator new(uint32_t, void * addr) noexcept
{
    return addr;
}

void * operator new[](uint32_t size) noexcept
{
    return memory::default_allocator.allocate(size);
}

extern "C" void __cxa_pure_virtual()
{
    PANIC("Pure virtual function called!");
}

void _panic(const char * X, const char * FILE, uint64_t /*LINE*/, const char * FUNC)
{
    if (screen::output.backbuffer_start())
    {
        screen::line();
        screen::printl("PANIC: ", X);
        screen::printl("File: ", FILE); //, ", line ", LINE);
        screen::printl("Function: ", FUNC);
    }

    asm ("cli; hlt");
}

void * __dso_handle = 0;

extern "C" int __cxa_atexit(void (*)(void *), void *, void *)
{
    return 0;
}

extern "C" void * memset(void * dest, int ch, uint32_t count)
{
    unsigned char * d = (unsigned char *)dest;
    for (uint32_t i = 0; i < count; ++i)
    {
        *d++ = (unsigned char)ch;
    }

    return dest;
}

extern "C" void * memcpy(void * dest, const void * src, size_t count)
{
    memory::copy((uint8_t *)src, (uint8_t *)dest, count);
    return dest;
}
