#include "basicfs.h"
#include "minifs_cache.h"



namespace mfs{
	// すべてのハンドルを連結リストから削除する
	void IBasicFs::RemoveAllHandles(void){
		// 開いているディレクトリをすべて閉じる
		DirHandle *volatile &pheaddir = (DirHandle*&)m_pDirListHead;
		while (pheaddir != nullptr){
			pheaddir->close();
		}

		// 開いているファイルをすべて閉じる
		FileHandle *volatile &pheadfile = (FileHandle*&)m_pFileListHead;
		while (pheadfile != nullptr){
			pheadfile->close();
		}
	}

	// ハンドルを連結リストに追加する
	void IBasicFs::AddHandle(IMiniFSHandle *&phead, IMiniFSHandle &handle){
		if (phead != nullptr){
			phead->m_pPreviousHandle = &handle;
		}
		handle.m_pPreviousHandle = nullptr;
		handle.m_pNextHandle = phead;
		phead = &handle;
	}

	// ハンドルを連結リストから削除する
	void IBasicFs::RemoveHandle(IMiniFSHandle *&phead, IMiniFSHandle &handle){
		IMiniFSHandle *pprev = handle.m_pPreviousHandle;
		IMiniFSHandle *pnext = handle.m_pNextHandle;
		if (pnext != nullptr){
			pnext->m_pPreviousHandle = pprev;
			handle.m_pNextHandle = nullptr;
		}
		if (pprev != nullptr){
			pprev->m_pNextHandle = pnext;
			handle.m_pPreviousHandle = nullptr;
		}
		else{
			phead = pnext;
		}
	}

	// 指定されたファイル・ディレクトリがすでに開かれているか調べる
	bool IBasicFs::IsAlreadyOpened(IMiniFSHandle *phead, DirEntry_t &direntry){
		IMiniFSHandle *p = phead;
		while (p != nullptr){
			if (p->m_Chain.start_cluster == direntry.start_cluster){
				return true;
			}
			p = p->m_pNextHandle;
		}
		return false;
	}
	






}
