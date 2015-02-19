#pragma once
#ifndef MINIFS_HANDLE_H
#define MINIFS_HANDLE_H

#include "minifs_common.h"
#include "minifs_filesystem.h"



namespace mfs{
	// ハンドルを提供するインターフェースクラス
	class IMiniFSHandle{
		friend class IBasicFs;

	private:
		// 代入の禁止
		void operator =(const IMiniFSHandle &src){}

		// コピーコンストラクタの禁止
		IMiniFSHandle(const IMiniFSHandle &src){}

	protected:
		// ファイルが存在するファイルシステムへのポインタ
		IMiniFSFileSystem *m_pFileSystem;

		// 連結リストの前のハンドルのポインタ
		IMiniFSHandle *m_pPreviousHandle;

		// 連結リストの次のハンドルのポインタ
		IMiniFSHandle *m_pNextHandle;

		// 管理情報
		Manage_t m_Manage;

	protected:
		// コンストラクタ
		IMiniFSHandle(void) : m_pFileSystem(&NullFs), m_pPreviousHandle(nullptr), m_pNextHandle(nullptr), m_Manage(){}

		// 初期化する
		void init(void){
			m_pFileSystem = &NullFs;
		}
	};
}



#endif // MINIFS_HANDLE_H
