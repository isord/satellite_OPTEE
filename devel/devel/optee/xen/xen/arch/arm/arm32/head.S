/*
 * xen/arch/arm/head.S
 *
 * Start-of-day code for an ARMv7-A with virt extensions.
 *
 * Tim Deegan <tim@xen.org>
 * Copyright (c) 2011 Citrix Systems.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <asm/page.h>
#include <asm/early_printk.h>

#define ZIMAGE_MAGIC_NUMBER 0x016f2818

#define PT_PT     0xf7f /* nG=1 AF=1 SH=11 AP=01 NS=1 ATTR=111 T=1 P=1 */
#define PT_MEM    0xf7d /* nG=1 AF=1 SH=11 AP=01 NS=1 ATTR=111 T=0 P=1 */
#define PT_MEM_L3 0xf7f /* nG=1 AF=1 SH=11 AP=01 NS=1 ATTR=111 T=1 P=1 */
#define PT_DEV    0xe71 /* nG=1 AF=1 SH=10 AP=01 NS=1 ATTR=100 T=0 P=1 */
#define PT_DEV_L3 0xe73 /* nG=1 AF=1 SH=10 AP=01 NS=1 ATTR=100 T=1 P=1 */

#define PT_UPPER(x) (PT_##x & 0xf00)
#define PT_LOWER(x) (PT_##x & 0x0ff)

/* Convenience defines to get slot used by Xen mapping. */
#define XEN_FIRST_SLOT      first_table_offset(XEN_VIRT_START)
#define XEN_SECOND_SLOT     second_table_offset(XEN_VIRT_START)

/* Offset between the early boot xen mapping and the runtime xen mapping */
#define XEN_TEMPORARY_OFFSET      (TEMPORARY_XEN_VIRT_START - XEN_VIRT_START)

#if defined(CONFIG_EARLY_PRINTK) && defined(CONFIG_EARLY_PRINTK_INC)
#include CONFIG_EARLY_PRINTK_INC
#endif

/*
 * Move an immediate constant into a 32-bit register using movw/movt
 * instructions.
 */
.macro mov_w reg, word
        movw  \reg, #:lower16:\word
        movt  \reg, #:upper16:\word
.endm

/*
 * Pseudo-op for PC relative adr <reg>, <symbol> where <symbol> is
 * within the range +/- 4GB of the PC.
 *
 * @dst: destination register
 * @sym: name of the symbol
 */
.macro adr_l, dst, sym
        mov_w \dst, \sym - .Lpc\@
        .set  .Lpc\@, .+ 8          /* PC bias */
        add   \dst, \dst, pc
.endm

.macro load_paddr rb, sym
        mov_w \rb, \sym
        add   \rb, \rb, r10
.endm

/*
 * Flush local TLBs
 *
 * @tmp: Scratch register
 *
 * See asm/arm32/flushtlb.h for the explanation of the sequence.
 */
.macro flush_xen_tlb_local tmp
        dsb   nshst
        mcr   CP32(\tmp, TLBIALLH)
        dsb   nsh
        isb
.endm

/*
 * Common register usage in this file:
 *   r0  -
 *   r1  -
 *   r2  -
 *   r3  -
 *   r4  -
 *   r5  -
 *   r6  -
 *   r7  -
 *   r8  - DTB address (boot CPU only)
 *   r9  - paddr(start)
 *   r10 - phys offset
 *   r11 - UART address
 *   r12 - Temporary mapping created
 *   r13 - SP
 *   r14 - LR
 *   r15 - PC
 */
#ifdef CONFIG_EARLY_PRINTK
/*
 * Macro to print a string to the UART, if there is one.
 *
 * Clobbers r0 - r3
 */
#define PRINT(_s)           \
        mov   r3, lr       ;\
        adr_l r0, 98f      ;\
        bl    puts         ;\
        mov   lr, r3       ;\
        RODATA_STR(98, _s)

