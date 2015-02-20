#pragma once
#ifndef FILE_DISKIO_H
#define FILE_DISKIO_H

#include "minifs/minifs_diskio.h"
#include <Windows.h>



// ファイルをディスクと見なしディスクI/Oを提供するクラス
class FileDiskIO : public mfs::IMiniFSDiskIO{
private:
	static const uint32_t BYTES_PER_SECTOR_SHIFT = 9;
	static const uint32_t SECTORS_PER_BLOCK_SHIFT = 7;
	
	bool m_Initialized;
	uint32_t m_SuppressCount;
	uint32_t m_BytesPerSectorShift;
	uint32_t m_BytesPerSector;
	uint32_t m_SectorsPerBlockShift;
	HANDLE m_hFile;
	uint32_t m_SectorCount;
	uint32_t m_FilePointer;
	
public:
	// コンストラクタ
	FileDiskIO(const wchar_t *path, uint32_t block_size_shift = SECTORS_PER_BLOCK_SHIFT, uint32_t bytes_per_sector_shift = BYTES_PER_SECTOR_SHIFT){
		m_Initialized = false;
		m_SuppressCount = 0;
		m_BytesPerSectorShift = bytes_per_sector_shift;
		m_BytesPerSector = 1UL << bytes_per_sector_shift;
		m_SectorsPerBlockShift = block_size_shift;
		m_hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		m_SectorCount = 0;
		m_FilePointer = 0;
	}
	
	// デストラクタ
	~FileDiskIO(){
		if (m_hFile != INVALID_HANDLE_VALUE){
			CloseHandle(m_hFile);
		}
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
		mfs::RESULT_e result;
		do{
			if (m_hFile != INVALID_HANDLE_VALUE){
				LARGE_INTEGER ret;
				if (GetFileSizeEx(m_hFile, &ret) == FALSE){
					GET_LENGTH_INFORMATION info;
					DWORD ret_byte;
					if (DeviceIoControl(m_hFile, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &info, sizeof(GET_LENGTH_INFORMATION), &ret_byte, nullptr) == FALSE){
						result = mfs::RES_DISK_ERROR;
						break;
					}
					ret = info.Length;
				}

				uint64_t count;
				count = (uint64_t)ret.QuadPart / m_BytesPerSector;
				m_SectorCount = (count & 0xFFFFFFFF00000000ULL) ? 0xFFFFFFFFUL : (uint32_t)count;
				m_Initialized = true;
				result = mfs::RES_SUCCEEDED;
				break;
			}
			else{
				result = mfs::RES_NOT_READY;
				break;
			}
		} while (false);
		if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
			wprintf(L"  [FileDiskIO:disk_initialize() => %d]\n", result);
		}
		return result;
	}

	// ディスクの情報を取得する
	virtual uint32_t disk_info(mfs::DISKINFO_e param) override{
		uint32_t ret = 0;
		switch (param){
		case mfs::INFO_STATUS:
			ret |= (m_Initialized == false) ? mfs::STA_NOT_INITIALIZED : 0;
			ret |= (m_hFile == INVALID_HANDLE_VALUE) ? mfs::STA_NO_DISK : 0;
			//ret |= mfs::STA_WRITE_PROTECTED;
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [FileDiskIO:disk_info(STATUS) => 0x%X]\n", ret);
			}
			break;
			
		case mfs::INFO_BYTES_PER_SECTOR_SHIFT:
			ret = m_BytesPerSectorShift;
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [FileDiskIO:disk_info(BYTES_PER_SECTOR_SHIFT) => %d]\n", ret);
			}
			break;

		case mfs::INFO_SECTOR_COUNT:
			ret = m_SectorCount;
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [FileDiskIO:disk_info(SECTOR_COUNT) => %d]\n", ret);
			}
			break;

		case mfs::INFO_SECTORS_PER_BLOCK_SHIFT:
			ret = m_SectorsPerBlockShift;
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [FileDiskIO:disk_info(SECTORS_PER_BLOCK_SHIFT) => %d]\n", ret);
			}
			break;

		default:
			if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
				wprintf(L"  [FileDiskIO:disk_info(UNKNOWN(%d)) => 0]\n", param);
			}
			break;
		}
		return ret;
	}
	
	// セクターを読み出す
	virtual mfs::RESULT_e disk_read(void *buf, uint32_t sector, uint32_t count) override{
		mfs::RESULT_e result;
		do{
			if (m_hFile != INVALID_HANDLE_VALUE){
				if (m_Initialized == true){
					if ((sector + count) <= m_SectorCount){
						if (sector != m_FilePointer){
							LARGE_INTEGER ptr;
							ptr.QuadPart = (uint64_t)sector * m_BytesPerSector;
							if (SetFilePointerEx(m_hFile, ptr, nullptr, FILE_BEGIN) == FALSE){
								result = mfs::RES_DISK_ERROR;
								break;
							}
						}
						DWORD read_byte;
						if (ReadFile(m_hFile, buf, count * m_BytesPerSector, &read_byte, nullptr) != FALSE){
							m_FilePointer = sector + count;
							result = mfs::RES_SUCCEEDED;
							break;
						}

					}
					uint32_t error = GetLastError();
					result = mfs::RES_DISK_ERROR;
					break;
				}
				else{
					result = mfs::RES_NOT_READY;
					break;
				}
			}
			else{
				result = mfs::RES_NOT_READY;
				break;
			}
		} while (false);
		if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
			wprintf(L"  [FileDiskIO:disk_read(%d, %d) => %d]\n", sector, count, result);
		}
		return result;
	}
	
	// セクターへ書き込む
	virtual mfs::RESULT_e disk_write(const void *buf, uint32_t sector, uint32_t count) override{
		mfs::RESULT_e result;
		do{
			if (m_hFile != INVALID_HANDLE_VALUE){
				if (m_Initialized == true){
					if ((sector + count) <= m_SectorCount){
						if (sector != m_FilePointer){
							LARGE_INTEGER ptr;
							ptr.QuadPart = (uint64_t)sector * m_BytesPerSector;
							if (SetFilePointerEx(m_hFile, ptr, nullptr, FILE_BEGIN) == FALSE){
								result = mfs::RES_DISK_ERROR;
								break;
							}
						}
						DWORD written_byte;
						if (WriteFile(m_hFile, buf, count * m_BytesPerSector, &written_byte, nullptr) != FALSE){
							m_FilePointer = sector + count;
							result = mfs::RES_SUCCEEDED;
							break;
						}
					}
					result = mfs::RES_DISK_ERROR;
					break;
				}
				else{
					result = mfs::RES_NOT_READY;
					break;
				}
			}
			else{
				result = mfs::RES_NOT_READY;
				break;
			}
		} while (false);
		if ((OUTPUT_ACCESS_LOG == true) && (m_SuppressCount == 0)){
			wprintf(L"  [FileDiskIO:disk_write(%d, %d) => %d]\n", sector, count, result);
		}
		return result;
	}
};



#endif // FILE_DISKIO_H
