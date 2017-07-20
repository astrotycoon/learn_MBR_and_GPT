#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "pt-mbr.h"
#include "pt-gpt.h"

static off_t extended_partition_lba_offset;	/* 扩展分区的LAB地址 */

static void print_gpt_partition(int fd, const uint8_t *mbr);
static void print_gpt_partition_entry(const struct gpt_entry *gpt, int index);

static void print_dos_partition(int fd, uint8_t *mbr);
static void print_dos_subextended_partition(int fd, off_t start_lba_sector);

int main(int argc, const char *argv[])
{
	if (argc != 2) {
		(void)fprintf(stderr, "Usage: %s + disk_name\n", argv[0]);	
		exit(EXIT_FAILURE);
	}
	
	int fd;
	int result = 0;
	uint8_t mbr[512];	/* master boot record buffer */

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(EXIT_FAILURE);	
	}

	if (read(fd, mbr, sizeof(mbr)) != sizeof(mbr)) {
		perror("read");			
		result = -1;
		goto out;
	}

	if (mbr_is_valid_magic(mbr) == 0) {
		(void)fprintf(stderr, "Illegal mbr!!!\n");	
		result = -2;
		goto out;
	}

	struct dos_partition *p = mbr_get_partition(mbr, 0);
	if (p->sys_ind == MBR_GPT_PARTITION) {
		print_gpt_partition(fd, mbr);
	} else {
		print_dos_partition(fd, mbr);
	}

out:
	if (fd) {
		(void)close(fd);	
	}

	return (result);
}

static void print_gpt_partition_entry(const struct gpt_entry *gpt, int index)
{
	printf("  [%3d] ", index);

	char buf[128];

	// type
	(void)snprintf(buf, sizeof(buf), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
			gpt->type.time_low, gpt->type.time_mid, gpt->type.time_hi_and_version, 
			gpt->type.clock_seq_hi, gpt->type.clock_seq_low,
			gpt->type.node[0], gpt->type.node[1], gpt->type.node[2],
			gpt->type.node[3], gpt->type.node[4], gpt->type.node[5]);
	printf("%s", buf);
//	if (strcmp(buf, GPT_DEFAULT_ENTRY_TYPE) == 0) {
//		printf("Linux native partition type");	
//	}
	// partition_guid
	(void)snprintf(buf, sizeof(buf), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			gpt->partition_guid.time_low, gpt->partition_guid.time_mid, gpt->partition_guid.time_hi_and_version, 
			gpt->partition_guid.clock_seq_hi, gpt->partition_guid.clock_seq_low,
			gpt->partition_guid.node[0], gpt->partition_guid.node[1], gpt->partition_guid.node[2],
			gpt->partition_guid.node[3], gpt->partition_guid.node[4], gpt->partition_guid.node[5]);
	printf(" %s", buf);

	// lba_start
	printf("%10llu", gpt_partition_get_lab_start(gpt));
	// lba_end
	printf("%16llu", gpt_partition_get_lba_end(gpt));
	//  
	printf("%13llu    ", (gpt_partition_get_lba_end(gpt) - gpt_partition_get_lab_start(gpt) + 1) * 512);
	// name
	const uint16_t *p = gpt->name;	
	for ( ;; ) {
		const char c = (const char )*p;	
		if (c == '\0') break;
		printf("%c", c);
		p++;
	}
}

