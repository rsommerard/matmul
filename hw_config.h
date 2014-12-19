/* ------------------------------
   $Id: hw_config.h 105 2009-11-24 15:22:50Z simon_duquennoy $
   ------------------------------------------------------------

   Fichier de configuration des acces au materiel

   Philippe Marquet, march 2007

   Code au niveau applicatif la description du materiel qui est fournie
   par hardware.ini
   
*/

#ifndef _HW_CONFIG_H_
#define _HW_CONFIG_H_

#define HARDWARE_INI	"hardware.ini"

#define MMU_IRQ		13
#define MMU_CMD		0x66
#define MMU_FAULT_ADDR 	0xCD
#define TLB_ADD_ENTRY	0xCE
#define TLB_DEL_ENTRY	0xDE
#define TLB_SIZE	32
#define TLB_ENTRIES	0x800

#define PM_PAGES (1 << 8)
#define VM_PAGES (1 << 12)
#define PAGE_SIZE 4096
#define PM_SIZE (4096 * PM_PAGES)
#define VM_SIZE (4096 * VM_PAGES)

#define vpage_of_vaddr(vaddr) (((vaddr) >> 12) & 0xFFF)

struct tlb_entry_s
{
    unsigned tlbe_cfu:8;
    unsigned tlbe_virtual_page:12;
    unsigned tlbe_physical_page:8;
    unsigned tlbe_x_access:1;
    unsigned tlbe_w_access:1;
    unsigned tlbe_r_access:1;
    unsigned tlbe_used:1;
};

struct vm_mapping_s
{
    unsigned mapped:1;
    unsigned ppage:12;
};

struct pm_mapping_s
{
    unsigned mapped:1;
    unsigned vpage:12;
};

extern void user_process();

#endif

