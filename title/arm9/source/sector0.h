#ifndef SECTOR0
#define SECTOR0

#include <stdint.h>
#include <assert.h>

// https://3dbrew.org/wiki/NCSD#NCSD_header

#define SECTOR_SIZE 0x200

#define NCSD_PARTITIONS 8

#ifdef _MSC_VER
#pragma pack(push, 1)
#define __PACKED
#elif defined __GNUC__
#define __PACKED __attribute__ ((__packed__))
#endif

typedef struct {
	uint32_t offset;
	uint32_t length;
} __PACKED ncsd_partition_t;

typedef struct {
	uint8_t signature[0x100];
	uint32_t magic;
	uint32_t size;
	uint32_t media_id_l;
	uint32_t media_id_h;
	uint8_t fs_types[NCSD_PARTITIONS];
	uint8_t crypt_types[NCSD_PARTITIONS];
	ncsd_partition_t partitions[NCSD_PARTITIONS];
} __PACKED ncsd_header_t;

typedef struct {
	uint8_t head;
	uint8_t sector;
	uint8_t cylinder;
} __PACKED chs_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif
#undef __PACKED


#ifdef __cplusplus
extern "C" {
#endif

static_assert(sizeof(ncsd_header_t) == 0x160, "sizeof(ncsd_header_t) should equal 0x160");

int parse_ncsd(const uint8_t sector0[SECTOR_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // SECTOR0