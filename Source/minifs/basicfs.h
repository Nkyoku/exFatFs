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
			//lockMutex(true);
			AddHandle(m_pDirListHead, dirhandle);
			//unlockMutex();
		}

		// ファイルハンドルを連結リストに追加する
		void AddHandle(FileHandle &filehandle){
			//lockMutex(true);
			AddHandle(m_pFileListHead, filehandle);
			//unlockMutex();
		}

		// ディレクトリハンドルを連結リストから削除する
		void RemoveHandle(DirHandle &dirhandle){
			//lockMutex(true);
			RemoveHandle(m_pDirListHead, dirhandle);
			//unlockMutex();
		}

		// ファイルハンドルを連結リストから削除する
		void RemoveHandle(FileHandle &filehandle){
			//lockMutex(true);
			RemoveHandle(m_pFileListHead, filehandle);
			//unlockMutex();
		}

		// 指定されたファイル・ディレクトリがすでに開かれているか調べる
		bool IsAlreadyOpened(ManageBase_t &manage){
			return IsAlreadyOpened((manage.flags & ATTR_DIRECTORY) ? m_pDirListHead : m_pFileListHead, manage);
		}

		// 連結リストの最初のファイルハンドルを取得する
		/*FileHandle* GetFirstFileHandle(void){
			return (FileHandle*)m_pFileListHead;
		}

		// 連結リストの次のファイルハンドルを取得する
		FileHandle* GetNextFileHandle(FileHandle *phandle){
			return (FileHandle*)phandle->m_pNextHandle;
		}

		// 連結リストの最初のディレクトリハンドルを取得する
		DirHandle* GetFirstDirHandle(void){
			return (DirHandle*)m_pDirListHead;
		}

		// 連結リストの次のディレクトリハンドルを取得する
		DirHandle* GetNextDirHandle(DirHandle *phandle){
			return (DirHandle*)phandle->m_pNextHandle;
		}*/

	private:
		// ハンドルを連結リストに追加する
		static void AddHandle(IMiniFSHandle *&phead, IMiniFSHandle &handle);

		// ハンドルを連結リストから削除する
		static void RemoveHandle(IMiniFSHandle *&phead, IMiniFSHandle &handle);

		// 指定されたファイル・ディレクトリがすでに開かれているか調べる
		static bool IsAlreadyOpened(IMiniFSHandle *phead, ManageBase_t &manage);

	protected:
		// ディレクトリ・ファイルハンドルの管理情報を取得する
		static Manage_t& GetManage(IMiniFSHandle &handle);

		// ディレクトリハンドルの管理情報を取得する
		static DirManage_t* GetDirManage(DirHandle &handle);

		// ファイルハンドルの管理情報を取得する
		static FileManage_t* GetFileManage(FileHandle &handle);
	};



	// ディレクトリ・ファイルハンドルの管理情報を取得する
	inline Manage_t& IBasicFs::GetManage(IMiniFSHandle &handle){
		return handle.m_Manage;
	}

	// ディレクトリハンドルのファイル管理情報を取得する
	inline DirManage_t* IBasicFs::GetDirManage(DirHandle &handle){
		return &handle.m_DirManage;
	}

	// ファイルハンドルのファイル管理情報を取得する
	inline FileManage_t* IBasicFs::GetFileManage(FileHandle &handle){
		return &handle.m_FileManage;
	}
}



#endif // MINIFS_FILESYSTEM_H
