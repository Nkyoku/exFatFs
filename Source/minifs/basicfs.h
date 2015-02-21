#pragma once
#ifndef BASICFS_H
#define BASICFS_H

#include "minifs_diskio.h"
#include "minifs_filesystem.h"
#include "minifs_dirhandle.h"
#include "minifs_filehandle.h"



namespace mfs{
	// ファイルシステムを提供する基本クラス
	class IBasicFs : public IMiniFSFileSystem, public fsmutex_t{
	protected:
		// ディスクI/Oクラスへのポインタ
		IMiniFSDiskIO *m_pDiskIO;

	private:
		// マウントされている
		bool m_Mounted;
		
		// 書き込みをサポートする
		bool m_Writable;

		// 開いているディレクトリの連結リストの最初の要素
		// m_pDirListHead, m_pFileListHeadと分けることで、どちらもIMiniFSHandleだが
		// unmount()の際にcloseDir()、closeFile()を分けて呼び出せ、これらを仮想関数化せずに済む
		IMiniFSHandle *m_pDirListHead;

		// 開いているファイルの連結リストの最初の要素
		IMiniFSHandle *m_pFileListHead;

	public:
		// コンストラクタ
		IBasicFs(void) : IMiniFSFileSystem(), m_pDiskIO(nullptr), m_Mounted(false){}

	protected:
		// ファイルシステムのマウント処理を開始する
		void InitializeMounting(PartitionInfo_t &info){
			bool writable = !READ_ONLY && (~info.pdiskio->disk_status() & STA_WRITE_PROTECTED);
			StoreImmediate(m_pDiskIO, info.pdiskio);
			StoreImmediate(m_pDirListHead, nullptr);
			StoreImmediate(m_pFileListHead, nullptr);
			StoreImmediate(m_Writable, writable);
			createMutex();
		}

		// ファイルシステムのマウント処理を終了する
		void FinalizeMounting(void){
			StoreImmediate(m_Mounted, true);
		}

		// ファイルシステムのアンマウント処理を開始する
		void InitializeUnmounting(void){
			StoreImmediate(m_Mounted, false);
		}

		// ファイルシステムのアンマウント処理を終了する
		void FinalizeUnmounting(void){
			StoreImmediate(m_pDiskIO, nullptr);
			destroyMutex();
		}

		// ファイルシステムがマウントされたか
		bool Mounted(void){
			return LoadImmediate(m_Mounted);
		}

		// ファイルシステムが書き込み操作を行えるか
		bool Writable(void){
			return !READ_ONLY && LoadImmediate(m_Writable);
		}

		// 書き込みを禁止する
		void ProhibitWriting(void){
			StoreImmediate(m_Writable, false);
		}

	public:
		// パーティションをアンマウントする
		virtual void unmount(void) = 0;



	protected:
		// ハンドルがファイルシステムに属するか調べる
		bool Assigned(IMiniFSHandle &handle){
			return handle.m_pFileSystem == this;
		}

		// すべてのハンドルを連結リストから削除する
		void RemoveAllHandles(void);

		// ディレクトリハンドルを連結リストに追加する
		void AddHandle(DirHandle &dirhandle){
			AddHandle(m_pDirListHead, dirhandle);
		}

		// ファイルハンドルを連結リストに追加する
		void AddHandle(FileHandle &filehandle){
			AddHandle(m_pFileListHead, filehandle);
		}

		// ディレクトリハンドルを連結リストから削除する
		void RemoveHandle(DirHandle &dirhandle){
			RemoveHandle(m_pDirListHead, dirhandle);
		}

		// ファイルハンドルを連結リストから削除する
		void RemoveHandle(FileHandle &filehandle){
			RemoveHandle(m_pFileListHead, filehandle);
		}

		// 指定されたファイル・ディレクトリがすでに開かれているか調べる
		bool IsAlreadyOpened(DirEntry_t &direntry){
			return IsAlreadyOpened((direntry.flags & ATTR_DIRECTORY) ? m_pDirListHead : m_pFileListHead, direntry);
		}

	private:
		// ハンドルを連結リストに追加する
		static void AddHandle(IMiniFSHandle *&phead, IMiniFSHandle &handle);

		// ハンドルを連結リストから削除する
		static void RemoveHandle(IMiniFSHandle *&phead, IMiniFSHandle &handle);

		// 指定されたファイル・ディレクトリがすでに開かれているか調べる
		static bool IsAlreadyOpened(IMiniFSHandle *phead, DirEntry_t &direntry);

	protected:
		// ファイル・ディレクトリハンドルのクラスタチェインを取得する
		static Chain_t& GetChain(IMiniFSHandle &handle);
	};



	// ファイル・ディレクトリハンドルのクラスタチェインを取得する
	inline Chain_t& IBasicFs::GetChain(IMiniFSHandle &handle){
		return handle.m_Chain;
	}
}



#endif // MINIFS_FILESYSTEM_H
