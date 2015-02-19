#pragma once
#ifndef MINIFS_DISKIO_H
#define MINIFS_DISKIO_H

#include <stdint.h>
#include "minifs_common.h"



namespace mfs{
	// ディスクI/Oを提供するインターフェースクラス
	class IMiniFSDiskIO{
	public:
		// ディスクを使用可能にする
		virtual RESULT_e disk_initialize(void) = 0;

		// ディスクの情報を取得する
		virtual uint32_t disk_info(DISKINFO_e param) = 0;

		// セクターを読み出す
		virtual RESULT_e disk_read(void *buf, uint32_t sector, uint32_t count) = 0;

		// セクターへ書き込む
		virtual RESULT_e disk_write(const void *buf, uint32_t sector, uint32_t count){ return RES_NOT_SUPPORTED; }

		// 書き込み処理を完了させる
		virtual RESULT_e disk_sync(void){ return RES_SUCCEEDED; }



	public:
		// ディスクの状態を取得する
		uint32_t disk_status(void){ return disk_info(INFO_STATUS); }
		
		// セクターあたりのバイト数を取得する
		uint32_t disk_bytesPerSectorShift(void){ return disk_info(INFO_BYTES_PER_SECTOR_SHIFT); }

		// セクターあたりのバイト数を取得する
		uint32_t disk_bytesPerSector(void){ return 1UL << disk_bytesPerSectorShift(); }

		// セクター総数を取得する
		uint32_t disk_sectorCount(void){ return disk_info(INFO_SECTOR_COUNT); }

		// ブロックあたりのセクター数を取得する
		uint32_t disk_sectorsPerBlockShift(void){ return disk_info(INFO_SECTORS_PER_BLOCK_SHIFT); }

		// ブロックあたりのセクター数を取得する
		uint32_t disk_sectorsPerBlock(void){ return 1UL << disk_sectorsPerBlockShift(); }
	};
}



#endif // MINIFS_DISKIO_H
