#ifndef BOOT_HXX
#define BOOT_HXX

#include <types.hxx>
#include <vdisk.hxx>

/// @brief Multiboot protocol
namespace multiboot {
	struct header {
		uint32_t magic;
		uint32_t flags;
		int32_t checksum;
	} ALIGNED(4);
};

/// Limine protocol
namespace limine {
	// Misc
#define LIMINE_PTR(TYPE) TYPE
	/// @brief Base request
	template<typename Response>
	class request {
	public:
		uint64_t id[4];
		uint64_t revision;
		LIMINE_PTR(Response *) response;
	};

#define LIMINE_COMMON_MAGIC 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b
	struct uuid {
		uint32_t a;
		uint16_t b;
		uint16_t c;
		uint8_t d[8];
	};

#define LIMINE_MEDIA_TYPE_GENERIC 0
#define LIMINE_MEDIA_TYPE_OPTICAL 1
#define LIMINE_MEDIA_TYPE_TFTP 2
	struct file {
		uint64_t revision;
		LIMINE_PTR(void *) address;
		uint64_t size;
		LIMINE_PTR(char *) path;
		LIMINE_PTR(char *) cmdline;
		uint32_t media_type;
		uint32_t unused;
		uint32_t tftp_ip;
		uint32_t tftp_port;
		uint32_t partition_index;
		uint32_t mbr_disk_id;
		limine::uuid gpt_disk_uuid;
		limine::uuid gpt_part_uuid;
		limine::uuid part_uuid;
	};

	struct request_parser {
		uint64_t id[4];
		int (*parser)(void *req);
	};

// Boot info
#define LIMINE_BOOTLOADER_INFO_REQUEST { LIMINE_COMMON_MAGIC, 0xf55038d8e2a1202f, 0x279426fcf5f59740 }
	struct bootloader_info_response {
		uint64_t revision;
		LIMINE_PTR(char *) name;
		LIMINE_PTR(char *) version;
	};

	struct bootloader_info_request : public limine::request<limine::bootloader_info_response> {
		
	};

// Stack size
#define LIMINE_STACK_SIZE_REQUEST { LIMINE_COMMON_MAGIC, 0x224ef0460a8e8926, 0xe1cb0fc25f46ea3d }
	struct stack_size_response {
		uint64_t revision;
	};

	struct stack_size_request : public limine::request<limine::stack_size_response> {
		uint64_t stack_size;
	};

// HHDM
#define LIMINE_HHDM_REQUEST { LIMINE_COMMON_MAGIC, 0x48dcf1cb8ad2b852, 0x63984e959a98244b }
	struct hhdm_response {
		uint64_t revision;
		uint64_t offset;
	};

	struct hhdm_request : public limine::request<limine::hhdm_response> {

	};

// Framebuffer
#define LIMINE_FRAMEBUFFER_REQUEST { LIMINE_COMMON_MAGIC, 0x9d5827dcd881dd75, 0xa3148604f6fab11b }
#define LIMINE_FRAMEBUFFER_RGB 1
	struct framebuffer {
		LIMINE_PTR(void *) address;
		uint64_t width;
		uint64_t height;
		uint64_t pitch;
		uint16_t bpp;
		uint8_t memory_model;
		uint8_t red_mask_size;
		uint8_t red_mask_shift;
		uint8_t green_mask_size;
		uint8_t green_mask_shift;
		uint8_t blue_mask_size;
		uint8_t blue_mask_shift;
		uint8_t unused[7];
		uint64_t edid_size;
		LIMINE_PTR(void *) edid;
	};

	struct framebuffer_response {
		uint64_t revision;
		uint64_t framebuffer_count;
		LIMINE_PTR(struct limine::framebuffer **) framebuffers;
	};

	struct framebuffer_request : public limine::request<limine::framebuffer_response> {

	};

// Framebuffer (legacy)
#define LIMINE_FRAMEBUFFER_LEGACY_REQUEST { LIMINE_COMMON_MAGIC, 0xcbfe81d7dd2d1977, 0x063150319ebc9b71 }
	struct framebuffer_legacy {
		LIMINE_PTR(void *) address;
		uint16_t width;
		uint16_t height;
		uint16_t pitch;
		uint16_t bpp;
		uint8_t memory_model;
		uint8_t red_mask_size;
		uint8_t red_mask_shift;
		uint8_t green_mask_size;
		uint8_t green_mask_shift;
		uint8_t blue_mask_size;
		uint8_t blue_mask_shift;
		uint8_t unused;
		uint64_t edid_size;
		LIMINE_PTR(void *) edid;
	};