static void print_gpt_partition(int fd, const uint8_t *mbr)
{
	printf("Protective MBR:\n");
	printf("  Type: GPT Partition\n\n");

	struct gpt_header gpthdr;	
	int i;

	if (read(fd, &gpthdr, sizeof(gpthdr)) != sizeof(gpthdr)) {
		perror("read");	
		return;
	}

	if (gpthdr.signature != GPT_HEADER_SIGNATURE) {
		fprintf(stderr, "Wrong GPT signature\n");	
		return;
	}

	printf("GPT Header:\n");
	printf("  Signature: EFI PART\n");
	printf("  Version: 0x%08x\n", gpthdr.revision);
	printf("  Hdr Size: %u\n", gpthdr.size);
	printf("  Hdr Crc32: 0x%08x\n", gpthdr.crc32);
	printf("  Reserved: 0x%08x\n", gpthdr.reserved1);
	printf("  Hdr Start LBA: %llu\n", gpthdr.my_lba);
	printf("  Backup Hdr Start LBA: %llu\n", gpthdr.alternative_lba);
	printf("  Partition Start LBA: %llu\n", gpthdr.first_usable_lba);
	printf("  Partition End LBA: %llu\n", gpthdr.last_usable_lba);
	printf("  GUID: %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n", 
			gpthdr.disk_guid.time_low, gpthdr.disk_guid.time_mid,
			gpthdr.disk_guid.time_hi_and_version, 
			gpthdr.disk_guid.clock_seq_hi, gpthdr.disk_guid.clock_seq_low,
			gpthdr.disk_guid.node[0], gpthdr.disk_guid.node[1],
			gpthdr.disk_guid.node[2], gpthdr.disk_guid.node[3],
			gpthdr.disk_guid.node[4], gpthdr.disk_guid.node[5]);
	printf("  Partition Table Start LBA: %llu\n", gpthdr.partition_entry_lba);
	printf("  Number of Partition Entry: %u\n", gpthdr.npartition_entries);
	printf("  Size of Partition Entry: %u\n", gpthdr.sizeof_partition_entry);
	printf("  Partition Table Crc32: 0x%08x\n", gpthdr.partition_entry_array_crc32);
	printf("  Reserved2 must be all 0\n");
	
	printf("\n");

	uint8_t buf[gpthdr.npartition_entries * gpthdr.sizeof_partition_entry];
	if (read(fd, buf, sizeof(buf)) != sizeof(buf)) {
		perror("read");
		return;
	}

	printf("Partition Table:\n");
	printf("  [ Nr]        Partition Type GUID              Unique Partition Guid             Start(sector)    End(sector)    Size      Name\n");
	for (i = 0; i < gpthdr.npartition_entries; i++) {
		const struct gpt_entry *gpt = gpt_get_partition(buf, i);
		if (memcmp(&gpt->type, &GPT_UNUSED_ENTRY_GUID, sizeof(struct gpt_guid)) == 0)	continue;
		print_gpt_partition_entry(gpt, i);
		printf("\n");
	}
}

static void print_extend(int fd, off_t start_lba_sector)
{
	unsigned char ebr[512];	/* extended boot record buffer */
	
	off_t offset = start_lba_sector * 512;
	(void)lseek(fd, offset, SEEK_SET);
	if (read(fd, ebr, sizeof(ebr)) != sizeof(ebr)) {
		perror("read extended");	
		return;
	}

	struct dos_partition *p = mbr_get_partition(ebr, 0);
	printf(" %02x %4u/%3hhu/%2hhu  ~  %4u/%3hhu/%2hhu %8u %8u  %02x\n",
			p->boot_ind,
			chs_get_cylinder(p->bc, p->bs), p->bh, chs_get_sector(p->bs),
			chs_get_cylinder(p->ec, p->es), p->eh, chs_get_sector(p->es),
			dos_partition_get_start(p) , // + (unsigned int)start_lba_sector, /* 每个子扩展分区都把自己看成独立的硬盘 */
			dos_partition_get_size(p),
			p->sys_ind);

	p = mbr_get_partition(ebr, 1);	/* EBR中分区表表项二是描述下个子扩展分区的位置的 */
	if (IS_EXTENDED(p->sys_ind)) { 
//		printf("\tnext logic info: %02x %02x %u/%hhu/%hhu ~ %u/%hhu/%hhu %u(%u)\n",
//					p->sys_ind, p->boot_ind,
//					chs_get_cylinder(p->bc, p->bs), p->bh, chs_get_sector(p->bs),
//					chs_get_cylinder(p->ec, p->es), p->eh, chs_get_sector(p->es),
//					dos_partition_get_start(p),
//					dos_partition_get_size(p));
	} else {	/* 后续没有逻辑分区了，以此来结束递归 */
//		printf("no logic anymore\n");
		return;	
	} 
 
	print_extend(fd, extended_partition_lba_offset + dos_partition_get_start(p));	/* 第二个分区表项中的start是针对整个扩展分区的 */
}

static void print_dos_partition(int fd, uint8_t *mbr)
{
	int i;

	printf("Disk identifier: 0x%08x\n\n", mbr_get_id(mbr));

	printf("Boot Start-C/H/S ~    End-C/H/S Start-LBA    Size Type\n");

	for (i = 0; i < 4; i++) {
		struct dos_partition *p = mbr_get_partition(mbr, i);	
		if (dos_partition_get_start(p) == 0U)	continue;

		printf(" %02x %4u/%3hhu/%2hhu  ~  %4u/%3hhu/%2hhu %8u %8u  %02x\n",
				p->boot_ind,
				chs_get_cylinder(p->bc, p->bs), p->bh, chs_get_sector(p->bs),
				chs_get_cylinder(p->ec, p->es), p->eh, chs_get_sector(p->es),
				dos_partition_get_start(p),
				dos_partition_get_size(p),
				p->sys_ind);
		if (IS_EXTENDED(p->sys_ind)) { 	/* extended partition */
			extended_partition_lba_offset = dos_partition_get_start(p); 
			print_extend(fd, extended_partition_lba_offset);	/* 从extended_partition_lba_offset扇区处开始读扩展分区信息 */
		}
	}
}
