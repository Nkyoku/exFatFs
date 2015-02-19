#pragma once
#ifndef MINIFS_FILEHANDLE_H
#define MINIFS_FILEHANDLE_H

#include "minifs_handle.h"



namespace mfs{
	// ファイルハンドルを提供するクラス
	class FileHandle : public IMiniFSHandle{
		friend class IBasicFs;

	private:
		// ファイル管理情報
		FileManage_t m_FileManage;

	public:
		// コンストラクタ
		FileHandle(void) : IMiniFSHandle(), m_FileManage(){}

		// デストラクタ
		~FileHandle(){
			close();
		}

		// ファイルを開く
		RESULT_e open(IMiniFSFileSystem &filesystem, const fschar_t *path, uint32_t option = 0){
			RESULT_e result;
			close();
			m_Manage.init();
			m_FileManage.init();
			result = filesystem.OpenFile(*this, path, option);
			if (result == RES_SUCCEEDED){
				m_pFileSystem = &filesystem;
			}
			return result;
		}

		// ファイルを閉じる
		RESULT_e close(void){
			RESULT_e result;
			flush();
			result = m_pFileSystem->CloseFile(*this);
			if (result == RES_SUCCEEDED){
				init();
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

		// 書き込み可能か取得する
		bool writable(void){
			return (m_Manage.attributes & ATTR_WRITABLE) ? true : false;
		}




	};
}



#endif // MINIFS_FILEHANDLE_H
