#pragma once
#ifndef MINIFS_CONFIG_H
#define MINIFS_CONFIG_H

#include <stdint.h>

#include <Windows.h>
#include <stdio.h>



namespace mfs{
	

	
	// CPUはビッグエンディアンである
	static const bool BIG_ENDIAN = false;

	

	// 書き込みを行わない
	static const bool READ_ONLY = false;


	
	// 認識できるパーティションの最大数
	// サイズが0のパーティションは含まれないが、拡張パーティションの数は含まれる
	static const uint32_t MAXIMUM_PARTITIONS = 16;



	// 最初のパーティションを配置するセクター
	static const uint32_t FIRST_PARTITION_SECTOR = 63;



	// 4Kセクターをサポートする
	// このオプションを使用しないとき、セクターサイズは512バイトのみがサポートされる
	static const bool SUPPORT_4K_SECTOR = false;

	

	// exFATのフォーマットをサポートする
	// このオプションが有効なとき、USE_SIMPLE_UPPER_CASEオプションは無視される
	static const bool SUPPORT_EXFAT_FORMAT = true;



	// ファイルパスの最大長
	static const uint32_t MAXIMUM_PATH = 255;
	
	
	
	// UTF-16を格納する16bit整数型
	typedef wchar_t fschar_t;

	
	
	// UTF-16の大文字変換に簡易版テーブルを使用する
	// 簡易版は標準版と比べて変換テーブルのサイズが半減するが対応できない文字が現れる
	static const bool USE_SIMPLE_UPPER_CASE = false;

	
	
	// データの破損確認用のチェックサムを比較する
	static const bool CHECK_CHECKSUM = true;



	// 同期オブジェクトを管理するクラス
	class fsmutex_t{
	private:
		// ミューテックスのハンドル
		HANDLE m_hMutex;
		bool m_Locked;

	public:
		// コンストラクタ
		fsmutex_t() : m_Locked(false){}

		// 同期オブジェクトを作成する
		bool createMutex(void){
			m_Locked = false;
			m_hMutex = CreateMutex(NULL, FALSE, NULL);
			return (m_hMutex != INVALID_HANDLE_VALUE);
		}

		// 同期オブジェクトを削除する
		void destroyMutex(void){
			m_Locked = false;
			CloseHandle(m_hMutex);
		}

		// 同期オブジェクトをロックする
		bool lockMutex(bool infinite = false){
			if (m_Locked == true){
				wprintf(L"多重ロック\n");
			}
			m_Locked = true;
			return (WaitForSingleObject(m_hMutex, infinite ? INFINITE : 1000) == WAIT_OBJECT_0);
		}

		// 同期オブジェクトをアンロックする
		void unlockMutex(void){
			if (m_Locked == false){
				wprintf(L"多重アンロック\n");
			}
			m_Locked = false;
			ReleaseMutex(m_hMutex);
		}
	};



	// 時刻の形式
	enum TIMEFORMAT_e{
		TIMEFORMAT_NONE,		// 時刻をサポートしない
		TIMEFORMAT_MSDOS,		// MS-DOS形式の時刻(2s単位)
		TIMEFORMAT_UNIX,		// UNIX時刻(1s単位)
		TIMEFORMAT_UNIX_1MS,	// UNIX時刻(1ms単位)
	};

	// getTime()が返す時刻の形式
	static const bool TIME_FORMAT = TIMEFORMAT_NONE;

	// 現在時刻を取得する関数
	static inline uint64_t getTime(void){
		return 0;
	}











}



#endif // MINIFS_CONFIG_H
