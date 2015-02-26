#include "exfatfs.h"
#include "exfatfs_common.h"
#include "minifs_path.h"
#include "minifs_cache.h"

#include <stdio.h>



namespace mfs{
	// VBRのテンプレート
	static const uint8_t VBR_TEMPLATE[] = {
		0xEB, 0x76, 0x90, 0x45, 0x58, 0x46, 0x41, 0x54, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x78, 0x56, 0x34, 0x12, 0x00, 0x01, 0x00, 0x00, 0x09, 0x06, 0x01, 0x80,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xC9, 0x8E, 0xD1, 0xBC, 0xF0, 0x7B, 0x8E,
		0xD9, 0xA0, 0xFB, 0x7D, 0xB4, 0x7D, 0x8B, 0xF0, 0xAC, 0x98, 0x40, 0x74, 0x0C, 0x48, 0x74, 0x0E,
		0xB4, 0x0E, 0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 0xEF, 0xA0, 0xFD, 0x7D, 0xEB, 0xE6, 0xCD, 0x16,
		0xCD, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x0D, 0x0A, 0x52, 0x65, 0x6D, 0x6F, 0x76, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x73, 0x20, 0x6F,
		0x72, 0x20, 0x6F, 0x74, 0x68, 0x65, 0x72, 0x20, 0x6D, 0x65, 0x64, 0x69, 0x61, 0x2E, 0xFF, 0x0D,
		0x0A, 0x44, 0x69, 0x73, 0x6B, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF, 0x0D, 0x0A, 0x50, 0x72,
		0x65, 0x73, 0x73, 0x20, 0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x74, 0x6F, 0x20, 0x72,
		0x65, 0x73, 0x74, 0x61, 0x72, 0x74, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x1F, 0x2C, 0x55, 0xAA
	};

	// ルートディレクトリエントリのテンプレート
	static const uint8_t ROOT_DIR_ENTRY_TEMPLATE[] = {
		0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x56, 0x34, 0x12, 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01,
		0x82, 0x00, 0x00, 0x00, 0x0D, 0xD3, 0x19, 0xE6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x56, 0x34, 0x12, 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01
	};
	
	// ファイルシステムの識別文字列
	static const char FILESYSTEM_NAME[] = { 'E', 'X', 'F', 'A', 'T', ' ', ' ', ' ' };



	// ファイルシステムがexFATか調べる
	bool ExFatFs::isExFAT(PartitionInfo_t &info){
		// セクター0を読み出す
		uint8_t buf[MAX_SECTOR_SIZE];
		if (info.pdiskio->disk_read(buf, info.partition_sector, 1) == RES_SUCCEEDED){
			// ファイルシステム名を確認	
			if (memcmp(buf + 3, FILESYSTEM_NAME, sizeof(FILESYSTEM_NAME)) == 0){
				// ブートシグニチャを確認する
				if (LoadLE16(buf + 510) == BOOT_SIGNATURE){
					return true;
				}
			}
		}
		return false;
	}

	// パーティションをフォーマットする
	RESULT_e ExFatFs::format(PartitionInfo_t &info, uint32_t sectors_per_cluster_shift, uint8_t &id){
		if (READ_ONLY || !SUPPORT_EXFAT_FORMAT){
			return RES_NOT_SUPPORTED;
		}
		else{
			wprintf(L"パーティション (%d, %d) のフォーマットを開始します\n", info.partition_sector, info.partition_size);

			id = 0x07;	// exFAT, NTFS

			uint32_t bytes_per_sector_shift = info.pdiskio->disk_bytesPerSectorShift();
			uint32_t bytes_per_sector = 1UL << bytes_per_sector_shift;
			uint32_t sectors_per_block_shift = info.pdiskio->disk_sectorsPerBlockShift();
			if (24 < (bytes_per_sector_shift + sectors_per_cluster_shift)){
				sectors_per_cluster_shift = 24 - bytes_per_sector_shift;
			}

			uint64_t fat_offset64 = (uint64_t)((info.partition_sector + 24 - 1) & ~((1UL << sectors_per_block_shift) - 1)) + (uint64_t)(1UL << sectors_per_block_shift) - info.partition_sector;
			uint32_t fat_offset = (uint32_t)fat_offset64;
			uint32_t fat_length;
			uint64_t cluster_heap_offset64 = (uint64_t)((info.partition_sector + 24 - 1) & ~((1UL << sectors_per_block_shift) - 1)) + (uint64_t)(1UL << sectors_per_block_shift) - info.partition_sector;
			uint32_t cluster_heap_offset = (uint32_t)cluster_heap_offset64;
			uint64_t cluster_count64 = ((uint64_t)info.partition_size - cluster_heap_offset) >> sectors_per_cluster_shift;
			uint32_t cluster_count;

			wprintf(L"Partiton Offset = %d\n", info.partition_sector);
			wprintf(L"Volume Length = %d\n", info.partition_size);
			wprintf(L"Fat Offset = %lld\n", fat_offset64);
			wprintf(L"Cluster Heap Offset = %lld\n", cluster_heap_offset64);
			wprintf(L"Cluster Count = %lld\n", cluster_count64);

			// クラスタヒープの位置を調整する
			cluster_count = (cluster_count64 <= (uint64_t)MAXIMUM_CLUSTER_COUNT) ? (uint32_t)cluster_count64 : MAXIMUM_CLUSTER_COUNT;
			while (true){
				fat_length = RShiftCeiling32(cluster_count + MINIMUM_VALID_CLUSTER, bytes_per_sector_shift - 2);
				if ((fat_offset + fat_length) <= cluster_heap_offset){
					break;
				}
				if (sectors_per_cluster_shift <= sectors_per_block_shift){
					cluster_heap_offset += 1UL << sectors_per_block_shift;
					cluster_count -= 1UL << (sectors_per_block_shift - sectors_per_cluster_shift);
				}
				else{
					cluster_heap_offset += 1UL << sectors_per_cluster_shift;
					cluster_count--;
				}
			}

			wprintf(L"クラスタヒープの位置を調整しました\n");
			wprintf(L"Fat Offset = %d\n", fat_offset);
			wprintf(L"Fat Length = %d\n", fat_length);
			wprintf(L"Cluster Heap Offset = %d\n", cluster_heap_offset);
			wprintf(L"Cluster Count = %d\n", cluster_count);

			uint32_t alloc_bitmap_first_cluster = MINIMUM_VALID_CLUSTER;
			uint32_t upper_case_first_cluster = alloc_bitmap_first_cluster + RShiftCeiling32(cluster_count, bytes_per_sector_shift + sectors_per_cluster_shift + 3);
			uint32_t root_directory_first_cluster = upper_case_first_cluster + RShiftCeiling32(UPPER_CASE_TABLE_SIZE, bytes_per_sector_shift + sectors_per_cluster_shift);

			wprintf(L"Allocation Bitmap starts at %d\n", alloc_bitmap_first_cluster);
			wprintf(L"Upper Case Table starts at %d\n", upper_case_first_cluster);
			wprintf(L"Root Directory Entry starts at %d\n", root_directory_first_cluster);

			RESULT_e result;
			uint8_t buf[MAX_SECTOR_SIZE];

			// Main Boot Sectorを書き込む
			{
				memcpy(buf, VBR_TEMPLATE, sizeof(VBR_TEMPLATE));
				memset(buf + bytes_per_sector, 0x00, bytes_per_sector - sizeof(VBR_TEMPLATE));
				StoreLE64(buf + 64, info.partition_sector);
				StoreLE64(buf + 72, info.partition_size);
				StoreLE32(buf + 80, fat_offset);
				StoreLE32(buf + 84, fat_length);
				StoreLE32(buf + 88, cluster_heap_offset);
				StoreLE32(buf + 92, cluster_count);
				StoreLE32(buf + 96, root_directory_first_cluster);
				buf[108] = bytes_per_sector_shift;
				buf[109] = sectors_per_cluster_shift;
				result = info.pdiskio->disk_write(buf, info.partition_sector, 1);
				if (result != RES_SUCCEEDED) return result;
			}
			wprintf(L"MBSを書き込みました\n");

			// チェックサムを計算
			uint32_t checksum;
			{
				checksum = CalcChecksum32(0x00000000, buf, 106);
				checksum = CalcChecksum32(checksum, buf + 108, 4);
				checksum = CalcChecksum32(checksum, buf + 113, sizeof(VBR_TEMPLATE) - 113);
				for (uint32_t cnt = 1; cnt <= 8; cnt++){
					checksum = RRotate32(checksum, 30);
					checksum = CalcChecksum32(checksum, BOOT_SIGNATURE & 0xFF);
					checksum = CalcChecksum32(checksum, BOOT_SIGNATURE >> 8);
				}
			}

			// Main Extended Boot Sectorsを書き込む
			{
				memset(buf, 0x00, bytes_per_sector - 2);
				StoreLE16(buf + bytes_per_sector - 2, BOOT_SIGNATURE);
				for (uint32_t cnt = 1; cnt <= 8; cnt++){
					result = info.pdiskio->disk_write(buf, info.partition_sector + cnt, 1);
					if (result != RES_SUCCEEDED) return result;
				}
			}
			wprintf(L"MEBSを書き込みました\n");

			// OEM Parametersと予約領域を書き込む
			{
				*(uint16_t*)(buf + bytes_per_sector - 2) = 0x0000;
				result = info.pdiskio->disk_write(buf, info.partition_sector + 9, 1);
				if (result != RES_SUCCEEDED) return result;
				result = info.pdiskio->disk_write(buf, info.partition_sector + 10, 1);
				if (result != RES_SUCCEEDED) return result;
			}
			wprintf(L"OEMパラメータを書き込みました\n");
			wprintf(L"予約領域を初期化しました\n");

			// チェックサムを書き込む
			{
				for (uint32_t cnt = 0; cnt < bytes_per_sector; cnt += 4){
					StoreLE32(buf + cnt, checksum);
				}
				result = info.pdiskio->disk_write(buf, info.partition_sector + 11, 1);
				if (result != RES_SUCCEEDED) return result;
			}
			wprintf(L"チェックサムを書き込みました (0x%08X)\n", checksum);

			// VBRのバックアップを書き込む
			{
				for (uint32_t cnt = 0; cnt < 12; cnt++){
					result = info.pdiskio->disk_read(buf, info.partition_sector + cnt, 1);
					if (result != RES_SUCCEEDED) return result;
					result = info.pdiskio->disk_write(buf, info.partition_sector + cnt + 12, 1);
					if (result != RES_SUCCEEDED) return result;
				}
			}
			wprintf(L"VBRのバックアップを作成しました\n");

			// FATを書き込む
			{
				memset(buf, 0x00, bytes_per_sector);
				uint32_t sector = info.partition_sector + fat_offset;
				uint32_t *p = (uint32_t*)buf;
				uint32_t *p_end = (uint32_t*)(buf + bytes_per_sector);
				StoreLE32(p++, 0xFFFFFFF8);
				StoreLE32(p++, 0xFFFFFFFF);
				for (uint32_t cnt = alloc_bitmap_first_cluster + 1; cnt <= upper_case_first_cluster; cnt++){
					StoreLE32(p++, (cnt < upper_case_first_cluster) ? cnt : 0xFFFFFFFF);
					if (p_end <= p){
						p = (uint32_t*)buf;
						result = info.pdiskio->disk_write(buf, sector++, 1);
						if (result != RES_SUCCEEDED) return result;
					}
				}
				for (uint32_t cnt = upper_case_first_cluster + 1; cnt <= root_directory_first_cluster; cnt++){
					StoreLE32(p++, (cnt < root_directory_first_cluster) ? cnt : 0xFFFFFFFF);
					if (p_end <= p){
						p = (uint32_t*)buf;
						result = info.pdiskio->disk_write(buf, sector++, 1);
						if (result != RES_SUCCEEDED) return result;
					}
				}
				StoreLE32(p++, 0xFFFFFFFF);
				result = info.pdiskio->disk_write(buf, sector, 1);
				if (result != RES_SUCCEEDED) return result;
			}
			wprintf(L"FATを作成しました\n");

			// クラスタ割り当てビットマップを書き込む
			{
				uint32_t sector = ClusterToSector(info.partition_sector + cluster_heap_offset, alloc_bitmap_first_cluster, sectors_per_cluster_shift);
				uint32_t sector_end = sector + RShiftCeiling32(cluster_count, bytes_per_sector_shift + 3);
				uint32_t bitcount = root_directory_first_cluster - MINIMUM_VALID_CLUSTER + 1;
				memset(buf, 0xFF, bytes_per_sector);
				bool zero = false;
				do{
					if ((bytes_per_sector * 8) <= bitcount){
						bitcount -= bytes_per_sector * 8;
					}
					else if (0 < bitcount){
						uint32_t *p = (uint32_t*)buf;
						uint32_t *p_end = (uint32_t*)(buf + bytes_per_sector);
						do{
							if (32 <= bitcount){
								*p++ = 0xFFFFFFFFUL;
								bitcount -= 32;
							}
							else if (0 < bitcount){
								StoreLE32(p++, ~(0xFFFFFFFFUL << bitcount));
								bitcount = 0;
							}
							else{
								*p++ = 0x00000000UL;
							}
						} while (p < p_end);
					}
					else{
						if (zero == false){
							memset(buf, 0x00, bytes_per_sector);
						}
					}
					result = info.pdiskio->disk_write(buf, sector++, 1);
					if (result != RES_SUCCEEDED) return result;
				} while (sector < sector_end);
			}
			wprintf(L"クラスタ割り当てビットマップを作成しました\n");

			// 大文字変換テーブルを書き込む
			{
				uint32_t sector = ClusterToSector(info.partition_sector + cluster_heap_offset, upper_case_first_cluster, sectors_per_cluster_shift);
				const fschar_t *src = CharUpperTableStandard;
				for (int32_t remaining = sizeof(CharUpperTableStandard); 0 < remaining; remaining -= bytes_per_sector){
					uint32_t len = remaining;
					if (bytes_per_sector < len){
						len = bytes_per_sector;
					}
					else{
						memset(buf + len, 0x00, bytes_per_sector - len);
					}
					uint16_t *dst = (uint16_t*)buf;
					for (; 0 < len; len -= 2){
						StoreLE16(dst++, *src++);
					}
					result = info.pdiskio->disk_write(buf, sector++, 1);
					if (result != RES_SUCCEEDED) return result;
				}
			}
			wprintf(L"大文字変換テーブルを作成しました\n");

			// ルートディレクトリエントリを書き込む
			{
				memcpy(buf, ROOT_DIR_ENTRY_TEMPLATE, sizeof(ROOT_DIR_ENTRY_TEMPLATE));
				memset(buf + sizeof(ROOT_DIR_ENTRY_TEMPLATE), 0x00, bytes_per_sector - sizeof(ROOT_DIR_ENTRY_TEMPLATE));
				StoreLE32(buf + 52, alloc_bitmap_first_cluster);
				StoreLE64(buf + 56, RShiftCeiling32(cluster_count, 3));
				StoreLE32(buf + 84, upper_case_first_cluster);
				StoreLE64(buf + 88, sizeof(CharUpperTableStandard));
				uint32_t sector = ClusterToSector(info.partition_sector + cluster_heap_offset, root_directory_first_cluster, sectors_per_cluster_shift);
				result = info.pdiskio->disk_write(buf, sector, 1);
				if (result != RES_SUCCEEDED) return result;
				memset(buf, 0x00, sizeof(ROOT_DIR_ENTRY_TEMPLATE));
				for (uint32_t cnt = 1; cnt < (1UL << sectors_per_cluster_shift); cnt++){
					result = info.pdiskio->disk_write(buf, sector + cnt, 1);
					if (result != RES_SUCCEEDED) return result;
				}
			}
			wprintf(L"ルートディレクトリエントリを作成しました\n");

			wprintf(L"フォーマットが完了しました\n");
			return RES_SUCCEEDED;
		}
	}



