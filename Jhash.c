/*
 * ICheck.c
 *
 *  Created on: Jun 1, 2017
 *      Author: kangidjoel
 */

#include <stdio.h>
#include <stdlib.h>

/* user lib */
#include "Jlogger.h"
#include "Jglobalvar.h"
#include "Jhash.h"
#include "../secube-host/sha256.h"

int get_file_hash(char* _path, char *_res) {
	/* init sha256 */

	if((_path==NULL) || (_path[0]=='\0')){
		return LS3_ER;
	}

	FILE* file = fopen(_path, "rb");

	if (!file){
		*_res = "x";
		return LS3_OK;
	}

	unsigned char hash[LS3_LENGTH_32];
	B5_tSha256Ctx sha256;
	B5_Sha256_Init(&sha256);

	const int bufSize = 32768;
	char* buffer = malloc(bufSize);
	int bytesRead = 0;
	if (!buffer)
		return LS3_ER;

	while ((bytesRead = fread(buffer, 1, bufSize, file))) {
		B5_Sha256_Update(&sha256, buffer, bytesRead);
	}

	B5_Sha256_Finit(&sha256, hash);

	char printstr[LS3_LENGTH_64] = {00};
	for (int idx = 0; idx < LS3_LENGTH_64; idx++)
		sprintf(printstr+(idx*2), "%02x", hash[idx]);
	printstr[LS3_LENGTH_64] = '\0';

	memcpy(_res,printstr,LS3_LENGTH_64);

	fclose(file);
	free(buffer);

	return LS3_OK;
}
