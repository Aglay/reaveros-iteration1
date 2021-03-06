/**
 * Reaver Project OS, Rose License
 *
 * Copyright © 2011-2014 Michał "Griwes" Dominiak
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

#include <memory/memory.h>
#include <memory/vm.h>
#include <memory/pmm.h>
#include <screen/screen.h>
#include <scheduler/scheduler.h>
#include <processor/processor.h>
#include <utils/raf.h>

extern "C" void kernel_main(uint64_t initrd_start, uint64_t initrd_end, screen::mode * video, memory::map_entry *
    memory_map, uint64_t memory_map_size)
{
    screen::initialize_console();

    memory::copy_bootloader_data(video, memory_map, memory_map_size);

    memory::vm::initialize();
    memory::pmm::initialize(memory_map, memory_map_size);

    // memory map required to get preallocated backbuffer info from bootloader
    screen::initialize_terminal(video, memory_map, memory_map_size);

    screen::print("ReaverOS: Reaver Project Operating System, \"Rose\"\n");
    screen::print("Version: 0.0.4 dev; Release #1 \"Cotyledon\", built on ", __DATE__, " at ", __TIME__, "\n");
    screen::print("Copyright © 2012-2014 Reaver Project Team\n\n");

    screen::print(tag::memory, "Reporting memory manager status...\n");
    memory::pmm::boot_report();

    screen::print(tag::cpu, "Initializing processor...");
    processor::initialize();
    screen::done();

    screen::print(tag::scheduler, "Initializing scheduler...");
    scheduler::initialize();
    screen::done();

    utils::raf initrd{ initrd_start, initrd_end };

    screen::print(tag::scheduler, "Starting init process...");
    scheduler::create_process(initrd["init.srv"], nullptr, true, true);

/*    screen::print(tag::scheduler, "Initializing virtual memory manager...");
    scheduler::process vmm = scheduler::create_process(initrd["vmm.srv"]);
    screen::done();

    screen::print(tag::scheduler, "Starting process manager...");
    scheduler::process procmgr = scheduler::create_process(initrd["procmgr.srv"]);
    procmgr.set_priority(10);
    screen::done();

    screen::print(tag::scheduler, "Starting video service...");
    scheduler::process vsrv = scheduler::create_process(initrd["video.srv"]);
    screen::done();

    screen::print(tag::screen, "Switching to video service for console output...");
    screen::initialize_server(vsrv);
    screen::done();

    screen::print(tag::scheduler, "Starting device manager...");
    scheduler::process device_mgr = scheduler::create_process(initrd["devicemgr.srv"]);
    screen::done();

    screen::print(tag::scheduler, "Starting V(irtual)I(nput)O(utput)LA(yer) service...");
    scheduler::process viola = scheduler::create_process(initrd["viola.srv"]);
    screen::done();

    screen::print(tag::scheduler, "Starting storage device driver...");
    scheduler::process storage = scheduler::create_process(initrd["storage.srv"]);
    screen::done();

    screen::print(tag::scheduler, "Starting filesystem driver...");
    scheduler::process filesystem = scheduler::create_process(initrd["filesystem.srv"]);
    screen::done();

    screen::print(tag::scheduler, "Running /boot/init.srv...");
    scheduler::process init = scheduler::create_process("/boot/init.srv");
    screen::done();*/

    for (;;)
    {
        HLT;
    }
}

