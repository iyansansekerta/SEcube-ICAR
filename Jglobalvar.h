/*
 * Jglobalvar.h
 *
 *  Created on: Jun 12, 2017
 *      Author: kangidjoel
 */
#include <stdint.h>

#ifndef LOCALLIB_HOST_JGLOBALVAR_H_
#define LOCALLIB_HOST_JGLOBALVAR_H_

#define TEST_SIZE(plain_txt)     jstrlen(plain_txt)
#define ENC_SIZE(plain_size)    (((plain_size/SE3_L1_CRYPTOBLOCK_SIZE)+1)*SE3_L1_CRYPTOBLOCK_SIZE)

#define LS3_ONE 1
#define LS3_ZERO 0
#define LS3_THREE 3
#define LS3_LENGTH_4 4
#define LS3_LENGTH_7 7
#define LS3_LENGTH_8 8
#define LS3_LENGTH_32 32
#define LS3_LENGTH_64 64
#define LS3_LENGTH_14 14
#define LS3_LENGTH_19 19
#define LS3_LENGTH_100 100
#define LS3_LENGTH_128 128
#define LS3_LENGTH_148 148
#define LS3_LENGTH_256 256
#define LS3_LENGTH_300 300
#define LS3_LENGTH_500 500
#define LS3_LENGTH_1000 1000
#define LS3_LENGTH_2000 2000
#define LS3_LENGTH_3000 3000
#define LS3_LENGTH_DEV_PATH_0 144 // NO DEVICE
#define LS3_LENGTH_DEV_PATH_1 155 // WITH DEVICE
#define LS3_LOGFILE_NAME "D:\\Program\\1.0\\msys\\home\\kangidjoel\\logFile.log"
#define LS3_PATH "backup_sources\\" // run development
//#define LS3_PATH "secube\\backup_sources\\" // run simulation
#define LS3_DB_NAME "ls3src.db" // database source
//#define LS3_DB_NAME "..\\..\\SEcube_Host_EmptyProject\\src.db" //release run
#define LS3_QGET_HCDE "SELECT HCDE,LUPD,FSIZ FROM(SELECT DSPH||'\\'||FNME AS PATH, substr(HCDE,1,64) AS HCDE,replace(replace(replace(LUPD,'-',''),' ',''),':','') as LUPD, FSIZ  FROM SOURCE_DB) AS DBS WHERE DBS.PATH='&fnme'"
#define LS3_QGET_HCDE2 "SELECT HCDE,LUPD,FSIZ FROM(SELECT DSPH||'\\'||FNME AS PATH, substr(HCDE,1,64) AS HCDE,replace(replace(replace(LUPD,'-',''),' ',''),':','') as LUPD, FSIZ  FROM SOURCE_DB) AS DBS WHERE DBS.HCDE=substr('&hcde',1,64)"
#define LS3_QGET_FNME "SELECT FALS,FNME,'X' FROM " \
					  "(SELECT DISTINCT(FALS)||HCDE AS FALS, DSPH||'\\'||FNME AS FNME FROM SOURCE_DB) AS DBS " \
					  "WHERE DBS.FNME = '&fnme'"//AND HCDE = '&hcde'"
//#define LS3_QGET_FSIZ "SELECT FSIZ,'X','X' FROM (SELECT FSIZ, DSPH||'\\'||FNME AS FNME FROM SOURCE_DB) AS DBS WHERE DBS.FNME = '&fnme'"
#define LS3_QGET_FDTL "SELECT DISTINCT(HCDE),LUPD,'X' " \
					  "FROM SOURCE_DB " \
					  "WHERE FNME LIKE '&fnme' " \
					  "AND DSPH LIKE '&dsph' " \
					  "AND HCDE LIKE '&hcde' " \
					  "AND LUPD LIKE '&lupd'"
#define LS3_QSET_HCDE "INSERT OR REPLACE INTO SOURCE_DB" \
					  	  "(ID,FNME,FALS,HCDE,USER,DSPH,LUPD,FSIZ) " \
					  "VALUES(" \
						  "(SELECT ID " \
						  "FROM SOURCE_DB " \
						  "WHERE FNME='&fnme' AND DSPH='&dsph')," \
						  "'&fnme'," \
						  "substr('&fals',1,64)," \
						  "substr('&hcde',1,64)," \
						  "'&user'," \
						  "'&dsph'," \
						  "'&lupd'," \
						  "'&fsiz')" //datetime('now','localtime')
						  //AND HCDE='&hcde'
#define LS3_QGET_DLIST "SELECT ID, DSPH,'X' FROM SOURCE_DB WHERE DSPH like '&dsph' AND ID > '&id' GROUP BY DSPH ORDER BY ID ASC LIMIT 1"
#define LS3_QGET_FILE_BY_ID "SELECT ID, DSPH||'\\'||FNME AS DSPH,'X' FROM SOURCE_DB WHERE DSPH = '&dsph' AND ID > '&id' ORDER BY ID ASC LIMIT 1"
#define LS3_QSET_KEY "INSERT OR REPLACE INTO SOURCE_DB_KEY" \
	"(KEY_ID,KEY_NAME,KEY_SIZE,KEY_VALID,KEY_USER) " \
	"VALUES(" \
		   "'&kid'," \
		   "'&knm'," \
		   "'&ksz'," \
		   "datetime('&kvl','unixepoch','localtime')," \
		   "'&usr')"
#define LS3_QGET_KEY "SELECT KEY_ID,KEY_USER,'X' FROM SOURCE_DB_KEY WHERE KEY_ID=&kid"
//#define LS3_QDEL_KEY "DELETE FROM SOURCE_DB_KEY WHERE KEY_ID=&kid"
#define LS3_QDEL_FILE "DELETE FROM SOURCE_DB WHERE (DSPH||'\\'||FNME)='&path'"
#define LS3_OK 0
#define LS3_ER 1

extern int LS3_EXT;


int jstrlen(char* args[]);
void print_sn(uint8_t* v);
void arrtouint(int _start, int _size, char *_arr[], uint8_t *_ret);
long get_unixepoch_time(char* _date);
char* concat(const char *s1, const char *s2);
char* replace(char* str, char* a, char* b);
void arrtoarr(char* _arr, uint8_t* _ret);

#endif /* LOCALLIB_HOST_JGLOBALVAR_H_ */
