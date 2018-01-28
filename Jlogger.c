/*
 * logger.c
 *
 *  Created on: Jun 1, 2017
 *      Author: kangidjoel
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* user lib */
#include "Jlogger.h"
#include "Jglobalvar.h"

void writeLogFile(unsigned char _hscode[], char* _string) {

	int idx;
	FILE* pFile = fopen(LS3_LOGFILE_NAME, "a");
	if (!pFile) {
		printf("Cannot open logfile");
		return;
	}

	idx = strlen(_string);

	if (idx>0) {
		fprintf(pFile, "%s", _string);
	} else {
		for (idx = 0; idx < LS3_LENGTH_32; idx++)
			fprintf(pFile, "%02x", _hscode[idx]);
	}

	fprintf(pFile, "%s", "\n\n");
	fclose(pFile);

}

void writeLogFileArr(char* _str, char* _format, uint8_t* _chr, int _len) {

	FILE* pFile = fopen(LS3_LOGFILE_NAME, "a");
	if (!pFile) {
		printf("Cannot open logfile");
		return;
	}

	fprintf(pFile, "%s", _str);

	for (int i = 0; i < _len; i++) {
		fprintf(pFile, _format, (unsigned)_chr[i]);
	}

	fprintf(pFile, "%s", "\n\n");
	fclose(pFile);

}

void printHash(unsigned char _hscode[]) {
	int idx;
	for (idx = 0; idx < LS3_LENGTH_32; idx++)
		printf("%02x", _hscode[idx]);
	printf("\n");
}