	// パーティションをマウントする
	RESULT_e ExFatFs::mount(PartitionInfo_t &info){
		RESULT_e result;
		
		// 初めにアンマウントする
		unmount();

		// セクター0を読み出す
		uint8_t buf[MAX_SECTOR_SIZE];
		result = info.pdiskio->disk_read(buf, info.partition_sector, 1);
		if (result != RES_SUCCEEDED){
			return result;
		}

		// ファイルシステム名を確認	
		if (memcmp(buf + 3, FILESYSTEM_NAME, sizeof(FILESYSTEM_NAME)) != 0){
			return RES_NO_FILESYSTEM;
		}

		uint64_t partition_offset = LoadLE64(buf + 64);
		uint64_t volume_length = LoadLE64(buf + 72);
		uint32_t fat_offset = LoadLE32(buf + 80);
		uint32_t fat_length = LoadLE32(buf + 84);
		uint32_t cluster_heap_offset = LoadLE32(buf + 88);
		uint32_t cluster_count = LoadLE32(buf + 92);
		uint32_t root_directory_first_cluster = LoadLE32(buf + 96);
		//uint32_t volume_serial_number = LoadLE32(buf + 100);
		uint32_t filesystem_revision = LoadLE16(buf + 104);
		//uint32_t volume_flags = LoadLE16(buf + 106);
		uint32_t bytes_per_sector_shift = *(buf + 108);
		uint32_t sectors_per_cluster_shift = *(buf + 109);
		uint32_t number_of_fats = *(buf + 110);
		//uint32_t drive_select = *(buf + 111);
		//uint32_t percent_in_use = *(buf + 112);

		// ファイルシステムのバージョンを確認
		if (filesystem_revision != 0x0100){
			return RES_NOT_SUPPORTED;
		}

		// オフセットを確認
		if (info.partition_sector != partition_offset){
			return RES_NO_FILESYSTEM;
		}

		// ボリュームサイズの確認
		if ((info.partition_size <= VBR_LENGTH) || (info.partition_size < volume_length)){
			return RES_NO_FILESYSTEM;
		}

		// FATの数が1であるか確認
		if (number_of_fats != 1){
			return RES_NOT_SUPPORTED;
		}

		// FATの長さが十分か確認
		if ((fat_length << (bytes_per_sector_shift - 2)) < (cluster_count + MINIMUM_VALID_CLUSTER)){
			return RES_NO_FILESYSTEM;
		}

		// FATとデータ領域が重なっていないか確認する
		if (cluster_heap_offset < (fat_offset + fat_length)){
			return RES_NO_FILESYSTEM;
		}

		// データ領域がパーティションに収まっているか確認する
		if (volume_length < (LShift32to64(cluster_count, sectors_per_cluster_shift) + cluster_heap_offset)){
			return RES_NO_FILESYSTEM;
		}

		// クラスター総数が規格内かを確認
		if (MAXIMUM_CLUSTER_COUNT < cluster_count){
			return RES_NO_FILESYSTEM;
		}

		// セクターサイズを確認
		if (bytes_per_sector_shift != info.pdiskio->disk_bytesPerSectorShift()){
			return RES_NOT_SUPPORTED;
		}

		// クラスターサイズを確認
		if (24 < (bytes_per_sector_shift + sectors_per_cluster_shift)){
			return RES_NOT_SUPPORTED;
		}

		// ブートシグニチャを確認する
		if (LoadLE16(buf + 510) != BOOT_SIGNATURE){
			return RES_NO_FILESYSTEM;
		}

		if (CHECK_CHECKSUM == true){
			// VBRのチェックサムを計算する
			uint32_t checksum = 0x00000000;
			checksum = CalcChecksum32(0x00000000, buf, 106);
			checksum = CalcChecksum32(checksum, buf + 108, 4);
			checksum = CalcChecksum32(checksum, buf + 113, (1UL << bytes_per_sector_shift) - 113);
			for (uint32_t cnt = 1; cnt <= 10; cnt++){
				result = info.pdiskio->disk_read(buf, info.partition_sector + cnt, 1);
				if (result != RES_SUCCEEDED){
					return result;
				}
				checksum = CalcChecksum32(checksum, buf, 1UL << bytes_per_sector_shift);
			}
			result = info.pdiskio->disk_read(buf, info.partition_sector + 11, 1);
			if (result != RES_SUCCEEDED){
				return result;
			}
			if (checksum != LoadLE32(buf)){
				return RES_NO_FILESYSTEM;
			}
		}

		// マウント処理を開始する
		InitializeMounting(info);

		// 必要な情報をメンバー変数にコピー
		m_BytesPerSectorShift = bytes_per_sector_shift;
		m_SectorsPerClusterShift = sectors_per_cluster_shift;
		m_FATOffset = info.partition_sector + fat_offset;
		m_ClusterOffset = info.partition_sector + cluster_heap_offset - (2UL << sectors_per_cluster_shift);
		m_ClusterCount = cluster_count;
		m_FreeClusterCount = INVALID_CLUSTER_COUNT;
		m_RootDirEntryCluster = root_directory_first_cluster;
		m_AllocBitmapCluster = INVALID_CLUSTER;
		m_FirstFreeCluster = INVALID_CLUSTER;
		m_ContiguousFreeClusterCount = 0;

		// 特殊なディレクトリエントリを探す
		result = FindSpecialDirEntry();
		if (result != RES_SUCCEEDED){
			unmount();
		}

		// クラスタ割り当てビットマップを開く
		m_AllocBitmapChain.flags = 0;
		m_AllocBitmapChain.start_cluster = m_AllocBitmapCluster;
		m_AllocBitmapChain.size = 0;
		result = InitChain(m_AllocBitmapChain);
		if (result != RES_SUCCEEDED){
			return result;
		}
		m_AllocBitmapChain.pcache = &m_AllocBitmapCache;

		if (Writable() == true){
			// 書き込み可能な場合、空きクラスタ数を調べておく
			freeSpace();

			// 空きクラスタを探しておく
			result = FindFreeClusters(MINIMUM_VALID_CLUSTER, m_FirstFreeCluster, m_ContiguousFreeClusterCount);
			if (result != RES_SUCCEEDED){
				if (result != RES_NOT_FOUND){
					return result;
				}
				// 空きクラスタが見つからないため書き込みを禁止する
				ProhibitWriting();
			}
		}

		// マウント処理を終了する
		FinalizeMounting();

		return result;
	}