/*
 * Macro to print the value of register \rb
 *
 * Clobbers r0 - r4
 */
.macro print_reg rb
        mov   r0, \rb
        mov   r4, lr
        bl    putn
        mov   lr, r4
.endm

#else /* CONFIG_EARLY_PRINTK */
#define PRINT(s)

.macro print_reg rb
.endm

#endif /* !CONFIG_EARLY_PRINTK */

        .section .text.header, "ax", %progbits
        .arm

        /*
         * This must be the very first address in the loaded image.
         * It should be linked at XEN_VIRT_START, and loaded at any
         * 4K-aligned address.
         */
GLOBAL(start)
        /*
         * zImage magic header, see:
         * http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html#d0e309
         */
        .rept 8
        mov   r0, r0
        .endr
        b     past_zImage

        .word ZIMAGE_MAGIC_NUMBER    /* Magic numbers to help the loader */
        .word 0x00000000             /* absolute load/run zImage address or
                                      * 0 for PiC */
        .word (_end - start)         /* zImage end address */

past_zImage:
        cpsid aif                    /* Disable all interrupts */

        /* Save the bootloader arguments in less-clobberable registers */
        mov   r8, r2                 /* r8 := DTB base address */

        /* Find out where we are */
        mov_w r0, start
        adr   r9, start              /* r9  := paddr (start) */
        sub   r10, r9, r0            /* r10 := phys-offset */

        /* Using the DTB in the .dtb section? */
.ifnes CONFIG_DTB_FILE,""
        load_paddr r8, _sdtb
.endif

        /* Initialize the UART if earlyprintk has been enabled. */
#ifdef CONFIG_EARLY_PRINTK
        bl    init_uart
#endif
        PRINT("- Boot CPU booting -\r\n")

        bl    check_cpu_mode
        bl    cpu_init
        bl    create_page_tables

        /* Address in the runtime mapping to jump to after the MMU is enabled */
        mov_w lr, primary_switched
        b     enable_mmu
primary_switched:
        bl    setup_fixmap
#ifdef CONFIG_EARLY_PRINTK
        /* Use a virtual address to access the UART. */
        mov_w r11, EARLY_UART_VIRTUAL_ADDRESS
#endif
        bl    zero_bss
        PRINT("- Ready -\r\n")
        /* Setup the arguments for start_xen and jump to C world */
        mov   r0, r10                /* r0 := Physical offset */
        mov   r1, r8                 /* r1 := paddr(FDT) */
        mov_w r2, start_xen
        b     launch
ENDPROC(start)

GLOBAL(init_secondary)
        cpsid aif                    /* Disable all interrupts */

        /* Find out where we are */
        mov_w r0, start
        adr   r9, start              /* r9  := paddr (start) */
        sub   r10, r9, r0            /* r10 := phys-offset */

        mrc   CP32(r1, MPIDR)
        bic   r7, r1, #(~MPIDR_HWID_MASK) /* Mask out flags to get CPU ID */

        load_paddr r0, smp_up_cpu
        dsb
2:      ldr   r1, [r0]
        cmp   r1, r7
        beq   1f
        wfe
        b     2b
1:

#ifdef CONFIG_EARLY_PRINTK
        mov_w r11, CONFIG_EARLY_UART_BASE_ADDRESS   /* r11 := UART base address */
        PRINT("- CPU ")
        print_reg r7
        PRINT(" booting -\r\n")
#endif
        bl    check_cpu_mode
        bl    cpu_init
        bl    create_page_tables

        /* Address in the runtime mapping to jump to after the MMU is enabled */
        mov_w lr, secondary_switched
        b     enable_mmu
secondary_switched:
        /*
         * Non-boot CPUs need to move on to the proper pagetables, which were
         * setup in init_secondary_pagetables.
         *
         * XXX: This is not compliant with the Arm Arm.
         */
        mov_w r4, init_ttbr          /* VA of HTTBR value stashed by CPU 0 */
        ldrd  r4, r5, [r4]           /* Actual value */
        dsb
        mcrr  CP64(r4, r5, HTTBR)
        dsb
        isb
        flush_xen_tlb_local r0

