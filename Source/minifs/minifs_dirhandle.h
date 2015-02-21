#pragma once
#ifndef MINIFS_DIRHANDLE_H
#define MINIFS_DIRHANDLE_H

#include "minifs_handle.h"



namespace mfs{
	// ディレクトリハンドルを提供するクラス
	class DirHandle : public IMiniFSHandle{
		friend class IBasicFs;

	private:
		// ディレクトリ管理情報
		//DirManage_t m_DirManage;

	public:
		// コンストラクタ
		DirHandle(void) : IMiniFSHandle()/*, m_DirManage()*/{}

		// デストラクタ
		~DirHandle(){
			close();
		}

		// ディレクトリを開く
		RESULT_e open(IMiniFSFileSystem &filesystem, const fschar_t *path){
			// すでにディレクトリを開いているなら閉じる
			RESULT_e result;
			result = close();
			if (result == RES_SUCCEEDED){
				// メンバー変数を初期化してディレクトリを開く
				result = filesystem.OpenDir(*this, path);
				if (result == RES_SUCCEEDED){
					m_pFileSystem = &filesystem;
				}
			}
			return result;
		}

		// ディレクトリを閉じる
		RESULT_e close(void){
			RESULT_e result;
			result = m_pFileSystem->CloseDir(*this);
			if (result == RES_SUCCEEDED){
				m_pFileSystem = &NullFs;
			}
			return result;
		}

		// ディレクトリを列挙する
		RESULT_e read(FileInfo_t *info){
			return m_pFileSystem->ReadDir(*this, info);
		}

		// ポインタを先頭に戻す
		RESULT_e rewind(void){
			return m_pFileSystem->ReadDir(*this, nullptr);
		}



	};
}



#endif // MINIFS_DIRHANDLE_H
