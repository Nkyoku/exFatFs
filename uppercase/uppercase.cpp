#include <stdio.h>
#include <locale.h>
#include <stdint.h>

int main(void){
	_wsetlocale(LC_ALL, L"");

	FILE *fp_i, *fp_o;
	_wfopen_s(&fp_i, L"uppercase.bin", L"rb");
	_wfopen_s(&fp_o, L"uppercase.txt", L"w, ccs=UTF-16LE");
	//uint8_t bom[2] = { 0xFF, 0xFE };
	//fwrite(bom, 1, 2, fp_o);

	uint16_t buf[65536];
	uint32_t count = fread_s(buf, sizeof(buf), 2, 65536, fp_i);
	uint16_t index = 0;
	uint32_t col_cnt = 0;
	bool end = false;
	for (uint32_t i = 0; i < count; i++){
		uint16_t c = buf[i];
		if (c == 0xFFFE){
			end = true;
		}
		if ((c == 0xFFFF) && (end == false)){
			index += buf[i + 1];
			i += 1;
			fwprintf(fp_o, L"\n----------------\n");
			col_cnt = 0;
			continue;
		}

		/*if (index != c){
			fwprintf(fp_o, L"U+%04X '%c' -> U+%04X '%c'\n", index, index, c, c);
		}
		else{*/
			fwprintf(fp_o, L"0x%04X, ", c);
			col_cnt++;
			if (16 <= col_cnt){
				fwprintf(fp_o, L"\n");
				col_cnt = 0;
			}
		//}
		index++;
	}

	fclose(fp_i);
	fclose(fp_o);

	return 0;
}


