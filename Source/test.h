#pragma once
#include <stdint.h>
#include <intrin.h>



static inline uint8_t CalcTestPattern(uint64_t value){
	value++;
	union{
		uint64_t u64;
		uint8_t u8[8];
	} data1, data2;
	data1.u64 = value;
	data2.u8[1] = _rotl8(data1.u8[0], 1) ^ 0x23;
	data2.u8[3] = _rotl8(data1.u8[1], 2) ^ 0x45;
	data2.u8[5] = _rotl8(data1.u8[2], 4) ^ 0x67;
	data2.u8[7] = _rotl8(data1.u8[3], 3) ^ 0x89;
	data2.u8[0] = _rotl8(data1.u8[4], 7) ^ 0xAB;
	data2.u8[2] = _rotl8(data1.u8[5], 6) ^ 0xCD;
	data2.u8[4] = _rotl8(data1.u8[6], 5) ^ 0xEF;
	data2.u8[6] = _rotl8(data1.u8[7], 1) ^ 0x01;
	return data2.u8[0] - data2.u8[1] + data2.u8[2] - data2.u8[3] + data2.u8[4] - data2.u8[5] + data2.u8[6] - data2.u8[7];
}



static void TestGenerate(void *output, uint64_t pointer, uint64_t length){
	uint8_t *buf = (uint8_t*)output;
	while (0 < length){
		*buf++ = CalcTestPattern(pointer);
		pointer++;
		length--;
	}
}



static bool TestCompare(void *input, uint64_t pointer, uint64_t length){
	uint8_t *buf = (uint8_t*)input;
	while (0 < length){
		if (*buf++ != CalcTestPattern(pointer)){
			return false;
		}
		pointer++;
		length--;
	}
	return true;
}
