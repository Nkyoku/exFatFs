#include <stdio.h>
#include <locale.h>
#include "minifs/minifs.h"

#define OUTPUT_ACCESS_LOG true
#include "file_diskio.h"
#include "mem_diskio.h"

uint64_t MiB = 1ULL << 20;
uint64_t GiB = 1ULL << 30;
uint64_t TiB = 1ULL << 40;

//FileDiskIO diskio(L"D:\\Desktop\\Desktop_bak/image.bin");
//FileDiskIO diskio(L"D:\\Desktop\\SDImage2G.img");
FileDiskIO diskio(L"\\\\.\\PhysicalDrive1");
//MemDiskIO diskio(nullptr, 128ULL * MiB, 4ULL * GiB);



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

		
		//mfs::FileHandle *pfile;
		//filehandle.open(eff, L"/test.bin");

		/*mfs::DirHandle *pdir;
		pdir = pfs->openDir(L"/");
		if (pdir != nullptr){
			while (true){
				mfs::DirInfo_t info;
				mfs::RESULT_e result = pdir->read(&info);
				if (result != mfs::RES_SUCCEEDED){
					break;
				}
				wprintf(L"Item '%s' attr=0x%X\n", info.name, info.attributes);
			}
			delete pdir;
		}*/



		delete pfs;
	}
	else{
		wprintf(L"パーティションをマウントできませんでした\n");
	}//*/

	wprintf(L"\n何かキーを押すと終了します\n");
	getchar();
	return 0;
}
