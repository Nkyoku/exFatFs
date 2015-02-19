#pragma once
#ifndef MEMORY_DISKIO_H
#define MEMORY_DISKIO_H

#include "minifs/minifs_diskio.h"
#include <Windows.h>
#include <string.h>



// メモリーをディスクと見なしディスクI/Oを提供するクラス
class MemDiskIO : public mfs::IMiniFSDiskIO{
private:
	static const uint32_t BYTES_PER_SECTOR_SHIFT = 9;
	static const uint32_t SECTORS_PER_BLOCK_SHIFT = 7;

	bool m_Initialized;
	uint32_t m_SuppressCount;
	uint32_t m_BytesPerSectorShift;
	uint32_t m_BytesPerSector;
	uint32_t m_SectorsPerBlockShift;
	uint32_t m_PhysicalSectorCount;
	uint32_t m_VirtualSectorCount;
	uint8_t *m_pBuffer;
	
	
public:
	// コンストラクタ
	MemDiskIO(const wchar_t *path, uint64_t physical_size, uint64_t virtual_size, uint32_t block_size_shift = SECTORS_PER_BLOCK_SHIFT, uint32_t bytes_per_sector_shift = BYTES_PER_SECTOR_SHIFT){
		m_Initialized = false;
		m_SuppressCount = 0;
		m_BytesPerSectorShift = bytes_per_sector_shift;
		m_BytesPerSector = 1UL << bytes_per_sector_shift;
		m_SectorsPerBlockShift = block_size_shift;
		m_PhysicalSectorCount = (uint32_t)(physical_size / m_BytesPerSector);
		m_VirtualSectorCount = (uint32_t)(virtual_size / m_BytesPerSector);
		m_pBuffer = new uint8_t[m_PhysicalSectorCount * m_BytesPerSector];
		memset(m_pBuffer, 0x00, m_PhysicalSectorCount * m_BytesPerSector);

		if (path != nullptr){
			// ファイルから読み込む
			HANDLE hFile;
			hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile != INVALID_HANDLE_VALUE){
				LARGE_INTEGER ret;
				if (GetFileSizeEx(hFile, &ret) == FALSE){
					GET_LENGTH_INFORMATION info;
					DWORD ret_byte;
					if (DeviceIoControl(hFile, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &info, sizeof(GET_LENGTH_INFORMATION), &ret_byte, nullptr) != FALSE){
						ret = info.Length;
					}
					else{
						ret.QuadPart = 0;
					}
				}

				DWORD read_byte;
				if ((LONGLONG)physical_size <= ret.QuadPart){
					read_byte = (DWORD)physical_size;
				}
				else{
					read_byte = (DWORD)ret.QuadPart;
				}
				ReadFile(hFile, m_pBuffer, read_byte, &read_byte, nullptr);
				CloseHandle(hFile);
			}
		}
		else{
			// MBRを作成する
			if (0 < m_PhysicalSectorCount){
				*(uint32_t*)(m_pBuffer + 446) = 0xFFFFFE00;
				*(uint32_t*)(m_pBuffer + 450) = 0xFFFFFE00;
				*(uint32_t*)(m_pBuffer + 454) = mfs::FIRST_PARTITION_SECTOR;
				*(uint32_t*)(m_pBuffer + 458) = m_VirtualSectorCount - mfs::FIRST_PARTITION_SECTOR;
				*(uint16_t*)(m_pBuffer + 510) = 0xAA55;
			}
		}
	}
	
	// デストラクタ
	~MemDiskIO(){
		delete [] m_pBuffer;
	}

	// デバッグ出力を無効にする
	void suppressDebug(bool enable){
		if (enable == true){
			m_SuppressCount++;
		}
		else if (0 < m_SuppressCount){
			m_SuppressCount--;
		}
	}
	
	// ディスクを使用可能にする
	virtual mfs::RESULT_e disk_initialize(void) override{
		mfs::RESULT_e result = mfs::RES_SUCCEEDED;
		m_Initialized = true;
		if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
			wprintf(L"  [MemDiskIO:disk_initialize() => %d]\n", result);
		}
		return mfs::RES_SUCCEEDED;
	}

	// ディスクの情報を取得する
	virtual uint32_t disk_info(mfs::DISKINFO_e param) override{
		uint32_t ret = 0;
		switch (param){
		case mfs::INFO_STATUS:
			ret |= (m_Initialized == false) ? mfs::STA_NOT_INITIALIZED : 0;
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [MemDiskIO:disk_info(STATUS) => 0x%X]\n", ret);
			}
			break;
			
		case mfs::INFO_BYTES_PER_SECTOR_SHIFT:
			ret = m_BytesPerSectorShift;
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [MemDiskIO:disk_info(BYTES_PER_SECTOR_SHIFT) => %d]\n", ret);
			}
			break;

		case mfs::INFO_SECTOR_COUNT:
			ret = m_VirtualSectorCount;
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [MemDiskIO:disk_info(SECTOR_COUNT) => %d]\n", ret);
			}
			break;

		case mfs::INFO_SECTORS_PER_BLOCK_SHIFT:
			ret = m_SectorsPerBlockShift;
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [MemDiskIO:disk_info(SECTORS_PER_BLOCK_SHIFT) => %d]\n", ret);
			}
			break;

		default:
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [MemDiskIO:disk_info(UNKNOWN(%d)) => 0]\n", param);
			}
			break;
		}
		return ret;
	}
	
	// セクターを読み出す
	virtual mfs::RESULT_e disk_read(void *buf, uint32_t sector, uint32_t count) override{
		mfs::RESULT_e result;
		do{
			if (m_Initialized == true){
				if ((sector + count) <= m_VirtualSectorCount){
					uint32_t sec = sector;
					uint32_t cnt = count;
					while (0 < cnt--){
						if (sec < m_PhysicalSectorCount){
							memcpy(buf, m_pBuffer + m_BytesPerSector * sec, m_BytesPerSector);
						}
						else{
							memset(buf, 0x00, m_BytesPerSector);
						}
						buf = (uint8_t*)buf + m_BytesPerSector;
						sec++;
					}
					result = mfs::RES_SUCCEEDED;
				}else{
					result = mfs::RES_ERROR;
				}
				break;
			}
			else{
				result = mfs::RES_NOT_READY;
				break;
			}
		} while (false);
		if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
			wprintf(L"  [MemDiskIO:disk_read(%d, %d) => %d]\n", sector, count, result);
		}
		return result;
	}
	
	// セクターへ書き込む
	virtual mfs::RESULT_e disk_write(const void *buf, uint32_t sector, uint32_t count) override{
		mfs::RESULT_e result;
		do{
			if (m_Initialized == true){
				if ((sector + count) <= m_VirtualSectorCount){
					uint32_t sec = sector;
					uint32_t cnt = count;
					while ((0 < cnt--) && (sec < m_PhysicalSectorCount)){
						memcpy(m_pBuffer + m_BytesPerSector * sec, buf, m_BytesPerSector);
						buf = (uint8_t*)buf + m_BytesPerSector;
						sec++;
					}
					result = mfs::RES_SUCCEEDED;
				}
				else{
					result = mfs::RES_ERROR;
				}
				break;
			}
			else{
				result = mfs::RES_NOT_READY;
				break;
			}
		} while (false);
		if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
			wprintf(L"  [MemDiskIO:disk_write(%d, %d) => %d]\n", sector, count, result);
		}
		return result;
	}
};



#endif // FILE_DISKIO_H
