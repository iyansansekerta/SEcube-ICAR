/*
 * Logger.h
 *
 *  Created on: Jun 1, 2017
 *      Author: kangidjoel
 */

#include <stdint.h>

void writeLogFile(unsigned char hash[], char* _string);
void writeLogFileArr(char* _str, char* _format, uint8_t* _chr, int _len);
void printHash(unsigned char hash[]);

