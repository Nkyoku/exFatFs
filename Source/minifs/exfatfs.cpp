﻿#include "exfatfs.h"
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
			return RES_INVALID_DATA;
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
			return RES_INVALID_DATA;
		}

		// ボリュームサイズの確認
		if ((info.partition_size <= VBR_LENGTH) || (info.partition_size < volume_length)){
			return RES_INVALID_DATA;
		}

		// FATの数が1であるか確認
		if (number_of_fats != 1){
			return RES_NOT_SUPPORTED;
		}

		// FATの長さが十分か確認
		if ((fat_length << (bytes_per_sector_shift - 2)) < (cluster_count + MINIMUM_VALID_CLUSTER)){
			return RES_INVALID_DATA;
		}

		// FATとデータ領域が重なっていないか確認する
		if (cluster_heap_offset < (fat_offset + fat_length)){
			return RES_INVALID_DATA;
		}

		// データ領域がパーティションに収まっているか確認する
		if (volume_length < (LShift32to64(cluster_count, sectors_per_cluster_shift) + cluster_heap_offset)){
			return RES_INVALID_DATA;
		}

		// クラスター総数が規格内かを確認
		if (MAXIMUM_CLUSTER_COUNT < cluster_count){
			return RES_INVALID_DATA;
		}

		// セクターサイズを確認
		if (bytes_per_sector_shift != info.pdiskio->disk_bytesPerSectorShift()){
			return RES_NOT_SUPPORTED;
		}

		// クラスターサイズを確認
		if (24 < (bytes_per_sector_shift + sectors_per_cluster_shift)){
			return RES_INVALID_DATA;
		}

		// ブートシグニチャを確認する
		if (LoadLE16(buf + 510) != BOOT_SIGNATURE){
			return RES_INVALID_DATA;
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
				return RES_ERROR;
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
		m_FreeClusterCount = UNUSED_CLUSTER_COUNT;
		m_RootDirEntryCluster = root_directory_first_cluster;
		m_AllocBitmapCluster = INVALID_CLUSTER;
		m_FirstFreeCluster = MINIMUM_VALID_CLUSTER;

		// 特殊なディレクトリエントリを探す
		result = FindSpecialDirEntry();
		if (result != RES_SUCCEEDED){
			unmount();
		}

		// クラスタ割り当てビットマップを開く
		m_AllocBitmapManage.attributes = 0;
		m_AllocBitmapManage.start_cluster = m_AllocBitmapCluster;
		result = InitManage(m_AllocBitmapManage);
		if (result != RES_SUCCEEDED){
			return result;
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
					FlushCache(m_pDiskIO, m_AllocBitmapManage);

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
				if (m_FreeClusterCount == UNUSED_CLUSTER_COUNT){
					// クラスタ割り当てビットマップを検索して空きクラスタ数を調べる
					RESULT_e result;
					bool found_free_cluster = false;
					uint32_t free_cluster = MINIMUM_VALID_CLUSTER;
					uint32_t free_cluster_count = 0;
					result = SeekCluster(m_AllocBitmapManage, 0);
					if (result == RES_SUCCEEDED){
						// ビットマップを読み取り、空きクラスタを数える
						uint32_t remaining_cluster_count = m_ClusterCount;
						do{
							// ALLOC_BITMAP_UNITバイトだけ読み取る
							uint8_t bitmap[ALLOC_BITMAP_UNIT];
							result = ReadCluster(m_AllocBitmapManage, bitmap, ALLOC_BITMAP_UNIT);
							if (result != RES_SUCCEEDED){
								break;
							}

							// ビットの数を数える
							uint32_t *p = (uint32_t*)bitmap;
							uint32_t *p_end;
							if ((ALLOC_BITMAP_UNIT * 8) <= remaining_cluster_count){
								p_end = p + ALLOC_BITMAP_UNIT / 4;
								remaining_cluster_count -= ALLOC_BITMAP_UNIT * 8;
							}
							else{
								p_end = p + RShiftCeilingPV32(remaining_cluster_count, 5);
								if (remaining_cluster_count % 32){
									free_cluster_count -= PopCount32(~(LoadLE32(p_end - 1) | ((1UL << (remaining_cluster_count % 32)) - 1)));
								}
								remaining_cluster_count = 0;
							}
							if (found_free_cluster == false){
								// 空きクラスタを探す
								while (p < p_end){
									uint32_t bit = ~*p++;
									free_cluster_count += PopCount32(bit);
									if (bit != 0){
										free_cluster += CountTrailingZeros32(bit);
										found_free_cluster = true;
										break;
									}
									else{
										free_cluster += 32;
									}
								}
							}
							while (p < p_end){
								free_cluster_count += PopCount32(~*p++);
							}
						} while (0 < remaining_cluster_count);
					}

					m_FirstFreeCluster = free_cluster;
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
					Manage_t manage;
					result = FindDirEntry(manage, path, 0);
					if (result != RES_SUCCEEDED){
						return result;
					}
					if (IsValidCluster(manage.start_cluster) == true){
						// ファイル・ディレクトリはデータを持つ





					}




					result = RES_ERROR;
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
		// まずハンドルを閉じる
		RESULT_e result;
		result = dirhandle.close();
		if (result != RES_SUCCEEDED){
			return result;
		}

		if (lockMutex() == true){
			if (Mounted() == true){
				// 指定されたディレクトリのディレクトリエントリを探す
				result = FindDirEntry(GetManage(dirhandle), path, ATTR_IS_DIRECTORY);
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
	RESULT_e ExFatFs::ReadDir(DirHandle &dirhandle, DirInfo_t *pinfo){
		if (lockMutex() == true){
			RESULT_e result;
			if (Mounted() == true){
				if (Assigned(dirhandle) == true){
					Manage_t &manage = GetManage(dirhandle);
					if (pinfo == nullptr){
						return InitManage(manage);
					}
					else{
						// 検索条件を作成し検索する
						CONDITION_t conditions;
						conditions.attributes = 0;
						conditions.output_name_string = (uint16_t*)pinfo->name;
						conditions.target_name_string = nullptr;
						conditions.target_name_checksum = 0;
						result = FindDir(manage, conditions, true);
						if (result == RES_SUCCEEDED){
							// 検索結果を格納する
							*(DirInfoBase_t*)pinfo = (DirInfoBase_t&)conditions.managebase;
						}
					}
				}
				else{
					result = RES_INVALID_HANDLE;
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
		// まずハンドルを閉じる
		RESULT_e result;
		result = filehandle.close();
		if (result != RES_SUCCEEDED){
			return result;
		}
		
		if (lockMutex() == true){
			if (Mounted() == true){
				// optionによって動作を変える





				// 指定されたファイルのディレクトリエントリを探す
				result = FindDirEntry(GetManage(filehandle), path, ATTR_IS_FILE);
				if (result == RES_SUCCEEDED){
					if (option & O_TRUNCATE){
						return RES_REJECTED;	// ファイルが存在しないときはRES_NOT_FOUNDを返してほしいので、O_TRUNCATEのエラー処理はここ
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
				uint64_t size = filehandle.size();
				if (filehandle.writable() == false){
					if (size < offset) offset = size;
					result = SeekCluster(GetManage(filehandle), offset);
				}
				else if (offset <= size){
					result = SeekCluster(GetManage(filehandle), offset);
				}
				else{
					// ファイルサイズを拡張する
					result = RES_NOT_SUPPORTED;
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
				uint64_t remaining = filehandle.size() - filehandle.tell();
				if (remaining < (uint64_t)length) length = (uint32_t)remaining;
				if (ReadCluster(GetManage(filehandle), buf, length) != RES_SUCCEEDED){
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
		if (Writable() == true){
			if (lockMutex() == true){
				if (Mounted() == true){



					length = 0;
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
		else{
			return 0;
		}
	}

	// ファイルをフラッシュする
	RESULT_e ExFatFs::FlushFile(FileHandle &filehandle){
		if (Writable() == true){
			if (lockMutex() == true){
				RESULT_e result;
				if (Mounted() == true){
					result = FlushCache(m_pDiskIO, GetManage(filehandle));;
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
			return RES_SUCCEEDED;
		}
	}

	// ファイルを切り詰める
	RESULT_e ExFatFs::TruncateFile(FileHandle &filehandle){
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



	// クラスタへアクセスする前にManage_tを初期化する
	RESULT_e ExFatFs::InitManage(Manage_t &manage){
		manage.init();
		manage.current_cluster = manage.start_cluster;
		return LoadFragment(manage, manage.current_cluster);
	}

	// フラグメントを把握する
	RESULT_e ExFatFs::LoadFragment(Manage_t &manage, uint32_t start_cluster){
		manage.frag_head_cluster = start_cluster;

		if (start_cluster == INVALID_CLUSTER){
			// ファイルが存在しない
			manage.frag_tail_cluster = INVALID_CLUSTER;
			manage.next_frag_cluster = INVALID_CLUSTER;
		}
		else if (manage.attributes & ATTR_CONTIGUOUS){
			// クラスタチェインが存在しない
			manage.frag_tail_cluster = manage.start_cluster + RShiftCeilingPV64to32(manage.size, m_SectorsPerClusterShift) - 1;
			manage.next_frag_cluster = INVALID_CLUSTER;
		}
		else{
			// start_clusterの含まれるFATの一部をキャッシュする
			RESULT_e result;
			uint32_t sector_addr = m_FATOffset + (start_cluster >> (m_BytesPerSectorShift - 2));
			result = ReadToCache(m_pDiskIO, manage, sector_addr);
			if (result != RES_SUCCEEDED){
				return result;
			}

			// クラスタチェインが途切れるかセクターの境界まで達したらそこまでをフラグメントとする
			uint32_t cluster = start_cluster;
			uint32_t next_cluster = INVALID_CLUSTER;
			while (true){
				//uint32_t mask = (m_SectorSize / 4) - 1;
				uint32_t mask = (1UL << (m_BytesPerSectorShift - 2)) - 1;
				next_cluster = LoadLE32(manage.cache + 4 * (cluster & mask));
				if (IsValidCluster(next_cluster) == false){
					// 次のクラスタが終端ならフラグメントの終端
					next_cluster = INVALID_CLUSTER;
					break;
				}
				if ((cluster & mask) == mask){
					// セクターの境界を跨いだらフラグメントの終端として扱う(適当なところで切ってLoadFragment()の実行時間を抑えるため)
					break;
				}
				if ((cluster + 1) != next_cluster){
					// クラスタが不連続ならフラグメントの終端
					break;
				}
				cluster++;
			}
			manage.frag_tail_cluster = cluster;
			manage.next_frag_cluster = next_cluster;
		}

		return RES_SUCCEEDED;
	}

	// ポインタをシークする
	RESULT_e ExFatFs::SeekCluster(Manage_t &manage, uint64_t offset){
		if (manage.start_cluster == INVALID_CLUSTER){
			return RES_NO_DATA;
		}
		else{
			RESULT_e result;
			uint32_t bytes_per_cluster = 1UL << (m_BytesPerSectorShift + m_SectorsPerClusterShift);
			uint32_t pointer_lapped_cluster = (uint32_t)manage.pointer & (bytes_per_cluster - 1);
			uint32_t offset_lapped_cluster = (uint32_t)offset & (bytes_per_cluster - 1);

			if (offset == manage.pointer){
				// ポインタは動かない
				return RES_SUCCEEDED;
			}
			else if (offset < manage.pointer){
				// ポインタは後退する
				if (manage.current_cluster != INVALID_CLUSTER){
					// シーク先が現在のフラグメントの中にあるか
					uint32_t back = (uint32_t)((manage.pointer - offset) >> (m_SectorsPerClusterShift + m_BytesPerSectorShift));
					if (pointer_lapped_cluster < offset_lapped_cluster){
						back++;
					}
					if (manage.frag_head_cluster <= (manage.current_cluster - back)){
						// シーク先は現在のフラグメント中にある
						manage.current_cluster -= back;
						manage.pointer = offset;
						return RES_SUCCEEDED;
					}
				}

				// シーク先は現在のフラグメント中にない
				// ポインタをクラスタチェインの最初に戻しシークしなおす
				manage.current_cluster = manage.start_cluster;
				manage.pointer = 0;
				result = LoadFragment(manage, manage.current_cluster);
				if (result != RES_SUCCEEDED){
					return result;
				}
			}

			// ポインタは前進する
			if (manage.current_cluster == INVALID_CLUSTER){
				// ポインタはクラスタチェインの終端に達している
				return RES_NO_DATA;
			}
			else{
				// クラスタチェインを目的のクラスタ数だけ辿る
				uint32_t forward = (uint32_t)((offset - manage.pointer) >> (m_SectorsPerClusterShift + m_BytesPerSectorShift));
				if (offset_lapped_cluster < pointer_lapped_cluster){
					forward++;
				}
				manage.pointer = offset;
				while (true){
					uint32_t diff = manage.frag_tail_cluster - manage.current_cluster + 1;
					if (forward < diff){
						// シーク先は現在のフラグメント中にある
						manage.current_cluster += forward;
						return RES_SUCCEEDED;
					}
					else{
						// シーク先は現在のフラグメント中にない。次のフラグメントへ移動する
						forward -= diff;
						manage.current_cluster = manage.next_frag_cluster;
						result = LoadFragment(manage, manage.current_cluster);
						if (result != RES_SUCCEEDED){
							return result;
						}
					}
				}
			}
		}
	}

	// クラスタを読み出す
	RESULT_e ExFatFs::ReadCluster(Manage_t &manage, void *buf, uint32_t length){
		if ((manage.start_cluster == INVALID_CLUSTER) || (manage.current_cluster == INVALID_CLUSTER)){
			// 読み出せるデータがない
			return RES_NO_DATA;
		}

		RESULT_e result;
		uint32_t sector_in_cluster = ((uint32_t)manage.pointer >> m_BytesPerSectorShift) & ((1UL << m_SectorsPerClusterShift) - 1);	// クラスタ内でのセクター番号
		uint32_t offset_in_sector = (uint32_t)manage.pointer & ((1UL << m_BytesPerSectorShift) - 1);							// セクター内でのオフセット
		while (0 < length){
			bool next_cluster = false;
			uint32_t read_bytes = 0;

			uint32_t sector_addr = m_ClusterOffset + (manage.current_cluster << m_SectorsPerClusterShift) + sector_in_cluster;
			if (offset_in_sector == 0){
				// シークポインタはセクターの境界
				//if ((1UL << m_BytesPerSectorShift) <= length){
				if (0 < (length >> m_BytesPerSectorShift)){
					// バッファがセクターサイズ以上あるため、セクターをバッファに直接読み込む
					result = m_pDiskIO->disk_read(buf, sector_addr, 1);
					if (result != RES_SUCCEEDED){
						return result;
					}
					read_bytes = 1UL << m_BytesPerSectorShift;
					sector_in_cluster = (sector_in_cluster + 1) & ((1UL << m_SectorsPerClusterShift) - 1);
					next_cluster = (sector_in_cluster == 0);
				}
				else{
					// セクターをキャッシュに読み込んでからバッファに転送する
					result = ReadWithCache(m_pDiskIO, manage, sector_addr, buf, 0, length);
					if (result != RES_SUCCEEDED){
						return result;
					}
					read_bytes = length;
				}
			}
			else{
				// シークポインタはセクターの途中
				uint32_t last_bytes = (1UL << m_BytesPerSectorShift) - offset_in_sector;
				read_bytes = (length < last_bytes) ? length : last_bytes;

				// キャッシュを通してバッファに転送する
				result = ReadWithCache(m_pDiskIO, manage, sector_addr, buf, offset_in_sector, read_bytes);
				if (result != RES_SUCCEEDED){
					return result;
				}

				offset_in_sector = 0;
			}

			buf = (uint8_t*)buf + read_bytes;
			length -= read_bytes;
			manage.pointer += read_bytes;

			if (next_cluster == true){
				// 次のクラスタへ移動する
				if (manage.current_cluster == manage.frag_tail_cluster){
					// 現在のクラスタはフラグメントの最後のクラスタであるため、次のフラグメントを読み込む
					manage.current_cluster = manage.next_frag_cluster;
					result = LoadFragment(manage, manage.next_frag_cluster);
					if (result != RES_SUCCEEDED){
						return result;
					}
				}
				else{
					// フラグメント内の次のクラスタに移動する
					manage.current_cluster++;
				}
			}
		}

		return RES_SUCCEEDED;
	}

	// クラスタへ書き込む
	RESULT_e ExFatFs::WriteCluster(Manage_t &manage, const void *buf, uint32_t length){
		return RES_NOT_SUPPORTED;
	}



	// 特殊なディレクトリエントリを検索する
	RESULT_e ExFatFs::FindSpecialDirEntry(void){
		RESULT_e result;

		// ルートディレクトリのManage_tを作成
		Manage_t manage;
		manage.attributes = 0;
		manage.start_cluster = m_RootDirEntryCluster;
		result = InitManage(manage);
		if (result != RES_SUCCEEDED){
			return result;
		}

		uint8_t entry[DIR_ENTRY_LENGTH];
		while (true){
			// ディレクトリエントリを1つ読み取る
			result = ReadCluster(manage, entry, DIR_ENTRY_LENGTH);
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
	RESULT_e ExFatFs::FindDir(Manage_t &manage, CONDITION_t &conditions, bool detail){
		uint8_t entry[DIR_ENTRY_LENGTH];
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
			uint32_t offset = (uint32_t)manage.pointer / DIR_ENTRY_LENGTH;
			result = ReadCluster(manage, entry, DIR_ENTRY_LENGTH);
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
						conditions.managebase.entry_cluster = manage.start_cluster;
						conditions.managebase.entry_offset = offset;
						conditions.managebase.attributes = LoadLE16(entry + 4);
						conditions.managebase.creation_time = LoadLE32(entry + 8);
						conditions.managebase.modified_time = LoadLE32(entry + 12);
						conditions.managebase.accessed_time = LoadLE32(entry + 16);
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
					if (entry[1] & 0x2) conditions.managebase.attributes |= ATTR_CONTIGUOUS;
					conditions.managebase.start_cluster = LoadLE32(entry + 20);
					conditions.managebase.size = LoadLE64(entry + 24);

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

	// ディレクトリエントリを検索する
	RESULT_e ExFatFs::FindDirEntry(Manage_t &manage, const fschar_t *path, uint32_t attributes){
		// ルートディレクトリをセット
		manage.size = 0;
		manage.creation_time = 0;
		manage.modified_time = 0;
		manage.accessed_time = 0;
		manage.attributes = ATTR_DIRECTORY;
		manage.entry_cluster = 0;
		manage.entry_offset = 0;
		manage.start_cluster = m_RootDirEntryCluster;

		// パスをリストに変換
		uint16_t path_list[MAXIMUM_PATH + 1];
		if (ConvertPathToList(path_list, path) == false){
			// パスの形式が不正だった
			return RES_INVALID_PATH;
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
						result = InitManage(manage);
						if (result != RES_SUCCEEDED){
							return result;
						}
						result = FindDir(manage, cond, (*p == 0x0000));	// リストの最後の項目だったらプロパティを要求する
						if (result != RES_SUCCEEDED){
							return result;
						}
						(ManageBase_t&)manage = cond.managebase;

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

		return InitManage(manage);
	}

	// 空きクラスタを検索する
	RESULT_e ExFatFs::FindFreeCluster(uint32_t start_cluster, uint32_t &found_cluster){
		RESULT_e result;
		uint32_t cluster = start_cluster;
		if ((MINIMUM_VALID_CLUSTER + m_ClusterCount) <= cluster){
			return RES_NOT_FOUND;
		}
		cluster = (cluster < MINIMUM_VALID_CLUSTER) ? 0 : (cluster - MINIMUM_VALID_CLUSTER);

		// 検索を始めるところにシークする
		uint32_t offset = (cluster >> 3) & ~(ALLOC_BITMAP_UNIT - 1);	// offsetをALLOC_BITMAP_UNITの倍数にする
		result = SeekCluster(m_AllocBitmapManage, offset);
		if (result != RES_SUCCEEDED){
			return result;
		}

		// ビットマップを読み取り、空きクラスタを探す
		uint32_t remaining_cluster_count = m_ClusterCount - cluster;
		while (true){
			// ALLOC_BITMAP_UNITバイトだけ読み取る
			uint8_t bitmap[ALLOC_BITMAP_UNIT];
			result = ReadCluster(m_AllocBitmapManage, bitmap, ALLOC_BITMAP_UNIT);
			if (result != RES_SUCCEEDED){
				return result;
			}

			uint32_t *p;
			uint32_t *p_end = (uint32_t*)bitmap + ALLOC_BITMAP_UNIT / 4;
			uint32_t count;
			uint32_t droping = cluster & (ALLOC_BITMAP_UNIT * 8 - 1);
			if (droping != 0){
				// 空きクラスタを探す
				count = (ALLOC_BITMAP_UNIT * 8) - droping;
				uint32_t remaining_bits = count & 0x1F;
				p = (uint32_t*)bitmap + droping / 32;
				uint32_t bit = ~*p++ >> (cluster & 0x1F);
				if (bit != 0){
					cluster += CountTrailingZeros32(bit);
					goto finish;
				}
				cluster += remaining_bits;
			}
			else{
				p = (uint32_t*)bitmap;
				count = ((ALLOC_BITMAP_UNIT * 8) < remaining_cluster_count) ? (ALLOC_BITMAP_UNIT * 8) : remaining_cluster_count;
			}
			
			// 空きクラスタを探す
			while (p < p_end){
				uint32_t bit = ~*p++;
				if (bit != 0){
					cluster += CountTrailingZeros32(bit);
					goto finish;
				}
				else{
					cluster += 32;
				}
			}

			if (remaining_cluster_count <= count){
				break;
			}
			remaining_cluster_count -= count;
		};
	finish:
		if (m_ClusterCount <= cluster){
			found_cluster = INVALID_CLUSTER;
		}

		found_cluster = MINIMUM_VALID_CLUSTER + cluster;
		return RES_SUCCEEDED;
	}













}