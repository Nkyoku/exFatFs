#pragma once
#ifndef MINIFS_FILESYSTEM_H
#define MINIFS_FILESYSTEM_H

#include "minifs_common.h"



namespace mfs{
	// ディレクトリI/Oを提供するクラス
	class DirHandle;

	// ファイルI/Oを提供するクラス
	class FileHandle;



	// ファイルシステムを提供するインターフェースクラス
	class IMiniFSFileSystem{
		friend class DirHandle;
		friend class FileHandle;

	public:
		// デストラクタ
		~IMiniFSFileSystem(){ unmount(); }

	//protected:
		// パーティションをマウントする
		//virtual RESULT_e mount(PartitionInfo_t &info){ return RES_NOT_MOUNTED; }

	public:
		// パーティションをアンマウントする
		virtual void unmount(void){}

		// ボリュームの容量を取得する(バイト単位)
		virtual uint64_t capacity(void){ return 0; }

		// ボリュームの空き領域を取得する(バイト単位)
		virtual uint64_t freeSpace(void){ return 0; }

	public:
		// ファイル・ディレクトリを削除する
		virtual RESULT_e remove(const fschar_t *path){ return RES_NOT_MOUNTED; }

		// ファイル・ディレクトリを移動する
		virtual RESULT_e move(const fschar_t *src, const fschar_t *dst){ return RES_NOT_MOUNTED; }

		// ディレクトリを作成する
		virtual RESULT_e makeDir(const fschar_t *path){ return RES_NOT_MOUNTED; }

	public:
		// ディレクトリを開く
		DirHandle* openDir(const fschar_t *path);

		// ファイルを開く
		FileHandle* openFile(const fschar_t *path, uint32_t option = 0);

	protected:
		// ディレクトリを開く
		virtual RESULT_e OpenDir(DirHandle &dirhandle, const fschar_t *path){ return RES_NOT_MOUNTED; }

		// ディレクトリを閉じる
		virtual RESULT_e CloseDir(DirHandle &dirhandle){ return RES_SUCCEEDED; }

		// ディレクトリを列挙する
		virtual RESULT_e ReadDir(DirHandle &dirhandle, DirInfo_t *info){ return RES_NOT_MOUNTED; }

	protected:
		// ファイルを開く
		virtual RESULT_e OpenFile(FileHandle &filehandle, const fschar_t *path, uint32_t option){ return RES_NOT_MOUNTED; }

		// ファイルを閉じる
		virtual RESULT_e CloseFile(FileHandle &filehandle){ return RES_SUCCEEDED; }

		// ファイルポインタをシークする
		virtual RESULT_e SeekFile(FileHandle &filehandle, uint64_t offset){ return RES_NOT_MOUNTED; }

		// ファイルを読み出す
		virtual uint32_t ReadFile(FileHandle &filehandle, void *buf, uint32_t length){ return 0; }

		// ファイルへ書き込む
		virtual uint32_t WriteFile(FileHandle &filehandle, const void *buf, uint32_t length){ return 0; }

		// ファイルをフラッシュする
		virtual RESULT_e FlushFile(FileHandle &filehandle){ return RES_SUCCEEDED; }

		// ファイルを切り詰める
		virtual RESULT_e TruncateFile(FileHandle &filehandle){ return RES_NOT_MOUNTED; }
	};



	extern IMiniFSFileSystem NullFs;
}



#endif // MINIFS_FILESYSTEM_H
