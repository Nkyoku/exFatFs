#include <stdio.h>
#include <locale.h>
#include "minifs/minifs.h"
#include "test.h"

#define OUTPUT_ACCESS_LOG true
#include "file_diskio.h"
#include "mem_diskio.h"

static const uint64_t MiB = 1ULL << 20;
static const uint64_t GiB = 1ULL << 30;
static const uint64_t TiB = 1ULL << 40;

//FileDiskIO diskio(L"D:\\Desktop\\Desktop_bak/image.bin");
//FileDiskIO diskio(L"D:\\Desktop\\SDImage2G.img");
FileDiskIO diskio(L"\\\\.\\PhysicalDrive1");
//MemDiskIO diskio(nullptr, 128ULL * MiB, 4ULL * GiB);



uint8_t g_Buffer[1UL * MiB];



int main(void){
	_wsetlocale(LC_ALL, L"");

	/*uint32_t size_list[] = {
		0xFFFFFFFF
	};
	mfs::MiniFS::initPartitions(diskio, size_list, 1, false);
	*/
	int32_t result;
	mfs::PartitionInfo_t list[16];
	result = mfs::MiniFS::getPartitionInfoList(diskio, list, 16);
	for (int32_t cnt = 0; cnt < result; cnt++){
		wprintf(L"Partition:%d type=%d, offset=%d, size=%d\n", cnt, list[cnt].partition_type, list[cnt].partition_sector, list[cnt].partition_size);
	}

	//mfs::MiniFS::formatPartition(list[0], mfs::PID_EXFAT, 2);
	
	mfs::MiniFS::getPartitionInfoList(diskio, list, 16);
	mfs::IMiniFSFileSystem *pfs = mfs::MiniFS::mountPartition(list[0]);
	if (pfs != nullptr){
		wprintf(L"パーティションをExFatFsでマウントしました\n");

		wprintf(L"パーティションの空き領域は%lld、容量は%lldです\n", pfs->freeSpace(), pfs->capacity());

		
		

		/*mfs::DirHandle *pdir;
		pdir = pfs->openDir(L"/");
		if (pdir != nullptr){
			while (true){
				mfs::FileInfo_t info;
				mfs::RESULT_e result = pdir->read(&info);
				if (result != mfs::RES_SUCCEEDED){
					break;
				}
				wprintf(L"Item '%s' attr=0x%X\n", info.name, info.attributes);
			}
			delete pdir;
		}//*/

		/*mfs::FileHandle *pfile;
		pfile = pfs->openFile(L"/test01.bin");
		if (pfile != nullptr){
			wprintf(L"size=%lld, pointer=%lld\n", pfile->size(), pfile->tell());
			FILE *fp;
			fopen_s(&fp, "trial01.bin", "wb");
			diskio.suppressDebug(true);
			bool first = true;
			while (true){
				uint64_t pointer = pfile->tell();
				uint64_t rem = pfile->size() - pfile->tell();
				uint32_t len = sizeof(g_Buffer);
				result = pfile->read(g_Buffer, len);
				if (result == 0){
					break;
				}
				wprintf(L"trial : p=%lld, len=%d, result=%d, matched=%d\n", pointer, len, result, TestCompare(g_Buffer, pointer, result));
				fwrite(g_Buffer, 1, result, fp);
				if ((first == true) && ((5UL * MiB) <= pointer)){
					first = false;
					mfs::RESULT_e res;
					res = pfile->seek(42UL * MiB + 4121);
					if (res != mfs::RES_SUCCEEDED){
						break;
					}
					wprintf(L"trial : seek %lld\n", pfile->tell());
				}
			}
			diskio.suppressDebug(false);
			fclose(fp);



			
			delete pfile;
		}//*/



		delete pfs;
	}
	else{
		wprintf(L"パーティションをマウントできませんでした\n");
	}//*/

	wprintf(L"\n何かキーを押すと終了します\n");
	getchar();
	return 0;
}
