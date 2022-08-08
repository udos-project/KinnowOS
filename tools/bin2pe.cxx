#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <errno.h>
#include <sys/stat.h>

#if defined(__s390x__) || defined(__s390__)
#	define b16_to_be(x) (x)
#	define b32_to_be(x) (x)
#	define b64_to_be(x) (x)
#else
//#	define b16_to_be(x) bswap16(x)
//#	define b32_to_be(x) bswap32(x)
//#	define b64_to_be(x) bswap64(x)
#	define b16_to_be(x) __bswap_16(x)
#	define b32_to_be(x) __bswap_32(x)
#	define b64_to_be(x) __bswap_64(x)
#endif

#define EBCDIC_TXT "\xE3\xE7\xE3"
#define EBCDIC_END "\xC5\xD5\xC4"
#define EBCDIC_NUM(x) ((x) + 0xf0)
#define EBCDIC_SPACE ((0x0f * 3) + 1)
#define MAX_REC_SIZE (2048)

int bin2rec(FILE *in, FILE *out)
{
    const unsigned char zero[80] = {0};
    uint32_t addr = 0;
    uint32_t end_addr = MAX_REC_SIZE;
    uint16_t len = b16_to_be(56);
    while(!feof(in)) {
        unsigned char tmp[56];

        /* Read from binary */
        fread(&tmp[0], 1, 56, in);

        /* New punchcard line */
        size_t w_len = 0, line_len = 0;

        w_len = 1; /* COLUMN 0,0 */
        fwrite("\x02", 1, w_len, out);
        line_len += w_len;

        w_len = 3; /* COLUMN 1,3 */
        if(addr >= end_addr || feof(in)) {
            fwrite(EBCDIC_END, 1, w_len, out);
            w_len = 80 - line_len; /* COLUMN 4,80 */
            fwrite(&zero, 1, w_len, out);
            line_len += w_len;
            break;
        } else {
            fwrite(EBCDIC_TXT, 1, w_len, out);
        }
        line_len += w_len;

        w_len = 1; /* COLUMN 4,4 */
        fwrite(&zero, 1, w_len, out);
        line_len += w_len;

        w_len = 3; /* COLUMN 5,7 */
        uint32_t b_addr = b32_to_be(addr);
        fwrite((char *)&b_addr + 1, 1, w_len, out);
        line_len += w_len;

        w_len = 2; /* COLUMN 8,9 */
        fwrite(&zero, 1, w_len, out);
        line_len += w_len;

        w_len = sizeof(uint16_t); /* COLUMN 10,11 */
        fwrite(&len, 1, w_len, out);
        line_len += w_len;

        w_len = 16 - line_len; /* COLUMN 12,15 */
        fwrite(&zero, 1, w_len, out);
        line_len += w_len;

        w_len = 56; /* COLUMN 16,68 */
        fwrite(&tmp[0], 1, w_len, out);
        line_len += w_len;

        w_len = 80 - line_len; /* COLUMN 68,80 */
        fwrite(&zero, 1, w_len, out);
        line_len += w_len;

        addr += 56;
        if(line_len > 80) {
            printf("Line is bigger than 80 (%u)\n", line_len);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if(argc <= 2) {
        fprintf(stderr, "Usage: bin2txt [in] [out]\n");
        exit(EXIT_FAILURE);
    }

    std::unique_ptr<FILE, decltype(::fclose)*> inp(::fopen(argv[1], "rb"), ::fclose);
    if(inp.get() == nullptr) {
        fprintf(stderr, "Cannot open file %s: %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    std::unique_ptr<FILE, decltype(::fclose)*> out(::fopen(argv[2], "wb"), ::fclose);
    if(out.get() == nullptr) {
        fprintf(stderr, "Cannot create file %s: %s\n", argv[2], strerror(errno));
        exit(EXIT_FAILURE);
    }

    bin2rec(inp.get(), out.get());
    return 0;
}
