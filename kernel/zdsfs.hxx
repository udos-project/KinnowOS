/*
 * z/OS Dataset filesystem support
 */

#ifndef ZDSFS_HXX
#define ZDSFS_HXX

#include <types.hxx>
#include <vdisk.hxx>

#define ZDSFS_IOCTL_NEW_FILE 0x01
#define ZDSFS_IOCTL_FTELL 0x02
#define ZDSFS_IOCTL_SEEK 0x03

#define ZDSFS_SEEK_SET 0
#define ZDSFS_SEEK_CUR 1
#define ZDSFS_SEEK_END 2

namespace zdsfs {
	/**
	 * @brief File dataset control block, used for tapes and disks and tracking files
	 * 
	 */
	struct fdscb {
		uint16_t cyl;
		uint16_t head;
		uint8_t rec;
	};

	/* A nice way to structure dates */
	struct date {
		uint8_t year; /* Add 1900 to obtain the final year */
		uint16_t day;
	} PACKED;

	/** @todo Document this */
	struct vtoc {
		uint8_t unknown[15];
		uint16_t chain_cyl;
		uint16_t chain_head;
		uint8_t chain_rec;
	} PACKED;

	/**
	 * @brief The structure of a DSCB format 1, adapted from PDOS/370 code
	 * 
	 */
	struct dscb_fmt1 {
		uint8_t name[44];
		uint8_t format_id;
		uint8_t serial_volume[6];
		uint8_t vol_seq[2]; /* Volume sequence number */
		zdsfs::date creation_date;
		zdsfs::date expiry_date;
		uint8_t n_extents;
		uint8_t n_free_bytes;
		uint8_t flag_1;
		uint8_t sys_code[13];
		zdsfs::date reference_date; /* Access date/Reference date */
		uint8_t msg_fg;
		uint8_t scxtf;
		uint16_t scxtv;
		uint16_t dsorg; /* Dataset organization PS or PO */
		uint8_t recfm; /* Record format F/V/U */
		uint8_t opt_code; /* BDAM and ISAM things */
		uint16_t block_size;
		uint16_t lrecl; /* Logical record length */
		uint8_t keylen; /* Key lenght */
		uint8_t rkp[2];
		uint8_t dsind;
		uint8_t cal1; /* Is this a cylinder request? */
		uint8_t scal3[3]; /* Size of secondary allocation */
		uint8_t lstar[3]; /* Last TTR actually used */
		uint8_t trbal[2]; /* ??? */
		uint8_t reserved1;
		uint8_t ttthi; /* High byte of the TTR number */
		uint8_t ext1[2]; /* First extent, first byte is an extent indicator
						* the second is the extent sequence number */
		uint16_t start_cc;
		uint16_t start_hh;
		uint16_t end_cc;
		uint16_t end_hh;
		uint8_t ext2[10];
		uint8_t ext3[10];
		uint16_t fmt3_cc; /* CCHHR pointing to Format-3 chain */
		uint16_t fmt3_hh;
		uint8_t fmt3_r;
		/* The structure of the Format 3 for reference:
		* https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.idas300/s3022.htm */
	} PACKED;

	/**
	 * @brief Global driver data
	 * 
	 */
	struct driver_data {
		virtual_disk::driver *driver = nullptr;
		virtual_disk::handle *dev = nullptr;
	};

	/**
	 * @brief Data stored per node by the driver
	 * 
	 */
	struct node_data {
		zdsfs::dscb_fmt1 dscb1;
		zdsfs::driver_data *driver_data;
	};
	
	/**
	 * @brief Data stored per handle by the ZDSFS driver
	 * 
	 */
	struct handle_data {
		long seekpos;
	};

	int init(virtual_disk::handle& dev);
}

#endif
