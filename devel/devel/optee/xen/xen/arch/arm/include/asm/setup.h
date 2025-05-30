#ifndef __ARM_SETUP_H_
#define __ARM_SETUP_H_

#include <public/version.h>
#include <asm/p2m.h>
#include <xen/device_tree.h>

#define MIN_FDT_ALIGN 8
#define MAX_FDT_SIZE SZ_2M

#define NR_MEM_BANKS 256

#define MAX_MODULES 32 /* Current maximum useful modules */

typedef enum {
    BOOTMOD_XEN,
    BOOTMOD_FDT,
    BOOTMOD_KERNEL,
    BOOTMOD_RAMDISK,
    BOOTMOD_XSM,
    BOOTMOD_GUEST_DTB,
    BOOTMOD_UNKNOWN
}  bootmodule_kind;

enum membank_type {
    /*
     * The MEMBANK_DEFAULT type refers to either reserved memory for the
     * device/firmware (when the bank is in 'reserved_mem') or any RAM (when
     * the bank is in 'mem').
     */
    MEMBANK_DEFAULT,
    /*
     * The MEMBANK_STATIC_DOMAIN type is used to indicate whether the memory
     * bank is bound to a static Xen domain. It is only valid when the bank
     * is in reserved_mem.
     */
    MEMBANK_STATIC_DOMAIN,
    /*
     * The MEMBANK_STATIC_HEAP type is used to indicate whether the memory
     * bank is reserved as static heap. It is only valid when the bank is
     * in reserved_mem.
     */
    MEMBANK_STATIC_HEAP,
};

/* Indicates the maximum number of characters(\0 included) for shm_id */
#define MAX_SHM_ID_LENGTH 16

struct membank {
    paddr_t start;
    paddr_t size;
    enum membank_type type;
#ifdef CONFIG_STATIC_SHM
    char shm_id[MAX_SHM_ID_LENGTH];
    unsigned int nr_shm_borrowers;
#endif
};

struct meminfo {
    unsigned int nr_banks;
    struct membank bank[NR_MEM_BANKS];
};

/*
 * The domU flag is set for kernels and ramdisks of "xen,domain" nodes.
 * The purpose of the domU flag is to avoid getting confused in
 * kernel_probe, where we try to guess which is the dom0 kernel and
 * initrd to be compatible with all versions of the multiboot spec. 
 */
#define BOOTMOD_MAX_CMDLINE 1024
struct bootmodule {
    bootmodule_kind kind;
    bool domU;
    paddr_t start;
    paddr_t size;
};

/* DT_MAX_NAME is the node name max length according the DT spec */
#define DT_MAX_NAME 41
struct bootcmdline {
    bootmodule_kind kind;
    bool domU;
    paddr_t start;
    char dt_name[DT_MAX_NAME];
    char cmdline[BOOTMOD_MAX_CMDLINE];
};

struct bootmodules {
    int nr_mods;
    struct bootmodule module[MAX_MODULES];
};

struct bootcmdlines {
    unsigned int nr_mods;
    struct bootcmdline cmdline[MAX_MODULES];
};

struct bootinfo {
    struct meminfo mem;
    /* The reserved regions are only used when booting using Device-Tree */
    struct meminfo reserved_mem;
    struct bootmodules modules;
    struct bootcmdlines cmdlines;
#ifdef CONFIG_ACPI
    struct meminfo acpi;
#endif
    bool static_heap;
};

struct map_range_data
{
    struct domain *d;
    p2m_type_t p2mt;
    /* Set if mapping of the memory ranges must be skipped. */
    bool skip_mapping;
    /* Rangeset to store IRQs and IOMEM for overlay nodes. */
    struct rangeset *iomem_ranges;
    struct rangeset *irq_ranges;
};

extern struct bootinfo bootinfo;

extern domid_t max_init_domid;

void copy_from_paddr(void *dst, paddr_t paddr, unsigned long len);

size_t estimate_efi_size(unsigned int mem_nr_banks);

void acpi_create_efi_system_table(struct domain *d,
                                  struct membank tbl_add[]);

void acpi_create_efi_mmap_table(struct domain *d,
                                const struct meminfo *mem,
                                struct membank tbl_add[]);

int acpi_make_efi_nodes(void *fdt, struct membank tbl_add[]);

void create_domUs(void);
void create_dom0(void);
void alloc_static_evtchn(void);

void discard_initial_modules(void);
void fw_unreserved_regions(paddr_t s, paddr_t e,
                           void (*cb)(paddr_t, paddr_t), unsigned int first);

size_t boot_fdt_info(const void *fdt, paddr_t paddr);
const char *boot_fdt_cmdline(const void *fdt);

bool check_reserved_regions_overlap(paddr_t region_start, paddr_t region_size);

struct bootmodule *add_boot_module(bootmodule_kind kind,
                                   paddr_t start, paddr_t size, bool domU);
struct bootmodule *boot_module_find_by_kind(bootmodule_kind kind);
struct bootmodule * boot_module_find_by_addr_and_kind(bootmodule_kind kind,
                                                             paddr_t start);
void add_boot_cmdline(const char *name, const char *cmdline,
                      bootmodule_kind kind, paddr_t start, bool domU);
struct bootcmdline *boot_cmdline_find_by_kind(bootmodule_kind kind);
struct bootcmdline * boot_cmdline_find_by_name(const char *name);
const char *boot_module_kind_as_string(bootmodule_kind kind);

extern uint32_t hyp_traps_vector[];
void init_traps(void);

void device_tree_get_reg(const __be32 **cell, uint32_t address_cells,
                         uint32_t size_cells, paddr_t *start, paddr_t *size);

u32 device_tree_get_u32(const void *fdt, int node,
                        const char *prop_name, u32 dflt);

int handle_device(struct domain *d, struct dt_device_node *dev, p2m_type_t p2mt,
                  struct rangeset *iomem_ranges, struct rangeset *irq_ranges);

int map_device_irqs_to_domain(struct domain *d, struct dt_device_node *dev,
                              bool need_mapping, struct rangeset *irq_ranges);

int map_irq_to_domain(struct domain *d, unsigned int irq,
                      bool need_mapping, const char *devname);

int map_range_to_domain(const struct dt_device_node *dev,
                        uint64_t addr, uint64_t len, void *data);

extern lpae_t boot_pgtable[XEN_PT_LPAE_ENTRIES];

#ifdef CONFIG_ARM_64
extern lpae_t boot_first_id[XEN_PT_LPAE_ENTRIES];
#endif
extern lpae_t boot_second_id[XEN_PT_LPAE_ENTRIES];
extern lpae_t boot_third_id[XEN_PT_LPAE_ENTRIES];

/* Find where Xen will be residing at runtime and return a PT entry */
lpae_t pte_of_xenaddr(vaddr_t);

extern const char __ro_after_init_start[], __ro_after_init_end[];

struct init_info
{
    /* Pointer to the stack, used by head.S when entering in C */
    unsigned char *stack;
    /* Logical CPU ID, used by start_secondary */
    unsigned int cpuid;
};

#endif
/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
