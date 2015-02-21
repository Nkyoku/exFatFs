#pragma once
#ifndef EXFATFS_COMMON_H
#define EXFATFS_COMMON_H



namespace mfs{
	// VBRのセクター数
	static const uint32_t VBR_LENGTH = 12;

	// 無効なクラスタ番号
	static const uint32_t INVALID_CLUSTER = 0UL;
	
	// 未使用のクラスタ番号
	static const uint32_t UNUSED_CLUSTER = 0xFFFFFFFFUL;

	// 存在できる最小のクラスタ番号
	static const uint32_t MINIMUM_VALID_CLUSTER = 2UL;
	
	// 存在できる最大のクラスタ番号
	static const uint32_t MAXIMUM_VALID_CLUSTER = 0xFFFFFFF6UL;

	// 存在できる最大のクラスタ総数
	static const uint32_t MAXIMUM_CLUSTER_COUNT = MAXIMUM_VALID_CLUSTER - MINIMUM_VALID_CLUSTER + 1;

	// 未使用のクラスタ総数
	static const uint32_t UNUSED_CLUSTER_COUNT = 0xFFFFFFFFUL;

	// ディレクトリエントリの長さ
	static const uint32_t DIR_ENTRY_LENGTH = 32;

	// クラスタ割り当てビットマップを読み取る単位
	static const uint32_t ALLOC_BITMAP_UNIT = 32;

	// 大文字変換テーブルの占めるサイズ
	static const uint32_t UPPER_CASE_TABLE_SIZE = 8192;

	// ファイルアトリビュートの検索条件
	enum COND_ATTRIBUTES_e{
		ATTR_IS_NOT_READONLY = (ATTR_READONLY << 16),
		ATTR_IS_NOT_HIDDEN = (ATTR_HIDDEN << 16),
		ATTR_IS_NOT_SYSTEM = (ATTR_SYSTEM << 16),
		ATTR_IS_FILE = (ATTR_DIRECTORY << 16),
		ATTR_IS_NOT_ARCHIVE = (ATTR_ARCHIVE << 16),
		ATTR_IS_READONLY = (ATTR_READONLY << 16) | ATTR_READONLY,
		ATTR_IS_HIDDEN = (ATTR_HIDDEN << 16) | ATTR_HIDDEN,
		ATTR_IS_SYSTEM = (ATTR_SYSTEM << 16) | ATTR_SYSTEM,
		ATTR_IS_DIRECTORY = (ATTR_DIRECTORY << 16) | ATTR_DIRECTORY,
		ATTR_IS_ARCHIVE = (ATTR_ARCHIVE << 16) | ATTR_ARCHIVE
	};

	// ファイル・ディレクトリを探す際の検索条件
	struct CONDITION_t{
		Property_t property;				// 検索結果のファイル・ディレクトリのプロパティ
		DirEntry_t direntry;				// 検索結果のファイル・ディレクトリの格納場所
		uint32_t attributes;				// 検索するアトリビュートのマスク(上位16ビット)と値(下位16ビット)
		uint16_t *output_name_string;		// 発見したファイル・ディレクトリ名の出力先(nullptrならファイル名は出力されない)
		const uint16_t *target_name_string;	// 対象のファイル・ディレクトリ名(nullptrならファイル名は比較しない)
		uint16_t target_name_checksum;		// 対象のファイル・ディレクトリ名のチェックサム
	};

	// DirEntry_tのフラグ
	enum FLAG_e{
		FLAG_READONLY = ATTR_READONLY,		// 読み込み専用
		FLAG_HIDDEN = ATTR_HIDDEN,			// 隠しファイル・ディレクトリ
		FLAG_SYSTEM = ATTR_SYSTEM,			// システムのファイル・ディレクトリ
		FLAG_DIRECTORY = ATTR_DIRECTORY,	// ディレクトリ
		FLAG_ARCHIVE = ATTR_ARCHIVE,		// アーカイブ
		FLAG_WRITABLE = 0x10000,			// 書き込みモードで開かれている
		FLAG_CONTIGUOUS = 0x20000,			// フラグメンテーションが起きていない
	};

	// クラスタ番号からセクター番号を計算する
	static inline uint32_t ClusterToSector(uint32_t offset, uint32_t cluster, uint32_t sectors_per_cluster_shift){
		return offset + ((cluster - MINIMUM_VALID_CLUSTER) << sectors_per_cluster_shift);
	}

	// チェックサムを計算する(16ビット)
	static inline uint16_t CalcChecksum16(uint16_t checksum, uint8_t data){
		return RRotate16(checksum) + (uint16_t)data;
	}

	// チェックサムを計算する(16ビット)
	static inline uint16_t CalcChecksum16(uint16_t checksum, const uint8_t *data, uint32_t length){
		const uint8_t *p = data;
		const uint8_t *end = data + length;
		while (p != end){
			checksum = CalcChecksum16(checksum, *p++);
		}
		return checksum;
	}

	// チェックサムを計算する(32ビット)
	static inline uint32_t CalcChecksum32(uint32_t checksum, uint8_t data){
		return RRotate32(checksum) + (uint32_t)data;
	}

	// チェックサムを計算する(32ビット)
	static inline uint32_t CalcChecksum32(uint32_t checksum, const uint8_t *data, uint32_t length){
		const uint8_t *p = data;
		const uint8_t *end = data + length;
		while (p != end){
			checksum = CalcChecksum32(checksum, *p++);
		}
		return checksum;
	}

	// 有効なクラスタ番号か調べる
	static inline bool IsValidCluster(uint32_t cluster){
		if (cluster < MINIMUM_VALID_CLUSTER) return false;
		else if (MAXIMUM_VALID_CLUSTER < cluster) return false;
		else return true;
	}
	
	
	
}



#endif // EXFATFS_COMMON_H
