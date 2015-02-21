#include "minifs_cache.h"
#include "minifs_diskio.h"



namespace mfs{
	// キャッシュをフラッシュする
	RESULT_e Cache::Flush(IMiniFSDiskIO *pdiskio){
		if (m_Modified == true){
			m_Modified = false;
			if (m_Sector != INVALID_SECTOR){
				// キャッシュが書き換えられているため、ディスクに書き戻す
				return pdiskio->disk_write(m_Buffer, m_Sector, 1);
			}
			else{
				return RES_INTERNAL_ERROR;
			}
		}
		else{
			return RES_SUCCEEDED;
		}
	}

	// セクターをキャッシュに読み込む
	RESULT_e Cache::ReadTo(IMiniFSDiskIO *pdiskio, uint32_t sector){
		if (m_Sector != sector){
			RESULT_e result;
			result = Flush(pdiskio);
			if (result == RES_SUCCEEDED){
				m_Sector = sector;
				result = pdiskio->disk_read(m_Buffer, m_Sector, 1);
			}
			if (result != RES_SUCCEEDED){
				m_Sector = INVALID_SECTOR;
				return result;
			}
		}
		return RES_SUCCEEDED;
	}
	
	// セクターをキャッシュに読み込んでバッファにコピー
	RESULT_e Cache::ReadWith(IMiniFSDiskIO *pdiskio, uint32_t sector, void *buf, uint32_t offset, uint32_t length){
		RESULT_e result;
		result = ReadTo(pdiskio, sector);
		if (result == RES_SUCCEEDED){
			result = ReadFrom(pdiskio, sector, buf, offset, length);
		}
		return result;
	}

	// キャッシュへ書き込む
	RESULT_e Cache::WriteTo(IMiniFSDiskIO *pdiskio, uint32_t sector, void *buf, uint32_t offset, uint32_t length){
		RESULT_e result;
		result = ReadTo(pdiskio, sector);
		if (result != RES_SUCCEEDED){
			return result;
		}
		memcpy(m_Buffer + offset, buf, length);
		m_Modified = true;
		return RES_SUCCEEDED;
	}
}