	struct framebuffer_legacy_response {
		uint64_t revision;
		uint64_t framebuffer_count;
		LIMINE_PTR(struct limine::framebuffer_legacy **) framebuffers;
	};

	struct framebuffer_legacy_request : public limine::request<limine::framebuffer_legacy_response> {

	};

// Terminal
#define LIMINE_TERMINAL_REQUEST { LIMINE_COMMON_MAGIC, 0xc8ac59310c2b0844, 0xa68d0c7265d38878 }

#define LIMINE_TERMINAL_CB_DEC 10
#define LIMINE_TERMINAL_CB_BELL 20
#define LIMINE_TERMINAL_CB_PRIVATE_ID 30
#define LIMINE_TERMINAL_CB_STATUS_REPORT 40
#define LIMINE_TERMINAL_CB_POS_REPORT 50
#define LIMINE_TERMINAL_CB_KBD_LEDS 60
#define LIMINE_TERMINAL_CB_MODE 70
#define LIMINE_TERMINAL_CB_LINUX 80

#define LIMINE_TERMINAL_CTX_SIZE ((uint64_t)(-1))
#define LIMINE_TERMINAL_CTX_SAVE ((uint64_t)(-2))
#define LIMINE_TERMINAL_CTX_RESTORE ((uint64_t)(-3))
#define LIMINE_TERMINAL_FULL_REFRESH ((uint64_t)(-4))
	struct terminal;

	typedef void (*terminal_write)(limine::terminal *, const char *, uint64_t);
	typedef void (*terminal_callback)(limine::terminal *, uint64_t, uint64_t, uint64_t, uint64_t);

	struct terminal {
		uint64_t columns;
		uint64_t rows;
		LIMINE_PTR(limine::framebuffer *) framebuffer;
		// Bootloader specific
		virtual_disk::handle *hdl;
	};

	struct terminal_response {
		uint64_t revision;
		uint64_t terminal_count;
		LIMINE_PTR(limine::terminal **) terminals;
		LIMINE_PTR(limine::terminal_write) write;
	};

	struct terminal_request : public limine::request<limine::terminal_response> {
		LIMINE_PTR(limine::terminal_callback) callback;
	};

// Terminal (legacy)
#define LIMINE_TERMINAL_LEGACY_REQUEST { LIMINE_COMMON_MAGIC, 0x0785a0aea5d0750f, 0x1c1936fee0d6cf6e }
	struct terminal_legacy;

	typedef void (*terminal_legacy_write)(struct limine::terminal_legacy *, const char *, uint64_t);
	typedef void (*terminal_legacy_callback)(struct limine::terminal_legacy *, uint64_t, uint64_t, uint64_t, uint64_t);

	struct terminal_legacy {
		uint32_t columns;
		uint32_t rows;
		LIMINE_PTR(limine::framebuffer_legacy *) framebuffer;
	};

	struct terminal_legacy_response {
		uint64_t revision;
		uint64_t terminal_count;
		LIMINE_PTR(limine::terminal_legacy **) terminals;
		LIMINE_PTR(limine::terminal_legacy_write) write;
	};

	struct terminal_legacy_request : public limine::request<limine::terminal_legacy_response> {
		LIMINE_PTR(terminal_legacy_callback) callback;
	};

// 5-level paging
#define LIMINE_5_LEVEL_PAGING_REQUEST { LIMINE_COMMON_MAGIC, 0x94469551da9b3192, 0xebe5e86db7382888 }
	struct five_level_paging_response {
		uint64_t revision;
	};

	struct five_level_paging_request : public limine::request<limine::five_level_paging_response> {

	};

// SMP
#define LIMINE_SMP_REQUEST { LIMINE_COMMON_MAGIC, 0x95a67b819a1b857e, 0xa0b61b723b6a73e0 }
#define LIMINE_SMP_X2APIC (1 << 0)
	struct smp_info;

	typedef void (*goto_address)(struct limine::smp_info *);

	struct smp_info {
		uint32_t processor_id;
		uint32_t lapic_id;
		uint64_t reserved;
		LIMINE_PTR(limine::goto_address) goto_address;
		uint64_t extra_argument;
	};

