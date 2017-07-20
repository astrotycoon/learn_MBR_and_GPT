#ifndef _PT_GPT_H
#define _PT_GPT_H

#include <inttypes.h>

/**
 * SECTION: gpt
 * @title: UEFI GPT
 * @short_description: specific functionality
 */

#define GPT_HEADER_SIGNATURE 0x5452415020494645LL /* EFI PART */
#define GPT_HEADER_REVISION_V1_02 0x00010200
#define GPT_HEADER_REVISION_V1_00 0x00010000
#define GPT_HEADER_REVISION_V0_99 0x00009900
#define GPT_HEADER_MINSZ          92 /* bytes */

#define GPT_PMBR_LBA        0
#define GPT_MBR_PROTECTIVE  1
#define GPT_MBR_HYBRID      2

#define GPT_PRIMARY_PARTITION_TABLE_LBA 0x00000001ULL

#define EFI_PMBR_OSTYPE     0xEE
#define MSDOS_MBR_SIGNATURE 0xAA55
#define GPT_PART_NAME_LEN   (72 / sizeof(uint16_t))
#define GPT_NPARTITIONS     FDISK_GPT_NPARTITIONS_DEFAULT

/* Globally unique identifier */
struct gpt_guid {
	uint32_t   time_low;
	uint16_t   time_mid;
	uint16_t   time_hi_and_version;
	uint8_t    clock_seq_hi;
	uint8_t    clock_seq_low;
	uint8_t    node[6];
};


/* only checking that the GUID is 0 is enough to verify an empty partition. */
#define GPT_UNUSED_ENTRY_GUID						\
	((struct gpt_guid) { 0x00000000, 0x0000, 0x0000, 0x00, 0x00,	\
			     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }})

/* Linux native partition type */
#define GPT_DEFAULT_ENTRY_TYPE "0FC63DAF-8483-4772-8E79-3D69D8477DE4"

/*
 * Attribute bits
 */
enum {
	/* UEFI specific */
	GPT_ATTRBIT_REQ      = 0,
	GPT_ATTRBIT_NOBLOCK  = 1,
	GPT_ATTRBIT_LEGACY   = 2,

	/* GUID specific (range 48..64)*/
	GPT_ATTRBIT_GUID_FIRST	= 48,
	GPT_ATTRBIT_GUID_COUNT	= 16
};

#define GPT_ATTRSTR_REQ		"RequiredPartition"
#define GPT_ATTRSTR_REQ_TYPO	"RequiredPartiton"
#define GPT_ATTRSTR_NOBLOCK	"NoBlockIOProtocol"
#define GPT_ATTRSTR_LEGACY	"LegacyBIOSBootable"

/* The GPT Partition entry array contains an array of GPT entries. */
struct gpt_entry {
	struct gpt_guid     type; /* purpose and type of the partition */
	struct gpt_guid     partition_guid;
	uint64_t            lba_start;
	uint64_t            lba_end;
	uint64_t            attrs;
	uint16_t            name[GPT_PART_NAME_LEN];
}  __attribute__ ((packed));

/* GPT header */
struct gpt_header {
	uint64_t            signature; /* header identification */
	uint32_t            revision; /* header version */
	uint32_t            size; /* in bytes */
	uint32_t            crc32; /* header CRC checksum */
	uint32_t            reserved1; /* must be 0 */
	uint64_t            my_lba; /* LBA of block that contains this struct (LBA 1) */
	uint64_t            alternative_lba; /* backup GPT header */
	uint64_t            first_usable_lba; /* first usable logical block for partitions */
	uint64_t            last_usable_lba; /* last usable logical block for partitions */
	struct gpt_guid     disk_guid; /* unique disk identifier */
	uint64_t            partition_entry_lba; /* LBA of start of partition entries array */
	uint32_t            npartition_entries; /* total partition entries - normally 128 */
	uint32_t            sizeof_partition_entry; /* bytes for each GUID pt */
	uint32_t            partition_entry_array_crc32; /* partition CRC checksum */
	uint8_t             reserved2[512 - 92]; /* must all be 0 */
} __attribute__ ((packed));

static inline struct gpt_entry *gpt_get_partition(uint8_t *gpt, int i)   
{
	return (struct gpt_entry *)(gpt + (i * sizeof(struct gpt_entry)));
}

static inline uint64_t gpt_partition_get_lab_start(const struct gpt_entry *gpt) 
{
	return gpt->lba_start;
}

static inline void gpt_partition_set_lab_start(struct gpt_entry *gpt, uint64_t start)
{
	gpt->lba_start = start;
}

static inline uint64_t gpt_partition_get_lba_end(const struct gpt_entry *gpt)
{
	return gpt->lba_end;
}

static inline void gpt_partition_set_lba_end(struct gpt_entry *gpt, uint64_t end)
{
	gpt->lba_end = end;
}

static inline const char *gpt_partition_get_name(struct gpt_entry *gpt)
{
	return (const char *)gpt->name;
}

static inline void gpt_partition_set_name(struct gpt_entry *gpt, const char *name)
{
	(void)snprintf((char *)gpt->name, sizeof(gpt->name), "%s", name);
}

#endif	/* _PT_GPT_H */
