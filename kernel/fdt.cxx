#include <fdt.hxx>
#include <storage.hxx>
#include <locale.hxx>

constinit void *fdt::fdt_address = nullptr;

const void *fdt::get_node(const fdt::header *hdr, const char *key) {
	const auto *fdt = reinterpret_cast<const uint8_t *>(hdr);
	const auto *off = fdt;

	// Translate if required
	char keybuf[100];
	if(locale::charset::NATIVE != locale::charset::ASCII) {
		storage_string::copy(keybuf, key);
		for(size_t i = 0; i < storage_string::length(keybuf); i++)
			keybuf[i] = locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>(keybuf[i]);
		key = keybuf;
	}
	
	key = storage_string::find_char(key, locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>('/'));
	if(key == nullptr) return nullptr;
	key++;

	char *tmpbuf = storage::alloc<char>(storage_string::length(key) + 1);
	if(tmpbuf == nullptr) return nullptr;

	storage_string::copy(tmpbuf, key);

	// Search node
	while(1) {
		const uint32_t token = fdt::get_token(off);
		debug_printf("FDT,SEARCH=%s,OFF=%p,FDT=%p,TOK=%u", key, off, fdt, static_cast<size_t>(token));

		off += sizeof(uint32_t);
		switch(token) {
		case fdt::token_type::NOP:
			break;
		case fdt::token_type::BEGIN_NODE: {
			// Remove the address (if any) off the name for better shit
			auto *addr = storage_string::find_char(tmpbuf, locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>('@'));
			if(addr != nullptr) *addr = '\0';

			// Only preserve name
			addr = storage_string::find_char(tmpbuf, locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>('/'));
			if(addr != nullptr) *addr = '\0';

			debug_printf("FDT,NODE=%s", off);
			// TODO: This can cause some problems...
			if(!storage::compare(off, tmpbuf, storage_string::length(tmpbuf))) {
				// Next / in path
				key = storage_string::find_char(key, locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>('/'));
				// End of path
				if(key == nullptr) {
					storage::free(tmpbuf);
					return off;
				}
				// Need to find next node
				else {
					key++;
					storage_string::copy(tmpbuf, key);
				}
			}

			// Skip until zero-found
			while(*off != '\0') off++;
			off = fdt::padding<uint8_t>(off);
		} break;
		case fdt::token_type::END_NODE:
			break;
		case fdt::token_type::PROP: {
			const auto *prop = reinterpret_cast<const fdt::property_entry *>(off);
			off += sizeof(fdt::property_entry);
			off += be_to_cpu32(prop->len);
			off = fdt::padding<uint8_t>(off);
		} break;
		case fdt::token_type::END:
			goto end;
		default:
			break;
		}
	}
end:
	storage::free(tmpbuf);
	return nullptr;
}

void *fdt::get_node_address(const void *node) {
	// Addresses are in hex
	const auto *addr = storage_string::find_char(reinterpret_cast<const char *>(node), locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>('@'));
	if(addr != nullptr) {
		addr++; // Skip the at (@) symbol

		// Obtain integer in 16-base decimal
		uintptr_t num = 0;
		auto ch = locale::toupper(locale::convert<char, locale::charset::ASCII, locale::charset::NATIVE>(*addr));
		while(locale::isxdigit(ch)) {
			num *= 16;
			switch(ch) {
			case '0':
				num += 0;
				break;
			case '1':
				num += 1;
				break;
			case '2':
				num += 2;
				break;
			case '3':
				num += 3;
				break;
			case '4':
				num += 4;
				break;
			case '5':
				num += 5;
				break;
			case '6':
				num += 6;
				break;
			case '7':
				num += 7;
				break;
			case '8':
				num += 8;
				break;
			case '9':
				num += 9;
				break;
			case 'A':
				num += 10;
				break;
			case 'B':
				num += 11;
				break;
			case 'C':
				num += 12;
				break;
			case 'D':
				num += 13;
				break;
			case 'E':
				num += 14;
				break;
			case 'F':
				num += 15;
				break;
			default:
				break;
			}
			addr++;
			ch = locale::toupper(locale::convert<char, locale::charset::ASCII, locale::charset::NATIVE>(*addr));
		}
		return reinterpret_cast<void *>(num);
	}
	return nullptr;
}

const fdt::property_entry *fdt::get_prop(const fdt::header *hdr, const void *_off, const char *key) {
	const auto *off = reinterpret_cast<const uint8_t *>(_off);
	const auto *fdt = reinterpret_cast<const uint8_t *>(hdr);

	// Translate if required
	char keybuf[100];
	if(locale::charset::NATIVE != locale::charset::ASCII) {
		storage_string::copy(keybuf, key);
		for(size_t i = 0; i < storage_string::length(keybuf); i++)
			keybuf[i] = locale::convert<char, locale::charset::NATIVE, locale::charset::ASCII>(keybuf[i]);
		key = keybuf;
	}

	while(1) {
		uint32_t token = fdt::get_token(off);
		off += sizeof(uint32_t);
		switch(token) {
		case fdt::token_type::NOP:
			break;
		case fdt::token_type::BEGIN_NODE:
			while (*off != '\0')
				off++;

			off = fdt::padding<uint8_t>(off);
			break;
		case fdt::token_type::END_NODE:
			goto end;
		case fdt::token_type::PROP: {
			const auto *prop = reinterpret_cast<const fdt::property_entry *>(off);
			off += sizeof(fdt::property_entry);
			const void *prop_key = fdt + be_to_cpu32(hdr->off_dt_strings) + be_to_cpu32(prop->off_name);
			if(!storage_string::compare(reinterpret_cast<const char *>(prop_key), key))
				return prop;
			off += be_to_cpu32(prop->len);
			off = fdt::padding<uint8_t>(off);
		} break;
		case fdt::token_type::END:
			goto end;
		default:
			break;
		}
	}
end:
	return nullptr;
}

const void *fdt::get_prop_value(const struct fdt::property_entry *prop) {
	return reinterpret_cast<const void *>(prop + 1);
}

uint32_t fdt::get_prop_size(const struct fdt::property_entry *prop) {
	return be_to_cpu32(prop->len);
}