	struct smp_response {
		uint64_t revision;
		uint32_t flags;
		uint32_t bsp_lapic_id;
		uint64_t cpu_count;
		LIMINE_PTR(limine::smp_info **) cpus;
	};

	struct smp_request : public limine::request<limine::smp_response> {
		uint64_t flags;
	};

// Memory map
#define LIMINE_MEMMAP_REQUEST { LIMINE_COMMON_MAGIC, 0x67cf3d9d378a806f, 0xe304acdfc50c3c62 }

#define LIMINE_MEMMAP_USABLE				 0
#define LIMINE_MEMMAP_RESERVED			   1
#define LIMINE_MEMMAP_ACPI_RECLAIMABLE	   2
#define LIMINE_MEMMAP_ACPI_NVS			   3
#define LIMINE_MEMMAP_BAD_MEMORY			 4
#define LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define LIMINE_MEMMAP_KERNEL_AND_MODULES	 6
#define LIMINE_MEMMAP_FRAMEBUFFER			7
	struct memmap_entry {
		uint64_t base;
		uint64_t length;
		uint64_t type;
	};

	struct memmap_response {
		uint64_t revision;
		uint64_t entry_count;
		LIMINE_PTR(limine::memmap_entry **) entries;
	};

	struct memmap_request : public limine::request<limine::memmap_response> {

	};

// Entry point
#define LIMINE_ENTRY_POINT_REQUEST { LIMINE_COMMON_MAGIC, 0x13d86c035a1cd3e1, 0x2b0caa89d8f3026a }
	typedef void (*entry_point)();
	struct entry_point_response {
		uint64_t revision;
	};

	struct entry_point_request : public limine::request<limine::entry_point_response> {
		LIMINE_PTR(limine::entry_point) entry;
	};

// Kernel File
#define LIMINE_KERNEL_FILE_REQUEST { LIMINE_COMMON_MAGIC, 0xad97e90e83f1ed67, 0x31eb5d1c5ff23b69 }
	struct kernel_file_response {
		uint64_t revision;
		LIMINE_PTR(limine::file *) kernel_file;
	};

	struct kernel_file_request : public limine::request<limine::kernel_file_response> {

	};

// Module
#define LIMINE_MODULE_REQUEST { LIMINE_COMMON_MAGIC, 0x3e7e279702be32af, 0xca1c4f3bd1280cee }
	struct module_response {
		uint64_t revision;
		uint64_t module_count;
		LIMINE_PTR(limine::file **) modules;
	};

	struct module_request : public limine::request<limine::module_response> {

	};

// RSDP
#define LIMINE_RSDP_REQUEST { LIMINE_COMMON_MAGIC, 0xc5e77b6b397e7b43, 0x27637845accdcf3c }
	struct rsdp_response {
		uint64_t revision;
		LIMINE_PTR(void *) address;
	};

	struct rsdp_request : public limine::request<limine::rsdp_response> {

	};

// SMBIOS
#define LIMINE_SMBIOS_REQUEST { LIMINE_COMMON_MAGIC, 0x9e9046f11e095391, 0xaa4a520fefbde5ee }
	struct smbios_response {
		uint64_t revision;
		LIMINE_PTR(void *) entry_32;
		LIMINE_PTR(void *) entry_64;
	};

	struct smbios_request : public limine::request<limine::smbios_response> {

	};

// EFI system table
#define LIMINE_EFI_SYSTEM_TABLE_REQUEST { LIMINE_COMMON_MAGIC, 0x5ceba5163eaaf6d6, 0x0a6981610cf65fcc }
	struct efi_system_table_response {
		uint64_t revision;
		LIMINE_PTR(void *) address;
	};

	struct efi_system_table_request : public limine::request<limine::efi_system_table_response> {

	};

// Boot time
#define LIMINE_BOOT_TIME_REQUEST { LIMINE_COMMON_MAGIC, 0x502746e184c088aa, 0xfbc5ec83e6327893 }
	struct boot_time_response {
		uint64_t revision;
		int64_t boot_time;
	};

	struct boot_time_request : public limine::request<limine::boot_time_response> {

	};

// Kernel address
#define LIMINE_KERNEL_ADDRESS_REQUEST { LIMINE_COMMON_MAGIC, 0x71ba76863cc55f63, 0xb2644a48c516a487 }
	struct kernel_address_response {
		uint64_t revision;
		uint64_t physical_base;
		uint64_t virtual_base;
	};

	struct kernel_address_request : public limine::request<limine::kernel_address_response> {

	};
}

