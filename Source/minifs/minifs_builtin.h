#pragma once
#ifndef MINIFS_BUILTIN_H
#define MINIFS_BUILTIN_H

#include "minifs_config.h"



namespace mfs{
	//**** バイトスワップ関数 ****//
	
	// バイトスワップ(uint16_t)
	static inline uint16_t Swap16(uint16_t val){
		return (val << 8) | (val >> 8);
	}

	// バイトスワップ(uint32_t)
	static inline uint32_t Swap32(uint32_t val){
		return (val << 24) | ((val << 8) & 0xFF0000) | ((val >> 8) & 0xFF00) | (val >> 24);
	}

	// バイトスワップ(uint64_t)
	static inline uint64_t Swap64(uint64_t val){
		return ((uint64_t)Swap32((uint32_t)val) << 32) | (uint64_t)Swap32((uint32_t)(val >> 32));
	}

	//****************************//



	//**** エンディアン変換関数 ****//

	// リトルエンディアンからネイティブエンディアンに変換する (uint16_t)
	static inline uint16_t LEtoNE16(uint16_t val){
		return !BIG_ENDIAN ? val : Swap16(val);
	}

	// リトルエンディアンからネイティブエンディアンに変換する (uint32_t)
	static inline uint32_t LEtoNE32(uint32_t val){
		return !BIG_ENDIAN ? val : Swap32(val);
	}

	// リトルエンディアンからネイティブエンディアンに変換する (uint64_t)
	static inline uint64_t LEtoNE64(uint64_t val){
		return !BIG_ENDIAN ? val : Swap64(val);
	}

	// ネイティブエンディアンからリトルエンディアンに変換する (uint16_t)
	static inline uint16_t NEtoLE16(uint16_t val){
		return LEtoNE16(val);
	}

	// ネイティブエンディアンからリトルエンディアンに変換する (uint32_t)
	static inline uint32_t NEtoLE32(uint32_t val){
		return LEtoNE32(val);
	}
	
	// ネイティブエンディアンからリトルエンディアンに変換する (uint64_t)
	static inline uint64_t NEtoLE64(uint64_t val){
		return LEtoNE64(val);
	}

	// リトルエンディアンからネイティブエンディアンに変換する (uint16_t *)
	static inline uint16_t LoadLE16(const void *val){
		return LEtoNE16(*(const uint16_t*)val);
	}

	// リトルエンディアンからネイティブエンディアンに変換する (uint32_t *)
	static inline uint32_t LoadLE32(const void *val){
		return LEtoNE32(*(const uint32_t*)val);
	}

	// リトルエンディアンからネイティブエンディアンに変換する (uint64_t *)
	static inline uint64_t LoadLE64(const void *val){
		return LEtoNE64(*(const uint64_t*)val);
	}

	// ネイティブエンディアンからリトルエンディアンに変換する (uint16_t *)
	static inline void StoreLE16(void *dst, uint16_t val){
		*(uint16_t*)dst = NEtoLE16(val);
	}

	// ネイティブエンディアンからリトルエンディアンに変換する (uint32_t *)
	static inline void StoreLE32(void *dst, uint32_t val){
		*(uint32_t*)dst = NEtoLE32(val);
	}

	// ネイティブエンディアンからリトルエンディアンに変換する (uint64_t *)
	static inline void StoreLE64(void *dst, uint64_t val){
		*(uint64_t*)dst = NEtoLE64(val);
	}

	//******************************//



	//**** ビットシフト関数 ****//

	// 右に1bitローテートする (uint16_t)
	static inline uint16_t RRotate16(uint16_t val, uint32_t r = 1){
		r &= 0xF;
		return (val << (16 - r)) | (val >> r);
	}

	// 右に1bitローテートする (uint32_t)
	static inline uint32_t RRotate32(uint32_t val, uint32_t r = 1){
		r &= 0x1F;
		return (val << (32 - r)) | (val >> r);
	}

	// 左にシフトする (uint64_t) <= (uint32_t) << (uint32_t) 
	static inline uint64_t LShift32to64(uint32_t val, uint32_t s){
		// x86
		return (uint64_t)val << s;
		
		// その他、ARMなど
		//return ((uint64_t)(val >> (32 - s)) << 32) | (uint64_t)((uint32_t)(val << s));
	}

	// 右にシフトして切り上げる (uint32_t)
	static inline uint32_t RShiftCeiling32(uint32_t val, uint32_t s){
		return (val >> s) + ((val & ((1UL << s) - 1)) ? 1 : 0);
	}

	// 右にシフトして切り上げる (0 < val) (uint32_t)
	static inline uint32_t RShiftCeilingPV32(uint32_t val, uint32_t s){
		return ((val - 1) >> s) + 1;
	}

	// 右にシフトして切り上げる (uint64_t)
	static inline uint32_t RShiftCeiling64to32(uint64_t val, uint32_t s){
		return (uint32_t)(val >> s) + (((uint32_t)val & ((1UL << s) - 1)) ? 1 : 0);
	}

	// 右にシフトして切り上げる (0 < val) (uint32_t)
	static inline uint32_t RShiftCeilingPV64to32(uint64_t val, uint32_t s){
		return (uint32_t)((val - 1) >> s) + 1;
	}

	//**************************//



	//**** ビット演算関数 ****//

	// 1になっているビットを数える (uint32_t)
	static inline uint32_t PopCount32(uint32_t val){
		val = (val & 0x55555555UL) + ((val & 0xAAAAAAAAUL) >> 1);
		val = (val & 0x33333333UL) + ((val & 0xCCCCCCCCUL) >> 2);
		val = (val & 0x0F0F0F0FUL) + ((val & 0xF0F0F0F0UL) >> 4);
		val = (val & 0x00FF00FFUL) + ((val & 0xFF00FF00UL) >> 8);
		val = (val & 0x0000FFFFUL) + ((val & 0xFFFF0000UL) >> 16);
		return val;
	}

	// LSBから連続する0の数を数える (uint32_t)
	static inline uint32_t CountTrailingZeros32(uint32_t val){
		return PopCount32((val & (-(int32_t)val)) - 1);
	}

	//************************//
	


	//**** 整数演算関数 ****//

	// 乗算する (uint64_t) <= (uint32_t) * (uint32_t) 
	static inline uint64_t Mul32x32to64(uint32_t val1, uint32_t val2){
		return (uint64_t)val1 * (uint64_t)val2;
	}

	//**********************//



	//**** ロード・ストア関数 ****//

	// その時点で値をストアする
	template <typename T>
	static inline void StoreImmediate(T &dst, T val){
		(T volatile &)dst = val;
	}

	// その時点で値をストアする
	template <typename T>
	static inline void StoreImmediate(T *&dst, void *p){
		(T * volatile &)dst = (T*)p;
	}

	// その時点の値をロードする
	template <typename T>
	static inline T LoadImmediate(T &dst){
		return (T volatile &)dst;
	}

	//**********************//




}



#endif // MINIFS_BUILTIN_H
