#pragma once
#ifndef EXFATFS_H
#define EXFATFS_H

#include "basicfs.h"



namespace mfs{
	// ファイルを探す際の検索条件
	struct CONDITION_t;



	// exFATにアクセスするファイルシステムクラス
	class ExFatFs : public IBasicFs{
	protected:
		// セクターあたりのバイト数
		uint32_t m_BytesPerSectorShift;

		// クラスターあたりのセクター数
		uint32_t m_SectorsPerClusterShift;

		// FATのオフセット(ディスク先頭を0とする絶対セクター番号)
		uint32_t m_FATOffset;

		// クラスタのオフセット(ディスク先頭を0とする絶対セクター番号)
		uint32_t m_ClusterOffset;

		// クラスタ総数
		uint32_t m_ClusterCount;

		// 空きクラスタ総数
		uint32_t m_FreeClusterCount;

		// ルートディレクトリのクラスタ番号
		uint32_t m_RootDirEntryCluster;

		// クラスタ割り当てビットマップのあるクラスタ番号
		uint32_t m_AllocBitmapCluster;

		// クラスタ割り当てビットマップへアクセスするためのManage_t
		Manage_t m_AllocBitmapManage;

		// 空きクラスタの最小のクラスタ番号
		uint32_t m_FirstFreeCluster;



	public:
		// ファイルシステムがexFATか調べる
		static bool isExFAT(PartitionInfo_t &info);

		// パーティションをexFATでフォーマットする
		static RESULT_e format(PartitionInfo_t &info, uint32_t sectors_per_cluster_shift, uint8_t &id);

	public:
		// パーティションをマウントする
		RESULT_e mount(PartitionInfo_t &info);

		// パーティションをアンマウントする
		virtual void unmount(void) override;

		// ボリュームの容量を取得する(バイト単位)
		virtual uint64_t capacity(void) override;

		// ボリュームの空き領域を取得する(バイト単位)
		virtual uint64_t freeSpace(void) override;

	public:
		// ファイル・ディレクトリを削除する
		virtual RESULT_e remove(const fschar_t *path) override;

		// ファイル・ディレクトリを移動する
		virtual RESULT_e move(const fschar_t *src, const fschar_t *dst) override;

		// ディレクトリを作成する
		virtual RESULT_e makeDir(const fschar_t *path) override;

	protected:
		// ディレクトリを開く
		virtual RESULT_e OpenDir(DirHandle &dirhandle, const fschar_t *path) override;

		// ディレクトリを閉じる
		virtual RESULT_e CloseDir(DirHandle &dirhandle) override;

		// ディレクトリを列挙する
		virtual RESULT_e ReadDir(DirHandle &dirhandle, DirInfo_t *pinfo) override;

	protected:
		// ファイルを開く
		virtual RESULT_e OpenFile(FileHandle &filehandle, const fschar_t *path, uint32_t option = 0) override;

		// ファイルを閉じる
		virtual RESULT_e CloseFile(FileHandle &filehandle) override;

		// ファイルポインタをシークする
		virtual RESULT_e SeekFile(FileHandle &filehandle, uint64_t offset) override;

		// ファイルを読み出す
		virtual uint32_t ReadFile(FileHandle &filehandle, void *buf, uint32_t length) override;

		// ファイルへ書き込む
		virtual uint32_t WriteFile(FileHandle &filehandle, const void *buf, uint32_t length) override;

		// ファイルをフラッシュする
		virtual RESULT_e FlushFile(FileHandle &filehandle) override;

		// ファイルを切り詰める
		virtual RESULT_e TruncateFile(FileHandle &filehandle) override;


	protected:
		// クラスタへアクセスする前にManage_tを初期化する
		RESULT_e InitManage(Manage_t &manage);

		// フラグメントを把握する
		RESULT_e LoadFragment(Manage_t &manage, uint32_t start_cluster);

		// ポインタをシークする
		RESULT_e SeekCluster(Manage_t &manage, uint64_t offset);

		// クラスタを読み出す
		RESULT_e ReadCluster(Manage_t &manage, void *buf, uint32_t length);

		// クラスタへ書き込む
		RESULT_e WriteCluster(Manage_t &manage, const void *buf, uint32_t length);

	protected:
		// 特殊なディレクトリエントリを検索する
		RESULT_e FindSpecialDirEntry(void);

		// 条件に一致するディレクトリを検索する
		RESULT_e FindDir(Manage_t &manage, CONDITION_t &conditions, bool detail);

		// ディレクトリエントリを検索する
		RESULT_e FindDirEntry(Manage_t &manage, const fschar_t *path, uint32_t attributes);

		// 空きクラスタを検索する
		RESULT_e FindFreeCluster(uint32_t start_cluster, uint32_t &found_cluster);





	};
}



#endif // EXFATFS_H