// Stivale1
namespace stivale1 {
	// Anchor for non ELF kernels
	struct anchor {
		uint8_t anchor[15];
		uint8_t bits;
		uint64_t phys_load_addr;
		uint64_t phys_bss_start;
		uint64_t phys_bss_end;
		uint64_t phys_stivalehdr;
	};

	/// @brief  Information passed from the kernel to the bootloader
	struct header {
		uint64_t stack;
		uint16_t flags;
		uint16_t framebuffer_width;
		uint16_t framebuffer_height;
		uint16_t framebuffer_bpp;
		uint64_t entry_point;
	};

	/// @brief  Information passed from the bootloader to the kernel
	struct module {
		uint64_t begin;
		uint64_t end;
		char string[128];
		uint64_t next;
	};

#define STIVALE_MMAP_USABLE				 1
#define STIVALE_MMAP_RESERVED			   2
#define STIVALE_MMAP_ACPI_RECLAIMABLE	   3
#define STIVALE_MMAP_ACPI_NVS			   4
#define STIVALE_MMAP_BAD_MEMORY			 5
#define STIVALE_MMAP_KERNEL_AND_MODULES	 10
#define STIVALE_MMAP_BOOTLOADER_RECLAIMABLE 0x1000
#define STIVALE_MMAP_FRAMEBUFFER			0x1002
	struct mmap_entry {
		uint64_t base;
		uint64_t length;
		uint32_t type;
		uint32_t unused;
	};

#define STIVALE_FBUF_MMODEL_RGB 1
	struct framebuffer {
		uint64_t cmdline;
		uint64_t memory_map_addr;
		uint64_t memory_map_entries;
		uint64_t framebuffer_addr;
		uint16_t framebuffer_pitch;
		uint16_t framebuffer_width;
		uint16_t framebuffer_height;
		uint16_t framebuffer_bpp;
		uint64_t rsdp;
		uint64_t module_count;
		uint64_t modules;
		uint64_t epoch;
		uint64_t flags; /// bit 0: 1 if booted with BIOS, 0 if booted with UEFI
						/// bit 1: 1 if extended colour information passed, 0 if not
		uint8_t  fb_memory_model;
		uint8_t  fb_red_mask_size;
		uint8_t  fb_red_mask_shift;
		uint8_t  fb_green_mask_size;
		uint8_t  fb_green_mask_shift;
		uint8_t  fb_blue_mask_size;
		uint8_t  fb_blue_mask_shift;
		uint8_t  reserved;
		uint64_t smbios_entry_32;
		uint64_t smbios_entry_64;
	};
}

// Stivale2
namespace stivale2 {
#if (defined (_STIVALE2_SPLIT_64) && defined (__i386__)) || defined(_STIVALE2_SPLIT_64_FORCE)
# define _stivale2_split64(NAME) \
	union { uint32_t NAME; uint32_t NAME##_lo; }; \
	uint32_t NAME##_hi;
#else
# define _stivale2_split64(NAME) uint64_t NAME
#endif
	/// @brief Anchor for non ELF kernels
	struct anchor {
		uint8_t anchor[15];
		uint8_t bits;
		_stivale2_split64(phys_load_addr);
		_stivale2_split64(phys_bss_start);
		_stivale2_split64(phys_bss_end);
		_stivale2_split64(phys_stivale2hdr);
	};

	struct tag {
		uint64_t identifier;
		_stivale2_split64(next);
	};
	
