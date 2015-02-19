#pragma once
#ifndef MINIFS_PATH_H
#define MINIFS_PATH_H

#include "minifs_common.h"



namespace mfs{
	// 大文字変換テーブル
	extern const fschar_t CharUpperTableStandard[2918];

	// 大文字変換テーブル分割1
	extern const fschar_t CharUpperTableDiv1[];
	
	// 大文字変換テーブル分割2
	extern const fschar_t CharUpperTableDiv2[];
	
	// 大文字変換テーブル分割3
	extern const fschar_t CharUpperTableDiv3[];
	
	
	
	
	// UTF-16の大文字変換関数
	static inline fschar_t CharUpperUTF16(fschar_t c_){
		uint16_t c = (uint16_t)c_;
		if (!READ_ONLY && SUPPORT_EXFAT_FORMAT){
			// exFATフォーマットあり標準版
			if (c <= 0x0586){
				return CharUpperTableStandard[c];
			}
			else if (c < 0x1D7D){
				return c;
			}
			else if (c <= 0x2184){
				return CharUpperTableStandard[c - 0x1D7D + 1417];
			}
			else if (c < 0x24D0){
				return c;
			}
			else if (c <= 0x24E9){
				return c - 0x001A;
			}
			else if (c < 0x2C30){
				return c;
			}
			else if (c <= 0x2D25){
				return CharUpperTableStandard[c - 0x2C30 + 2479];
			}
		}
		else{
			// exFATフォーマットなし
			if (c <= 0x0586){
				return CharUpperTableDiv1[c];
			}
			else{
				if (USE_SIMPLE_UPPER_CASE == false){
					// 標準版
					if (c < 0x1D7D){
						return c;
					}
					else if (c <= 0x2184){
						return CharUpperTableDiv2[c - 0x1D7D];
					}
					else if (c < 0x24D0){
						return c;
					}
					else if (c <= 0x24E9){
						return c - 0x001A;
					}
					else if (c < 0x2C30){
						return c;
					}
					else if (c <= 0x2D25){
						return CharUpperTableDiv3[c - 0x2C30];
					}
				}
				else{
					// 簡易版
					if (c < 0x2170){
						return c;
					}
					else if (c <= 0x217F){
						return c - 0x0010;
					}
					else if (c < 0x24D0){
						return c;
					}
					else if (c <= 0x24E9){
						return c - 0x001A;
					}
				}
			}
		}
		if (c < 0xFF41){
			return c;
		}
		else if (c <= 0xFF5A){
			return c - 0x0020;
		}
		else{
			return c;
		}
	}

	// パスをヌル区切りリストに変換
	bool ConvertPathToList(uint16_t *dst, const fschar_t *src1, const fschar_t *src2 = nullptr);



}



#endif // MINIFS_PATH_H
