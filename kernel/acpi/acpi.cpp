/**
 * Reaver Project OS, Rose License
 *
 * Copyright © 2012-2013 Michał "Griwes" Dominiak
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

#include <acpi/acpi.h>
#include <acpi/tables.h>
#include <memory/vm.h>
#include <processor/processor.h>
#include <screen/screen.h>
#include <processor/core.h>
#include <processor/ioapic.h>
#include <processor/interrupt_entry.h>
#include <time/hpet.h>

namespace
{
    virt_addr_t _root_address;
    virt_addr_t _table_address;

    acpi::rsdt * root = nullptr;
    acpi::xsdt * new_root = nullptr;

    void _install_rsdt(acpi::rsdp * ptr)
    {
        new_root = nullptr;

        memory::vm::map(_root_address, _root_address + 8 * 4096, ptr->rsdt_ptr);

        if ((static_cast<acpi::rsdt *>(_root_address + static_cast<uint32_t>(ptr->rsdt_ptr) % 4096))->validate("RSDT"))
        {
            root = static_cast<acpi::rsdt *>(_root_address + static_cast<uint32_t>(ptr->rsdt_ptr) % 4096);

            return;
        }

        else
        {
            PANIC("RSDT invalid");
        }
    }

    void _install_xsdt(acpi::rsdp * ptr)
    {
        memory::vm::map(_root_address, _root_address + 8 * 4096, ptr->xsdt_ptr);

        if ((static_cast<acpi::xsdt *>(_root_address + static_cast<uint64_t>(ptr->xsdt_ptr) % 4096))->validate("XSDT"))
        {
            new_root = static_cast<acpi::xsdt *>(_root_address + static_cast<uint64_t>(ptr->xsdt_ptr) % 4096);

            return;
        }

        else
        {
            memory::vm::unmap(_root_address, _root_address + 8 * 4096, false);

            screen::debug(tag::acpi, "XSDT invalid, falling back to RSDT\n");

            _install_rsdt(ptr);
        }
    }

    void _install_root(acpi::rsdp * ptr)
    {
        if (ptr->revision)
        {
            _install_xsdt(ptr);
        }

        else
        {
            _install_rsdt(ptr);
        }
    }

    // TODO: external (bootloader) version of this function
    acpi::rsdp * _find_rsdp()
    {
        virt_addr_t ebda{ static_cast<uint64_t>(*reinterpret_cast<uint16_t *>(0x40E) << 4) };
        acpi::rsdp * ptr = static_cast<acpi::rsdp *>(ebda);

        while (ptr < static_cast<acpi::rsdp *>(ebda + 1024))
        {
            if (ptr->validate())
            {
                _install_root(ptr);

                return ptr;
            }

            else
            {
                ptr = static_cast<acpi::rsdp *>(static_cast<virt_addr_t>(ptr) + 16);
            }
        }

        ptr = reinterpret_cast<acpi::rsdp *>(0xe0000);

        while (ptr < reinterpret_cast<acpi::rsdp *>(0x100000))
        {
            if (ptr->validate())
            {
                _install_root(ptr);

                return ptr;
            }

            else
            {
                ptr = static_cast<acpi::rsdp *>(static_cast<virt_addr_t>(ptr) + 16);
            }
        }

        PANIC("RSDP not found!");

        return nullptr;
    }

    acpi::description_table_header * _find_table(const char * sign)
    {
        acpi::description_table_header * table;

        if (new_root)
        {
            for (uint64_t i = 0; i < (new_root->length - 36) / 8; ++i)
            {
                table = static_cast<acpi::description_table_header *>(_table_address + static_cast<uint64_t>(new_root->entries[i]) % 4096);

                memory::vm::map(_table_address, _table_address + 8 * 4096, new_root->entries[i]);

                if (table->validate(sign))
                {
                    return table;
                }

                memory::vm::unmap(_table_address, _table_address + 8 * 4096, false);
            }
        }

        else
        {
            for (uint64_t i = 0; i < (root->length - 36) / 4; ++i)
            {
                table = static_cast<acpi::description_table_header *>(_table_address + static_cast<uint32_t>(root->entries[i]) % 4096);

                memory::vm::map(_table_address, _table_address + 8 * 4096, root->entries[i]);

                if (table->validate(sign))
                {
                    return table;
                }

                memory::vm::unmap(_table_address, _table_address + 8 * 4096, false);
            }
        }

        return nullptr;
    }

    void _free_table()
    {
        memory::vm::unmap(_table_address, _table_address + 8 * 4096, false);
    }
}

void acpi::initialize()
{
    _root_address = memory::vm::allocate_address_range(8 * 4096);
    _table_address = memory::vm::allocate_address_range(8 * 4096);

    _find_rsdp();
    _find_table("FFFF");
}

void acpi::parse_madt(processor::core *& cores, uint64_t & core_num, processor::ioapic *& ioapics, uint64_t & ioapic_num,
    processor::interrupt_entry * ints)
{
    auto table = static_cast<madt *>(_find_table("APIC"));

    if (!table)
    {
        return;
    }

    phys_addr_t lic_address = table->lic_address;

    core_num = 0;
    ioapic_num = 0;

    auto entry = table->entries;

    while (reinterpret_cast<uint64_t>(entry) - reinterpret_cast<uint64_t>(table) < table->length)
    {
        if (entry->type == 0 || entry->type == 9)
        {
            ++core_num;
        }

        else if (entry->type == 1)
        {
            ++ioapic_num;
        }

        entry = reinterpret_cast<acpi::madt_entry *>(reinterpret_cast<uint64_t>(entry) + entry->length);
    }

    {
        auto cores_address = memory::vm::allocate_address_range(core_num * sizeof(processor::core));
        auto ioapics_address = memory::vm::allocate_address_range(ioapic_num * sizeof(processor::ioapic));

        memory::vm::map(cores_address, cores_address + core_num * sizeof(processor::core));
        memory::vm::map(ioapics_address, ioapics_address + ioapic_num * sizeof(processor::ioapic));

        cores = static_cast<processor::core *>(cores_address);
        ioapics = static_cast<processor::ioapic *>(ioapics_address);
    }

    core_num = 0;
    ioapic_num = 0;

    entry = table->entries;

    while (reinterpret_cast<uint64_t>(entry) - reinterpret_cast<uint64_t>(table) < table->length)
    {
        switch (entry->type)
        {
            case 0:
            {
                auto lapic = reinterpret_cast<acpi::madt_lapic_entry *>(entry + 1);

                if (lapic->flags & 1)
                {
                    new (static_cast<void *>(cores + core_num++)) processor::core{ lapic->apic_id, lapic->acpi_id };

                    screen::debug("\nFound LAPIC entry: ", lapic->apic_id);
                }

                break;
            }

            case 1:
            {
                auto ioapic = reinterpret_cast<acpi::madt_ioapic_entry *>(entry + 1);

                new (static_cast<void *>(ioapics + ioapic_num++)) processor::ioapic{ ioapic->apic_id, ioapic->base_int, ioapic->base_address };

                screen::debug("\nFound I/O APIC entry: ", ioapic->apic_id, ", handling vectors from ", ioapic->base_int, " to ", ioapics[ioapic_num - 1].end());

                break;
            }

            case 2:
            {
                auto iso = reinterpret_cast<acpi::madt_int_override_entry *>(entry + 1);

                ints[iso->source].set(iso->source, iso->int_number, iso->flags);

                screen::debug("\nFound redirection entry: ", iso->source, " -> ", iso->int_number, ", polarity: ");

                switch (iso->flags & 3)
                {
                    case 0:
                        screen::debug("standard");
                        break;
                    case 1:
                        screen::debug("active high");
                        break;
                    case 2:
                        screen::debug("RESERVED");
                        PANIC("Reserved polarity in MADT");
                    case 3:
                        screen::debug("active low");
                }

                screen::debug(", trigger mode: ");

                switch ((iso->flags >> 2) & 3)
                {
                    case 0:
                        screen::debug("standard");
                        break;
                    case 1:
                        screen::debug("edge");
                        break;
                    case 2:
                        screen::debug("RESERVED");
                        PANIC("Reserved trigger mode in MADT");
                    case 3:
                        screen::debug("level");
                }
            }

            case 3:
            {
                auto nmi = reinterpret_cast<acpi::madt_nmi_source_entry *>(entry + 1);

                for (uint64_t i = 0; i < ioapic_num; ++i)
                {
                    if (ioapics[i].set_global_nmi(nmi->int_number, nmi->flags))
                    {
                        break;
                    }
                }

                break;
            }

            case 5:
            {
                auto override = reinterpret_cast<acpi::madt_lapic_address_override_entry *>(entry + 1);

                lic_address = override->base_address;

                break;
            }

            case 9:
            {
                auto x2apic = reinterpret_cast<acpi::madt_x2apic_entry *>(entry + 1);

                if (x2apic->flags & 1)
                {
                    new (static_cast<void *>(cores + core_num++)) processor::core{ x2apic->x2apic_id, x2apic->acpi_uuid, false };

                    screen::debug("\nFound x2APIC entry: ", x2apic->x2apic_id);
                }

                break;
            }
        }

        entry = reinterpret_cast<acpi::madt_entry *>(reinterpret_cast<uint64_t>(entry) + entry->length);
    }

    entry = table->entries;

    while (reinterpret_cast<uint64_t>(entry) - reinterpret_cast<uint64_t>(table) < table->length)
    {
        switch (entry->type)
        {
            case 4:
            {
                auto lapic_nmi = reinterpret_cast<acpi::madt_lapic_nmi_entry *>(entry + 1);

                if (lapic_nmi->acpi_id == 0xff)
                {
                    for (uint64_t i = 0; i < core_num; ++i)
                    {
                        if (cores[i].lapic())
                        {
                            cores[i].set_nmi(lapic_nmi->int_number, lapic_nmi->flags);
                        }
                    }

                    break;
                }

                for (uint64_t idx = 0; idx < core_num; ++idx)
                {
                    if (cores[idx].acpi_id() == lapic_nmi->acpi_id)
                    {
                        cores[idx].set_nmi(lapic_nmi->int_number, lapic_nmi->flags);

                        break;
                    }

                    if (idx == core_num - 1)
                    {
                        screen::debug(tag::acpi, "Ignoring NMI vector entry for unknown ACPI ID ", lapic_nmi->acpi_id, "\n");
                    }
                }

                break;
            }


            case 10:
            {
                auto x2apic_nmi = reinterpret_cast<acpi::madt_x2apic_nmi_entry *>(entry + 1);

                if (x2apic_nmi->acpi_uuid == 0xffffffff)
                {
                    for (uint64_t i = 0; i < core_num; ++i)
                    {
                        cores[i].set_nmi(x2apic_nmi->int_number, x2apic_nmi->flags);
                    }

                    break;
                }

                for (uint64_t idx = 0; idx < core_num; ++idx)
                {
                    if (cores[idx].acpi_id() == x2apic_nmi->acpi_uuid)
                    {
                        cores[idx].set_nmi(x2apic_nmi->int_number, x2apic_nmi->flags);

                        break;
                    }

                    if (idx == core_num - 1)
                    {
                        screen::debug(tag::acpi, "Ignoring x2APIC NMI entry for unknown ACPI UUID ", x2apic_nmi->acpi_uuid, "\n");
                    }
                }

                break;
            }
        }

        entry = reinterpret_cast<acpi::madt_entry *>(reinterpret_cast<uint64_t>(entry) + entry->length);
    }

    memory::vm::map(processor::lapic_base(), lic_address);

    _free_table();
}

void acpi::parse_hpet(time::hpet::timer *& timers, uint64_t & timers_num)
{
    hpet * table = static_cast<hpet *>(_find_table("HPET"));

    timers_num = 0;
    timers = nullptr;

    if (!table)
    {
        screen::debug("\nNo HPET ACPI table, falling back to PIT");
        return;
    }

    screen::debug("\nFound HPET.");
    screen::debug("\nNumber: ", table->hpet_number);
    screen::debug("\nPCI vendor ID: ", table->pci_vendor_id);
    screen::debug("\nAddress: ", table->address.address);
    screen::debug("\nCounter size: ", 32 + 32 * table->counter_size);
    screen::debug("\nNumber of comparators: ", table->comparator_count + 1);
    screen::debug("\nMinimum tick: ", table->minimum_tick);

    auto mmio = memory::vm::allocate_address_range(4096);
    memory::vm::map(mmio, table->address.address);

    timers_num = 1;
    auto address = memory::vm::allocate_address_range(sizeof(time::hpet::timer));
    memory::vm::map(address, address + sizeof(time::hpet::timer));
    timers = new (address) time::hpet::timer{ table->hpet_number, table->pci_vendor_id, mmio, table->counter_size, static_cast<uint8_t>(table->comparator_count + 1),
        table->minimum_tick, table->page_protection };

    _free_table();
}
