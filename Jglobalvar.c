/*
 * Jglobalvar.c
 *
 *  Created on: Jun 23, 2017
 *      Author: kangidjoel
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "Jglobalvar.h"
#include "../secube-host/L1.h"

int jstrlen(char* args[]){
	bool ada = true;
	int j = 0;
	while (ada) {
		j++;
		printf("%d ",(int) strtol(args[j], NULL, 10));
		if (args[j] == NULL) {
			ada = false;
		}

	}
	return j-1;

}

void arrtoarr(char* _arr, uint8_t* _ret){
	for (int i = 0; i < strlen(_arr); i++) {
		_ret[i] = _arr[i];
	}
}

void arrtouint(int _start, int _size, char *_arr[], uint8_t *_ret){
	uint8_t *buf= (uint8_t*) calloc(ENC_SIZE(_size), sizeof(uint8_t));
	for (int i = 0; i < _size; i++) {
		_ret[i] = (int) strtol(_arr[i+_start], NULL, 10);
	}
}

long get_unixepoch_time(char* _date){

	/* parm date format should in form of YmdHMS */

	char subbuff[4];
	struct tm t;
	time_t t_of_day;
	memcpy(subbuff, &_date[0], 4);
	subbuff[4] = '\0';
	t.tm_year = (int) strtol(subbuff, NULL, 10) - 1900;
	memcpy(subbuff, &_date[4], 2);
	subbuff[2] = '\0';
	t.tm_mon = (int) strtol(subbuff, NULL, 10) - 1;           // Month, 0 - jan
	memcpy(subbuff, &_date[6], 2);
	subbuff[2] = '\0';
	t.tm_mday = (int) strtol(subbuff, NULL, 10);          // Day of the month
	memcpy(subbuff, &_date[8], 2);
	subbuff[2] = '\0';
	t.tm_hour = (int) strtol(subbuff, NULL, 10);
	memcpy(subbuff, &_date[10], 2);
	subbuff[2] = '\0';
	t.tm_min = (int) strtol(subbuff, NULL, 10);
	memcpy(subbuff, &_date[12], 2);
	subbuff[2] = '\0';
	t.tm_sec = (int) strtol(subbuff, NULL, 10);
	t.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
	t_of_day = mktime(&t);


	return (long) t_of_day;

}

void print_sn(uint8_t* v) {
	size_t i;

	for (i = 0; i < LS3_LENGTH_32; i++) {
		printf("%u ", (unsigned) v[i]);

	}
	printf("\n");

	return ;
}

char* concat(const char *s1, const char *s2) {
	const size_t len1 = strlen(s1);
	const size_t len2 = strlen(s2);
	char *result = malloc(len1 + len2 + 1); //+1 for the zero-terminator
	//in real code you would check for errors in malloc here
	memcpy(result, s1, len1);
	memcpy(result + len1, s2, len2 + 1);    //+1 to copy the null-terminator
	return result;
}

char* replace(char* str, char* a, char* b){
    int len  = strlen(str);
    int lena = strlen(a), lenb = strlen(b);
    for (char* p = str; p = strstr(p, a); ++p) {
        if (lena != lenb) // shift end as needed
            memmove(p+lenb, p+lena, len - (p - str) + lenb);
        memcpy(p, b, lenb);
    }
    return str;
}
