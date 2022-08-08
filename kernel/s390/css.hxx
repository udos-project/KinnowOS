#pragma once

#include <types.hxx>
#include <storage.hxx>
#include <s390/asm.hxx>
#include <printf.hxx>

#include <mutex.hxx>

#define MAX_CSS_REQUESTS 1024
#define CSS_CCW_CD ((1) << S390_BIT(8, 0)) // Command chain word flags
#define CSS_CCW_CC ((1) << S390_BIT(8, 1))
#define CSS_CCW_SLI ((1) << S390_BIT(8, 2))
#define CSS_CCW_SPK ((1) << S390_BIT(8, 3))
#define CSS_CCW_PCI ((1) << S390_BIT(8, 4))
#define CSS_CCW_IDA ((1) << S390_BIT(8, 5))
#define CSS_CCW_S ((1) << S390_BIT(8, 6))
#define CSS_CCW_MIDA ((1) << S390_BIT(8, 7))
#define CSS_PMCW_ISC(x) ((x) << S390_BIT(16, 2)) // I/O Interrupt subclass code
#define CSS_PMCW_ENABLED ((1) << S390_BIT(16, 8)) // Enabled for all I/O functions
#define CSS_PMCW_LIMIT ((1) << S390_BIT(16, 9)) // Limit mode
#define CSS_PMCW_MM_ENABLE ((1) << S390_BIT(16, 11)) // Measurement mode enable
#define CSS_PMCW_MULTIPATH_MODE ((1) << S390_BIT(16, 13)) // Multipath mode
#define CSS_PMCW_TIMING ((1) << S390_BIT(16, 14)) // Timing facility
#define CSS_PMCW_DNV ((1) << S390_BIT(16, 15)) // Device number valid
#define CSS_ORB_SUSPEND ((1) << S390_BIT(32, 4)) // Suspend control
#define CSS_ORB_STREAMING_MODE ((1) << S390_BIT(32, 5)) // Streaming mode for subchannel mode
#define CSS_ORB_SYNC(x) ((x) << S390_BIT(32, 6)) // Synchronization control
#define CSS_ORB_FORMAT ((1) << S390_BIT(32, 8)) // Format control
#define CSS_ORB_PREFETCH ((1) << S390_BIT(32, 9)) // Prefetch control
#define CSS_ORB_ISI ((1) << S390_BIT(32, 10)) // Initial status interrupt control
#define CSS_ORB_ADDRESS_LIMIT ((1) << S390_BIT(32, 11)) // Address limit control
#define CSS_ORB_SUPRESS_SUSPEND_INT ((1) << S390_BIT(32, 12)) // Supress suspend interrupt control
#define CSS_ORB_FORMAT_2_IDAW ((1) << S390_BIT(32, 14)) // Format IDAW for CCW
#define CSS_ORB_2K_IDAW ((1) << S390_BIT(32, 15)) // 2K Indirect data address word control
#define CSS_ORB_LPM(x) ((x) << S390_BIT(32, 16)) // Logical path mask control
#define CSS_ORB_MODIFIED_IDA(x) ((x) << S390_BIT(32, 25)) // Modified CCW indirect data addressing control
#define CSS_ORB_EXTENSION(x) ((x) << S390_BIT(32, 31)) // ORB Extension Control
#define CSS_SCSW_DS_ATTENTION ((1) << S390_BIT(8, 0)) // Attention bit

namespace css {
	/* Subchannel id */
	struct schid {
		uint16_t id;
		uint16_t num;
	} PACKED;

	/* Flags and stuff */
	struct ccw {
		constexpr ccw(uint8_t _cmd, uint8_t _flags, uint16_t _length, void *_addr)
			: cmd{ _cmd },
			flags{ _flags },
			length{ _length },
			addr{ 0 }
		{
			this->set_addr(_addr);
		}

#if MACHINE >= M_S390
		/* Channel control word format 1 */
		template<typename T>
		constexpr void set_addr(T *a)
		{
			this->addr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(a));
		}

		uint8_t cmd;
		uint8_t flags;
		uint16_t length;
		uint32_t addr;
#else
		/* Channel control word format 0 */
		template<typename T>
		constexpr void set_addr(T *a)
		{
			this->lo_addr = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(a) & 0xffff);\
			this->hi_addr = static_cast<uint8_t>(((reinterpret_cast<uintptr_t>(a)) >> 16) & 0xff);
		}

		uint8_t cmd;
		uint16_t lo_addr;
		uint8_t hi_addr;
		uint8_t flags;
		uint8_t reserved;
		uint16_t length;