	/// @brief Information passed from the kernel to the bootloader
	struct header {
		_stivale2_split64(entry_point);
		_stivale2_split64(stack);
		uint64_t flags;
		_stivale2_split64(tags);
	};

#define STIVALE2_HEADER_TAG_ANY_VIDEO_ID 0xc75c9fa92a44c4db
	struct header_tag_any_video {
		stivale2::tag tag;
		uint64_t preference;
	};

#define STIVALE2_HEADER_TAG_FRAMEBUFFER_ID 0x3ecc1bc43d0f7971
	struct header_tag_framebuffer {
		stivale2::tag tag;
		uint16_t framebuffer_width;
		uint16_t framebuffer_height;
		uint16_t framebuffer_bpp;
		uint16_t unused;
	};

#define STIVALE2_HEADER_TAG_FB_MTRR_ID 0x4c7bb07731282e00
#define STIVALE2_HEADER_TAG_TERMINAL_ID 0xa85d499b1823be72
	struct header_tag_terminal {
		stivale2::tag tag;
		uint64_t flags;
		_stivale2_split64(callback);
	};

#define STIVALE2_TERM_CB_DEC 10
#define STIVALE2_TERM_CB_BELL 20
#define STIVALE2_TERM_CB_PRIVATE_ID 30
#define STIVALE2_TERM_CB_STATUS_REPORT 40
#define STIVALE2_TERM_CB_POS_REPORT 50
#define STIVALE2_TERM_CB_KBD_LEDS 60
#define STIVALE2_TERM_CB_MODE 70
#define STIVALE2_TERM_CB_LINUX 80

#define STIVALE2_TERM_CTX_SIZE ((uint64_t)(-1))
#define STIVALE2_TERM_CTX_SAVE ((uint64_t)(-2))
#define STIVALE2_TERM_CTX_RESTORE ((uint64_t)(-3))
#define STIVALE2_TERM_FULL_REFRESH ((uint64_t)(-4))

#define STIVALE2_HEADER_TAG_SMP_ID 0x1ab015085f3273df
	struct header_tag_smp {
		stivale2::tag tag;
		uint64_t flags;
	};

#define STIVALE2_HEADER_TAG_5LV_PAGING_ID 0x932f477032007e8f

#define STIVALE2_HEADER_TAG_UNMAP_NULL_ID 0x92919432b16fe7e7
	/// @brief Information passed from the bootloader to the kernel
	struct boot_struct {
#define STIVALE2_BOOTLOADER_BRAND_SIZE 64
		char bootloader_brand[STIVALE2_BOOTLOADER_BRAND_SIZE];
#define STIVALE2_BOOTLOADER_VERSION_SIZE 64
		char bootloader_version[STIVALE2_BOOTLOADER_VERSION_SIZE];
		uint64_t tags;
	};

#define STIVALE2_STRUCT_TAG_PMRS_ID 0x5df266a64047b6bd

#define STIVALE2_PMR_EXECUTABLE ((uint64_t)1 << 0)
#define STIVALE2_PMR_WRITABLE   ((uint64_t)1 << 1)
#define STIVALE2_PMR_READABLE   ((uint64_t)1 << 2)
	struct pmr {
		uint64_t base;
		uint64_t length;
		uint64_t permissions;
	};

	struct tag_pmrs {
		stivale2::tag tag;
		uint64_t entries;
		stivale2::pmr pmrs[];
	};

#define STIVALE2_STRUCT_TAG_CMDLINE_ID 0xe5e76a1b4597a781
	struct tag_cmdline {
		stivale2::tag tag;
		uint64_t cmdline;
	};

#define STIVALE2_STRUCT_TAG_MEMMAP_ID 0x2187f79e8612de07

#define STIVALE2_MMAP_USABLE				 1
#define STIVALE2_MMAP_RESERVED			   2
#define STIVALE2_MMAP_ACPI_RECLAIMABLE	   3
#define STIVALE2_MMAP_ACPI_NVS			   4
#define STIVALE2_MMAP_BAD_MEMORY			 5
#define STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE 0x1000
#define STIVALE2_MMAP_KERNEL_AND_MODULES	 0x1001
#define STIVALE2_MMAP_FRAMEBUFFER			0x1002
	struct mmap_entry {
		uint64_t base;
		uint64_t length;
		uint32_t type;
		uint32_t unused;
	};

	struct tag_memmap {
		stivale2::tag tag;
		uint64_t entries;
		stivale2::mmap_entry memmap[];
	};

#define STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID 0x506461d2950408fa

#define STIVALE2_FBUF_MMODEL_RGB 1
	struct tag_framebuffer {
		stivale2::tag tag;
		uint64_t framebuffer_addr;
		uint16_t framebuffer_width;
		uint16_t framebuffer_height;
		uint16_t framebuffer_pitch;
		uint16_t framebuffer_bpp;
		uint8_t  memory_model;
		uint8_t  red_mask_size;
		uint8_t  red_mask_shift;
		uint8_t  green_mask_size;
		uint8_t  green_mask_shift;
		uint8_t  blue_mask_size;
		uint8_t  blue_mask_shift;
		uint8_t  unused;
	};

#define STIVALE2_STRUCT_TAG_EDID_ID 0x968609d7af96b845
	struct tag_edid {
		stivale2::tag tag;
		uint64_t edid_size;
		uint8_t  edid_information[];
	};

#define STIVALE2_STRUCT_TAG_TEXTMODE_ID 0x38d74c23e0dca893
	struct tag_textmode {
		stivale2::tag tag;
		uint64_t address;
		uint16_t unused;
		uint16_t rows;
		uint16_t cols;
		uint16_t bytes_per_char;
	};

#define STIVALE2_STRUCT_TAG_FB_MTRR_ID 0x6bc1a78ebe871172

#define STIVALE2_STRUCT_TAG_TERMINAL_ID 0xc2b3f4c3233b0974
	struct tag_terminal {
		stivale2::tag tag;
		uint32_t flags;
		uint16_t cols;
		uint16_t rows;
		uint64_t term_write;
		uint64_t max_length;
	};

#define STIVALE2_STRUCT_TAG_MODULES_ID 0x4b6fe466aade04ce
	struct module {
		uint64_t begin;
		uint64_t end;
#define STIVALE2_MODULE_STRING_SIZE 128
		char string[STIVALE2_MODULE_STRING_SIZE];
	};

