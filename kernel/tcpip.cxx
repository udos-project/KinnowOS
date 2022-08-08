#include <tcpip.hxx>
#include <storage.hxx>
#include <errcode.hxx>
#include <byteswap.hxx>

tcpip::ethernet_packet *tcpip::ethernet_packet::create(tcpip::mac_addr dest_mac, tcpip::mac_addr src_mac)
{
	// Allocate the temporal packet structure
	auto *eth = (tcpip::ethernet_packet *)storage::allocz(sizeof(tcpip::ethernet_packet));
	if(eth == nullptr) return nullptr;
	eth->src_mac = src_mac; // Copy mac addresses
	eth->dest_mac = dest_mac;
	return eth;
}

int tcpip::ethernet_packet::send(virtual_disk::handle& hdl, void *data, size_t n_data)
{
	if(n_data == 0) return error::INVALID_PARAM;
	const auto rem = n_data % MAX_ETHERNET_PAYLOAD;
	// Send the entire packets with max. payloads
	for(size_t n_packets = (n_data - rem) / MAX_ETHERNET_PAYLOAD; n_packets != 0; n_packets--) {
		this->length = MAX_ETHERNET_PAYLOAD;
		storage::copy(this->payload, data, MAX_ETHERNET_PAYLOAD);
		this->crc = this->calc_crc32(0, reinterpret_cast<const uint8_t *>(&this->dest_mac), 14 + MAX_ETHERNET_PAYLOAD);
		hdl.write(this, 22 + MAX_ETHERNET_PAYLOAD + 4); // 22 bytes of header + payload + 32-bit crc
	}
	// Now send the remainder bytes
	debug_assert(rem <= MAX_ETHERNET_PAYLOAD);
	this->length = cpu_to_be16(static_cast<uint16_t>(rem)); // This cast is okay, rem can't ever be >MAX_ETHERNET_PAYLOAD
	storage::copy(this->payload, data, rem);
	// The CRC is not at the end of the 1500 payload but rather inside it
	const uint32_t _crc = cpu_to_be32(this->calc_crc32(0, reinterpret_cast<const uint8_t *>(&this->dest_mac), 14 + rem));
	storage::copy(&this->payload[rem], &_crc, sizeof(uint32_t));
	hdl.write(this, 22 + rem + 4);
	return 0;
}

void tcpip::ethernet_packet::destroy(tcpip::ethernet_packet *eth)
{
	storage::free(eth);
}