	// パーティションをアンマウントする
	void ExFatFs::unmount(void){
		if (lockMutex(true) == true){
			if (Mounted() == true){
				// アンマウント処理を開始する
				InitializeUnmounting();

				unlockMutex();

				// 開かれているすべてのハンドルを閉じる
				RemoveAllHandles();

				if (Writable() == true){
					// キャッシュをフラッシュ
					m_AllocBitmapCache.Flush(m_pDiskIO);
					m_FATAndDirCache.Flush(m_pDiskIO);

					// ディスクの書き込み処理を完了させる
					m_pDiskIO->disk_sync();
				}

				// アンマウント処理を終了する
				FinalizeUnmounting();
			}
			else{
				unlockMutex();
			}
		}
	}

	// ボリュームの容量を取得する(バイト単位)
	uint64_t ExFatFs::capacity(void){
		if (lockMutex() == true){
			uint64_t cap = (Mounted() == true) ? LShift32to64(m_ClusterCount, m_SectorsPerClusterShift + m_BytesPerSectorShift) : 0ULL;
			unlockMutex();
			return cap;
		}
		else{
			return 0;
		}
	}

	// ボリュームの空き領域を取得する(バイト単位)
	uint64_t ExFatFs::freeSpace(void){
		if (lockMutex() == true){
			uint64_t free_space;
			if (Mounted() == true){
				if (m_FreeClusterCount == INVALID_CLUSTER_COUNT){
					// 空きクラスタの検索結果を合計する
					uint32_t free_cluster_count = 0;;
					uint32_t cluster = MINIMUM_VALID_CLUSTER;
					while (true){
						uint32_t count;
						RESULT_e result;
						result = FindFreeClusters(cluster, cluster, count);
						if (result == RES_SUCCEEDED){
							cluster += count;
							free_cluster_count += count;
						}
						else{
							if (result != RES_NOT_FOUND){
								free_cluster_count = 0;
							}
							break;
						}
					}
					m_FreeClusterCount = free_cluster_count;
				}
				free_space = LShift32to64(m_FreeClusterCount, m_SectorsPerClusterShift + m_BytesPerSectorShift);
			}
			else{
				free_space = 0;
			}
			unlockMutex();
			return free_space;
		}
		else{
			return 0;
		}
	}

	// ファイル・ディレクトリを削除する
	RESULT_e ExFatFs::remove(const fschar_t *path){
		if (Writable() == true){
			if (lockMutex() == true){
				RESULT_e result;
				if (Mounted() == true){
					// ファイル・ディレクトリを探す
					Chain_t chain;
					result = OpenChain(chain, 0, path);
					if (result != RES_SUCCEEDED){
						goto finish;
					}
					if (chain.start_cluster == m_RootDirEntryCluster){
						// ルートディレクトリなので削除できない
						result = RES_INVALID_PARAMETERS;
						goto finish;
					}

					// ファイル・ディレクトリがオープンされていないか確認する
					if (IsAlreadyOpened(chain) == true){
						result = RES_LOCKED;
						goto finish;
					}

					if (IsValidCluster(chain.start_cluster) == true){
						// ファイル・ディレクトリはデータを持つ
						if (chain.flags & FLAG_DIRECTORY){
							// ディレクトリが空か確認する
							CONDITION_t conditions;
							conditions.attributes = 0;
							conditions.output_name_string = nullptr;
							conditions.target_name_string = nullptr;
							conditions.target_name_checksum = 0;
							result = FindDir(chain, conditions, true);
							if (result != RES_NOT_FOUND){
								if (result == RES_SUCCEEDED){
									result = RES_NOT_EMPTY;
								}
								goto finish;
							}
						}

						// クラスタチェインを削除する
						result = DeleteChain(chain, chain.start_cluster, true);
						if (result != RES_SUCCEEDED){
							goto finish;
						}
					}

					// 親ディレクトリを開く
					uint32_t entry_offset = chain.entry_offset;
					chain.flags = chain.entry_flags;
					chain.start_cluster = chain.entry_cluster;
					result = InitChain(chain);
					if (result != RES_SUCCEEDED){
						goto finish;
					}

					// ディレクトリエントリを抹消する
					result = Unlink(chain, entry_offset);
				}
				else{
					result = RES_NOT_MOUNTED;
				}
			finish:
				unlockMutex();
				return result;
			}
			else{
				return RES_TIMEOUT;
			}
		}
		else{
			return RES_NOT_SUPPORTED;
		}
	}

	// ファイル・ディレクトリを移動する
	RESULT_e ExFatFs::move(const mfs::fschar_t *src, const fschar_t *dst){
		if (Writable() == true){
			if (lockMutex() == true){
				RESULT_e result;
				if (Mounted() == true){












					result = RES_NOT_SUPPORTED;
				}
				else{
					result = RES_NOT_MOUNTED;
				}
				unlockMutex();
				return result;
			}
			else{
				return RES_TIMEOUT;
			}
		}
		else{
			return RES_NOT_SUPPORTED;
		}
	}

	// ディレクトリを作成する
	RESULT_e ExFatFs::makeDir(const fschar_t *path){
		if (Writable() == true){
			if (lockMutex() == true){
				RESULT_e result;
				if (Mounted() == true){
					result = RES_NOT_SUPPORTED;
				}
				else{
					result = RES_NOT_MOUNTED;
				}
				unlockMutex();
				return result;
			}
			else{
				return RES_TIMEOUT;
			}
		}
		else{
			return RES_NOT_SUPPORTED;
		}
	}

	// ディレクトリを開く
	RESULT_e ExFatFs::OpenDir(DirHandle &dirhandle, const fschar_t *path){
		if (lockMutex() == true){
			RESULT_e result;
			if (Mounted() == true){
				// 指定されたディレクトリのディレクトリエントリを探す
				result = OpenChain(GetChain(dirhandle), ATTR_IS_DIRECTORY, path);
				if (result == RES_SUCCEEDED){
					AddHandle(dirhandle);
				}
			}
			else{
				result = RES_NOT_MOUNTED;
			}
			unlockMutex();
			return result;
		}
		else{
			return RES_TIMEOUT;
		}
	}

	// ディレクトリを閉じる
	RESULT_e ExFatFs::CloseDir(DirHandle &dirhandle){
		lockMutex(true);
		RemoveHandle(dirhandle);
		unlockMutex();
		return RES_SUCCEEDED;
	}

	// ディレクトリを列挙する
	RESULT_e ExFatFs::ReadDir(DirHandle &dirhandle, FileInfo_t *pinfo){
		if (lockMutex() == true){
			RESULT_e result;
			if (Mounted() == true){
				result = dirhandle.lastError();
				if (result == RES_SUCCEEDED){
					Chain_t &chain = GetChain(dirhandle);
					if (pinfo == nullptr){
						result = InitChain(chain);
					}
					else{
						// 検索条件を作成し検索する
						CONDITION_t conditions;
						conditions.attributes = 0;
						conditions.output_name_string = (uint16_t*)pinfo->name;
						conditions.target_name_string = nullptr;
						conditions.target_name_checksum = 0;
						result = FindDir(chain, conditions, true);
						if (result == RES_SUCCEEDED){
							// 検索結果を格納する
							*(Property_t*)pinfo = (Property_t&)conditions.property;
						}
					}
					chain.last_error = result;
				}
			}
			else{
				result = RES_NOT_MOUNTED;
			}
			unlockMutex();
			return result;
		}
		else{
			return RES_TIMEOUT;
		}
	}