	struct tag_modules {
		stivale2::tag tag;
		uint64_t module_count;
		stivale2::module modules[];
	};

#define STIVALE2_STRUCT_TAG_RSDP_ID 0x9e1786930a375e78
	struct tag_rsdp {
		stivale2::tag tag;
		uint64_t rsdp;
	};

#define STIVALE2_STRUCT_TAG_EPOCH_ID 0x566a7bed888e1407
	struct tag_epoch {
		stivale2::tag tag;
		uint64_t epoch;
	};

#define STIVALE2_STRUCT_TAG_FIRMWARE_ID 0x359d837855e3858c
#define STIVALE2_FIRMWARE_BIOS (1 << 0)
	struct tag_firmware {
		stivale2::tag tag;
		uint64_t flags;
	};

#define STIVALE2_STRUCT_TAG_EFI_SYSTEM_TABLE_ID 0x4bc5ec15845b558e
	struct tag_efi_system_table {
		stivale2::tag tag;
		uint64_t system_table;
	};

#define STIVALE2_STRUCT_TAG_KERNEL_FILE_ID 0xe599d90c2975584a
	struct tag_kernel_file {
		stivale2::tag tag;
		uint64_t kernel_file;
	};

#define STIVALE2_STRUCT_TAG_KERNEL_FILE_V2_ID 0x37c13018a02c6ea2
	struct tag_kernel_file_v2 {
		stivale2::tag tag;
		uint64_t kernel_file;
		uint64_t kernel_size;
	};

#define STIVALE2_STRUCT_TAG_KERNEL_SLIDE_ID 0xee80847d01506c57
	struct tag_kernel_slide {
		stivale2::tag tag;
		uint64_t kernel_slide;
	};

#define STIVALE2_STRUCT_TAG_SMBIOS_ID 0x274bd246c62bf7d1
	struct tag_smbios {
		stivale2::tag tag;
		uint64_t flags;
		uint64_t smbios_entry_32;
		uint64_t smbios_entry_64;
	};

#define STIVALE2_STRUCT_TAG_SMP_ID 0x34d1d96339647025
	struct stivale2_smp_info {
		uint32_t processor_id;
		uint32_t lapic_id;
		uint64_t target_stack;
		uint64_t goto_address;
		uint64_t extra_argument;
	};

	struct tag_smp {
		stivale2::tag tag;
		uint64_t flags;
		uint32_t bsp_lapic_id;
		uint32_t unused;
		uint64_t cpu_count;
		struct stivale2_smp_info smp_info[];
	};

#define STIVALE2_STRUCT_TAG_PXE_SERVER_INFO 0x29d1e96239247032
	struct tag_pxe_server_info {
		stivale2::tag tag;
		uint32_t server_ip;
	};

#define STIVALE2_STRUCT_TAG_MMIO32_UART 0xb813f9b8dbc78797
	struct tag_mmio32_uart {
		stivale2::tag tag;
		uint64_t addr;
	};

#define STIVALE2_STRUCT_TAG_DTB 0xabb29bd49a2833fa
	struct tag_dtb {
		stivale2::tag tag;
		uint64_t addr;
		uint64_t size;
	};

#define STIVALE2_STRUCT_TAG_VMAP 0xb0ed257db18cb58f
	struct vmap {
		stivale2::tag tag;
		uint64_t addr;
	};

#undef _stivale2_split64
}

// Boot services initialization
namespace boot {
	int init();
}

#endif