#endif
	} __attribute__((packed, aligned(4)));

	namespace cmd {
		/// @brief General purpouse channel subsystem command codes
		/// See z/Architecture Principles of Operation, Page 29, Figure 15-5
		enum cmd {
			WRITE = 0x01,
			READ = 0x02,
			READ_BACKWARDS = 0x0C,
			CONTROL = 0x03,
			// Obtain basic sense information from device (for identifying the type of
			// device of course)
			SENSE = 0x04,
			SENSE_ID = 0xE4,
			TIC = 0x08 // Transfer in Chnannel - Usuaully to retry last failed operation
			// "Enable this device" */
		};
	};

	/* Path management control word */
	struct pmcw {
		uint32_t int_param;
		uint16_t flags;
		uint16_t dev_num;
		uint8_t lpm; /* Logical path mask */
		uint8_t pnom;
		uint8_t lpum;
		uint8_t pim;
		uint16_t mbi;
		uint8_t pom;
		uint8_t pam;
		uint8_t chpid[8];
		uint8_t zero[3];
		uint8_t last_flags; /* Last 3 bits contains flags */
	} PACKED;

	/* See z/Architecture Principles of Operation, Page 33 */
	/* Extended status word (format 0) */
	struct esw0 {
		uint32_t sc_logout; /* Subchannel logout */
		uint32_t report; /* Extended report word */
		uint64_t fail_addr; /* Failing storage address */
	} PACKED;

	/* Subchannel status word */
	struct scsw {
		uint32_t flags;
		uint32_t cpa_addr;
		uint8_t device_status;
		uint8_t subchannel_status;
		uint16_t count;
	} PACKED;

	/* Subchannel information block */
	struct schib {
		css::pmcw pmcw;
		css::scsw scsw;
		union {
			uint64_t mb_addr;
			uint32_t md_data[3];
		};
	} __attribute__((packed, aligned(4)));

	/* Operation request block */
	struct orb {
		uint32_t int_param;
		uint32_t flags;
		uint32_t cpa_addr;
		/* Only used when the ORB extension control is set */
		uint8_t css_priority;
		uint8_t reserved1;
		uint8_t cu_priority;
		uint8_t reserved2;
		uint32_t reserved3[4];
	} __attribute__((packed, aligned(4)));

	/* Interrupt request block */
	struct irb {
		css::scsw scsw;
		uint32_t esw[5];
		uint32_t ecw[8];
		uint32_t emw[8];
	} __attribute__((packed, aligned(4)));

	/* Command information word */
	typedef uint32_t ciw_t;

	/* SenseId data */
	struct senseid {
		uint8_t reserved;
		uint16_t cu_type;
		uint8_t cu_model;
		uint16_t dev_type;
		uint8_t dev_model;
		uint8_t unused;

		/* Extended SENSEID data */
		ciw_t ciw[8];
	} __attribute__((packed, aligned(4)));

	struct device {
		using id = int16_t; // For safety!

		device& operator=(device&) = delete;
		const device& operator=(const device&) = delete;
		
		css::orb orb;
		css::irb irb;
		css::schid schid;
		css::schib schib;
		css::senseid sense;
		base::mutex lock;
	};

	struct request {
		request& operator=(request&) = delete;
		const request& operator=(const request&) = delete;

		static css::request *create(css::device& dev, size_t n_ccws);
		static void destroy(css::request *req);
		int send();
		void dump() const;

		// Flags, used both to indicate the spooler how to operate & the status of this request
		unsigned char flags;
		css::schid schid;
		storage::dynamic_list<css::ccw> ccws;
		// Return code set by spooler, not valid until css::request_flags::DONE is set on flags
		int retcode;
		base::mutex lock;
	};

	namespace request_flags {
		enum request_flags {
			MODIFY = 0x01,
			IGNORE_CC = 0x02,
			WAIT_ATTENTION = 0x04,
			/* Set by the spooler to indicate the request is done */
			DONE = 0x10,
		};
	};

	namespace status {
		enum status {
			OK = 0,
			PENDING = 1,
			NOT_PRESENT = 3,
		};
	};

	static inline int channel_start(css::schid schid, css::orb *schib)
	{
		register css::schid r1 asm("1") = schid;
		int cc = -1;
		debug_assertm(schid.id != 0, "Invalid schid");
		asm volatile("SSCH %2\r\nIPM %0\r\n" : "+d"(cc) : "d"(r1), "m"(*schib) : "cc");
		return cc >> 28;
	}

	static inline int channel_store(css::schid schid, css::schib *schib)
	{
		register css::schid r1 asm("1") = schid;
		int cc = -1;
		debug_assertm(schid.id != 0, "Invalid schid");
		asm volatile("STSCH %1\r\nIPM %0\r\n" : "+d"(cc), "=m"(*schib) : "d"(r1) : "cc");
		return cc >> 28;
	}

	static inline int channel_modify(css::schid schid, css::orb *schib)
	{
		register css::schid r1 asm("1") = schid;
		int cc = -1;
		debug_assertm(schid.id != 0, "Invalid schid");
		asm volatile("MSCH %2\r\nIPM %0\r\n" : "=d"(cc) : "d"(r1), "m"(*schib) : "cc");
		return cc >> 28;
	}

	static inline int channel_test(css::schid schid, css::irb *schib)
	{
		register css::schid r1 asm("1") = schid;
		int cc = -1;
		debug_assertm(schid.id != 0, "Invalid schid");
		asm volatile("TSCH %1\r\nIPM %0\r\n" : "=d"(cc), "=m"(*schib) : "d"(r1) : "cc");
		return cc >> 28;
	}

	int init();
	int request_perform();
	css::device::id add_device(css::schid schid);
	css::device *get_device(css::schid schid);
	css::device *get_device(css::device::id id);
	int probe();
	int dev_enable(css::device& dev);
}