	// ファイルを開く
	RESULT_e ExFatFs::OpenFile(FileHandle &filehandle, const fschar_t *path, uint32_t option){
		if (lockMutex() == true){
			RESULT_e result;
			if (Mounted() == true){
				// optionによって動作を変える





				// 指定されたファイルのディレクトリエントリを探す
				result = OpenChain(GetChain(filehandle), ATTR_IS_FILE, path);
				if (result == RES_SUCCEEDED){
					if (option & O_TRUNCATE){
						
					}
					AddHandle(filehandle);
				}
			}
			else{
				result = RES_NOT_MOUNTED;
			}
			unlockMutex();
			return result;
		}
		else{
			return RES_TIMEOUT;
		}
	}

	// ファイルを閉じる
	RESULT_e ExFatFs::CloseFile(FileHandle &filehandle){
		lockMutex(true);
		RemoveHandle(filehandle);
		unlockMutex();
		return RES_SUCCEEDED;
	}

	// ファイルポインタをシークする
	RESULT_e ExFatFs::SeekFile(FileHandle &filehandle, uint64_t offset){
		if (lockMutex() == true){
			RESULT_e result;
			if (Mounted() == true){
				result = filehandle.lastError();
				if (result == RES_SUCCEEDED){
					Chain_t &chain = GetChain(filehandle);
					if (chain.flags & FLAG_WRITABLE){
						if (chain.size < offset) offset = chain.size;
						result = SeekChain(chain, offset);
					}
					else if (offset <= chain.size){
						result = SeekChain(chain, offset);
					}
					else{
						// ファイルサイズを拡張する



						result = RES_NOT_SUPPORTED;
					}
					chain.last_error = result;
				}
			}
			else{
				result = RES_NOT_MOUNTED;
			}
			unlockMutex();
			return result;
		}
		else{
			return RES_TIMEOUT;
		}
	}

	// ファイルを読み出す
	uint32_t ExFatFs::ReadFile(FileHandle &filehandle, void *buf, uint32_t length){
		if (lockMutex() == true){
			if (Mounted() == true){
				RESULT_e result;
				result = filehandle.lastError();
				if (result == RES_SUCCEEDED){
					Chain_t &chain = GetChain(filehandle);
					uint64_t remaining = chain.size - chain.pointer;
					if (remaining < (uint64_t)length) length = (uint32_t)remaining;
					result = ReadChain(chain, buf, length);
					chain.last_error = result;
				}
				if (result != RES_SUCCEEDED){
					length = 0;
				}
			}
			else{
				length = 0;
			}
			unlockMutex();
			return length;
		}
		else{
			return 0;
		}
	}

	// ファイルへ書き込む
	uint32_t ExFatFs::WriteFile(FileHandle &filehandle, const void *buf, uint32_t length){
		Chain_t &chain = GetChain(filehandle);
		if ((Writable() == true) && (chain.flags & FLAG_WRITABLE)){
			if (lockMutex() == true){
				if (Mounted() == true){
					RESULT_e result;
					result = filehandle.lastError();
					if (result == RES_SUCCEEDED){
						uint64_t pointer = chain.pointer;
						result = WriteChain(GetChain(filehandle), buf, length);
						length = (uint32_t)(chain.pointer - pointer);
						chain.last_error = result;
					}
					if (result != RES_SUCCEEDED){
						length = 0;
					}
				}
				else{
					length = 0;
				}
				unlockMutex();
				return length;
			}
		}
		return 0;
	}

	// ファイルをフラッシュする
	RESULT_e ExFatFs::FlushFile(FileHandle &filehandle){
		if (REDUCE_CACHE == false){
			if (Writable() == true){
				if (lockMutex() == true){
					RESULT_e result;
					if (Mounted() == true){
						Chain_t &chain = GetChain(filehandle);
						result = filehandle.lastError();
						if (result == RES_SUCCEEDED){
							result = GetChain(filehandle).pcache->Flush(m_pDiskIO);
							if (result == RES_SUCCEEDED){
								result = m_pDiskIO->disk_sync();
							}
							chain.last_error = result;
						}
					}
					else{
						result = RES_NOT_MOUNTED;
					}
					unlockMutex();
					return result;
				}
				else{
					return RES_TIMEOUT;
				}
			}
		}
		return RES_SUCCEEDED;
	}

	// ファイルを切り詰める
	RESULT_e ExFatFs::TruncateFile(FileHandle &filehandle){
		if (Writable() == true){
			if (lockMutex() == true){
				RESULT_e result;
				if (Mounted() == true){
					result = filehandle.lastError();
					if (result == RES_SUCCEEDED){

						Chain_t &chain = GetChain(filehandle);
						if (0 < chain.size){





						}


						result = RES_NOT_SUPPORTED;

						chain.last_error = result;
					}
				}
				else{
					result = RES_NOT_MOUNTED;
				}
				unlockMutex();
				return result;
			}
			else{
				return RES_TIMEOUT;
			}
		}
		else{
			return RES_NOT_SUPPORTED;
		}
	}



	// フラグメントを把握する
	RESULT_e ExFatFs::LoadFragment(Chain_t &chain, uint32_t start_cluster){
		if (chain.flags & FLAG_CONTIGUOUS){
			// 断片化していない
			chain.frag_head_cluster = start_cluster;
			//chain.frag_head_cluster = chain.start_cluster;
			chain.frag_tail_cluster = chain.start_cluster + RShiftCeilingPV64to32(chain.size, m_SectorsPerClusterShift) - 1;
			chain.next_frag_cluster = INVALID_CLUSTER;
			return RES_SUCCEEDED;
		}
		else{
			// FATを参照してフラグメントを調べる
			chain.frag_head_cluster = start_cluster;
			return ReadContinuousFAT(chain.frag_head_cluster, chain.frag_tail_cluster, chain.next_frag_cluster);
		}
	}

	// クラスタへアクセスする前にChain_tを初期化する
	RESULT_e ExFatFs::InitChain(Chain_t &chain){
		//chain.frag_head_cluster = INVALID_CLUSTER;
		//chain.frag_tail_cluster = INVALID_CLUSTER;
		//chain.next_frag_cluster = INVALID_CLUSTER;
		chain.last_error = RES_SUCCEEDED;
		chain.frag_offset_cluster = 0;
		chain.pointer = 0;
		chain.pcache = &m_FATAndDirCache;
		return LoadFragment(chain, chain.start_cluster);
	}

	// クラスタチェインを開く
	RESULT_e ExFatFs::OpenChain(Chain_t &chain, uint32_t attributes, const fschar_t *path){
		// ルートディレクトリを開く
		chain.flags = ATTR_DIRECTORY;
		chain.start_cluster = m_RootDirEntryCluster;
		chain.size = 0;
		chain.entry_flags = ATTR_DIRECTORY;
		chain.entry_cluster = 0;
		chain.entry_offset = 0;

		// パスをリストに変換
		uint16_t path_list[MAXIMUM_PATH + 1];
		if (ConvertPathToList(path_list, path) == false){
			// パスの形式が不正だった
			return RES_INVALID_NAME;
		}
		else{
			if (path_list[0] == 0x0000){
				// ルートディレクトリ
				if ((ATTR_DIRECTORY & (attributes >> 16)) != (attributes & 0xFFFF)){
					// アトリビュートが一致しない
					return RES_NOT_FOUND;
				}
			}
			else{
				// それ以外のディレクトリ
				uint16_t *p = path_list;
				uint16_t *name = path_list;
				uint32_t length = 0;
				uint16_t checksum = 0;
				CONDITION_t cond;
				cond.attributes = attributes;
				cond.output_name_string = nullptr;
				while (true){
					uint16_t c = *p++;
					if (c == 0x0000){
						if (length == 0){
							break;
						}

						// 検索条件を作成
						cond.target_name_string = name;
						cond.target_name_checksum = checksum;

						// ディレクトリエントリを検索する
						RESULT_e result;
						result = InitChain(chain);
						if (result != RES_SUCCEEDED){
							return result;
						}
						result = FindDir(chain, cond, (*p == 0x0000));	// リストの最後の項目だったらプロパティを要求する
						if (result != RES_SUCCEEDED){
							return result;
						}
						(DirEntry_t&)chain = cond.direntry;

						name = p;
						length = 0;
						checksum = 0;
					}
					else{
						// ファイル名のチェックサムを計算する
						checksum = CalcChecksum16(checksum, (uint8_t)c);
						checksum = CalcChecksum16(checksum, (uint8_t)(c >> 8));

						length++;
					}
				}
			}
		}

		return InitChain(chain);
	}

