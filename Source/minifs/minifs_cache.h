#pragma once
#ifndef MINIFS_CACHE_H
#define MINIFS_CACHE_H

#include "minifs_common.h"
#include <string.h>



namespace mfs{
	// ディスクI/Oを提供するインターフェースクラス
	class IMiniFSDiskIO;



	// キャッシュ機構を提供するクラス
	class Cache{
	private:
		// キャッシュが書き換えられたことを示すフラグ
		bool m_Modified;
		
		// キャッシュしているセクター番号
		uint32_t m_Sector;

		// バッファ
		uint8_t m_Buffer[MAX_SECTOR_SIZE];

	public:
		// コンストラクタ
		Cache() : m_Modified(false), m_Sector(INVALID_SECTOR){}

		// キャッシュしたデータを無効化する
		void Kill(void){
			m_Sector = INVALID_SECTOR;
		}

		// キャッシュをフラッシュする
		RESULT_e Flush(IMiniFSDiskIO *pdiskio);

		// セクターをキャッシュに読み込む
		RESULT_e ReadTo(IMiniFSDiskIO *pdiskio, uint32_t sector);

		// セクターをキャッシュから読み込む
		RESULT_e ReadFrom(IMiniFSDiskIO *pdiskio, uint32_t sector, void *buf, uint32_t offset, uint32_t length){
			memcpy(buf, m_Buffer + offset, length);
			return RES_SUCCEEDED;
		}

		// セクターをキャッシュに読み込んでバッファにコピー
		RESULT_e ReadWith(IMiniFSDiskIO *pdiskio, uint32_t sector, void *buf, uint32_t offset, uint32_t length);

		// キャッシュへ書き込む
		RESULT_e WriteTo(IMiniFSDiskIO *pdiskio, uint32_t sector, const void *buf, uint32_t offset, uint32_t length);

		// キャッシュを書き換えたことにする
		void SetModified(bool modified){
			m_Modified = modified;
		}

		// バッファを取得する
		void* GetBuffer(uint32_t offset){
			return m_Buffer + offset;
		}
	};
}



#endif // MINIFS_CACHE_H
