#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardware.h"
#include "mi_syscall.h"


static int current_process;

int ppage_of_vaddr(int process, unsigned vaddr)
{
    if(vaddr > ((int)virtual_memory+(N/2)*PAGE_SIZE-1) || vaddr < (int)virtual_memory)
        return -1;
    
    return vaddr + 1 + process * (N/2);
}

static void mmuhandler() 
{
    unsigned vaddr = _in(MMU_FAULT_ADDR);
    int ppage;
    struct tlb_entry_s tlb_entry;
    
    if((ppage = ppage_of_vaddr(current_process, vaddr)) == -1)
        exit(EXIT_FAILURE);
        
    tlb_entry.tlbe_cfu = 0;
    tlb_entry.tlbe_virtual_page = vpage_of_vaddr(vaddr);
    tlb_entry.tlbe_physical_page = ppage;
    tlb_entry.tlbe_x_access = 1;
    tlb_entry.tlbe_w_access = 1;
    tlb_entry.tlbe_r_access = 1;
    tlb_entry.tlbe_used = 1;

    _out(TLB_ADD_ENTRY, *((int*)&tlb_entry));
}

static void switch_to_process0(void) 
{
    current_process = 0;
    _out(MMU_CMD, MMU_RESET);
}

static void switch_to_process1(void) 
{
    current_process = 1;
    _out(MMU_CMD, MMU_RESET);
}

int main(int argc, char **argv) 
{

    void *ptr;
    int res;
    
    /* init hardware */
    if(init_hardware("hardware.ini") == 0)
    {
	    fprintf(stderr, "Error in hardware initialization\n");
	    exit(EXIT_FAILURE);
    }
    
    IRQVECTOR[16] = switch_to_process0;
    IRQVECTOR[17] = switch_to_process1;
    IRQVECTOR[MMU_IRQ] = mmuhandler;
    _mask(0x1001);

    ptr = virtual_memory;

    _int(16);
    memset(ptr, 1, PAGE_SIZE * N/2);
    _int(17);
    memset(ptr, 3, PAGE_SIZE * N/2);

    _int(16);
    res = sum(ptr);
    printf("Resultat du processus 0 : %d\n",res);
    _int(17);
    res = sum(ptr);
    printf("Resultat du processus 1 : %d\n",res);
    
    return 0;
}
















