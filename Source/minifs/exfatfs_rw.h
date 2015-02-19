#pragma once
#ifndef EXFATFS_RW_H
#define EXFATFS_RW_H

#include "exfatfs_ro.h"



namespace mfs{
	// exFATにアクセスするファイルシステムクラス(リードライト)
	class ExFatFsRW : public ExFatFsRO{
	private:
		


	public:
		// パーティションをフォーマットする
		static RESULT_e format(PartitionInfo_t &info, uint32_t sectors_per_cluster_shift, uint8_t &id);

	public:
		// パーティションをマウントする
		RESULT_e mount(PartitionInfo_t &info);

		// パーティションをアンマウントする
		virtual void unmount(void) override;

	public:
		// ファイル・ディレクトリを削除する
		virtual RESULT_e remove(const fschar_t *path) override;

		// ファイル・ディレクトリを移動する
		virtual RESULT_e move(const fschar_t *src, const fschar_t *dst) override;

		// ディレクトリを作成する
		virtual RESULT_e makeDir(const fschar_t *path) override;

	protected:
		// ファイルを開く
		virtual RESULT_e OpenFile(FileHandle &filehandle, const fschar_t *path, uint32_t option = 0) override;
		
		// ファイルポインタをシークする
		virtual RESULT_e SeekFile(FileHandle &filehandle, uint64_t offset) override;

		// ファイルへ書き込む
		virtual uint32_t WriteFile(FileHandle &filehandle, const void *buf, uint32_t length) override;

		// ファイルをフラッシュする
		virtual RESULT_e FlushFile(FileHandle &filehandle) override;

		// ファイルを切り詰める
		virtual RESULT_e TruncateFile(FileHandle &filehandle) override;



	private:
		// クラスタへ書き込む
		RESULT_e WriteCluster(Manage_t &manage, const void *buf, uint32_t length);




	};
}



#endif // EXFATFS_RW_H