	// クラスタチェインをシークする
	RESULT_e ExFatFs::SeekChain(Chain_t &chain, uint64_t offset){
		if (chain.start_cluster == INVALID_CLUSTER){
			return RES_INTERNAL_ERROR;
		}

		RESULT_e result;
		uint32_t bytes_per_cluster_shift = m_BytesPerSectorShift + m_SectorsPerClusterShift;
		uint32_t bytes_per_cluster = 1UL << bytes_per_cluster_shift;
		uint32_t pointer_cluster = RShift64to32(chain.pointer, bytes_per_cluster_shift);
		uint32_t offset_cluster = RShift64to32(offset, bytes_per_cluster_shift);

		if (pointer_cluster == offset_cluster){
			// ポインタは同じクラスタ内を移動する
			chain.pointer = offset;
			return RES_SUCCEEDED;
		}
		else if (offset_cluster < pointer_cluster){
			// ポインタは前のクラスタに移動する
			uint32_t back = pointer_cluster - offset_cluster;
			if (back <= chain.frag_offset_cluster){
				// シーク先は現在のフラグメント中にある
				chain.frag_offset_cluster -= back;
				chain.pointer = offset;
				return RES_SUCCEEDED;
			}
			else{
				// シーク先は現在のフラグメント中にない
				// ポインタをクラスタチェインの最初に戻しシークしなおす
				chain.frag_offset_cluster = 0;
				chain.pointer = 0;
				pointer_cluster = 0;
				result = LoadFragment(chain, chain.start_cluster);
				if (result != RES_SUCCEEDED){
					return result;
				}
			}
		}

		// クラスタチェインを目的のクラスタ数だけ進む
		uint32_t forward = offset_cluster - pointer_cluster;
		chain.pointer &= ~((uint64_t)bytes_per_cluster - 1);	// ポインタをクラスタ境界に戻す
		while (true){
			uint32_t remaining = chain.frag_tail_cluster - chain.frag_head_cluster + 1 - chain.frag_offset_cluster;
			if (forward < remaining){
				// シーク先は現在のフラグメント中にある
				chain.frag_offset_cluster += forward;
				chain.pointer += LShift32to64(forward, bytes_per_cluster_shift);
				chain.pointer += (uint32_t)offset & (bytes_per_cluster - 1);
				return RES_SUCCEEDED;
			}
			else{
				// シーク先は現在のフラグメント中にない。次のフラグメントへ移動する
				forward -= remaining;
				chain.pointer += LShift32to64(remaining, bytes_per_cluster_shift);
				if (chain.next_frag_cluster == INVALID_CLUSTER){
					chain.frag_offset_cluster += remaining;
					return RES_NO_DATA;
				}
				chain.frag_offset_cluster = 0;
				result = LoadFragment(chain, chain.next_frag_cluster);
				if (result != RES_SUCCEEDED){
					return result;
				}
			}
		}
	}

	// クラスタチェインをキャッシュする
	RESULT_e ExFatFs::CacheChain(Chain_t &chain){
		if (chain.start_cluster == INVALID_CLUSTER){
			return RES_INTERNAL_ERROR;
		}
		if ((chain.frag_tail_cluster - chain.frag_head_cluster) < chain.frag_offset_cluster){
			// 読み出せるデータがない
			return RES_NO_DATA;
		}

		uint32_t sectors_per_cluster = 1UL << m_SectorsPerClusterShift;
		uint32_t bytes_per_cluster = 1UL << m_BytesPerSectorShift;
		uint32_t sector_offset = ((uint32_t)chain.pointer >> m_BytesPerSectorShift) & (sectors_per_cluster - 1);	// そのクラスタ内で何番目のセクターか
		uint32_t sector = m_ClusterOffset + ((chain.frag_head_cluster + chain.frag_offset_cluster) << m_SectorsPerClusterShift) + sector_offset;
		return chain.pcache->ReadTo(m_pDiskIO, sector);
	}

	// クラスタチェインを読み出す
	RESULT_e ExFatFs::ReadChain(Chain_t &chain, void *buf, uint32_t length){
		if (chain.start_cluster == INVALID_CLUSTER){
			return RES_INTERNAL_ERROR;
		}
		if ((chain.frag_tail_cluster - chain.frag_head_cluster) < chain.frag_offset_cluster){
			// 読み出せるデータがない
			return RES_NO_DATA;
		}

		RESULT_e result;
		uint32_t sectors_per_cluster = 1UL << m_SectorsPerClusterShift;
		uint32_t bytes_per_sector = 1UL << m_BytesPerSectorShift;
		uint32_t sector_offset = ((uint32_t)chain.pointer >> m_BytesPerSectorShift) & (sectors_per_cluster - 1);	// そのクラスタ内で何番目のセクターか
		uint32_t byte_offset = (uint32_t)chain.pointer & (bytes_per_sector - 1);		// そのセクター内で何バイト目か
		while (0 < length){
			uint32_t sector = m_ClusterOffset + ((chain.frag_head_cluster + chain.frag_offset_cluster) << m_SectorsPerClusterShift) + sector_offset;
			uint32_t read_bytes;
			if (byte_offset == 0){
				// シークポインタはセクターの境界
				uint32_t count = length >> m_BytesPerSectorShift;
				if (0 < count){
					// バッファがセクターサイズ以上あるため、セクターをバッファに直接読み込む
					uint32_t remaining = sectors_per_cluster - sector_offset;
					if (remaining < count) count = remaining;
					result = m_pDiskIO->disk_read(buf, sector, count);
					if (result != RES_SUCCEEDED){
						return result;
					}
					read_bytes = count << m_BytesPerSectorShift;
					sector_offset += count;
				}
				else{
					// キャッシュを通してバッファに転送する
					result = chain.pcache->ReadWith(m_pDiskIO, sector, buf, 0, length);
					if (result != RES_SUCCEEDED){
						return result;
					}
					read_bytes = length;
				}
			}
			else{
				// ポインタはセクターの途中
				read_bytes = bytes_per_sector - byte_offset;
				
				if (length < read_bytes){
					read_bytes = length;
				}
				else{
					sector_offset++;
				}

				// キャッシュを通してバッファに転送する
				result = chain.pcache->ReadWith(m_pDiskIO, sector, buf, byte_offset, read_bytes);
				if (result != RES_SUCCEEDED){
					return result;
				}

				byte_offset = 0;
			}

			buf = (uint8_t*)buf + read_bytes;
			length -= read_bytes;
			chain.pointer += read_bytes;

			if (sector_offset == sectors_per_cluster){
				sector_offset = 0;

				// 次のクラスタへ移動する
				if ((chain.frag_tail_cluster - chain.frag_head_cluster) <= chain.frag_offset_cluster){
					// 次のフラグメントのクラスタに移動する
					if (chain.next_frag_cluster != INVALID_CLUSTER){
						chain.frag_offset_cluster = 0;
						result = LoadFragment(chain, chain.next_frag_cluster);
						if (result != RES_SUCCEEDED){
							return result;
						}
					}
					else{
						return RES_INTERNAL_ERROR;
					}
				}
				else{
					// フラグメント内の次のクラスタに移動する
					chain.frag_offset_cluster++;
				}
			}
		}

		return RES_SUCCEEDED;
	}

	// クラスタチェインへ書き込む
	RESULT_e ExFatFs::WriteChain(Chain_t &chain, const void *buf, uint32_t length){
		if (length == 0){
			return RES_SUCCEEDED;
		}
		
		RESULT_e result;

		if (chain.start_cluster == INVALID_CLUSTER){
			// クラスタチェインを作成する必要がある
			uint32_t count = RShiftCeilingPV32(length, m_BytesPerSectorShift + m_SectorsPerClusterShift);
			result = ExtendChain(chain, count);
			if (result != RES_SUCCEEDED){
				return result;
			}
		}
		
		uint32_t sectors_per_cluster = 1UL << m_SectorsPerClusterShift;
		uint32_t bytes_per_sector = 1UL << m_BytesPerSectorShift;
		uint32_t sector_offset = ((uint32_t)chain.pointer >> m_BytesPerSectorShift) & (sectors_per_cluster - 1);	// そのクラスタ内で何番目のセクターか
		uint32_t byte_offset = (uint32_t)chain.pointer & (bytes_per_sector - 1);		// そのセクター内で何バイト目か
		while (0 < length){
			if ((chain.frag_tail_cluster - chain.frag_head_cluster) < chain.frag_offset_cluster){
				chain.frag_offset_cluster = 0;
				if (chain.next_frag_cluster != INVALID_CLUSTER){
					// 次のフラグメントのクラスタに移動する
					result = LoadFragment(chain, chain.next_frag_cluster);
					if (result != RES_SUCCEEDED){
						return result;
					}
				}
				else{
					// 新しいフラグメントを作成する
					uint32_t count = RShiftCeilingPV32(length, m_BytesPerSectorShift + m_SectorsPerClusterShift);
					result = ExtendChain(chain, count);
					if (result != RES_SUCCEEDED){
						return result;
					}




				}
			}

			uint32_t sector = m_ClusterOffset + ((chain.frag_head_cluster + chain.frag_offset_cluster) << m_SectorsPerClusterShift) + sector_offset;
			uint32_t written_bytes;
			if (byte_offset == 0){
				// シークポインタはセクターの境界
				uint32_t count = length >> m_BytesPerSectorShift;
				if (0 < count){
					// バッファがセクターサイズ以上あるため、セクターをバッファに直接読み込む
					uint32_t remaining = sectors_per_cluster - sector_offset;
					if (remaining < count) count = remaining;
					result = m_pDiskIO->disk_write(buf, sector, count);
					if (result != RES_SUCCEEDED){
						return result;
					}
					written_bytes = count << m_BytesPerSectorShift;
					sector_offset += count;
				}
				else{
					// キャッシュを通して書き込む
					result = chain.pcache->WriteTo(m_pDiskIO, sector, buf, 0, length);
					if (result != RES_SUCCEEDED){
						return result;
					}
					written_bytes = length;
				}
			}
			else{
				// ポインタはセクターの途中
				written_bytes = bytes_per_sector - byte_offset;

				if (length < written_bytes){
					written_bytes = length;
				}
				else{
					sector_offset++;
				}

				// キャッシュを通して書き込む
				result = chain.pcache->WriteTo(m_pDiskIO, sector, buf, byte_offset, written_bytes);
				if (result != RES_SUCCEEDED){
					return result;
				}

				byte_offset = 0;
			}

			buf = (const uint8_t*)buf + written_bytes;
			length -= written_bytes;
			chain.pointer += written_bytes;
			if (chain.size < chain.pointer){
				chain.size = chain.pointer;
			}

			if (sector_offset == sectors_per_cluster){
				sector_offset = 0;

				// 次のクラスタへ移動する
				if ((chain.frag_tail_cluster - chain.frag_head_cluster) <= chain.frag_offset_cluster){
					// 次のフラグメントのクラスタに移動する
					if (chain.next_frag_cluster != INVALID_CLUSTER){
						chain.frag_offset_cluster = 0;
						result = LoadFragment(chain, chain.next_frag_cluster);
						if (result != RES_SUCCEEDED){
							return result;
						}
					}
					else{
						return RES_INTERNAL_ERROR;
					}
				}
				else{
					// フラグメント内の次のクラスタに移動する
					chain.frag_offset_cluster++;
				}
			}
		}

		return RES_SUCCEEDED;
	}

