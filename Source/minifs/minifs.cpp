#include "minifs.h"
#include <string.h>

#include <stdio.h>

namespace mfs{
	// パーティションの種類を判別する
	static inline PartitionType_e GetPartitionType(uint8_t type, PartitionInfo_t &info){
		switch (type){
		case 0x00:
			return PID_EMPTY;

		case 0x01:	// FAT12
		case 0x04:	// FAT16 (32MB以下)
		case 0x06:	// FAT16 (32MB越え)
		case 0x0E:	// FAT16X (LBA)
		case 0x0B:	// FAT32
		case 0x0C:	// FAT32X (LBA)
			return PID_FAT;

		case 0x07:	// NTFS, exFAT
			if (ExFatFs::isExFAT(info) == true){
				return PID_EXFAT;
			}
			else{
				return PID_OTHERS;
			}

		case 0x05:	// 拡張パーティション(8GB以前)
		case 0x0F:	// 拡張パーティション(8GB以降)
			return PID_EXTENDED;

		default:
			return PID_OTHERS;
		}
	}

	// パーティションのリストを取得する
	int32_t MiniFS::getPartitionInfoList(IMiniFSDiskIO &diskio, PartitionInfo_t *info_list, uint32_t length_of_list){
		// ディスクのステータスを取得
		uint32_t status;
		status = diskio.disk_status();
		if (status & STA_NO_DISK){
			// ディスクが存在しないので失敗
			return -1;
		}
		if (status & STA_NOT_INITIALIZED){
			// ディスクが未初期化であるので初期化する
			if (diskio.disk_initialize() != RES_SUCCEEDED){
				// 初期化できなかったので失敗
				return -1;
			}
		}

		// セクターサイズとセクター総数を取得
		uint32_t bytes_per_sector_shift = diskio.disk_bytesPerSectorShift();
		uint32_t sector_count = diskio.disk_sectorCount();
		if ((bytes_per_sector_shift < MIN_SECTOR_SIZE_SHIFT) || (MAX_SECTOR_SIZE_SHIFT < bytes_per_sector_shift)){
			// 非対応のセクターサイズであるので失敗
			return -1;
		}

		if (sector_count == 0){
			// MBRが読み取れないため失敗
			return -1;
		}
		
		uint32_t found_partitions = 0;
		uint32_t found_valid_partitions = 0;
		if (0 < length_of_list){
			uint8_t buf[MAX_SECTOR_SIZE];
			uint32_t epbr_offset = 0;
			uint32_t current_offset;
			uint32_t extended_offset = 0;
			do{
				current_offset = extended_offset;
				if ((epbr_offset == 0) && (extended_offset != 0)){
					epbr_offset = extended_offset;
				}

				// MBRを読み込む
				if (diskio.disk_read(buf, current_offset, 1) == RES_SUCCEEDED){
					// ブートシグニチャを確認する
					if (LoadLE16(buf + 510) == BOOT_SIGNATURE){
						// パーティションテーブルを読み出す
						for (uint32_t offset = 446; offset < 510; offset += 16){
							uint8_t item[16];
							memcpy(item, buf + offset, 16);

							// アクティブフラグのチェック
							if (item[0] & 0x7F){
								continue;
							}

							// パーティション情報をコピーする
							uint32_t ptoffset = LoadLE32(item + 8);
							uint32_t ptsize = LoadLE32(item + 12);
							PartitionInfo_t info;
							info.pdiskio = &diskio;
							info.table_sector = current_offset;
							info.table_offset = offset;
							info.active_flag = (item[0] & 0x80) ? true : false;
							info.partition_sector = current_offset + ptoffset;
							info.partition_size = ptsize;
							info.partition_type = GetPartitionType(item[4], info);

							//wprintf(L"%d id=0x%02X, offset=%d, size=%d\n", current_offset, item[4], ptoffset, ptsize);

							if ((0 < ptoffset) && (0 < ptsize)){
								if (info.partition_type != PID_EXTENDED){
									if ((current_offset + ptoffset + ptsize) <= sector_count){
										found_partitions++;
										// 有効なパーティションを見つけたのでリストに追加する
										*info_list++ = info;
										found_valid_partitions++;
										length_of_list--;
										if (length_of_list == 0){
											goto finish;
										}
									}
								}
								else{
									found_partitions++;
									if ((epbr_offset + ptoffset + ptsize) <= sector_count){
										// 拡張パーティションを見つけたので、次のループでこれを読み出す
										extended_offset = epbr_offset + ptoffset;
									}
								}
							}
						}
					}
				}
			} while ((current_offset != extended_offset) && (found_partitions < MAXIMUM_PARTITIONS));
		}

	finish:
		return found_valid_partitions;
	}

