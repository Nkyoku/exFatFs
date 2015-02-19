#pragma once
#ifndef MINIFS_CACHE_H
#define MINIFS_CACHE_H

#include "minifs_common.h"
#include "minifs_diskio.h"
#include <string.h>



namespace mfs{
	static const uint32_t INVALID_SECTOR = 0;	// 使用されないセクター番号
	
	// キャッシュをフラッシュする
	static inline RESULT_e FlushCache(IMiniFSDiskIO *pdiskio, Manage_t &manage){
		if (manage.cache_modified == true){
			manage.cache_modified = false;
			// キャッシュが書き換えられているため、ディスクに書き戻す
			return pdiskio->disk_write(manage.cache, manage.cached_sector, 1);;
		}
		else{
			return RES_SUCCEEDED;
		}
	}

	// セクターをキャッシュに読み込む
	static inline RESULT_e ReadToCache(IMiniFSDiskIO *pdiskio, Manage_t &manage, uint32_t sector){
		if (manage.cached_sector != sector){
			RESULT_e result;
			result = FlushCache(pdiskio, manage);
			if (result == RES_SUCCEEDED){
				manage.cached_sector = sector;
				result = pdiskio->disk_read(manage.cache, manage.cached_sector, 1);
			}
			if (result != RES_SUCCEEDED){
				manage.cached_sector = INVALID_SECTOR;
				return result;
			}
		}
		return RES_SUCCEEDED;
	}

	// セクターをキャッシュから読み込む
	static inline RESULT_e ReadFromCache(IMiniFSDiskIO *pdiskio, Manage_t &manage, uint32_t sector, void *buf, uint32_t offset, uint32_t length){
		memcpy(buf, manage.cache + offset, length);
		return RES_SUCCEEDED;
	}
	
	// セクターをキャッシュに読み込んでバッファにコピー
	static inline RESULT_e ReadWithCache(IMiniFSDiskIO *pdiskio, Manage_t &manage, uint32_t sector, void *buf, uint32_t offset, uint32_t length){
		RESULT_e result;
		result = ReadToCache(pdiskio, manage, sector);
		if (result == RES_SUCCEEDED){
			result = ReadFromCache(pdiskio, manage, sector, buf, offset, length);
		}
		return result;
	}

	// キャッシュへ書き込む
	static inline RESULT_e WriteToCache(IMiniFSDiskIO *pdiskio, Manage_t &manage, uint32_t sector, void *buf, uint32_t offset, uint32_t length){
		RESULT_e result;
		result = ReadToCache(pdiskio, manage, sector);
		if (result != RES_SUCCEEDED){
			return result;
		}
		memcpy(manage.cache + offset, buf, length);
		manage.cache_modified = true;
		return RES_SUCCEEDED;
	}
}



#endif // MINIFS_CACHE_H
