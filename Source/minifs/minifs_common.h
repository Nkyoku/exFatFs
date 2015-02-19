#pragma once
#ifndef MINIFS_COMMON_H
#define MINIFS_COMMON_H

#include <stdint.h>
#include "minifs_builtin.h"


namespace mfs{
	// ディスクI/Oを提供するインターフェースクラス
	class IMiniFSDiskIO;



	// セクターサイズの最小値
	static const uint32_t MIN_SECTOR_SIZE = 512;

	// セクターサイズの最小値のシフト量
	static const uint32_t MIN_SECTOR_SIZE_SHIFT = 9;

	// セクターサイズの最大値
	static const uint32_t MAX_SECTOR_SIZE = !SUPPORT_4K_SECTOR ? 512 : 4096;

	// セクターサイズの最大値のシフト量
	static const uint32_t MAX_SECTOR_SIZE_SHIFT = !SUPPORT_4K_SECTOR ? 9 : 12;

	// MBRのブートシグネチャ
	static const uint16_t BOOT_SIGNATURE = 0xAA55;



	// ディスク情報
	enum DISKINFO_e{
		INFO_STATUS,
		INFO_BYTES_PER_SECTOR_SHIFT,
		INFO_SECTOR_COUNT,
		INFO_SECTORS_PER_BLOCK_SHIFT,
	};

	// ディスクステータスフラグ
	enum DISKSTATUS_e{
		STA_NOT_INITIALIZED = 0x1,		// ディスクは初期化されていない
		STA_NO_DISK = 0x2,				// ディスクは存在しない
		STA_WRITE_PROTECTED = 0x4,		// 書き込みが禁止されている
	};

	// 戻り値
	enum RESULT_e{
		RES_SUCCEEDED = 0,				// 関数は正常に終了した
		RES_ERROR,						// 何らかのエラーが発生した
		RES_REJECTED,					// ディスクアクセスが拒否された
		RES_NOT_READY,					// ディスクの準備ができていない
		RES_NO_DISK,					// ディスクが存在しない
		RES_INVALID_DATA,				// データが異常である
		RES_NOT_SUPPORTED,				// 非対応の形式である
		RES_NOT_MOUNTED,				// ファイルシステムはマウントされていない
		RES_NOT_FOUND,					// 条件に一致しなかった、ファイル・ディレクトリは存在しない
		RES_INVALID_PATH,				// ファイルパスの形式が不正である
		RES_NO_DATA,					// 読み出せるデータが存在しない
		RES_INVALID_HANDLE,				// ファイル・ディレクトリハンドルが異常である
		RES_TIMEOUT,					// 操作がタイムアウトした
		RES_SIZE_EXCEEDED,				// サイズがオーバーした
	};

	// パーティションの種類
	enum PartitionType_e{
		PID_EMPTY = 0,					// 空のパーティション
		PID_FAT,						// FAT12, FAT16, FAT32
		PID_EXFAT,						// exFAT
		PID_EXTENDED,					// (拡張パーティション)
		PID_OTHERS						// 判別できないファイルシステム
	};

	// ファイルアトリビュート
	enum ATTRIBUTES_e{
		ATTR_READONLY	= 0x0001,		// 読み込み専用
		ATTR_HIDDEN		= 0x0002,		// 隠しファイル・ディレクトリ
		ATTR_SYSTEM		= 0x0004,		// システムのファイル・ディレクトリ
		ATTR_DIRECTORY	= 0x0010,		// ディレクトリ
		ATTR_ARCHIVE	= 0x0020,		// アーカイブ
		ATTR_WRITABLE	= 0x00010000,	// (書き込みモードで開かれている)
		ATTR_CONTIGUOUS	= 0x00020000,	// (フラグメンテーションが起きていない)
	};

	// パーティション情報を格納する構造体
	struct PartitionInfo_t{
		IMiniFSDiskIO *pdiskio;			// ディスクI/Oへのポインタ
		uint32_t table_sector;			// パーティションテーブルのあるセクター番号
		uint32_t table_offset;			// パーティションテーブルのオフセット
		bool active_flag;				// アクティブフラグ
		PartitionType_e partition_type;	// パーティションの種類
		uint32_t partition_sector;		// パーティションの開始セクター番号
		uint32_t partition_size;		// パーティションのセクター総数
	};

	// ファイル・ディレクトリの情報を格納する構造体
	struct DirInfoBase_t{
		uint64_t size;					// サイズ
		uint32_t creation_time;			// 作成日時
		uint32_t modified_time;			// 更新日時
		uint32_t accessed_time;			// アクセス日時
		uint32_t attributes;			// アトリビュート
	};

	// ファイル・ディレクトリの情報を格納する構造体
	struct DirInfo_t : DirInfoBase_t{
		fschar_t name[MAXIMUM_PATH + 1];// ファイル・ディレクトリ名
	};

	// ファイル・ディレクトリを管理する構造体
	struct ManageBase_t : DirInfoBase_t{
		uint32_t entry_cluster;			// ディレクトリエントリのあるディレクトリのクラスタ番号
		uint32_t entry_offset;			// ディレクトリエントリのあるオフセット
		uint32_t start_cluster;			// データのある最初のクラスタ番号
	};

	// ファイル・ディレクトリにアクセスするための構造体
	struct Manage_t : ManageBase_t{
		uint32_t frag_head_cluster;		// 現在アクセス中のフラグメントの最初のクラスタ番号
		uint32_t frag_tail_cluster;		// 現在アクセス中のフラグメントの最後のクラスタ番号
		uint32_t next_frag_cluster;		// 次のフラグメントの最初のクラスタ番号
		uint32_t current_cluster;		// 現在のポインタがあるクラスタ番号
		uint64_t pointer;				// ポインタ
		
		bool cache_modified;			// キャッシュが書き換えられたことを示すフラグ
		uint32_t cached_sector;			// キャッシュしているセクター番号
		uint8_t cache[MAX_SECTOR_SIZE];	// キャッシュ

		// 初期化する
		void init(void){
			frag_head_cluster = 0;
			frag_tail_cluster = 0;
			next_frag_cluster = 0;
			current_cluster = 0;
			pointer = 0;
			cache_modified = false;
			cached_sector = 0;
		}
	};

	// ディレクトリを管理する構造体
	struct DirManage_t{
		uint32_t dummy;
		
		// 初期化する
		void init(void){
			
		}
	};

	// ファイルを管理する構造体
	struct FileManage_t{
		uint32_t dummy;
		
		// 初期化する
		void init(void){
			
		}
	};

	// ファイルを開くオプション
	enum OPENOPTION_e{
		// 読み込みモードは常に有効となっている
		O_WRITE = 0x1,				// 書き込みモードで開く
		O_CREATE = 0x2,				// ファイルが存在しない場合、新規作成する
		// O_CREATEオプションを指定しないと、ファイルが存在しない場合はエラーとなる
		O_TRUNCATE = 0x4			// 開いたファイルのサイズを0にする
	};


	



}



#endif // MINIFS_COMMON_H
