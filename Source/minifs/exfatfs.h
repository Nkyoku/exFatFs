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

		// クラスタ割り当てビットマップのクラスタチェイン
		Chain_t m_AllocBitmapChain;

		// クラスタ割り当てビットマップのキャッシュ
		Cache m_AllocBitmapCache;

		// FATとディレクトリエントリのキャッシュ
		Cache m_FATAndDirCache;

		// 最小の空きクラスタ番号
		uint32_t m_FirstFreeCluster;

		// 最小の空きクラスタ番号から連続して空いているクラスタ数
		uint32_t m_ContiguousFreeClusterCount;


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
		virtual RESULT_e ReadDir(DirHandle &dirhandle, FileInfo_t *pinfo) override;

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
		// フラグメントを把握する
		RESULT_e LoadFragment(Chain_t &chain, uint32_t start_cluster);
		
		// クラスタへアクセスする前にChain_tを初期化する
		RESULT_e InitChain(Chain_t &chain);

		// クラスタチェインを開く
		RESULT_e OpenChain(Chain_t &chain, uint32_t attributes, const fschar_t *path);

		// クラスタチェインをシークする
		RESULT_e SeekChain(Chain_t &chain, uint64_t offset);

		// クラスタチェインをキャッシュする
		RESULT_e CacheChain(Chain_t &chain);

		// クラスタチェインを読み出す
		RESULT_e ReadChain(Chain_t &chain, void *buf, uint32_t length);

		// クラスタチェインへ書き込む
		RESULT_e WriteChain(Chain_t &chain, const void *buf, uint32_t length);

		// クラスタチェインを伸ばす
		RESULT_e ExtendChain(Chain_t &chain, uint32_t cluster_count);

		// クラスタチェインを削除する
		// start_clusterから次のクラスタを削除する
		RESULT_e DeleteChain(Chain_t &chain, uint32_t start_cluster, bool contain_start);

		// クラスタチェインを確保する
		// start_clusterからcluster_countだけクラスタを確保する
		// 不足分は拡張され、余剰分は削除される
		RESULT_e AllocateChain(Chain_t &chain, uint32_t start_cluster, uint32_t cluster_count, uint32_t &allocated_count);



	protected:
		// FATを読み込む
		RESULT_e ReadFAT(uint32_t previous_cluster, uint32_t &next_cluster);

		// FATに書き込む
		RESULT_e WriteFAT(uint32_t previous_cluster, uint32_t next_cluster);

		// FATを連続で読み込む
		RESULT_e ReadContinuousFAT(uint32_t start_cluster, uint32_t &tail_cluster, uint32_t &next_cluster);

		// FATに連続で書き込む
		RESULT_e WriteContinuousFAT(uint32_t previous_cluster, uint32_t start_cluster, uint32_t cluster_count);



	protected:
		// 特殊なディレクトリエントリを検索する
		RESULT_e FindSpecialDirEntry(void);

		// 条件に一致するディレクトリを検索する
		RESULT_e FindDir(Chain_t &chain, CONDITION_t &conditions, bool detail);

		// 空きクラスタを検索する
		RESULT_e FindFreeClusters(uint32_t start_cluster, uint32_t &found_cluster, uint32_t &contiguous_clusters);

		// クラスタ割り当てビットマップの値を変更する
		RESULT_e FillAllocationBitmap(uint32_t start_cluster, uint32_t cluster_count, bool value);



	protected:
		// ディレクトリエントリを作成する
		RESULT_e Link(DirHandle &dirhandle, DirEntry_t &direntry, FileInfo_t *info);

		// ディレクトリエントリを削除する
		RESULT_e Unlink(Chain_t &dirchain, uint32_t target_offset);




	};
}



#endif // EXFATFS_H