	// クラスタチェインを伸ばす
	RESULT_e ExFatFs::ExtendChain(Chain_t &chain, uint32_t cluster_count){
		if (m_FreeClusterCount < (cluster_count + SPARE_CLUSTER_COUNT)){
			return RES_NO_SPACE;
		}

		// クラスタチェインを最後部まで辿る
		RESULT_e result;
		uint32_t tail_cluster = chain.frag_tail_cluster;
		uint32_t next_cluster = chain.next_frag_cluster;
		//bool update_fragment = (chain.next_frag_cluster == INVALID_CLUSTER);
		while (next_cluster != INVALID_CLUSTER){
			result = ReadContinuousFAT(next_cluster, tail_cluster, next_cluster);
			if (result != RES_SUCCEEDED){
				return result;
			}
		}

		// 現在のクラスタチェインのすぐ後方に空きクラスタがあるか調べる
		uint32_t start_cluster = (chain.start_cluster != INVALID_CLUSTER) ? tail_cluster : MINIMUM_VALID_CLUSTER;

		while (0 < cluster_count){
			uint32_t free_cluster;
			uint32_t free_cluster_count;
			result = FindFreeClusters(start_cluster, free_cluster, free_cluster_count);
			if (result == RES_SUCCEEDED){
				// 続きから空きクラスタを検索する
				start_cluster = free_cluster + free_cluster_count;

				// クラスタ割り当てビットマップを更新する
				uint32_t count = (cluster_count <= free_cluster_count) ? cluster_count : free_cluster_count;
				result = FillAllocationBitmap(free_cluster, count, true);
				if (result != RES_SUCCEEDED){
					return result;
				}
				m_FreeClusterCount -= count;
				cluster_count -= count;

				uint32_t prev_tail_cluster = tail_cluster;
				tail_cluster = free_cluster + count - 1;

				if (chain.flags & FLAG_CONTIGUOUS){
					// FATにクラスタチェインが存在しない(断片化していない)
					if (chain.start_cluster == INVALID_CLUSTER){
						// 確保したクラスタを最初のクラスタチェインにする
						chain.start_cluster = free_cluster;
						chain.frag_head_cluster = free_cluster;
						chain.frag_tail_cluster = free_cluster + count - 1;
						continue;
					}
					else if ((prev_tail_cluster + 1) == free_cluster){
						// クラスタ割り当てビットマップを更新するのみ
						continue;
					}

					// 断片化していなかったため書き込まれていなかったFATを更新する
					result = WriteContinuousFAT(INVALID_CLUSTER, chain.start_cluster, prev_tail_cluster - chain.start_cluster + 1);
					if (result != RES_SUCCEEDED){
						return result;
					}
					chain.flags &= ~FLAG_CONTIGUOUS;
				}
				
				// FATを更新する
				result = WriteContinuousFAT(prev_tail_cluster, free_cluster, count);
				if (result != RES_SUCCEEDED){
					return result;
				}
				
			}else{
				// start_cluster以降に空きクラスタは存在しない
				if (result != RES_NOT_FOUND){
					return result;
				}
				if (start_cluster == MINIMUM_VALID_CLUSTER){
					return RES_INTERNAL_ERROR;
				}

				// クラスタヒープの先頭から空きクラスタを検索しなおす
				start_cluster = MINIMUM_VALID_CLUSTER;
			}
		}

		return RES_SUCCEEDED;
	}

	// クラスタチェインを削除する
	// start_clusterから次のクラスタを削除する
	RESULT_e ExFatFs::DeleteChain(Chain_t &chain, uint32_t start_cluster, bool contain_start){
		if (chain.flags & FLAG_CONTIGUOUS){
			return RES_SUCCEEDED;
		}
		else{
			RESULT_e result;

			uint32_t head_cluster = start_cluster;
			if (contain_start == true){
				start_cluster = INVALID_CLUSTER;
			}
			while (head_cluster != INVALID_CLUSTER){
				uint32_t tail_cluster, next_cluster;
				
				// FATを読む
				result = ReadContinuousFAT(head_cluster, tail_cluster, next_cluster);
				if (result != RES_SUCCEEDED){
					return result;
				}

				if (start_cluster == head_cluster){
					head_cluster++;

					// start_clusterからのリンクを切る
					result = WriteFAT(start_cluster, TERMINAL_CLUSTER);
					if (result != RES_SUCCEEDED){
						return result;
					}
				}

				// クラスタ割り当てビットマップの当該領域をクリア
				uint32_t cluster_count = tail_cluster - head_cluster + 1;
				result = FillAllocationBitmap(head_cluster, cluster_count, false);
				if (result != RES_SUCCEEDED){
					return result;
				}

				m_FreeClusterCount += cluster_count;
				head_cluster = next_cluster;
			}
		}
		
		return RES_SUCCEEDED;
	}

	// クラスタチェインを確保する
	// start_clusterからcluster_countだけクラスタを確保する
	// 不足分は拡張され、余剰分は削除される
	RESULT_e ExFatFs::AllocateChain(Chain_t &chain, uint32_t start_cluster, uint32_t cluster_count, uint32_t &allocated_count){
		if (start_cluster == INVALID_CLUSTER){
			return RES_INTERNAL_ERROR;
		}
		
		RESULT_e result;
		uint32_t previous_cluster = INVALID_CLUSTER, current_cluster, tail_cluster, next_cluster;

		allocated_count = 0;

		if (cluster_count == 0){
			// クラスタチェインを先頭からすべて削除する
			if (chain.start_cluster == INVALID_CLUSTER){
				return RES_SUCCEEDED;
			}
			if (chain.start_cluster != start_cluster){
				// cluster_count = 0 を指定できるのは start_cluster = chain.start_cluster であるときのみ
				return RES_INTERNAL_ERROR;
			}
			current_cluster = INVALID_CLUSTER;
			tail_cluster = INVALID_CLUSTER;
			next_cluster = start_cluster;
		}
		else{
			if (chain.start_cluster == INVALID_CLUSTER){
				current_cluster = INVALID_CLUSTER;
				//tail_cluster = INVALID_CLUSTER;
				//next_cluster = INVALID_CLUSTER;
			}
			else if (chain.flags & FLAG_CONTIGUOUS){
				uint32_t count = RShiftCeilingPV32(chain.size, m_BytesPerSectorShift + m_SectorsPerClusterShift);
				if ((start_cluster < chain.start_cluster) || ((chain.start_cluster + count) <= start_cluster)){
					return RES_INTERNAL_ERROR;
				}
				current_cluster = start_cluster;
				tail_cluster = chain.start_cluster + count - 1;
				next_cluster = INVALID_CLUSTER;
			}
			else{
				current_cluster = start_cluster;
				result = ReadContinuousFAT(current_cluster, tail_cluster, next_cluster);
				if (result != RES_SUCCEEDED){
					return result;
				}
			}
		}

		// クラスタチェインを辿り、クラスタ数が不足しているなら拡張する
		while (0 < cluster_count){
			if (current_cluster == INVALID_CLUSTER){
				// クラスタチェインを拡張する必要がある
				if (m_FreeClusterCount < (cluster_count + SPARE_CLUSTER_COUNT)){
					// 空きクラスタが足りない
					return RES_NO_SPACE;
				}

				// 空きクラスタを探す
				//uint32_t free_cluster;
				uint32_t free_cluster_count;
				result = FindFreeClusters(previous_cluster, current_cluster, free_cluster_count);
				if (result != RES_SUCCEEDED){
					if (result != RES_NOT_FOUND){
						return result;
					}
					result = FindFreeClusters(MINIMUM_VALID_CLUSTER, current_cluster, free_cluster_count);
					if (result != RES_SUCCEEDED){
						if (result != RES_NOT_FOUND){
							return result;
						}
						return RES_INTERNAL_ERROR;
					}
				}

				// クラスタ割り当てビットマップを更新する
				uint32_t count = (cluster_count < free_cluster_count) ? cluster_count : free_cluster_count;
				result = FillAllocationBitmap(current_cluster, count, true);
				if (result != RES_SUCCEEDED){
					return result;
				}
				m_FreeClusterCount -= count;
				tail_cluster = current_cluster + count - 1;

				if (chain.flags & FLAG_CONTIGUOUS){
					// FATにクラスタチェインが存在しない(断片化していない)
					if (chain.start_cluster == INVALID_CLUSTER){
						// 確保したクラスタを最初のクラスタチェインにする
						chain.start_cluster = cluster_count;
					}
					else if ((previous_cluster + 1) == cluster_count){
						// 断片化しなかったのでFATの更新は必要ない
					}
					else{
						// 断片化が発生するため書き込まれていなかった分のFATを更新する
						result = WriteContinuousFAT(INVALID_CLUSTER, chain.start_cluster, previous_cluster - chain.start_cluster + 1);
						if (result != RES_SUCCEEDED){
							return result;
						}
						chain.flags &= ~FLAG_CONTIGUOUS;
					}
				}

				if (~chain.flags & FLAG_CONTIGUOUS){
					// FATを更新する
					result = WriteContinuousFAT(previous_cluster, current_cluster, count);
					if (result != RES_SUCCEEDED){
						return result;
					}
				}
			}

			uint32_t remaining = tail_cluster - current_cluster + 1;
			if (cluster_count < remaining){
				// 現在のフラグメント内で完結した
				allocated_count += cluster_count;
				current_cluster += cluster_count;
				previous_cluster = current_cluster - 1;
				cluster_count = 0;
				break;
			}
			else{
				// 現在のフラグメント全体で完結した、もしくは完結しなかった
				allocated_count += remaining;
				current_cluster = next_cluster;
				previous_cluster = tail_cluster;
				cluster_count -= remaining;

				// 次のフラグメントへ移動する
				result = ReadContinuousFAT(current_cluster, tail_cluster, next_cluster);
				if (result != RES_SUCCEEDED){
					return result;
				}
			}
		}

		// クラスタ数が過剰なら、以降のクラスタチェインを削除する
		if ((current_cluster < tail_cluster) || (next_cluster != INVALID_CLUSTER)){
			if (current_cluster != INVALID_CLUSTER){
				if (~chain.flags & FLAG_CONTIGUOUS){
					// current_clusterでクラスタチェインを終端する
					result = WriteFAT(current_cluster, TERMINAL_CLUSTER);
					if (result != RES_SUCCEEDED){
						return result;
					}
				}

				uint32_t count = tail_cluster - current_cluster;
				if (0 < count){
					// クラスタ割り当てビットマップの当該領域をクリア
					result = FillAllocationBitmap(current_cluster + 1, count, false);
					if (result != RES_SUCCEEDED){
						return result;
					}
					m_FreeClusterCount += count;
				}
			}

			while (next_cluster != INVALID_CLUSTER){
				// 次のフラグメントへ移動する
				current_cluster = next_cluster;
				result = ReadContinuousFAT(current_cluster, tail_cluster, next_cluster);
				if (result != RES_SUCCEEDED){
					return result;
				}

				// クラスタ割り当てビットマップの当該領域をクリア
				uint32_t count = tail_cluster - current_cluster + 1;
				result = FillAllocationBitmap(current_cluster, count, false);
				if (result != RES_SUCCEEDED){
					return result;
				}
				m_FreeClusterCount += count;
			}
		}

		return RES_SUCCEEDED;
	}



