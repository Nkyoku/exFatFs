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

		// クラスタチェイン
		Chain_t m_Chain;

	protected:
		// コンストラクタ
		IMiniFSHandle(void) : m_pFileSystem(&NullFs), m_pPreviousHandle(nullptr), m_pNextHandle(nullptr), m_Chain(){}

	public:
		// 最後に発生した致命的なエラーコードを取得する
		RESULT_e lastError(void){
			return m_Chain.last_error;
		}

		// 致命的なエラーが発生していないか取得する
		bool noError(void){
			return (m_Chain.last_error == RES_SUCCEEDED);
		}
	};
}



#endif // MINIFS_HANDLE_H
