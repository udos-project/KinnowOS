#ifndef TCPIP_HXX
#define TCPIP_HXX

#include <types.hxx>
#include <vdisk.hxx>
#include <byteswap.hxx>

#define MAX_ETHERNET_PAYLOAD 1500
#define MIN_ETHERNET_PAYLOAD 46

namespace tcpip {
	struct mac_addr {
		uint8_t addr[6];
	};

	struct ipv4_addr {
		uint8_t addr[4];
	};

	struct ipv6_addr {
		uint8_t addr[6];
	};

	// Ethernet Frame
	// Reference: https://en.wikipedia.org/wiki/Ethernet_frame
	struct ethernet_packet {
		ethernet_packet() = delete;
		~ethernet_packet() = delete;
		ethernet_packet& operator=(ethernet_packet&) = delete;
		const ethernet_packet& operator=(const ethernet_packet&) = delete;

		static tcpip::ethernet_packet *create(tcpip::mac_addr dest_mac, tcpip::mac_addr src_mac);
		static void destroy(tcpip::ethernet_packet *eth);
		
		constexpr uint32_t calc_crc32(uint32_t _crc, const void *_data, size_t len)
		{
			const auto *data = reinterpret_cast<const uint8_t *>(_data);
			_crc ^= 0xFFFFFFFF;
			while(len--) {
				_crc ^= *(data++);
				for(size_t i = 0; i < 8; i++)
					_crc = _crc & 1 ? (_crc >> 1) ^ 0xEDB88320 : _crc >> 1;
			}
			return _crc ^ 0xFFFFFFFF;
		}

		int send(virtual_disk::handle& hdl, void *data, size_t n_data);

		tcpip::mac_addr dest_mac; // Destination MAC address
		tcpip::mac_addr src_mac; // Source MAC address
		union {
			uint16_t length; // IEEE 802.3, only valid if <=1500
			uint16_t type; // Ethernet II, only valid if >1536
		};
		uint8_t payload[MAX_ETHERNET_PAYLOAD]; // Payload
		uint32_t crc; // Frame-Sequence-Check (FSC) or also CRC32
	};

#define ARP_HTYPE_ETHERNET 0x01
#define ARP_PTYPE_IPV4 0x0800
#define ARP_OP_REQUEST 0x01
#define ARP_OP_REPLY 0x02

	// ARP Frame
	// Reference: https://en.wikipedia.org/wiki/Address_Resolution_Protocol
	struct arp_packet {
		uint16_t hw_type;
		uint16_t prot_type;
		uint8_t hw_size;
		uint8_t prot_size;
		uint16_t opreq; /* Opcode request */
		tcpip::mac_addr src_hw_addr; /* Source MAC */
		tcpip::ipv4_addr src_prot_addr; /* Source IP */
		tcpip::mac_addr dest_hw_addr; /* Receiver MAC */
		tcpip::ipv4_addr dest_prot_addr; /* Receiver IP */
	};

	// IPv4 frame
	// Reference: https://en.wikipedia.org/wiki/IPv4_header
	struct ipv4_frame {
		uint16_t flags1; // 4b for version, 4b for IHL, 6b for DSCP and 2b for ECN
		uint16_t length; // Total length (including header)
		uint16_t id;
		union {
			uint16_t flags2;
			uint16_t offset; // Lower 3 bits are flags
		};
		uint8_t time_to_live;
		uint8_t protocol;
		uint16_t checksum; // Header checksum
		tcpip::ipv4_addr src_ip;
		tcpip::ipv4_addr dest_ip;
		// Options only if IHL > 5
	};

	struct ipv6_frame {
		uint32_t flags; // 4b for version, 8b for traffic class and 20b for flow label */
		uint16_t length; // Length of payload
		tcpip::ipv6_addr src_ip;
		tcpip::ipv6_addr dest_ip;
	};

	struct icmp_packet {
		constexpr uint32_t calc_crc32(const void *_data, const size_t len)
		{
			const auto *data = reinterpret_cast<const uint8_t *>(_data);
			uint32_t counter = 0;

			// All add bytes as if they were 16-bit words
			for(size_t i = 0; i < len; i += 2)
				counter += cpu_to_be16(*(reinterpret_cast<const uint16_t *>(data[i])));

			// Uneven bytes are also accounted for
			if(len % 2) counter += data[len - 1];

			// Fold into 16-bits
			while(counter & 0xffff0000)
				counter += ((counter >> 16) & 0xffff);
			
			// Return 1's complement
			return ~(counter);
		}

		uint8_t type; // Type of packet
		uint8_t code;
		uint16_t checksum; // Checksum of the packet
	};

	struct icmp_echo : public icmp_packet {
		uint16_t id; // Identifier (always zero)
		uint16_t seq; // Sequence number
	};

	// TCP packet
	// Reference: https://en.wikipedia.org/wiki/Transmission_Control_Protocol
	struct tcp_packet {
		uint16_t src_port;
		uint16_t dest_port;
		uint32_t seq; // Sequence number
		uint32_t acknum; // Acknewledge number (if ACK is set)
		uint8_t data_offset;
		uint8_t flags;
		uint16_t winsize;
		uint16_t checksum; // Checksum of the packet
		uint16_t urgent_ptr; // Urgent pointer (if URG is set)
	};
}

#endif
