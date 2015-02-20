#pragma once
#ifndef MINIFS_COMMON_H
#define MINIFS_COMMON_H

#include <stdint.h>
#include "minifs_builtin.h"


namespace mfs{
	// ディスクI/Oを提供するインターフェースクラス
	class IMiniFSDiskIO;

	// キャッシュ機構を提供するクラス
	class Cache;



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

	// 無効なセクター番号
	static const uint32_t INVALID_SECTOR = 0;



	// ディスク情報
	enum DISKINFO_e{
		INFO_STATUS,
		INFO_BYTES_PER_SECTOR_SHIFT,
		INFO_SECTOR_COUNT,
		INFO_SECTORS_PER_BLOCK_SHIFT,
	};

	// ディスクステータスフラグ
	enum DISKSTATUS_e{
		STA_NOT_INITIALIZED = 0x1,			// ディスクは初期化されていない
		STA_NO_DISK = 0x2,					// ディスクは存在しない
		STA_WRITE_PROTECTED = 0x4,			// 書き込みが禁止されている
	};

	// 戻り値
	enum RESULT_e{
		RES_SUCCEEDED = 0,					// 処理は正常に終了した
		RES_DISK_ERROR,						// ディスクI/Oの操作時にエラーが発生した。
		RES_NOT_READY,						// ディスクの用意ができていない。ディスクの初期化に失敗した。
		RES_WRITE_PROTECTED,				// 書き込み禁止のディスクに書き込もうとした。
		RES_INTERNAL_ERROR,					// 内部エラーが発生した。
		RES_NOT_SUPPORTED,					// 非対応の形式である。
		RES_NOT_MOUNTED,					// ファイルシステムがマウントされていない
		RES_NOT_FOUND,						// パスが存在しない。条件に一致しなかった。
		RES_INVALID_NAME,					// パスのフォーマットが無効である。
		RES_READ_ONLY,						// ATTR_READONLY属性を持つファイル・ディレクトリを変更しようとした。
		RES_NOT_EMPTY,						// 空でないディレクトリを削除しようとした。
		RES_ALREADY_EXIST,					// 作成しようとしたファイル・ディレクトリがすでに存在した。
		RES_INVALID_HANDLE,					// ファイル・ディレクトリハンドルが無効である。
		RES_NO_DATA,						// 読み出せるデータが存在しない
		RES_NO_FILESYSTEM,					// ファイルシステムが存在しない。
		RES_TIMEOUT,						// 操作がタイムアウトした。
		RES_LOCKED,							// 排他制御によってファイルが開けなかった。
		RES_INVALID_PARAMETERS,				// パラメータが無効である。
	};

	// パーティションの種類
	enum PartitionType_e{
		PID_EMPTY = 0,						// 空のパーティション
		PID_FAT,							// FAT12, FAT16, FAT32
		PID_EXFAT,							// exFAT
		PID_EXTENDED,						// (拡張パーティション)
		PID_OTHERS							// 判別できないファイルシステム
	};

	// ファイルアトリビュート(属性)
	enum ATTRIBUTES_e{
		ATTR_READONLY	= 0x0001,			// 読み込み専用
		ATTR_HIDDEN		= 0x0002,			// 隠しファイル・ディレクトリ
		ATTR_SYSTEM		= 0x0004,			// システムのファイル・ディレクトリ
		ATTR_DIRECTORY	= 0x0010,			// ディレクトリ
		ATTR_ARCHIVE	= 0x0020,			// アーカイブ
	};

	// パーティション情報を格納する構造体
	struct PartitionInfo_t{
		IMiniFSDiskIO *pdiskio;				// ディスクI/Oへのポインタ
		uint32_t table_sector;				// パーティションテーブルのあるセクター番号
		uint32_t table_offset;				// パーティションテーブルのオフセット
		bool active_flag;					// アクティブフラグ
		PartitionType_e partition_type;		// パーティションの種類
		uint32_t partition_sector;			// パーティションの開始セクター番号
		uint32_t partition_size;			// パーティションのセクター総数
	};

	// ファイル・ディレクトリのプロパティを格納する構造体
	struct Property_t{
		uint64_t size;						// サイズ
		uint32_t attributes;				// アトリビュート
		uint32_t creation_time;				// 作成日時
		uint32_t modified_time;				// 更新日時
		uint32_t accessed_time;				// アクセス日時
	};

	// ファイル・ディレクトリのプロパティと名前を格納する構造体
	struct FileInfo_t : Property_t{
		fschar_t name[MAXIMUM_PATH + 1];	// ファイル・ディレクトリ名
	};

	// ファイル・ディレクトリの格納場所を格納する構造体
	struct ManageBase_t{
		uint32_t flags;						// フラグ
		uint32_t start_cluster;				// データのある最初のクラスタ番号
		uint64_t size;						// サイズ
		uint32_t entry_flags;				// ディレクトリエントリのあるディレクトリのフラグ
		uint32_t entry_cluster;				// ディレクトリエントリのあるディレクトリの最初のクラスタ番号
		uint32_t entry_offset;				// ディレクトリエントリのあるオフセット
	};

	// ファイル・ディレクトリにアクセスするための構造体
	struct Manage_t : ManageBase_t{
		uint32_t frag_head_cluster;			// 現在アクセス中のフラグメントの最初のクラスタ番号
		uint32_t frag_tail_cluster;			// 現在アクセス中のフラグメントの最後のクラスタ番号
		uint32_t next_frag_cluster;			// 次のフラグメントの最初のクラスタ番号
		uint32_t current_cluster;			// 現在のファイルポインタがあるクラスタ番号
		uint64_t pointer;					// ファイルポインタ
		Cache *pcache;						// キャッシュへのポインタ

		// 初期化する
		void Init(Cache *pcache_){
			frag_head_cluster = 0;
			frag_tail_cluster = 0;
			next_frag_cluster = 0;
			current_cluster = 0;
			pointer = 0;
			pcache = pcache_;
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