#ifdef CONFIG_EARLY_PRINTK
        /* Use a virtual address to access the UART. */
        mov_w r11, EARLY_UART_VIRTUAL_ADDRESS
#endif
        PRINT("- Ready -\r\n")
        /* Jump to C world */
        mov_w r2, start_secondary
        b     launch
ENDPROC(init_secondary)

/*
 * Check if the CPU supports virtualization extensions and has been booted
 * in Hypervisor mode.
 *
 * This function will never return when the CPU doesn't support
 * virtualization extensions or is booted in another mode than
 * Hypervisor mode.
 *
 * Clobbers r0 - r3
 */
check_cpu_mode:
        /* Check that this CPU has Hyp mode */
        mrc   CP32(r0, ID_PFR1)
        and   r0, r0, #0xf000        /* Bits 12-15 define virt extensions */
        teq   r0, #0x1000            /* Must == 0x1 or may be incompatible */
        beq   1f
        PRINT("- CPU doesn't support the virtualization extensions -\r\n")
        b     fail
1:

        /* Check that we're already in Hyp mode */
        mrs   r0, cpsr
        and   r0, r0, #0x1f          /* Mode is in the low 5 bits of CPSR */
        teq   r0, #0x1a              /* Hyp Mode? */
        moveq pc, lr                 /* Yes, return */

        /* OK, we're boned. */
        PRINT("- Xen must be entered in NS Hyp mode -\r\n")
        PRINT("- Please update the bootloader -\r\n")
        b     fail
ENDPROC(check_cpu_mode)

/*
 * Zero BSS
 *
 * Clobbers r0 - r3
 */
zero_bss:
        PRINT("- Zero BSS -\r\n")
        mov_w r0, __bss_start        /* r0 := vaddr(__bss_start) */
        mov_w r1, __bss_end          /* r1 := vaddr(__bss_end)   */

        mov   r2, #0
1:      str   r2, [r0], #4
        cmp   r0, r1
        blo   1b

        mov   pc, lr
ENDPROC(zero_bss)

cpu_init:
        PRINT("- Setting up control registers -\r\n")

        mov   r5, lr                       /* r5 := return address */

        /* Get processor specific proc info into r1 */
        bl    __lookup_processor_type
        teq   r1, #0
        bne   1f
        mov   r4, r0
        PRINT("- Missing processor info: ")
        print_reg r4
        PRINT(" -\r\n")
        b     cpu_init_done
