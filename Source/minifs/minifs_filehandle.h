#pragma once
#ifndef MINIFS_FILEHANDLE_H
#define MINIFS_FILEHANDLE_H

#include "minifs_handle.h"
#include "minifs_cache.h"



namespace mfs{
	// ファイルハンドルを提供するクラス
	class FileHandle : public IMiniFSHandle{
		friend class IBasicFs;

	private:
		// キャッシュ
		Cache m_Cache;

		// ファイル管理情報
		FileManage_t m_FileManage;

	public:
		// コンストラクタ
		FileHandle(void) : IMiniFSHandle(), m_Cache(), m_FileManage(){}

		// デストラクタ
		~FileHandle(){
			close();
		}

		// ファイルを開く
		RESULT_e open(IMiniFSFileSystem &filesystem, const fschar_t *path, uint32_t option = 0){
			// すでにディレクトリを開いているなら閉じる
			RESULT_e result;
			result = close();
			if (result == RES_SUCCEEDED){
				// メンバー変数を初期化してディレクトリを開く
				result = filesystem.OpenFile(*this, path, option);
				if (result == RES_SUCCEEDED){
					m_pFileSystem = &filesystem;
					m_Manage.pcache = &m_Cache;
				}
			}
			return result;
		}

		// ファイルを閉じる
		RESULT_e close(void){
			RESULT_e result;
			flush();
			result = m_pFileSystem->CloseFile(*this);
			if (result == RES_SUCCEEDED){
				m_pFileSystem = &NullFs;
			}
			return result;
		}

		// ファイルポインタをシークする
		RESULT_e seek(uint64_t offset){
			return m_pFileSystem->SeekFile(*this, offset);
		}

		// ファイルを読み出す
		uint32_t read(void *buf, uint32_t length){
			return m_pFileSystem->ReadFile(*this, buf, length);
		}

		// ファイルへ書き込む
		uint32_t write(const void *buf, uint32_t length){
			return m_pFileSystem->WriteFile(*this, buf, length);
		}

		// ファイルをフラッシュする
		RESULT_e flush(void){
			return m_pFileSystem->FlushFile(*this);
		}

		// ファイルを切り詰める
		RESULT_e truncate(void){
			return m_pFileSystem->TruncateFile(*this);
		}

		// ファイルサイズを取得する
		uint64_t size(void){
			return m_Manage.size;
		}

		// ファイルポインタの位置を取得する
		uint64_t tell(void){
			return m_Manage.pointer;
		}





	};
}



#endif // MINIFS_FILEHANDLE_H
