/**
 * @file fat.cxx
 * @author wxwisiasdf
 * @brief Implements FAT table
 * @version 0.1
 * @date 2022-06-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <types.hxx>

/**
 * @brief FAT BIOS parameter block, at the start of the disk
 * 
 */
struct fat_bpb {
	uint8_t jmp_code[3];
	uint64_t oem_id;
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t fat_tables;
	uint16_t root_dir_entries;
	/* Total number of sectors in the logical volume */
	uint16_t total_logical_sectors;
	uint8_t media_type;
	uint16_t n_sectors;
	uint16_t n_sectors_per_track;
	uint16_t n_heads;
	uint32_t n_hidden_sectors;
	uint32_t lsector_count;
};
