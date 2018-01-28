/*
 * Jdbopt.c
 *
 *  Created on: Jun 12, 2017
 *      Author: kangidjoel
 */

#include <stdio.h>
#include <string.h>
/* user lib */
#include "Jglobalvar.h"
#include "sqlite3.h"
#include "Jlogger.h"
#include "Jdbopt.h"

int get_db_hash(char *_filename, char *r1, char *r2, char *r3){

	char *row1;
	char *row2;
	char *row3;

	char sql[LS3_LENGTH_1000];
	int len = strlen(LS3_QGET_HCDE);

	memset(sql, 0, LS3_LENGTH_1000*sizeof(char));
	memcpy(sql, LS3_QGET_HCDE, len);
	sql[len]=0;

	replace(sql, "&fnme", _filename);

	if (exec(sql, &row1, &row2, &row3) != LS3_OK){
		return LS3_ER;
	}

	if(r1) memcpy(r1,row1,strlen(row1));
	if(r2) memcpy(r2,row2,strlen(row2));
	if(r3) memcpy(r3,row3,strlen(row3));

	memset(sql, 0, LS3_LENGTH_1000*sizeof(char));

	return LS3_OK;
}

int get_db_hash_by_hfile(char *_hash, char *r1, char *r2, char *r3){

	char *row1;
	char *row2;
	char *row3;

	char sql[LS3_LENGTH_1000];
	int len = strlen(LS3_QGET_HCDE2);

	memset(sql, 0, LS3_LENGTH_1000*sizeof(char));
	memcpy(sql, LS3_QGET_HCDE2, len);
	sql[len]=0;

	replace(sql, "&hcde", _hash);

	if (exec(sql, &row1, &row2, &row3) != LS3_OK){
		return LS3_ER;
	}

	if(r1) memcpy(r1,row1,strlen(row1));
	if(r2) memcpy(r2,row2,strlen(row2));
	if(r3) memcpy(r3,row3,strlen(row3));

	memset(sql, 0, LS3_LENGTH_1000*sizeof(char));

	return LS3_OK;
}

int get_db_data(char* _sql, char* _ret1, char* _ret2, char* _ret3){
	char* ret1;
	char* ret2;
	char* ret3;

	int len1,len2,len3;

	if (exec(_sql, &ret1, &ret2, &ret3) != LS3_OK){
		if (_ret1) _ret1[0] = 0;
		if (_ret2) _ret2[0] = 0;
		if (_ret3) _ret3[0] = 0;
		return LS3_ER;
	}

	len1=strlen(ret1);
	len2=strlen(ret2);
	len3=strlen(ret3);

	if (_ret1) memcpy(_ret1, ret1, len1);
	if (_ret2) memcpy(_ret2, ret2, len2);
	if (_ret3) memcpy(_ret3, ret3, len3);

	return LS3_OK;
}

int get_key_id(char* _key, char* _id, char* _usr){
	char *id;
	char *usr;

	char sql[LS3_LENGTH_1000];
	int len = strlen(LS3_QGET_KEY);

	memset(sql, 0, LS3_LENGTH_1000*sizeof(char));
	memcpy(sql, LS3_QGET_KEY, len);
	sql[len]=0;

	replace(sql, "&kid",_key);

	if (exec(sql, &id, &usr, NULL) != LS3_OK){
			return LS3_ER;
	}

	if (_id)memcpy(_id, id, strlen(id));
	if (_usr)memcpy(_usr, usr, strlen(usr));

	memset(sql, 0, LS3_LENGTH_1000*sizeof(char));

	return LS3_OK;
}

int set_db_data(char *_sql){

	if (exec(_sql, NULL, NULL, NULL) != LS3_OK)
		return LS3_ER;

	writeLogFile("", "Database updated!");

	return LS3_OK;
}

int exec(char *_sql, char **_row1, char **_row2, char **_row3) {

	sqlite3 *db;
	struct sqlite3_stmt *res;
	int rc;
	int count=0;

	char row1[LS3_LENGTH_500];
	char row2[LS3_LENGTH_500];
	char row3[LS3_LENGTH_500];

	memset(row1, 0, LS3_LENGTH_500*sizeof(char));
	memset(row2, 0, LS3_LENGTH_500*sizeof(char));
	memset(row3, 0, LS3_LENGTH_500*sizeof(char));

	rc = sqlite3_open(LS3_DB_NAME, &db);

	if (rc) {
		writeLogFile("",concat("Can't open database: ", sqlite3_errmsg(db)));
		if(_row1)*_row1 = NULL;
		if(_row2)*_row2 = NULL;
		if(_row3)*_row3 = NULL;
		return LS3_OK;
	}

	/* Execute SQL statement for select from db */
//	writeLogFile("",_sql);
	rc = sqlite3_prepare_v2(db, _sql, -1, &res, NULL);

	if (rc != SQLITE_OK ) {
		writeLogFile("",concat("SQL error: ",sqlite3_errmsg(db)));
		if (_row1)*_row1 = NULL;
		if (_row2)*_row2 = NULL;
		if (_row3)*_row3 = NULL;
		return LS3_OK;
	}

	while (sqlite3_step(res) == SQLITE_ROW) {

		sprintf(row1, "%s", sqlite3_column_text(res, 0));
		sprintf(row2, "%s", sqlite3_column_text(res, 1));
		sprintf(row3, "%s", sqlite3_column_text(res, 2));

		/*sprintf(msg,"Return\nRow1 : %s\nRow2 : %s\nRow3 : %s",row1,row2,row3);

		writeLogFile("",msg);*/

		count++;
	}

	sqlite3_finalize(res);

	sqlite3_close(db);

	if(count==0 && _row1){
		if(_row1)*_row1 = NULL;
		if(_row2)*_row2 = NULL;
		if(_row3)*_row3 = NULL;
		return LS3_ER;
	}

	if (count==1){

		if(_row1)*_row1 = row1;
		if(_row2)*_row2 = row2;
		if(_row3)*_row3 = row3;

	}

//	memset(sql, 0, LS3_LENGTH_1000 * sizeof(char));

	return LS3_OK;
}