	// FATを読み込む
	RESULT_e ExFatFs::ReadFAT(uint32_t previous_cluster, uint32_t &next_cluster){
		RESULT_e result;
		uint32_t entry_per_sector_shift = m_BytesPerSectorShift - 2;
		uint32_t entry_mask = (1UL << entry_per_sector_shift) - 1;
		uint32_t sector = m_FATOffset + (previous_cluster >> entry_per_sector_shift);
		uint32_t *fat = (uint32_t*)m_FATAndDirCache.GetBuffer(0);
		result = m_FATAndDirCache.ReadTo(m_pDiskIO, sector);
		if (result != RES_SUCCEEDED){
			return result;
		}
		next_cluster = LoadLE32(fat + (previous_cluster & entry_mask));
		return RES_SUCCEEDED;
	}

	// FATに書き込む
	RESULT_e ExFatFs::WriteFAT(uint32_t previous_cluster, uint32_t next_cluster){
		RESULT_e result;
		uint32_t entry_per_sector_shift = m_BytesPerSectorShift - 2;
		uint32_t entry_mask = (1UL << entry_per_sector_shift) - 1;
		uint32_t sector = m_FATOffset + (previous_cluster >> entry_per_sector_shift);
		uint32_t *fat = (uint32_t*)m_FATAndDirCache.GetBuffer(0);
		result = m_FATAndDirCache.ReadTo(m_pDiskIO, sector);
		if (result != RES_SUCCEEDED){
			return result;
		}
		StoreLE32(fat + (previous_cluster & entry_mask), next_cluster);
		m_FATAndDirCache.SetModified(true);
		return RES_SUCCEEDED;
	}

	// FATを連続で読み込む
	RESULT_e ExFatFs::ReadContinuousFAT(uint32_t start_cluster, uint32_t &tail_cluster, uint32_t &next_cluster){
		if (start_cluster == INVALID_CLUSTER){
			// ファイルが存在しない
			tail_cluster = INVALID_CLUSTER;
			next_cluster = INVALID_CLUSTER;
		}
		else{
			uint32_t cluster = start_cluster;
			while (true){
				RESULT_e result;
				uint32_t next;
				result = ReadFAT(cluster, next);
				if (IsValidCluster(next) == false){
					// 次のクラスタが終端ならフラグメントの終端
					next_cluster = INVALID_CLUSTER;
					break;
				}
				if ((cluster + 1) != next){
					// クラスタが不連続ならフラグメントの終端
					next_cluster = next;
					break;
				}
				cluster++;
			}
			tail_cluster = cluster;
		}

		return RES_SUCCEEDED;
	}

	// FATに連続で書き込む
	RESULT_e ExFatFs::WriteContinuousFAT(uint32_t previous_cluster, uint32_t start_cluster, uint32_t cluster_count){
		RESULT_e result;
		
		if (previous_cluster != INVALID_CLUSTER){
			// previous_clusterからstart_clusterにリンクを作成
			result = WriteFAT(previous_cluster, start_cluster);
			if (result != RES_SUCCEEDED){
				return result;
			}
		}

		// FATにクラスタチェインを書き込む
		uint32_t cluster = start_cluster;
		while (0 < cluster_count){
			cluster_count--;
			uint32_t next_cluster = (cluster_count != 0) ? (cluster + 1) : TERMINAL_CLUSTER;
			result = WriteFAT(cluster, next_cluster);
			if (result != RES_SUCCEEDED){
				return result;
			}
			cluster++;
		}

		return RES_SUCCEEDED;
	}



	// 特殊なディレクトリエントリを検索する
	RESULT_e ExFatFs::FindSpecialDirEntry(void){
		RESULT_e result;

		// ルートディレクトリのChain_tを作成
		Chain_t chain;
		chain.flags = 0;
		chain.start_cluster = m_RootDirEntryCluster;
		chain.size = 0;
		result = InitChain(chain);
		if (result != RES_SUCCEEDED){
			return result;
		}

		uint8_t entry[DIR_ENTRY_LENGTH];
		while (true){
			// ディレクトリエントリを1つ読み取る
			result = ReadChain(chain, entry, DIR_ENTRY_LENGTH);
			if (result != RES_SUCCEEDED){
				return result;
			}

			switch (entry[0]){
			case 0x81:
				// Allocation Bitmap Directory Entry
				m_AllocBitmapCluster = LoadLE32(entry + 20);
				break;

			case 0x83:
				// Volume Label Directory Entry
				break;

			case 0x03:
				// Volume Label Directory Entry (not in use)
				break;

			case 0x00:
				if (m_AllocBitmapCluster != INVALID_CLUSTER){
					return RES_SUCCEEDED;
				}
				else{
					return RES_NOT_FOUND;
				}
			}
		}
	}

	// 条件に一致するディレクトリを検索する
	RESULT_e ExFatFs::FindDir(Chain_t &chain, CONDITION_t &conditions, bool detail){
		bool matched = false;					// 今のところ条件に一致している
		uint16_t *output_name_string;			// 出力中のファイル・ディレクトリ名
		const uint16_t *testing_name_string;	// 比較中のファイル・ディレクトリ名
		uint32_t testing_name_length;			// 比較中のファイル・ディレクトリ名の全体の長さ
		uint32_t testing_name_count;			// 比較中のファイル・ディレクトリ名の現在の長さ
		uint16_t stored_checksum;				// 格納されていたディレクトリエントリチェックサム
		uint16_t calculating_checksum;			// 計算中のディレクトリエントリのチェックサム
		while (true){
			// ディレクトリエントリを1つ読み取る
			RESULT_e result;
			uint32_t offset = (uint32_t)chain.pointer / DIR_ENTRY_LENGTH;
			uint8_t entry[DIR_ENTRY_LENGTH];
			result = ReadChain(chain, entry, DIR_ENTRY_LENGTH);
			if (result != RES_SUCCEEDED){
				return result;
			}

			switch (entry[0]){
			case 0x85:
				// File/Directory Entry

				// アトリビュートを比較
				matched = ((LoadLE16(entry + 4) & (conditions.attributes >> 16)) == (conditions.attributes & 0xFFFF));
				if (matched == true){
					// アトリビュートの条件が一致した
					if (detail == true){
						// プロパティをコピー
						conditions.property.attributes = LoadLE16(entry + 4);
						conditions.property.creation_time = LoadLE32(entry + 8);
						conditions.property.modified_time = LoadLE32(entry + 12);
						conditions.property.accessed_time = LoadLE32(entry + 16);
						conditions.direntry.flags = conditions.property.attributes;
						conditions.direntry.entry_flags = chain.flags;
						conditions.direntry.entry_cluster = chain.start_cluster;
						conditions.direntry.entry_offset = offset;
					}

					if (CHECK_CHECKSUM == true){
						// ディレクトリエントリのチェックサムを計算する
						stored_checksum = LoadLE16(entry + 2);
						calculating_checksum = CalcChecksum16(0x85, entry[1]);
						calculating_checksum = CalcChecksum16(calculating_checksum, entry + 4, 28);
					}
				}
				break;

			case 0xC0:
				// Stream Extension Directory Entry

				// ファイル名のチェックサムを比較
				matched &= (conditions.target_name_string == nullptr) | (conditions.target_name_checksum == LoadLE16(entry + 4));
				if (matched == true){
					// ファイル名のチェックサムが一致した
					output_name_string = conditions.output_name_string;
					testing_name_string = conditions.target_name_string;
					testing_name_length = entry[3];
					testing_name_count = 0;
					if (entry[1] & 0x2){
						conditions.direntry.flags |= FLAG_CONTIGUOUS;
					}
					conditions.direntry.start_cluster = LoadLE32(entry + 20);
					conditions.direntry.size = LoadLE64(entry + 24);
					conditions.property.size = conditions.direntry.size;

					if (CHECK_CHECKSUM == true){
						// ディレクトリエントリのチェックサムを計算する
						calculating_checksum = CalcChecksum16(calculating_checksum, entry, 32);
					}
				}
				break;

			case 0xC1:
				// File Name Extension Directory Entry

				if (matched == true){
					int32_t tlen = testing_name_length - testing_name_count;
					if (15 < tlen) tlen = 15;
					testing_name_count += tlen;

					if (testing_name_string != nullptr){
						// ファイル名の一部を比較
						const uint16_t *p = (const uint16_t*)(entry + 2);
						for (int32_t cnt = 0; cnt < tlen; cnt++){
							matched &= ((uint16_t)CharUpperUTF16((fschar_t)LoadLE16(p++)) == *testing_name_string++);
						}
					}

					if (output_name_string != nullptr){
						// ファイル名を出力する
						const uint16_t *p = (const uint16_t*)(entry + 2);
						for (int32_t cnt = 0; cnt < tlen; cnt++){
							*output_name_string++ = LoadLE16(p++);
						}
					}

					if (matched == true){
						if (CHECK_CHECKSUM == true){
							// ディレクトリエントリのチェックサムを計算する
							calculating_checksum = CalcChecksum16(calculating_checksum, entry, 32);
						}

						if (testing_name_length == testing_name_count){
							if ((CHECK_CHECKSUM == false) || (stored_checksum == calculating_checksum)){
								// すべてが条件に一致し、ディレクトリエントリのチェックサムも一致した
								if (output_name_string != nullptr){
									*output_name_string = 0x0000;	// u'\0'
								}
								return RES_SUCCEEDED;
							}
						}
					}
				}
				break;

			case 0x00:
				return RES_NOT_FOUND;
			}
		}
	}

