/*
 * FileCopy.c
 *
 *  Created on: Jun 10, 2017
 *      Author: kangidjoel
 */

int compare_file(char* _filename1, char * _filename2);
int compare_hash(char* _filename, char *_hash);
int integrity_test(char* _filename, char* _socpath, int _restore);
int integrity_test_2(char* _dirname, char* _socpath, int _restore, int _rec);
int copy_file(char _source1[], char _source2[]);
int copy_file2(char *old_filename, char  *new_filename);
char *get_modified_file(char* _filepath,int _opt);
long get_file_size(char* _filepath);
void update_mod_file(char* _fname, time_t _epoch);
void directory_list();
