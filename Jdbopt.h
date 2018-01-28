/*
 * Jdbopt.h
 *
 *  Created on: Jun 13, 2017
 *      Author: kangidjoel
 */

#ifndef LOCALLIB_HOST_JDBOPT_H_
#define LOCALLIB_HOST_JDBOPT_H_


int get_db_hash(char *_filename, char *r1, char *r2, char *r3);
int get_db_data(char* _sql, char* _ret1, char* _ret2, char* _ret3);
int set_db_data(char *_sql);
int exec(char *_sql, char **_row1, char **_row2, char **_row3);


#endif /* LOCALLIB_HOST_JDBOPT_H_ */