1:
        /* Jump to cpu_init */
        ldr   r1, [r1, #PROCINFO_cpu_init]  /* r1 := vaddr(init func) */
        adr   lr, cpu_init_done             /* Save return address */
        add   pc, r1, r10                   /* Call paddr(init func) */

cpu_init_done:
        /* Set up memory attribute type tables */
        mov_w r0, MAIR0VAL
        mov_w r1, MAIR1VAL
        mcr   CP32(r0, HMAIR0)
        mcr   CP32(r1, HMAIR1)

        /*
         * Set up the HTCR:
         * PT walks use Inner-Shareable accesses,
         * PT walks are write-back, write-allocate in both cache levels,
         * Full 32-bit address space goes through this table.
         */
        mov_w r0, (TCR_RES1|TCR_SH0_IS|TCR_ORGN0_WBWA|TCR_IRGN0_WBWA|TCR_T0SZ(0))
        mcr   CP32(r0, HTCR)

        mov_w r0, HSCTLR_SET
        mcr   CP32(r0, HSCTLR)
        isb

        mov   pc, r5                        /* Return address is in r5 */
ENDPROC(cpu_init)

/*
 * Macro to find the slot number at a given page-table level
 *
 * slot:     slot computed
 * virt:     virtual address
 * lvl:      page-table level
 *
 * Note that ubxf is unpredictable when the end bit is above 32-bit. So we
 * can't use it for first level offset.
 */
.macro get_table_slot, slot, virt, lvl
    .if \lvl == 1
        lsr   \slot, \virt, #XEN_PT_LEVEL_SHIFT(\lvl)
    .else
        ubfx  \slot, \virt, #XEN_PT_LEVEL_SHIFT(\lvl), #XEN_PT_LPAE_SHIFT
    .endif
.endm

/*
 * Macro to create a page table entry in \ptbl to \tbl (physical
 * address)
 *
 * ptbl:    table symbol where the entry will be created
 * tbl:     physical address of the table to point to
 * virt:    virtual address
 * lvl:     page-table level
 *
 * Preserves \virt
 * Clobbers \tbl, r1 - r3
 *
 * Note that \tbl and \virt should be in a register other than r1 - r3
 */
.macro create_table_entry_from_paddr, ptbl, tbl, virt, lvl
        get_table_slot r1, \virt, \lvl  /* r1 := slot in \tbl */
        lsl   r1, r1, #3                /* r1 := slot offset in \tbl */

        movw  r2, #PT_PT             /* r2:r3 := right for linear PT */
        orr   r2, r2, \tbl           /*           + \tbl paddr */
        mov   r3, #0

        adr_l \tbl, \ptbl            /* \tbl := (v,p)addr of \ptbl */

        strd  r2, r3, [\tbl, r1]
.endm


/*
 * Macro to create a page table entry in \ptbl to \tbl (symbol)
 *
 * ptbl:    table symbol where the entry will be created
 * tbl:     table symbol to point to
 * virt:    virtual address
 * lvl:     page-table level
 *
 * Preserves \virt
 * Clobbers r1 - r4
 *
 * Also use r10 for the phys offset.
 *
 * Note that \virt should be in a register other than r1 - r4
 */
.macro create_table_entry, ptbl, tbl, virt, lvl
        load_paddr r4, \tbl
        create_table_entry_from_paddr \ptbl, r4, \virt, \lvl
 .endm

/*
 * Macro to create a mapping entry in \tbl to \paddr. Only mapping in 3rd
 * level table (i.e page granularity) is supported.
 *
 * ptbl:     table symbol where the entry will be created
 * virt:    virtual address
 * phys:    physical address
 * type:    mapping type. If not specified it will be normal memory (PT_MEM_L3)
 *
 * Preserves \virt, \phys
 * Clobbers r1 - r4
 *
 * Note that \virt and \paddr should be in other registers than r1 - r4
 * and be distinct.
 */
.macro create_mapping_entry, ptbl, virt, phys, type=PT_MEM_L3
        mov_w r2, XEN_PT_LPAE_ENTRY_MASK
        lsr   r1, \virt, #THIRD_SHIFT
        and   r1, r1, r2             /* r1 := slot in \tlb */
        lsl   r1, r1, #3             /* r1 := slot offset in \tlb */

        lsr   r4, \phys, #THIRD_SHIFT
        lsl   r4, r4, #THIRD_SHIFT   /* r4 := PAGE_ALIGNED(phys) */

        movw  r2, #\type             /* r2:r3 := right for section PT */
        orr   r2, r2, r4             /*          + PAGE_ALIGNED(phys) */
        mov   r3, #0

        adr_l r4, \ptbl

        strd  r2, r3, [r4, r1]
.endm

/*
 * Rebuild the boot pagetable's first-level entries. The structure
 * is described in mm.c.
 *
 * After the CPU enables paging it will add the fixmap mapping
 * to these page tables, however this may clash with the 1:1
 * mapping. So each CPU must rebuild the page tables here with
 * the 1:1 in place.
 *
 * Inputs:
 *   r9 : paddr(start)
 *   r10: phys offset
 *
 * Output:
 *   r12: Was a temporary mapping created?
 *
 * Clobbers r0 - r5
 */
create_page_tables:
        /* Prepare the page-tables for mapping Xen */
        mov_w r0, XEN_VIRT_START

        /*
         * We need to use a stash register because
         * create_table_entry_paddr() will clobber the register storing
         * the physical address of the table to point to.
         */
        load_paddr r5, boot_third
        mov_w r4, XEN_VIRT_START
.rept XEN_NR_ENTRIES(2)
        mov   r0, r5                        /* r0 := paddr(l3 table) */
        create_table_entry_from_paddr boot_second, r0, r4, 2
        add   r4, r4, #XEN_PT_LEVEL_SIZE(2) /* r4 := Next vaddr */
        add   r5, r5, #PAGE_SIZE            /* r5 := Next table */
.endr

        /*
         * Find the size of Xen in pages and multiply by the size of a
         * PTE. This will then be compared in the mapping loop below.
         *
         * Note the multiplication is just to avoid using an extra
         * register/instruction per iteration.
         */
        mov_w r0, _start            /* r0 := vaddr(_start) */
        mov_w r1, _end              /* r1 := vaddr(_end) */
        sub   r0, r1, r0            /* r0 := effective size of Xen */
        lsr   r0, r0, #PAGE_SHIFT   /* r0 := Number of pages for Xen */
        lsl   r0, r0, #3            /* r0 := Number of pages * PTE size */

        /* Setup boot_third: */
        adr_l r4, boot_third

        lsr   r2, r9, #THIRD_SHIFT  /* Base address for 4K mapping */
        lsl   r2, r2, #THIRD_SHIFT
        orr   r2, r2, #PT_UPPER(MEM_L3) /* r2:r3 := map */
        orr   r2, r2, #PT_LOWER(MEM_L3)
        mov   r3, #0x0

        /* ... map of vaddr(start) in boot_third */
        mov   r1, #0
1:      strd  r2, r3, [r4, r1]       /* Map vaddr(start) */
        add   r2, r2, #PAGE_SIZE     /* Next page */
        add   r1, r1, #8             /* Next slot */
        cmp   r1, r0                 /* Loop until we map all of Xen */
        blo   1b

        /*
         * Setup the 1:1 mapping so we can turn the MMU on. Note that
         * only the first page of Xen will be part of the 1:1 mapping.
         */
        create_table_entry boot_pgtable, boot_second_id, r9, 1
        create_table_entry boot_second_id, boot_third_id, r9, 2
        create_mapping_entry boot_third_id, r9, r9

        /*
         * Find the first slot used. If the slot is not the same
         * as TEMPORARY_AREA_FIRST_SLOT, then we will want to switch
         * to the temporary mapping before jumping to the runtime
         * virtual mapping.
         */
        get_table_slot r1, r9, 1     /* r1 := first slot */
        cmp   r1, #TEMPORARY_AREA_FIRST_SLOT
        bne   use_temporary_mapping

        mov_w r0, XEN_VIRT_START
        create_table_entry boot_pgtable, boot_second, r0, 1
        mov   r12, #0                /* r12 := temporary mapping not created */
        mov   pc, lr

use_temporary_mapping:
        /*
         * The identity mapping is not using the first slot
         * TEMPORARY_AREA_FIRST_SLOT. Create a temporary mapping.
         * See switch_to_runtime_mapping for more details.
         */
        PRINT("- Create temporary mapping -\r\n")

        /* Map boot_second (cover Xen mappings) to the temporary 1st slot */
        mov_w r0, TEMPORARY_XEN_VIRT_START
        create_table_entry boot_pgtable, boot_second, r0, 1

        mov   r12, #1                /* r12 := temporary mapping created */
        mov   pc, lr
ENDPROC(create_page_tables)

/*
 * Turn on the Data Cache and the MMU. The function will return
 * to the virtual address provided in LR (e.g. the runtime mapping).
 *
 * Inputs:
 *   r9 : paddr(start)
 *  r12 : Was the temporary mapping created?
 *   lr : Virtual address to return to
 *
 * Clobbers r0 - r5
 */
enable_mmu:
        PRINT("- Turning on paging -\r\n")

        /*
         * The state of the TLBs is unknown before turning on the MMU.
         * Flush them to avoid stale one.
         */
        flush_xen_tlb_local r0

        /* Write Xen's PT's paddr into the HTTBR */
        load_paddr r0, boot_pgtable
        mov   r1, #0                 /* r0:r1 is paddr (boot_pagetable) */
        mcrr  CP64(r0, r1, HTTBR)
        isb

        mrc   CP32(r0, HSCTLR)
        /* Enable MMU and D-cache */
        orr   r0, r0, #(SCTLR_Axx_ELx_M|SCTLR_Axx_ELx_C)
        dsb                          /* Flush PTE writes and finish reads */
        mcr   CP32(r0, HSCTLR)       /* now paging is enabled */
        isb                          /* Now, flush the icache */

        /*
         * The MMU is turned on and we are in the 1:1 mapping. Switch
         * to the runtime mapping.
         */
        mov   r5, lr                /* Save LR before overwritting it */
        mov_w lr, 1f                /* Virtual address in the runtime mapping */
        b     switch_to_runtime_mapping
1:
        mov   lr, r5                /* Restore LR */
        /*
         * At this point, either the 1:1 map or the temporary mapping
         * will be present. The former may clash with other parts of the
         * Xen virtual memory layout. As both of them are not used
         * anymore, remove them completely to avoid having to worry
         * about replacing existing mapping afterwards.
         *
         * On return this will jump to the virtual address requested by
         * the caller.
         */
        teq   r12, #0
        beq   remove_identity_mapping
        b     remove_temporary_mapping
ENDPROC(enable_mmu)

/*
 * Switch to the runtime mapping. The logic depends on whether the
 * runtime virtual region is clashing with the physical address
 *
 *  - If it is not clashing, we can directly jump to the address in
 *    the runtime mapping.
 *  - If it is clashing, create_page_tables() would have mapped Xen to
 *    a temporary virtual address. We need to switch to the temporary
 *    mapping so we can remove the identity mapping and map Xen at the
 *    correct position.
 *
 * Inputs
 *    r9: paddr(start)
 *   r12: Was a temporary mapping created?
 *    lr: Address in the runtime mapping to jump to
 *
 * Clobbers r0 - r4
 */
switch_to_runtime_mapping:
        /*
         * Jump to the runtime mapping if the virt and phys are not
         * clashing
         */
        teq   r12, #0
        beq   ready_to_switch

        /* We are still in the 1:1 mapping. Jump to the temporary Virtual address. */
        mov_w r0, 1f
        add   r0, r0, #XEN_TEMPORARY_OFFSET /* r0 := address in temporary mapping */
        mov   pc, r0

1:
        /* Remove boot_second_id */
        mov   r2, #0
        mov   r3, #0
        adr_l r0, boot_pgtable
        get_table_slot r1, r9, 1            /* r1 := first slot */
        lsl   r1, r1, #3                    /* r1 := first slot offset */
        strd  r2, r3, [r0, r1]

        flush_xen_tlb_local r0

        /* Map boot_second into boot_pgtable */
        mov_w r0, XEN_VIRT_START
        create_table_entry boot_pgtable, boot_second, r0, 1

        /* Ensure any page table updates are visible before continuing */
        dsb   nsh
        /*
         * The function will return on the runtime mapping. So we want
         * to prevent instruction fetch before the dsb completes.
         */
        isb

ready_to_switch:
        mov   pc, lr
ENDPROC(switch_to_runtime_mapping)

/*
 * Remove the 1:1 map from the page-tables. It is not easy to keep track
 * where the 1:1 map was mapped, so we will look for the top-level entry
 * exclusive to the 1:1 map and remove it.
 *
 * Inputs:
 *   r9 : paddr(start)
 *
 * Clobbers r0 - r3
 */
remove_identity_mapping:
        /* r2:r3 := invalid page-table entry */
        mov   r2, #0x0
        mov   r3, #0x0

        /* Find the first slot used and remove it */
        get_table_slot r1, r9, 1     /* r1 := first slot */
        mov_w r0, boot_pgtable       /* r0 := root table */
        lsl   r1, r1, #3             /* r1 := Slot offset */
        strd  r2, r3, [r0, r1]

        flush_xen_tlb_local r0
        mov   pc, lr
ENDPROC(remove_identity_mapping)

/*
 * Remove the temporary mapping of Xen starting at TEMPORARY_XEN_VIRT_START.
 *
 * Clobbers r0 - r3
 */
remove_temporary_mapping:
        /* r2:r3 := invalid page-table entry */
        mov   r2, #0
        mov   r3, #0

        adr_l r0, boot_pgtable
        mov_w r1, TEMPORARY_XEN_VIRT_START
        get_table_slot r1, r1, 1     /* r1 := first slot */
        lsl   r1, r1, #3             /* r1 := first slot offset */
        strd  r2, r3, [r0, r1]

        flush_xen_tlb_local r0

        mov  pc, lr
ENDPROC(remove_temporary_mapping)

/*
 * Map the UART in the fixmap (when earlyprintk is used) and hook the
 * fixmap table in the page tables.
 *
 * The fixmap cannot be mapped in create_page_tables because it may
 * clash with the 1:1 mapping.
 *
 * Inputs:
 *   r10: Physical offset
 *   r11: Early UART base physical address
 *
 * Clobbers r0 - r4
 */
setup_fixmap:
#if defined(CONFIG_EARLY_PRINTK)
        /* Add UART to the fixmap table */
        mov_w r0, EARLY_UART_VIRTUAL_ADDRESS
        create_mapping_entry xen_fixmap, r0, r11, type=PT_DEV_L3
#endif
        /* Map fixmap into boot_second */
        mov_w r0, FIXMAP_ADDR(0)
        create_table_entry boot_second, xen_fixmap, r0, 2
        /* Ensure any page table updates made above have occurred. */
        dsb   nshst
        /*
         * The fixmap area will be used soon after. So ensure no hardware
         * translation happens before the dsb completes.
         */
        isb

        mov   pc, lr
ENDPROC(setup_fixmap)

/*
 * Setup the initial stack and jump to the C world
 *
 * Inputs:
 *   r0 : Argument 0 of the C function to call
 *   r1 : Argument 1 of the C function to call
 *   r2 : C entry point
 *
 * Clobbers r3
 */
launch:
        mov_w r3, init_data
        add   r3, #INITINFO_stack    /* Find the boot-time stack */
        ldr   sp, [r3]
        add   sp, #STACK_SIZE        /* (which grows down from the top). */
        sub   sp, #CPUINFO_sizeof    /* Make room for CPU save record */

        /* Jump to C world */
       bx    r2
ENDPROC(launch)

/* Fail-stop */
fail:   PRINT("- Boot failed -\r\n")
1:      wfe
        b     1b
ENDPROC(fail)

/*
 * Switch TTBR
 * r1:r0       ttbr
 *
 * TODO: This code does not comply with break-before-make.
 */
ENTRY(switch_ttbr)
        dsb                            /* Ensure the flushes happen before
                                        * continuing */
        isb                            /* Ensure synchronization with previous
                                        * changes to text */
        mcr   CP32(r0, TLBIALLH)       /* Flush hypervisor TLB */
        mcr   CP32(r0, ICIALLU)        /* Flush I-cache */
        mcr   CP32(r0, BPIALL)         /* Flush branch predictor */
        dsb                            /* Ensure completion of TLB+BP flush */
        isb

        mcrr  CP64(r0, r1, HTTBR)

        dsb                            /* ensure memory accesses do not cross
                                        * over the TTBR0 write */
        isb                            /* Ensure synchronization with previous
                                        * changes to text */
        mcr   CP32(r0, TLBIALLH)       /* Flush hypervisor TLB */
        mcr   CP32(r0, ICIALLU)        /* Flush I-cache */
        mcr   CP32(r0, BPIALL)         /* Flush branch predictor */
        dsb                            /* Ensure completion of TLB+BP flush */
        isb

        mov pc, lr
ENDPROC(switch_ttbr)

#ifdef CONFIG_EARLY_PRINTK
/*
 * Initialize the UART. Should only be called on the boot CPU.
 *
 * Output:
 *  r11: Early UART base physical address
 *
 * Clobbers r0 - r3
 */
init_uart:
        mov_w r11, CONFIG_EARLY_UART_BASE_ADDRESS
#ifdef CONFIG_EARLY_UART_INIT
        early_uart_init r11, r1, r2
#endif
        PRINT("- UART enabled -\r\n")
        mov   pc, lr
ENDPROC(init_uart)

/*
 * Print early debug messages.
 * r0: Nul-terminated string to print.
 * r11: Early UART base address
 * Clobbers r0-r1
 */
puts:
        early_uart_ready r11, r1
        ldrb  r1, [r0], #1           /* Load next char */
        teq   r1, #0                 /* Exit on nul */
        moveq pc, lr
        early_uart_transmit r11, r1
        b puts
ENDPROC(puts)

/*
 * Print a 32-bit number in hex.  Specific to the PL011 UART.
 * r0: Number to print.
 * r11: Early UART base address
 * Clobbers r0-r3
 */
putn:
        adr_l r1, hex
        mov   r3, #8
1:
        early_uart_ready r11, r2
        and   r2, r0, #0xf0000000    /* Mask off the top nybble */
        ldrb  r2, [r1, r2, lsr #28]  /* Convert to a char */
        early_uart_transmit r11, r2
        lsl   r0, #4                 /* Roll it through one nybble at a time */
        subs  r3, r3, #1
        bne   1b
        mov   pc, lr
ENDPROC(putn)

RODATA_STR(hex, "0123456789abcdef")

#else  /* CONFIG_EARLY_PRINTK */

ENTRY(early_puts)
init_uart:
puts:
putn:   mov   pc, lr

#endif /* !CONFIG_EARLY_PRINTK */

/* This provides a C-API version of __lookup_processor_type */
ENTRY(lookup_processor_type)
        stmfd sp!, {r4, r10, lr}
        mov   r10, #0                   /* r10 := offset between virt&phys */
        bl    __lookup_processor_type
        mov r0, r1
        ldmfd sp!, {r4, r10, pc}

/*
 *  Read processor ID register (CP#15, CR0), and Look up in the linker-built
 * supported processor list. Note that we can't use the absolute addresses for
 * the __proc_info lists since we aren't running with the MMU on (and therefore,
 * we are not in correct address space). We have to calculate the offset.
 *
 * r10: offset between virt&phys
 *
 * Returns:
 * r0: CPUID
 * r1: proc_info pointer
 * Clobbers r2-r4
 */
__lookup_processor_type:
        mrc   CP32(r0, MIDR)                /* r0 := our cpu id */
        load_paddr r1, __proc_info_start
        load_paddr r2, __proc_info_end
1:      ldr   r3, [r1, #PROCINFO_cpu_mask]
        and   r4, r0, r3                    /* r4 := our cpu id with mask */
        ldr   r3, [r1, #PROCINFO_cpu_val]   /* r3 := cpu val in current proc info */
        teq   r4, r3
        beq   2f                            /* Match => exit, or try next proc info */
        add   r1, r1, #PROCINFO_sizeof
        cmp   r1, r2
        blo   1b
        /* We failed to find the proc_info, return NULL */
        mov   r1, #0
2:
        mov   pc, lr
ENDPROC(__lookup_processor_type)

/*
 * Local variables:
 * mode: ASM
 * indent-tabs-mode: nil
 * End:
 */
