/**
 * @file extx.cxx
 * @author wxwisiasdf
 * @brief Implements all EXT(x) filesystems
 * @version 0.1
 * @date 2022-06-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef EXTX_HXX
#define EXTX_HXX

#include <types.hxx>
#include <printf.hxx>
#include <vdisk.hxx>

#define EXT4_MAGIC 0xEF53

/* Structures reference and sources:
 * https://wiki.osdev.org/Ext4
 */

/// @brief Features used by the ext4 filesystem, used in various superblock structures (journal + normal)
enum ext4_required_feature {
	/* Compression support */
	EXT4_FEAT_COMPRESS = 0x0001,
	/* Directory entries contain a type field */
	EXT4_FEAT_DIRENT_TYPE = 0x00002,
	/* Filesystem needs to replay a journal to recover data */
	EXT4_FEAT_JOURNAL_REPLAY = 0x00004,
	/* Filesystem has/uses a journal device */
	EXT4_FEAT_USES_JOURNAL = 0x00008,
	/* Makes use of meta groups */
	EXT4_FEAT_META_GROUPS = 0x00010,
	/* Uses extents for files */
	EXT4_FEAT_FILE_EXTENT = 0x00040,
	/* Has headers/information for 64-bits */
	EXT4_FEAT_64BIT = 0x00080,
	/* Includes the multiple mount protection */
	EXT4_FEAT_MMP = 0x00100,
	/* Flex block groups */
	EXT4_FEAT_FLEX_BLOCK_GROUPS = 0x00200,
	/* Extended iNode attributes */
	EXT4_FEAT_INODE_EXTATTR = 0x00400,
	/* Data located in directory entries */
	EXT4_FEAT_DIRENT_DATA = 0x01000,
	/* Stores checksum seed in superblock (allowing for changing the UUID without rewriting the meta-blocks) */
	EXT4_FEAT_SEED_CHECKSUM = 0x02000,
	/* Directories can be larger than 4 GB */
	EXT4_FEAT_4GB_DIR = 0x04000,
	/* Inline data in Inodes */
	EXT4_FEAT_INLINE_DATA = 0x08000,
	/* This filesystem makes use of an encryption algorithm */
	EXT4_FEAT_ENCRYPTION = 0x10000,
	/* Makes use of case-folding */
	EXT4_FEAT_CASE_FOLD = 0x20000,
};

/// @todo Add optional features
#define EXT4_FILESYSTEM_MAGIC 0xef53
struct ext4_superblock {
	uint32_t total_inodes;
	uint32_t total_blocks;
	uint32_t reserved_blocks;
	uint32_t total_unallocated_blocks;
	uint32_t total_unallocated_inodes;
	uint32_t superblock_num;
	uint32_t block_size;
	uint32_t fragment_size;
	uint32_t num_blocks_in_group;
	uint32_t num_fragments_in_group;
	uint32_t num_inodes_in_group;
	uint32_t last_mount_time;
	uint32_t last_write_time;
	uint16_t times_mounted;
	uint16_t num_mounts_fsck;
	uint16_t magic;
	uint16_t disk_state;
	uint16_t on_error;
	uint16_t minor_version;
	uint32_t last_fsck_time;
	uint32_t fsck_interval;
	uint32_t os_id;
	uint32_t major_version;
	uint16_t user_id;
	uint16_t group_id;
	
	constexpr bool is_valid() noexcept {
		if(this->magic != EXT4_MAGIC) {
			kprintf("\x01\x13 EXT4 signature");
			return false;
		}
		return true;
	}
};

struct ext4_journal_header {
	uint32_t magic;
	uint32_t block_type;
	uint32_t journal_transaction;
};

#define EXT4_JOURNAL_MAGIC 0xc03b3998
struct ext4_journal_uuid {
	uint64_t low;
	uint64_t high;
};

struct ext4_journal_superblock {
	struct ext4_journal_header header;
	/* Size of the blocks in this journal device */
	uint32_t block_size;
	/* Total number of blocks in this journal device */
	uint32_t total_dev_blocks;
	/* First journal information block*/
	uint32_t first_info_block;
	/* First journal transaction expected */
	uint32_t first_transact_expected;
	/* First journal block */
	uint32_t first_block;
	uint32_t errno;
	uint32_t required_features;
	uint32_t optional_features;
	/* If not implemented/supported then the system must be mounted as read-only */
	uint32_t wronly_features;
	struct ext4_journal_uuid journal_uuid;
	/* Number of filesystems that are using this journal */
	uint32_t n_client_fs;
	/* Block number of this superblock copy (aka. a mirror block) */
	uint32_t mirror_block;
	/* Max. allowed journal blocks per journal transaction */
	uint32_t max_journal_transact_blocks;
	/* Mac. data blocks per each journal transaction */
	uint32_t max_data_transact_blocks;
	uint8_t checksum_algorithm;
	uint8_t padding[3 + 168];
	uint32_t checksum;
	uint8_t disk_uuid[768];
};

#endif
