#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hardware.h"
#include "hw_config.h"

static FILE *swap_file;
int victime_rr = 1;

struct vm_mapping_s vm_mapping[VM_PAGES];
struct pm_mapping_s pm_mapping[PM_PAGES];

static char init_swap(char* path)
{
    swap_file = fopen(path, "w+");
    assert(swap_file != NULL);
    
    fseek(swap_file, VM_SIZE, SEEK_SET);
    fwrite("", 1, 1, swap_file);
    
    return 0;
}

static char store_to_swap(int vpage, int ppage) 
{
    fseek(swap_file, vpage*PAGE_SIZE, SEEK_SET);
    fwrite((char*)((int)physical_memory + (ppage << 12)), PAGE_SIZE, 1, swap_file);
    
    return 0;
}

static char fetch_from_swap(int vpage, int ppage)
{
    fseek(swap_file, vpage*PAGE_SIZE, SEEK_SET);
    fread((char*)((int)physical_memory + (ppage << 12)), PAGE_SIZE, 1, swap_file);
    
    return 0;
}

static void mmuhandler() 
{
    int fault = _in(MMU_FAULT_ADDR);
    struct tlb_entry_s tlb_entry;
    
    assert(fault >= (int)virtual_memory && fault <= (int)virtual_memory + VM_SIZE - 1);
    
    tlb_entry.tlbe_cfu = 0;
    tlb_entry.tlbe_x_access = 1;
    tlb_entry.tlbe_w_access = 1;
    tlb_entry.tlbe_r_access = 1;
    tlb_entry.tlbe_used = 1;
    
    if(vm_mapping[vpage_of_vaddr(fault)].mapped)
    {
        tlb_entry.tlbe_physical_page = vm_mapping[vpage_of_vaddr(fault)].ppage;
        tlb_entry.tlbe_virtual_page = vpage_of_vaddr(fault);
        _out(TLB_ADD_ENTRY, *((int*)&tlb_entry));
        return;
    }

    if(pm_mapping[victime_rr].mapped)
    {
        store_to_swap(pm_mapping[victime_rr].vpage, victime_rr);
        pm_mapping[victime_rr].mapped = 0;
        vm_mapping[pm_mapping[victime_rr].vpage].mapped = 0;
        tlb_entry.tlbe_physical_page = victime_rr;
        _out(TLB_DEL_ENTRY, *((int*)&tlb_entry));
    }

    fetch_from_swap(vpage_of_vaddr(fault), victime_rr);
    
    pm_mapping[victime_rr].mapped = 1;
    pm_mapping[victime_rr].vpage = vpage_of_vaddr(fault);
    
    vm_mapping[vpage_of_vaddr(fault)].mapped = 1;
    vm_mapping[vpage_of_vaddr(fault)].ppage = victime_rr;
    
    tlb_entry.tlbe_physical_page = victime_rr;
    tlb_entry.tlbe_virtual_page = vpage_of_vaddr(fault);
    
    _out(TLB_ADD_ENTRY, *((int*)&tlb_entry));
    
    victime_rr = (victime_rr % 255) + 1;
    
    return;
}

int main(int argc, char **argv)
{
    /* init hardware */
    if(init_hardware("hardware.ini") == 0)
    {
	    fprintf(stderr, "Error in hardware initialization\n");
	    exit(EXIT_FAILURE);
    }
    
    init_swap("swapfile");
    
    memset(vm_mapping, 0, sizeof(struct vm_mapping_s) * VM_PAGES);
    memset(pm_mapping, 0, sizeof(struct pm_mapping_s) * PM_PAGES);
    
    IRQVECTOR[MMU_IRQ] = mmuhandler;
    _mask(0x1001);

    user_process();
    
    return 0;
}

