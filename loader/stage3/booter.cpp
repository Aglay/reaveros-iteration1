#include "booter.h"
#include "screen.h"
#include "physmem.h"
#include "initrd.h"
#include "paging.h"

extern "C" void LoadCR3(Paging::PageDirectory *);
extern "C" void EnablePaging();
extern "C" void Execute(int, int);

// not very optimal, but who cares
void Zero(char * dest, int size)
{
    while (size--)
    {
        *dest = 0;
    }
}

void Copy(char * src, char * dest, int size)
{
    if (size % 4 == 0)
    {
        size /= 4;
        int * s = (int *)src;
        int * d = (int *)dest;
        
        while (size-- > 0)
        {
            *d = *s;
        }
    }
    
    else
    {
        while (size-- > 0)
        {
            *dest = *src;
        }
    }
}

namespace Booter
{
    InitRD::InitRD * initrd = 0;
    
    void Initialize(int pPlacementAddress)
    {
        PhysMemory::PlacementAddress = pPlacementAddress;
    }

    void Panic(char * sMessage)
    {
        if (Screen::kout != 0)
            Screen::kout->Print("\n\n")->Print(sMessage);

        for (;;) ;
    }   

    void SetupInitRD(int pInitrd)
    {
        Booter::initrd = (InitRD::InitRD *)PhysMemory::Manager::Place(sizeof(InitRD::InitRD));
        Booter::initrd->Initialize(pInitrd);   
    }
    
    void LoadStorageDriver(int)
    {
        // dummy for now
    }

    void LoadDriver(char *)
    {
        // dummy for now
    }
    
    void LoadKernel()
    {
        // for now, until I win with my laziness and implement storage and filesystem drivers, it'll be loaded from initrd
        // but now, I just want to get into kernel land
        
        InitRD::File * file = Booter::initrd->GetFile("kernel");
        
        if (!file)
        {
            Booter::Panic("Kernel image not found!");
        }
        
        char * kernel = file->GetContent();
        
        Copy(kernel, (char *)(0x1000000), file->GetSize() * 512);
        
        // well, I cheated here twice
        // for first cheat - look at comment at beginning of this function
        // for second cheat - oh well, where is kernel's initrd?
        // but it won't need it for some time
        
        // let's start that kernel development
    }
    
    void SetupKernelEnvironment()
    {
        // this one is tricky, we should setup paging here (and long mode, in the future)
        // for now, simple 32bit paging without PAE
        // all we need is identity map paging structures (they will be changed into kernel dynamic ones,
        // in kernel land) and map kernel to higher half
        // some work will be done in assembly (of course...)

        Paging::PageDirectory * dir = (Paging::PageDirectory *)PhysMemory::Manager::PlacePageAligned(sizeof(Paging::PageDirectory));
        Zero((char *)dir, sizeof(Paging::PageDirectory));
        
        // now, identity map first 16 MBs
        for (int i = 0; i < 4; i++)
        {
            Paging::PageTable * table = (Paging::PageTable *)PhysMemory::Manager::PlacePageAligned(sizeof(Paging::PageTable));
            Zero((char *)table, sizeof(Paging::PageTable));
            
            dir->Tables[i] = reinterpret_cast<int>(table);
            dir->Tables[i] |= 1 | (1 << 1);
            
            for (int j = 0; j < 1024; j++)
            {
                table->Pages[j] = (i * 4 * 1024 * 1024 + j * 4 * 1024) | 1 | (1 << 1);
            }
        }
        
        // now, map 16 MB - 48 MB to 1 GB - 1 GB 32 MB
        for (int i = 0; i < 8; i++)
        {
            Paging::PageTable * table = (Paging::PageTable *)PhysMemory::Manager::PlacePageAligned(sizeof(Paging::PageTable));
            Zero((char *)table, sizeof(Paging::PageTable));
            
            dir->Tables[768 + i] = reinterpret_cast<int>(table);
            dir->Tables[768 + i] |= 1 | (1 << 1);
            
            for (int j = 0; j < 1024; j++)
            {
                table->Pages[j] = ((i + 4) * 4 * 1024 * 1024 + j * 4  * 1024) | 1 | (1 << 1);
            }
        }

        LoadCR3(dir);
        EnablePaging();

        return;
    }
    
    void ExecuteKernel(int memmap, int memcount) 
    {
        // execute assembly function, that setups kernel stack and executes it
        // never returns
        Execute(memmap, memcount);
    }    
}