	// パーティションの分割をする
	RESULT_e MiniFS::initPartitions(IMiniFSDiskIO &diskio, const uint32_t *size_list, uint32_t length_of_list, bool clean){
		// ディスクのステータスを取得
		uint32_t status;
		status = diskio.disk_status();
		if (status & STA_NO_DISK){
			// ディスクが存在しないので失敗
			return RES_NOT_READY;
		}
		if (status & STA_NOT_INITIALIZED){
			// ディスクが未初期化であるので初期化する
			if (diskio.disk_initialize() != RES_SUCCEEDED){
				// 初期化できなかったので失敗
				return RES_NOT_READY;
			}
		}

		// セクターサイズとセクター総数とブロックサイズを取得
		uint32_t bytes_per_sector_shift = diskio.disk_bytesPerSectorShift();
		uint32_t bytes_per_sector = 1UL << bytes_per_sector_shift;
		uint32_t sector_count = diskio.disk_sectorCount();
		uint32_t sectors_per_block_shift = diskio.disk_sectorsPerBlockShift();
		if ((bytes_per_sector_shift < MIN_SECTOR_SIZE_SHIFT) || (MAX_SECTOR_SIZE_SHIFT < bytes_per_sector_shift)){
			// 非対応のセクターサイズであるので失敗
			return RES_NOT_SUPPORTED;
		}


		if ((MIN_SECTOR_SIZE != bytes_per_sector) && (MAX_SECTOR_SIZE != bytes_per_sector)){
			// 非対応のセクターサイズであるので失敗
			return RES_NOT_SUPPORTED;
		}

		if (sector_count == 0){
			// MBRが読み取れないため失敗
			return RES_NO_FILESYSTEM;
		}

		// パーティションの数は4つまで
		if (4 < length_of_list){
			return RES_NOT_SUPPORTED;
		}

		// サイズの合計を計算
		/*uint32_t total_sector_count;
		uint64_t total_sector_count64 = 1;
		for (uint32_t cnt = 0; cnt < length_of_list; cnt++){
			total_sector_count64 += size_list[cnt];
		}
		if ((uint64_t)sector_count < total_sector_count64){
			return RES_INTERNAL_ERROR;
		}
		total_sector_count = (uint64_t)total_sector_count64;*/

		// 最初のパーティションの位置を計算
		uint32_t ptoffset_lba = FIRST_PARTITION_SECTOR;

		// MBRを作成
		uint8_t buf[MAX_SECTOR_SIZE];
		memset(buf, 0x00, bytes_per_sector);
		StoreLE16(buf + 510, BOOT_SIGNATURE);
		for (uint32_t offset = 446; offset < 510; offset += 16){
			uint32_t ptstart_chs;
			uint32_t ptend_chs;
			//uint32_t ptoffset_lba;
			uint32_t ptsize_lba;
			if (0 < length_of_list){
				length_of_list--;

				// パーティションの位置とサイズを計算
				ptstart_chs = 0xFFFFFE;
				ptend_chs = 0xFFFFFE;
				ptoffset_lba = RShiftCeiling32(ptoffset_lba, sectors_per_block_shift) << sectors_per_block_shift;
				ptsize_lba = *size_list++;
				if (sector_count <= ptoffset_lba){
					return RES_INTERNAL_ERROR;
				}
				if ((sector_count - ptoffset_lba) < ptsize_lba){
					ptsize_lba = sector_count - ptoffset_lba;
				}
			}
			else{
				ptstart_chs = 0;
				ptend_chs = 0;
				ptoffset_lba = 0;
				ptsize_lba = 0;
			}

			//wprintf(L"%d offset=%d, size=%d\n", offset, ptoffset_lba, ptsize_lba);

			// MBRにコピー
			uint8_t item[16];
			StoreLE32(item, (ptstart_chs << 8) | 0x00);
			StoreLE32(item + 4, (ptend_chs << 8) | 0x00);
			StoreLE32(item + 8, ptoffset_lba);
			StoreLE32(item + 12, ptsize_lba);
			memcpy(buf + offset, item, 16);

			ptoffset_lba += ptsize_lba;
		}

		// MBRを書き込む
		RESULT_e result;
		result = diskio.disk_write(buf, 0, 1);
		
		if (clean == true){
			// データを消去する
			memset(buf, 0x00, bytes_per_sector);
			for (uint32_t sector = 1; (sector < sector_count) && (result == RES_SUCCEEDED); sector++){
				result = diskio.disk_write(buf, sector, 1);
			}
		}
		
		return result;
	}

	// パーティションをフォーマットする
	RESULT_e MiniFS::formatPartition(PartitionInfo_t &info, PartitionType_e type, uint32_t sectors_per_cluster_shift){
		if (READ_ONLY == true){
			return RES_NOT_SUPPORTED;
		}
		else{
			uint8_t pid = 0x00;
			RESULT_e result;
			switch (type){
			case PID_EMPTY:	// 空
				result = RES_SUCCEEDED;
				break;

			case PID_FAT:	// FAT
				result = RES_NOT_SUPPORTED;
				break;

			case PID_EXFAT:	// exFAT
				if (SUPPORT_EXFAT_FORMAT == true){
					result = ExFatFs::format(info, sectors_per_cluster_shift, pid);
				}
				else{
					result = RES_NOT_SUPPORTED;
				}
				break;

			default:
				result = RES_NOT_SUPPORTED;
			}
			if (result == RES_SUCCEEDED){
				// パーティションテーブルのパーティション識別子を書き換える
				uint8_t buf[MAX_SECTOR_SIZE];
				result = info.pdiskio->disk_read(buf, info.table_sector, 1);
				if (result == RES_SUCCEEDED){
					if (buf[info.table_offset + 4] != pid){
						buf[info.table_offset + 4] = pid;
						result = info.pdiskio->disk_write(buf, info.table_sector, 1);
					}
				}
			}
			return result;
		}
	}

	// パーティションをマウントする
	IMiniFSFileSystem* MiniFS::mountPartition(PartitionInfo_t &info){
		switch (info.partition_type){
		case PID_FAT:	// FAT
			{







				return nullptr;
			}
			
		case PID_EXFAT:	// exFAT
			{
				ExFatFs *pfs = new ExFatFs();
				if (pfs->mount(info) == RES_SUCCEEDED){
					return pfs;
				}
				else{
					delete pfs;
					return nullptr;
				}
			}

		default:
			return nullptr;
		}
	}



}