	// 空きクラスタを検索する
	RESULT_e ExFatFs::FindFreeClusters(uint32_t start_cluster, uint32_t &found_cluster, uint32_t &contiguous_clusters){
		RESULT_e result;
		start_cluster = (start_cluster < MINIMUM_VALID_CLUSTER) ? 0 : (start_cluster - MINIMUM_VALID_CLUSTER);
		if (m_ClusterCount <= start_cluster){
			return RES_NOT_FOUND;
		}

		// ビットマップを読み取り、空きクラスタを探す
		uint32_t bytes_per_sector = 1UL << m_BytesPerSectorShift;
		uint32_t bits_per_sector = 8UL << m_BytesPerSectorShift;
		uint32_t offset = (start_cluster >> 3) & ~(bytes_per_sector - 1);	// offsetをセクター境界にそろえる
		uint32_t cluster = start_cluster;
		uint32_t contiguous_zeros = 0;
		uint32_t remaining_cluster_count = m_ClusterCount - start_cluster;
		uint32_t skip_bits = start_cluster & (bits_per_sector - 1);
		uint8_t *bitmap = (uint8_t*)m_AllocBitmapCache.GetBuffer(0);	// ビットマップはキャッシュメモリーから直接取得する
		do{
			// シークしてキャッシュする
			result = SeekChain(m_AllocBitmapChain, offset);
			if (result != RES_SUCCEEDED){
				return result;
			}
			result = CacheChain(m_AllocBitmapChain);
			if (result != RES_SUCCEEDED){
				return result;
			}
			offset += bytes_per_sector;

			// 空きクラスタを探す
			uint32_t *p = (uint32_t*)bitmap + skip_bits / 32;
			uint32_t *p_end = (uint32_t*)bitmap + bits_per_sector / 32;
			uint32_t count = bits_per_sector - skip_bits;
			if (skip_bits != 0){
				skip_bits = 0;
				*p |= NEtoLE32((1UL << (cluster & 0x1F)) - 1);
				cluster &= ~0x1F;
				m_AllocBitmapCache.Kill();	// 処理の簡便化のためだけにキャッシュの内容を書き換えたのでデータを無効化しておく
			}
			if (count < remaining_cluster_count){
				remaining_cluster_count -= count;
			}
			else{
				remaining_cluster_count = 0;
			}
			while (p < p_end){
				uint32_t bit = ~LoadLE32(p++);
				uint32_t sr = bit;
				if (contiguous_zeros == 0){
					if (bit == 0){
						cluster += 32;
						continue;
					}
					else{
						uint32_t tz = CountTrailingZeros32(bit);
						cluster += tz;
						sr >>= tz;
					}
				}
				while (sr & 0x1){
					contiguous_zeros++;
					sr >>= 1;
				}
				if ((sr & ~0x1) || (~bit & 0x80000000UL)){
					goto finish;
				}
			}
		} while (0 < remaining_cluster_count);
	finish:
		if (m_ClusterCount <= cluster){
			return RES_NOT_FOUND;
		}
		else{
			if ((m_ClusterCount - cluster) < contiguous_zeros){
				contiguous_zeros = m_ClusterCount - cluster;
			}
			contiguous_clusters = contiguous_zeros;
			found_cluster = MINIMUM_VALID_CLUSTER + cluster;
			return RES_SUCCEEDED;
		}
	}

	// クラスタ割り当てビットマップの値を変更する
	RESULT_e ExFatFs::FillAllocationBitmap(uint32_t start_cluster, uint32_t cluster_count, bool value){
		RESULT_e result;
		if (start_cluster < MINIMUM_VALID_CLUSTER){
			return RES_INTERNAL_ERROR;
		}
		start_cluster -= MINIMUM_VALID_CLUSTER;
		if (m_ClusterCount <= start_cluster){
			return RES_INTERNAL_ERROR;
		}
		if ((m_ClusterCount - start_cluster) < cluster_count){
			return RES_INTERNAL_ERROR;
		}

		// ビットマップを読み取り、値を書き換える
		uint32_t bytes_per_sector = 1UL << m_BytesPerSectorShift;
		uint32_t bits_per_sector = 8UL << m_BytesPerSectorShift;
		uint32_t offset = (start_cluster >> 3) & ~(bytes_per_sector - 1);	// offsetをセクター境界にそろえる
		uint32_t cluster = start_cluster;
		uint32_t skip_bits = cluster & (bits_per_sector - 1);
		uint8_t *bitmap = (uint8_t*)m_AllocBitmapCache.GetBuffer(0);	// ビットマップはキャッシュメモリーから直接取得する
		while (0 < cluster_count){
			// シークしてキャッシュする
			result = SeekChain(m_AllocBitmapChain, offset);
			if (result != RES_SUCCEEDED){
				return result;
			}
			result = CacheChain(m_AllocBitmapChain);
			if (result != RES_SUCCEEDED){
				return result;
			}

			// ビットをセット・クリアする
			uint32_t *p = (uint32_t*)bitmap + skip_bits / 32;
			uint32_t *p_end;
			uint32_t head = skip_bits & 0x1F;
			uint32_t tail;
			if ((skip_bits + cluster_count) < bits_per_sector){
				tail = (skip_bits + cluster_count) & 0x1F;
				p_end = (uint32_t*)bitmap + (skip_bits + cluster_count) / 32;
				cluster_count = 0;
			}
			else{
				tail = 0;
				p_end = (uint32_t*)bitmap + bits_per_sector / 32;
				cluster_count -= bits_per_sector - skip_bits;
			}
			skip_bits = 0;
			uint32_t head_mask = NEtoLE32(0xFFFFFFFFUL << head);
			uint32_t tail_mask = NEtoLE32(0xFFFFFFFFUL << tail);
			if (p == p_end){
				*p = value ? (*p | (head_mask & ~tail_mask)) : (*p & (~head_mask | tail_mask));
			}
			else{
				*p = value ? (*p | head_mask) : (*p & ~head_mask);
				p++;
				if (0 < tail){
					*p_end = value ? (*p_end | ~tail_mask) : (*p_end & tail_mask);
				}
			}

			if (value == true){
				while (p < p_end){
					*p++ = 0xFFFFFFFFUL;
				}
			}
			else{
				while (p < p_end){
					*p++ = 0x00000000UL;
				}
			}
			
			// 書き換えたことにする
			m_AllocBitmapCache.SetModified(true);
			offset += bytes_per_sector;
		};

		return RES_SUCCEEDED;
	}



	// ディレクトリエントリを作成する
	RESULT_e ExFatFs::Link(DirHandle &dirhandle, DirEntry_t &direntry, FileInfo_t *info){


		return RES_NOT_SUPPORTED;
	}

	// ディレクトリエントリを削除する
	RESULT_e ExFatFs::Unlink(Chain_t &dirchain, uint32_t target_offset){
		// ディレクトリエントリのある場所にシーク
		RESULT_e result;
		result = SeekChain(dirchain, target_offset);
		if (result != RES_SUCCEEDED){
			return result;
		}

		uint32_t offset = target_offset;
		while (true){
			// ディレクトリエントリを読み取る
			uint8_t entry[DIR_ENTRY_LENGTH];
			result = ReadChain(dirchain, entry, DIR_ENTRY_LENGTH);
			if (result != RES_SUCCEEDED){
				return result;
			}

			uint8_t entry_type = entry[0];
			if ((~entry_type & 0x80) || ((entry_type == 0x85) && (offset != target_offset))){
				// ディレクトリの終端に達するか次の項目が現れたので終了
				break;
			}

			// 使用中フラグをクリアして書き込む
			entry[0] = entry_type & ~0x80;
			result = SeekChain(dirchain, offset);
			if (result != RES_SUCCEEDED){
				return result;
			}
			result = WriteChain(dirchain, entry, DIR_ENTRY_LENGTH);
			if (result != RES_SUCCEEDED){
				return result;
			}
			offset += DIR_ENTRY_LENGTH;
		}

		return RES_SUCCEEDED;
	}







}
