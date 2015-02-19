#pragma once
#ifndef MINIFS_H
#define MINIFS_H

#include "minifs_common.h"
#include "minifs_diskio.h"
#include "minifs_filesystem.h"
#include "minifs_dirhandle.h"
#include "minifs_filehandle.h"
#include "basicfs.h"
#include "exfatfs.h"



namespace mfs{
	// MiniFSの本体のクラス
	class MiniFS{
	private:


	public:




	public:
		// コンストラクタ
		MiniFS(void){}

		// デストラクタ
		~MiniFS(){}

		// パーティションのリストを取得する
		static int32_t getPartitionInfoList(IMiniFSDiskIO &diskio, PartitionInfo_t *info_list, uint32_t length_of_list);

		// パーティションの分割をする
		static RESULT_e initPartitions(IMiniFSDiskIO &diskio, const uint32_t *size_list, uint32_t length_of_list, bool clean = false);

		// パーティションをフォーマットする
		static RESULT_e formatPartition(PartitionInfo_t &info, PartitionType_e type, uint32_t sectors_per_cluster_shift);

		// パーティションをマウントする
		static IMiniFSFileSystem* mountPartition(PartitionInfo_t &info);













	};
}



#endif // MINIFS_H
